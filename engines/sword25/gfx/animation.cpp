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

#include "sword25/gfx/animation.h"

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/callbackregistry.h"
#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

#define BS_LOG_PREFIX "ANIMATION"

// Konstruktion / Destruktion
// --------------------------

Animation::Animation(RenderObjectPtr<RenderObject> ParentPtr, const Common::String &FileName) :
	TimedRenderObject(ParentPtr, RenderObject::TYPE_ANIMATION) {
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess) return;

	InitMembers();

	// Vom negativen Fall ausgehen.
	_initSuccess = false;

	InitializeAnimationResource(FileName);

	// Erfolg signalisieren.
	_initSuccess = true;
}

// -----------------------------------------------------------------------------

Animation::Animation(RenderObjectPtr<RenderObject> ParentPtr, const AnimationTemplate &Template) :
	TimedRenderObject(ParentPtr, RenderObject::TYPE_ANIMATION) {
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess) return;

	InitMembers();

	// Vom negativen Fall ausgehen.
	_initSuccess = false;

	m_AnimationTemplateHandle = AnimationTemplate::Create(Template);

	// Erfolg signalisieren.
	_initSuccess = true;
}

// -----------------------------------------------------------------------------

Animation::Animation(InputPersistenceBlock &Reader, RenderObjectPtr<RenderObject> ParentPtr, uint Handle) :
	TimedRenderObject(ParentPtr, RenderObject::TYPE_ANIMATION, Handle) {
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess) return;

	InitMembers();

	// Objekt vom Stream laden.
	_initSuccess = unpersist(Reader);
}

// -----------------------------------------------------------------------------

void Animation::InitializeAnimationResource(const Common::String &FileName) {
	// Die Resource wird für die gesamte Lebensdauer des Animations-Objektes gelockt.
	Resource *ResourcePtr = Kernel::GetInstance()->GetResourceManager()->RequestResource(FileName);
	if (ResourcePtr && ResourcePtr->GetType() == Resource::TYPE_ANIMATION)
		m_AnimationResourcePtr = static_cast<AnimationResource *>(ResourcePtr);
	else {
		BS_LOG_ERRORLN("The resource \"%s\" could not be requested. The Animation can't be created.", FileName.c_str());
		return;
	}

	// Größe und Position der Animation anhand des aktuellen Frames bestimmen.
	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------

void Animation::InitMembers() {
	m_CurrentFrame = 0;
	m_CurrentFrameTime = 0;
	m_Direction = FORWARD;
	m_Running = false;
	m_Finished = false;
	_relX = 0;
	_relY = 0;
	_scaleFactorX = 1.0f;
	_scaleFactorY = 1.0f;
	_modulationColor = 0xffffffff;
	m_AnimationResourcePtr = 0;
	m_AnimationTemplateHandle = 0;
	m_FramesLocked = false;
}

// -----------------------------------------------------------------------------

Animation::~Animation() {
	if (GetAnimationDescription()) {
		Stop();
		GetAnimationDescription()->unlock();
	}

	// Delete Callbacks
	Common::Array<ANIMATION_CALLBACK_DATA>::iterator it = m_DeleteCallbacks.begin();
	for (; it != m_DeleteCallbacks.end(); it++)((*it).Callback)((*it).Data);

}

// -----------------------------------------------------------------------------
// Steuermethoden
// -----------------------------------------------------------------------------

void Animation::Play() {
	// Wenn die Animation zuvor komplett durchgelaufen ist, wird sie wieder von Anfang abgespielt
	if (m_Finished) Stop();

	m_Running = true;
	LockAllFrames();
}

// -----------------------------------------------------------------------------

void Animation::Pause() {
	m_Running = false;
	UnlockAllFrames();
}

// -----------------------------------------------------------------------------

void Animation::Stop() {
	m_CurrentFrame = 0;
	m_CurrentFrameTime = 0;
	m_Direction = FORWARD;
	Pause();
}

// -----------------------------------------------------------------------------

void Animation::SetFrame(uint Nr) {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);

	if (Nr >= animationDescriptionPtr->getFrameCount()) {
		BS_LOG_ERRORLN("Tried to set animation to illegal frame (%d). Value must be between 0 and %d.",
		               Nr, animationDescriptionPtr->getFrameCount());
		return;
	}

	m_CurrentFrame = Nr;
	m_CurrentFrameTime = 0;
	ComputeCurrentCharacteristics();
	forceRefresh();
}

// -----------------------------------------------------------------------------
// Rendern
// -----------------------------------------------------------------------------

