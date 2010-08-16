/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/gfx/renderobject.h"

#include <algorithm>

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/gfx/renderobjectregistry.h"
#include "sword25/gfx/renderobjectmanager.h"
#include "sword25/gfx/graphicengine.h"

#include "sword25/gfx/bitmap.h"
#include "sword25/gfx/staticbitmap.h"
#include "sword25/gfx/dynamicbitmap.h"
#include "sword25/gfx/animation.h"
#include "sword25/gfx/panel.h"
#include "sword25/gfx/text.h"
#include "sword25/gfx/animationtemplate.h"

namespace Sword25 {

#define BS_LOG_PREFIX "RENDEROBJECT"

// Konstruktion / Destruktion
// --------------------------
BS_RenderObject::BS_RenderObject(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, TYPES Type, unsigned int Handle) :
	m_ManagerPtr(0),
	m_ParentPtr(ParentPtr),
	m_X(0),
	m_Y(0),
	m_Z(0),
	m_OldX(-1),
	m_OldY(-1),
	m_OldZ(-1),
	m_Width(0),
	m_Height(0),
	m_Visible(true),
	m_OldVisible(false),
	m_ChildChanged(true),
	m_Type(Type),
	m_InitSuccess(false),
	m_RefreshForced(true),
	m_Handle(0) {

	// Renderobject registrieren, abhängig vom Handle-Parameter entweder mit beliebigem oder vorgegebenen Handle.
	if (Handle == 0)
		m_Handle = BS_RenderObjectRegistry::GetInstance().RegisterObject(this);
	else
		m_Handle = BS_RenderObjectRegistry::GetInstance().RegisterObject(this, Handle);
	if (m_Handle == 0) return;

	UpdateAbsolutePos();

	// Dieses Objekt zu den Kindern der Elternobjektes hinzufügen, falls nicht Wurzel (ParentPtr ungültig) und dem
	// selben RenderObjektManager zuweisen.
	if (m_ParentPtr.IsValid()) {
		m_ManagerPtr = m_ParentPtr->GetManager();
		m_ParentPtr->AddObject(this->GetHandle());
	} else {
		if (GetType() != TYPE_ROOT) {
			BS_LOG_ERRORLN("Tried to create a non-root render object and has no parent. All non-root render objects have to have a parent.");
			return;
		}
	}

	UpdateObjectState();

	m_InitSuccess = true;
}

BS_RenderObject::~BS_RenderObject() {
	// Objekt aus dem Elternobjekt entfernen.
	if (m_ParentPtr.IsValid()) m_ParentPtr->DetatchChildren(this->GetHandle());

	DeleteAllChildren();

	// Objekt deregistrieren.
	BS_RenderObjectRegistry::GetInstance().DeregisterObject(this);
}

// Rendern
// -------
bool BS_RenderObject::Render() {
	// Objektänderungen validieren
	ValidateObject();

	// Falls das Objekt nicht sichtbar ist, muss gar nichts gezeichnet werden
	if (!m_Visible) return true;

	// Falls notwendig, wird die Renderreihenfolge der Kinderobjekte aktualisiert.
	if (m_ChildChanged) {
		SortRenderObjects();
		m_ChildChanged = false;
	}

	// Objekt zeichnen.
	DoRender();

	// Dann müssen die Kinder gezeichnet werden
	RENDEROBJECT_ITER it = m_Children.begin();
	for (; it != m_Children.end(); ++it)
		if (!(*it)->Render())
			return false;

	return true;
}

// Objektverwaltung
// ----------------

void BS_RenderObject::ValidateObject() {
	// Die Veränderungen in den Objektvariablen aufheben
	m_OldBBox = m_BBox;
	m_OldVisible = m_Visible;
	m_OldX = m_X;
	m_OldY = m_Y;
	m_OldZ = m_Z;
	m_RefreshForced = false;
}

bool BS_RenderObject::UpdateObjectState() {
	// Falls sich das Objekt verändert hat, muss der interne Zustand neu berechnet werden und evtl. Update-Regions für den nächsten Frame
	// registriert werden.
	if ((CalcBoundingBox() != m_OldBBox) ||
	        (m_Visible != m_OldVisible) ||
	        (m_X != m_OldX) ||
	        (m_Y != m_OldY) ||
	        (m_Z != m_OldZ) ||
	        m_RefreshForced) {
		// Renderrang des Objektes neu bestimmen, da sich dieser verändert haben könnte
		if (m_ParentPtr.IsValid()) m_ParentPtr->SignalChildChange();

		// Die Bounding-Box neu berechnen und Update-Regions registrieren.
		UpdateBoxes();

		// Änderungen Validieren
		ValidateObject();
	}

	// Dann muss der Objektstatus der Kinder aktualisiert werden.
	RENDEROBJECT_ITER it = m_Children.begin();
	for (; it != m_Children.end(); ++it)
		if (!(*it)->UpdateObjectState()) return false;

	return true;
}

void BS_RenderObject::UpdateBoxes() {
	// Bounding-Box aktualisieren
	m_BBox = CalcBoundingBox();
}

BS_Rect BS_RenderObject::CalcBoundingBox() const {
	// Die Bounding-Box mit der Objektgröße initialisieren.
	BS_Rect BBox(0, 0, m_Width, m_Height);

	// Die absolute Position der Bounding-Box berechnen.
	BBox.Move(m_AbsoluteX, m_AbsoluteY);

	// Die Bounding-Box am Elternobjekt clippen.
	if (m_ParentPtr.IsValid()) BBox.Intersect(m_ParentPtr->GetBBox(), BBox);

	return BBox;
}

void BS_RenderObject::CalcAbsolutePos(int &X, int &Y) const {
	X = CalcAbsoluteX();
	Y = CalcAbsoluteY();
}

int BS_RenderObject::CalcAbsoluteX() const {
	if (m_ParentPtr.IsValid())
		return m_ParentPtr->GetAbsoluteX() + m_X;
	else
		return m_X;
}

int BS_RenderObject::CalcAbsoluteY() const {
	if (m_ParentPtr.IsValid())
		return m_ParentPtr->GetAbsoluteY() + m_Y;
	else
		return m_Y;
}

// Baumverwaltung
// --------------

void BS_RenderObject::DeleteAllChildren() {
	// Es ist nicht notwendig die Liste zu iterieren, da jedes Kind für sich DetatchChildren an diesem Objekt aufruft und sich somit
	// selber entfernt. Daher muss immer nur ein beliebiges Element (hier das letzte) gelöscht werden, bis die Liste leer ist.
	while (!m_Children.empty()) {
		BS_RenderObjectPtr<BS_RenderObject> CurPtr = m_Children.back();
		CurPtr.Erase();
	}
}

bool BS_RenderObject::AddObject(BS_RenderObjectPtr<BS_RenderObject> pObject) {
	if (!pObject.IsValid()) {
		BS_LOG_ERRORLN("Tried to add a null object to a renderobject.");
		return false;
	}

	// Objekt in die Kinderliste einfügen.
	m_Children.push_back(pObject);

	// Sicherstellen, dass vor dem nächsten Rendern die Renderreihenfolge aktualisiert wird.
	if (m_ParentPtr.IsValid()) m_ParentPtr->SignalChildChange();

	return true;
}

bool BS_RenderObject::DetatchChildren(BS_RenderObjectPtr<BS_RenderObject> pObject) {
	// Kinderliste durchgehen und Objekt entfernen falls vorhanden
	RENDEROBJECT_ITER it = m_Children.begin();
	for (; it != m_Children.end(); ++it)
		if (*it == pObject) {
			m_Children.erase(it);
			return true;
		}

	BS_LOG_ERRORLN("Tried to detach children from a render object that isn't its parent.");
	return false;
}

void BS_RenderObject::SortRenderObjects() {
	Common::sort(m_Children.begin(), m_Children.end(), Greater);
}

void BS_RenderObject::UpdateAbsolutePos() {
	CalcAbsolutePos(m_AbsoluteX, m_AbsoluteY);

	RENDEROBJECT_ITER it = m_Children.begin();
	for (; it != m_Children.end(); ++it)
		(*it)->UpdateAbsolutePos();
}

// Get-Methoden
// ------------

bool BS_RenderObject::GetObjectIntersection(BS_RenderObjectPtr<BS_RenderObject> pObject, BS_Rect &Result) {
	return m_BBox.Intersect(pObject->GetBBox(), Result);
}

// Set-Methoden
// ------------
void BS_RenderObject::SetPos(int X, int Y) {
	m_X = X;
	m_Y = Y;
	UpdateAbsolutePos();
}

void BS_RenderObject::SetX(int X) {
	m_X = X;
	UpdateAbsolutePos();
}

void BS_RenderObject::SetY(int Y) {
	m_Y = Y;
	UpdateAbsolutePos();
}

void BS_RenderObject::SetZ(int Z) {
	if (Z < 0)
		BS_LOG_ERRORLN("Tried to set a negative Z value (%d).", Z);
	else
		m_Z = Z;
}

void BS_RenderObject::SetVisible(bool Visible) {
	m_Visible = Visible;
}

// -----------------------------------------------------------------------------
// Objekterzeuger
// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Animation> BS_RenderObject::AddAnimation(const Common::String &Filename) {
	BS_RenderObjectPtr<BS_Animation> AniPtr((new BS_Animation(this->GetHandle(), Filename))->GetHandle());
	if (AniPtr.IsValid() && AniPtr->GetInitSuccess())
		return AniPtr;
	else {
		if (AniPtr.IsValid()) AniPtr.Erase();
		return BS_RenderObjectPtr<BS_Animation>();
	}
}


// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Animation> BS_RenderObject::AddAnimation(const BS_AnimationTemplate &AnimationTemplate) {
	BS_Animation *AniPtr = new BS_Animation(this->GetHandle(), AnimationTemplate);
	if (AniPtr && AniPtr->GetInitSuccess())
		return AniPtr->GetHandle();
	else {
		delete AniPtr;
		return BS_RenderObjectPtr<BS_Animation>();
	}
}

// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Bitmap> BS_RenderObject::AddBitmap(const Common::String &Filename) {
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr((new BS_StaticBitmap(this->GetHandle(), Filename))->GetHandle());
	if (BitmapPtr.IsValid() && BitmapPtr->GetInitSuccess())
		return BS_RenderObjectPtr<BS_Bitmap>(BitmapPtr);
	else {
		if (BitmapPtr.IsValid()) BitmapPtr.Erase();
		return BS_RenderObjectPtr<BS_Bitmap>();
	}
}

// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Bitmap> BS_RenderObject::AddDynamicBitmap(unsigned int Width, unsigned int Height) {
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr((new BS_DynamicBitmap(this->GetHandle(), Width, Height))->GetHandle());
	if (BitmapPtr.IsValid() && BitmapPtr->GetInitSuccess())
		return BitmapPtr;
	else {
		if (BitmapPtr.IsValid()) BitmapPtr.Erase();
		return BS_RenderObjectPtr<BS_Bitmap>();
	}
}

// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Panel> BS_RenderObject::AddPanel(int Width, int Height, unsigned int Color) {
	BS_RenderObjectPtr<BS_Panel> PanelPtr((new BS_Panel(this->GetHandle(), Width, Height, Color))->GetHandle());
	if (PanelPtr.IsValid() && PanelPtr->GetInitSuccess())
		return PanelPtr;
	else {
		if (PanelPtr.IsValid()) PanelPtr.Erase();
		return BS_RenderObjectPtr<BS_Panel>();
	}
}

// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_Text> BS_RenderObject::AddText(const Common::String &Font, const Common::String &Text) {
	BS_RenderObjectPtr<BS_Text> TextPtr((new BS_Text(this->GetHandle()))->GetHandle());
	if (TextPtr.IsValid() && TextPtr->GetInitSuccess() && TextPtr->SetFont(Font)) {
		TextPtr->SetText(Text);
		return TextPtr;
	} else {
		if (TextPtr.IsValid()) TextPtr.Erase();
		return BS_RenderObjectPtr<BS_Text>();
	}
}

// Persistenz-Methoden
// -------------------

bool BS_RenderObject::Persist(BS_OutputPersistenceBlock &Writer) {
	// Typ und Handle werden als erstes gespeichert, damit beim Laden ein Objekt vom richtigen Typ mit dem richtigen Handle erzeugt werden kann.
	Writer.Write(static_cast<unsigned int>(m_Type));
	Writer.Write(m_Handle);

	// Restliche Objekteigenschaften speichern.
	Writer.Write(m_X);
	Writer.Write(m_Y);
	Writer.Write(m_AbsoluteX);
	Writer.Write(m_AbsoluteY);
	Writer.Write(m_Z);
	Writer.Write(m_Width);
	Writer.Write(m_Height);
	Writer.Write(m_Visible);
	Writer.Write(m_ChildChanged);
	Writer.Write(m_InitSuccess);
	Writer.Write(m_BBox.left);
	Writer.Write(m_BBox.top);
	Writer.Write(m_BBox.right);
	Writer.Write(m_BBox.bottom);
	Writer.Write(m_OldBBox.left);
	Writer.Write(m_OldBBox.top);
	Writer.Write(m_OldBBox.right);
	Writer.Write(m_OldBBox.bottom);
	Writer.Write(m_OldX);
	Writer.Write(m_OldY);
	Writer.Write(m_OldZ);
	Writer.Write(m_OldVisible);
	Writer.Write(m_ParentPtr.IsValid() ? m_ParentPtr->GetHandle() : 0);
	Writer.Write(m_RefreshForced);

	return true;
}