bool Animation::doRender() {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	BS_ASSERT(m_CurrentFrame < animationDescriptionPtr->getFrameCount());

	// Bitmap des aktuellen Frames holen
	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(animationDescriptionPtr->getFrame(m_CurrentFrame).FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmapResource = static_cast<BitmapResource *>(pResource);

	// Framebufferobjekt holen
	GraphicEngine *pGfx = static_cast<GraphicEngine *>(Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(pGfx);

	// Bitmap zeichnen
	bool Result;
	if (isScalingAllowed() && (_width != pBitmapResource->getWidth() || _height != pBitmapResource->getHeight())) {
		Result = pBitmapResource->blit(_absoluteX, _absoluteY,
		                               (animationDescriptionPtr->getFrame(m_CurrentFrame).FlipV ? BitmapResource::FLIP_V : 0) |
		                               (animationDescriptionPtr->getFrame(m_CurrentFrame).FlipH ? BitmapResource::FLIP_H : 0),
		                               0, _modulationColor, _width, _height);
	} else {
		Result = pBitmapResource->blit(_absoluteX, _absoluteY,
		                               (animationDescriptionPtr->getFrame(m_CurrentFrame).FlipV ? BitmapResource::FLIP_V : 0) |
		                               (animationDescriptionPtr->getFrame(m_CurrentFrame).FlipH ? BitmapResource::FLIP_H : 0),
		                               0, _modulationColor, -1, -1);
	}

	// Resource freigeben
	pBitmapResource->release();

	return Result;
}

// -----------------------------------------------------------------------------
// Frame Notifikation
// -----------------------------------------------------------------------------

void Animation::frameNotification(int TimeElapsed) {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	BS_ASSERT(TimeElapsed >= 0);

	// Nur wenn die Animation läuft wird sie auch weiterbewegt
	if (m_Running) {
		// Gesamte vergangene Zeit bestimmen (inkl. Restzeit des aktuellen Frames)
		m_CurrentFrameTime += TimeElapsed;

		// Anzahl an zu überpringenden Frames bestimmen
		int SkipFrames = animationDescriptionPtr->getMillisPerFrame() == 0 ? 0 : m_CurrentFrameTime / animationDescriptionPtr->getMillisPerFrame();

		// Neue Frame-Restzeit bestimmen
		m_CurrentFrameTime -= animationDescriptionPtr->getMillisPerFrame() * SkipFrames;

		// Neuen Frame bestimmen (je nach aktuellener Abspielrichtung wird addiert oder subtrahiert)
		int TmpCurFrame = m_CurrentFrame;
		switch (m_Direction) {
		case FORWARD:
			TmpCurFrame += SkipFrames;
			break;

		case BACKWARD:
			TmpCurFrame -= SkipFrames;
			break;

		default:
			BS_ASSERT(0);
		}

		// Überläufe behandeln
		if (TmpCurFrame < 0) {
			// Loop-Point Callbacks
			for (uint i = 0; i < m_LoopPointCallbacks.size();) {
				if ((m_LoopPointCallbacks[i].Callback)(m_LoopPointCallbacks[i].Data) == false) {
					m_LoopPointCallbacks.remove_at(i);
				} else
					i++;
			}

			// Ein Unterlauf darf nur auftreten, wenn der Animationstyp JOJO ist.
			BS_ASSERT(animationDescriptionPtr->getAnimationType() == AT_JOJO);
			TmpCurFrame = - TmpCurFrame;
			m_Direction = FORWARD;
		} else if (static_cast<uint>(TmpCurFrame) >= animationDescriptionPtr->getFrameCount()) {
			// Loop-Point Callbacks
			for (uint i = 0; i < m_LoopPointCallbacks.size();) {
				if ((m_LoopPointCallbacks[i].Callback)(m_LoopPointCallbacks[i].Data) == false) {
					m_LoopPointCallbacks.remove_at(i);
				} else
					i++;
			}

			switch (animationDescriptionPtr->getAnimationType()) {
			case AT_ONESHOT:
				TmpCurFrame = animationDescriptionPtr->getFrameCount() - 1;
				m_Finished = true;
				Pause();
				break;

			case AT_LOOP:
				TmpCurFrame = TmpCurFrame % animationDescriptionPtr->getFrameCount();
				break;

			case AT_JOJO:
				TmpCurFrame = animationDescriptionPtr->getFrameCount() - (TmpCurFrame % animationDescriptionPtr->getFrameCount()) - 1;
				m_Direction = BACKWARD;
				break;

			default:
				BS_ASSERT(0);
			}
		}

		if ((int)m_CurrentFrame != TmpCurFrame) {
			forceRefresh();

			if (animationDescriptionPtr->getFrame(m_CurrentFrame).Action != "") {
				// Action Callbacks
				for (uint i = 0; i < m_ActionCallbacks.size();) {
					if ((m_ActionCallbacks[i].Callback)(m_ActionCallbacks[i].Data) == false) {
						m_ActionCallbacks.remove_at(i);
					} else
						i++;
				}
			}
		}

		m_CurrentFrame = static_cast<uint>(TmpCurFrame);
	}

	// Größe und Position der Animation anhand des aktuellen Frames bestimmen
	ComputeCurrentCharacteristics();

	BS_ASSERT(m_CurrentFrame < animationDescriptionPtr->getFrameCount());
	BS_ASSERT(m_CurrentFrameTime >= 0);
}

// -----------------------------------------------------------------------------

void Animation::ComputeCurrentCharacteristics() {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	const AnimationResource::Frame &CurFrame = animationDescriptionPtr->getFrame(m_CurrentFrame);

	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(CurFrame.FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmap = static_cast<BitmapResource *>(pResource);

	// Größe des Bitmaps auf die Animation übertragen
	_width = static_cast<int>(pBitmap->getWidth() * _scaleFactorX);
	_height = static_cast<int>(pBitmap->getHeight() * _scaleFactorY);

	// Position anhand des Hotspots berechnen und setzen
	int PosX = _relX + ComputeXModifier();
	int PosY = _relY + ComputeYModifier();

	RenderObject::setPos(PosX, PosY);

	pBitmap->release();
}

// -----------------------------------------------------------------------------

bool Animation::LockAllFrames() {
	if (!m_FramesLocked) {
		AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
		BS_ASSERT(animationDescriptionPtr);
		for (uint i = 0; i < animationDescriptionPtr->getFrameCount(); ++i) {
			if (!Kernel::GetInstance()->GetResourceManager()->RequestResource(animationDescriptionPtr->getFrame(i).FileName)) {
				BS_LOG_ERRORLN("Could not lock all animation frames.");
				return false;
			}
		}

		m_FramesLocked = true;
	}

	return true;
}

// -----------------------------------------------------------------------------

bool Animation::UnlockAllFrames() {
	if (m_FramesLocked) {
		AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
		BS_ASSERT(animationDescriptionPtr);
		for (uint i = 0; i < animationDescriptionPtr->getFrameCount(); ++i) {
			Resource *pResource;
			if (!(pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(animationDescriptionPtr->getFrame(i).FileName))) {
				BS_LOG_ERRORLN("Could not unlock all animation frames.");
				return false;
			}

			// Zwei mal freigeben um den Request von LockAllFrames() und den jetzigen Request aufzuheben
			pResource->release();
			if (pResource->GetLockCount())
				pResource->release();
		}

		m_FramesLocked = false;
	}

	return true;
}

// -----------------------------------------------------------------------------
// Getter
// -----------------------------------------------------------------------------

Animation::ANIMATION_TYPES Animation::getAnimationType() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->getAnimationType();
}

// -----------------------------------------------------------------------------

int Animation::getFPS() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->getFPS();
}

// -----------------------------------------------------------------------------

int Animation::getFrameCount() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->getFrameCount();
}

// -----------------------------------------------------------------------------

bool Animation::isScalingAllowed() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->isScalingAllowed();
}

// -----------------------------------------------------------------------------

bool Animation::isAlphaAllowed() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->isAlphaAllowed();
}

// -----------------------------------------------------------------------------

bool Animation::isColorModulationAllowed() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->isColorModulationAllowed();
}

// -----------------------------------------------------------------------------
// Positionieren
// -----------------------------------------------------------------------------

void Animation::setPos(int relX, int relY) {
	_relX = relX;
	_relY = relY;

	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------

void Animation::setX(int relX) {
	_relX = relX;

	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------

void Animation::setY(int relY) {
	_relY = relY;

	ComputeCurrentCharacteristics();
}

// -----------------------------------------------------------------------------
// Darstellungsart festlegen
// -----------------------------------------------------------------------------

void Animation::setAlpha(int alpha) {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->isAlphaAllowed()) {
		BS_LOG_WARNINGLN("Tried to set alpha value on an animation that does not support alpha. Call was ignored.");
		return;
	}

	uint newModulationColor = (_modulationColor & 0x00ffffff) | alpha << 24;
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

// -----------------------------------------------------------------------------

void Animation::setModulationColor(uint modulationColor) {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->isColorModulationAllowed()) {
		BS_LOG_WARNINGLN("Tried to set modulation color on an animation that does not support color modulation. Call was ignored");
		return;
	}

	uint newModulationColor = (modulationColor & 0x00ffffff) | (_modulationColor & 0xff000000);
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

// -----------------------------------------------------------------------------

void Animation::setScaleFactor(float scaleFactor) {
	setScaleFactorX(scaleFactor);
	setScaleFactorY(scaleFactor);
}

// -----------------------------------------------------------------------------

void Animation::setScaleFactorX(float scaleFactorX) {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->isScalingAllowed()) {
		BS_LOG_WARNINGLN("Tried to set x scale factor on an animation that does not support scaling. Call was ignored");
		return;
	}

	if (scaleFactorX != _scaleFactorX) {
		_scaleFactorX = scaleFactorX;
		if (_scaleFactorX <= 0.0f)
			_scaleFactorX = 0.001f;
		forceRefresh();
		ComputeCurrentCharacteristics();
	}
}

// -----------------------------------------------------------------------------

void Animation::setScaleFactorY(float scaleFactorY) {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	if (!animationDescriptionPtr->isScalingAllowed()) {
		BS_LOG_WARNINGLN("Tried to set y scale factor on an animation that does not support scaling. Call was ignored");
		return;
	}

	if (scaleFactorY != _scaleFactorY) {
		_scaleFactorY = scaleFactorY;
		if (_scaleFactorY <= 0.0f)
			_scaleFactorY = 0.001f;
		forceRefresh();
		ComputeCurrentCharacteristics();
	}
}

// -----------------------------------------------------------------------------

const Common::String &Animation::GetCurrentAction() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	return animationDescriptionPtr->getFrame(m_CurrentFrame).Action;
}