// -----------------------------------------------------------------------------

bool BS_RenderObject::Unpersist(BS_InputPersistenceBlock &Reader) {
	// Typ und Handle wurden schon von RecreatePersistedRenderObject() ausgelesen. Jetzt werden die restlichen Objekteigenschaften ausgelesen.
	Reader.Read(m_X);
	Reader.Read(m_Y);
	Reader.Read(m_AbsoluteX);
	Reader.Read(m_AbsoluteY);
	Reader.Read(m_Z);
	Reader.Read(m_Width);
	Reader.Read(m_Height);
	Reader.Read(m_Visible);
	Reader.Read(m_ChildChanged);
	Reader.Read(m_InitSuccess);
	Reader.Read(m_BBox.left);
	Reader.Read(m_BBox.top);
	Reader.Read(m_BBox.right);
	Reader.Read(m_BBox.bottom);
	Reader.Read(m_OldBBox.left);
	Reader.Read(m_OldBBox.top);
	Reader.Read(m_OldBBox.right);
	Reader.Read(m_OldBBox.bottom);
	Reader.Read(m_OldX);
	Reader.Read(m_OldY);
	Reader.Read(m_OldZ);
	Reader.Read(m_OldVisible);
	unsigned int ParentHandle;
	Reader.Read(ParentHandle);
	m_ParentPtr = BS_RenderObjectPtr<BS_RenderObject>(ParentHandle);
	Reader.Read(m_RefreshForced);

	UpdateAbsolutePos();
	UpdateObjectState();

	return Reader.IsGood();
}

// -----------------------------------------------------------------------------

bool BS_RenderObject::PersistChildren(BS_OutputPersistenceBlock &Writer) {
	bool Result = true;

	// Kinderanzahl speichern.
	Writer.Write(m_Children.size());

	// Rekursiv alle Kinder speichern.
	RENDEROBJECT_LIST::iterator It = m_Children.begin();
	while (It != m_Children.end()) {
		Result &= (*It)->Persist(Writer);
		++It;
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_RenderObject::UnpersistChildren(BS_InputPersistenceBlock &Reader) {
	bool Result = true;

	// Kinderanzahl einlesen.
	unsigned int ChildrenCount;
	Reader.Read(ChildrenCount);
	if (!Reader.IsGood()) return false;

	// Alle Kinder rekursiv wieder herstellen.
	for (unsigned int i = 0; i < ChildrenCount; ++i) {
		if (!RecreatePersistedRenderObject(Reader).IsValid()) return false;
	}

	return Result && Reader.IsGood();
}

// -----------------------------------------------------------------------------

BS_RenderObjectPtr<BS_RenderObject> BS_RenderObject::RecreatePersistedRenderObject(BS_InputPersistenceBlock &Reader) {
	BS_RenderObjectPtr<BS_RenderObject> Result;

	// Typ und Handle auslesen.
	unsigned int Type;
	unsigned int Handle;
	Reader.Read(Type);
	Reader.Read(Handle);
	if (!Reader.IsGood()) return Result;

	switch (Type) {
	case TYPE_PANEL:
		Result = (new BS_Panel(Reader, this->GetHandle(), Handle))->GetHandle();
		break;

	case TYPE_STATICBITMAP:
		Result = (new BS_StaticBitmap(Reader, this->GetHandle(), Handle))->GetHandle();
		break;

	case TYPE_DYNAMICBITMAP:
		Result = (new BS_DynamicBitmap(Reader, this->GetHandle(), Handle))->GetHandle();
		break;

	case TYPE_TEXT:
		Result = (new BS_Text(Reader, this->GetHandle(), Handle))->GetHandle();
		break;

	case TYPE_ANIMATION:
		Result = (new BS_Animation(Reader, this->GetHandle(), Handle))->GetHandle();
		break;

	default:
		BS_LOG_ERRORLN("Cannot recreate render object of unknown type %d.", Type);
	}

	return Result;
}

// Hilfs-Methoden
// --------------
bool BS_RenderObject::Greater(const BS_RenderObjectPtr<BS_RenderObject> lhs, const BS_RenderObjectPtr<BS_RenderObject> rhs) {
	// Das Objekt mit dem kleinem Z-Wert müssen zuerst gerendert werden.
	if (lhs->m_Z != rhs->m_Z)
		return lhs->m_Z < rhs->m_Z;
	// Falls der Z-Wert gleich ist, wird das weiter oben gelegenen Objekte zuerst gezeichnet.
	return lhs->m_Y < rhs->m_Y;
}

} // End of namespace Sword25