// -----------------------------------------------------------------------------

int Animation::getX() const {
	return _relX;
}

// -----------------------------------------------------------------------------

int Animation::getY() const {
	return _relY;
}

// -----------------------------------------------------------------------------

int Animation::getAbsoluteX() const {
	return _absoluteX + (_relX - _x);
}

// -----------------------------------------------------------------------------

int Animation::getAbsoluteY() const {
	return _absoluteY + (_relY - _y);
}

// -----------------------------------------------------------------------------

int Animation::ComputeXModifier() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	const AnimationResource::Frame &CurFrame = animationDescriptionPtr->getFrame(m_CurrentFrame);

	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(CurFrame.FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmap = static_cast<BitmapResource *>(pResource);

	int Result = CurFrame.FlipV ? - static_cast<int>((pBitmap->getWidth() - 1 - CurFrame.HotspotX) * _scaleFactorX) :
	             - static_cast<int>(CurFrame.HotspotX * _scaleFactorX);

	pBitmap->release();

	return Result;
}

// -----------------------------------------------------------------------------

int Animation::ComputeYModifier() const {
	AnimationDescription *animationDescriptionPtr = GetAnimationDescription();
	BS_ASSERT(animationDescriptionPtr);
	const AnimationResource::Frame &CurFrame = animationDescriptionPtr->getFrame(m_CurrentFrame);

	Resource *pResource = Kernel::GetInstance()->GetResourceManager()->RequestResource(CurFrame.FileName);
	BS_ASSERT(pResource);
	BS_ASSERT(pResource->GetType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmap = static_cast<BitmapResource *>(pResource);

	int Result = CurFrame.FlipH ? - static_cast<int>((pBitmap->getHeight() - 1 - CurFrame.HotspotY) * _scaleFactorY) :
	             - static_cast<int>(CurFrame.HotspotY * _scaleFactorY);

	pBitmap->release();

	return Result;
}

// -----------------------------------------------------------------------------

void Animation::RegisterActionCallback(ANIMATION_CALLBACK Callback, uint Data) {
	ANIMATION_CALLBACK_DATA CD;
	CD.Callback = Callback;
	CD.Data = Data;
	m_ActionCallbacks.push_back(CD);
}

// -----------------------------------------------------------------------------

void Animation::RegisterLoopPointCallback(ANIMATION_CALLBACK Callback, uint Data) {
	ANIMATION_CALLBACK_DATA CD;
	CD.Callback = Callback;
	CD.Data = Data;
	m_LoopPointCallbacks.push_back(CD);
}

// -----------------------------------------------------------------------------

void Animation::RegisterDeleteCallback(ANIMATION_CALLBACK Callback, uint Data) {
	ANIMATION_CALLBACK_DATA CD;
	CD.Callback = Callback;
	CD.Data = Data;
	m_DeleteCallbacks.push_back(CD);
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

void Animation::PersistCallbackVector(OutputPersistenceBlock &Writer, const Common::Array<ANIMATION_CALLBACK_DATA> & Vector) {
	// Anzahl an Callbacks persistieren.
	Writer.write(Vector.size());

	// Alle Callbacks einzeln persistieren.
	Common::Array<ANIMATION_CALLBACK_DATA>::const_iterator It = Vector.begin();
	while (It != Vector.end()) {
		Writer.write(CallbackRegistry::getInstance().resolveCallbackPointer((void (*)(int))It->Callback));
		Writer.write(It->Data);

		++It;
	}
}

// -----------------------------------------------------------------------------

void Animation::UnpersistCallbackVector(InputPersistenceBlock &Reader, Common::Array<ANIMATION_CALLBACK_DATA> & Vector) {
	// Callbackvector leeren.
	Vector.resize(0);

	// Anzahl an Callbacks einlesen.
	uint CallbackCount;
	Reader.read(CallbackCount);

	// Alle Callbacks einzeln wieder herstellen.
	for (uint i = 0; i < CallbackCount; ++i) {
		ANIMATION_CALLBACK_DATA CallbackData;

		Common::String CallbackFunctionName;
		Reader.read(CallbackFunctionName);
		CallbackData.Callback = reinterpret_cast<ANIMATION_CALLBACK>(CallbackRegistry::getInstance().resolveCallbackFunction(CallbackFunctionName));

		Reader.read(CallbackData.Data);

		Vector.push_back(CallbackData);
	}
}

// -----------------------------------------------------------------------------

bool Animation::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= RenderObject::persist(writer);

	writer.write(_relX);
	writer.write(_relY);
	writer.write(_scaleFactorX);
	writer.write(_scaleFactorY);
	writer.write(_modulationColor);
	writer.write(m_CurrentFrame);
	writer.write(m_CurrentFrameTime);
	writer.write(m_Running);
	writer.write(m_Finished);
	writer.write(static_cast<uint>(m_Direction));

	// Je nach Animationstyp entweder das Template oder die Ressource speichern.
	if (m_AnimationResourcePtr) {
		uint Marker = 0;
		writer.write(Marker);
		writer.write(m_AnimationResourcePtr->getFileName());
	} else if (m_AnimationTemplateHandle) {
		uint Marker = 1;
		writer.write(Marker);
		writer.write(m_AnimationTemplateHandle);
	} else {
		BS_ASSERT(false);
	}

	//writer.write(m_AnimationDescriptionPtr);

	writer.write(m_FramesLocked);
	PersistCallbackVector(writer, m_LoopPointCallbacks);
	PersistCallbackVector(writer, m_ActionCallbacks);
	PersistCallbackVector(writer, m_DeleteCallbacks);

	result &= RenderObject::persistChildren(writer);

	return result;
}

// -----------------------------------------------------------------------------

bool Animation::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= RenderObject::unpersist(reader);

	reader.read(_relX);
	reader.read(_relY);
	reader.read(_scaleFactorX);
	reader.read(_scaleFactorY);
	reader.read(_modulationColor);
	reader.read(m_CurrentFrame);
	reader.read(m_CurrentFrameTime);
	reader.read(m_Running);
	reader.read(m_Finished);
	uint Direction;
	reader.read(Direction);
	m_Direction = static_cast<DIRECTION>(Direction);

	// Animationstyp einlesen.
	uint Marker;
	reader.read(Marker);
	if (Marker == 0) {
		Common::String ResourceFilename;
		reader.read(ResourceFilename);
		InitializeAnimationResource(ResourceFilename);
	} else if (Marker == 1) {
		reader.read(m_AnimationTemplateHandle);
	} else {
		BS_ASSERT(false);
	}

	reader.read(m_FramesLocked);
	if (m_FramesLocked) LockAllFrames();

	UnpersistCallbackVector(reader, m_LoopPointCallbacks);
	UnpersistCallbackVector(reader, m_ActionCallbacks);
	UnpersistCallbackVector(reader, m_DeleteCallbacks);

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
}

// -----------------------------------------------------------------------------

AnimationDescription *Animation::GetAnimationDescription() const {
	if (m_AnimationResourcePtr) return m_AnimationResourcePtr;
	else return AnimationTemplateRegistry::getInstance().resolveHandle(m_AnimationTemplateHandle);
}

} // End of namespace Sword25
