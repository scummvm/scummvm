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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

int RenderObject::_nextGlobalVersion = 0;

RenderObject::RenderObject(RenderObjectPtr<RenderObject> parentPtr, TYPES type, uint handle) :
	_managerPtr(0),
	_parentPtr(parentPtr),
	_x(0),
	_y(0),
	_z(0),
	_oldX(-1),
	_oldY(-1),
	_oldZ(-1),
	_width(0),
	_height(0),
	_visible(true),
	_oldVisible(false),
	_childChanged(true),
	_type(type),
	_initSuccess(false),
	_refreshForced(true),
	_handle(0),
	_version(++_nextGlobalVersion),
	_isSolid(false) {

	if (handle == 0)
		_handle = RenderObjectRegistry::instance().registerObject(this);
	else
		_handle = RenderObjectRegistry::instance().registerObject(this, handle);

	if (_handle == 0)
		error("Failed to initialize RenderObject()");

	updateAbsolutePos();

	// Add this item to the children of the parent object, if not root (ParentPtr is invalid),
	// assign to the same RenderObjectManager.
	if (_parentPtr.isValid()) {
		_managerPtr = _parentPtr->getManager();
		_parentPtr->addObject(this->getHandle());
	} else {
		if (getType() != TYPE_ROOT) {
			error("Tried to create a non-root render object and has no parent. All non-root render objects have to have a parent.");
			return;
		}
	}

	updateObjectState();

	_initSuccess = true;
}

RenderObject::~RenderObject() {
	// Remove object from its parent.
	if (_parentPtr.isValid())
		_parentPtr->detatchChildren(this->getHandle());

	deleteAllChildren();

	RenderObjectRegistry::instance().deregisterObject(this);
}

void RenderObject::preRender(RenderObjectQueue *renderQueue) {
	validateObject();

	if (!_visible)
		return;

	// If necessary, update the children rendering order of the updated objects.
	if (_childChanged) {
		sortRenderObjects();
		_childChanged = false;
	}

	renderQueue->add(this);

	RENDEROBJECT_ITER it = _children.begin();
	for (; it != _children.end(); ++it)
		(*it)->preRender(renderQueue);

}

bool RenderObject::render(RectangleList *updateRects, const Common::Array<int> &updateRectsMinZ) {

	// Falls das Objekt nicht sichtbar ist, muss gar nichts gezeichnet werden
	if (!_visible)
		return true;

	// Objekt zeichnen.
	bool needRender = false;
	int index = 0;

	// Only draw if the bounding box intersects any update rectangle and
	// the object is in front of the minimum Z value.
	for (RectangleList::iterator rectIt = updateRects->begin(); !needRender && rectIt != updateRects->end(); ++rectIt, ++index)
		needRender = (_bbox.contains(*rectIt) || _bbox.intersects(*rectIt)) && getAbsoluteZ() >= updateRectsMinZ[index];

	if (needRender)
		doRender(updateRects);

	// Draw all children
	RENDEROBJECT_ITER it = _children.begin();
	for (; it != _children.end(); ++it)
		if (!(*it)->render(updateRects, updateRectsMinZ))
			return false;

	return true;
}

void RenderObject::validateObject() {
	_oldBbox = _bbox;
	_oldVisible = _visible;
	_oldX = _x;
	_oldY = _y;
	_oldZ = _z;
	_refreshForced = false;
}

bool RenderObject::updateObjectState() {
	// If the object has changed, the internal state must be recalculated and possibly
	// update Regions be registered for the next frame.
	if ((calcBoundingBox() != _oldBbox) ||
	        (_visible != _oldVisible) ||
	        (_x != _oldX) ||
	        (_y != _oldY) ||
	        (_z != _oldZ) ||
	        _refreshForced) {
		if (_parentPtr.isValid())
			_parentPtr->signalChildChange();

		// Die Bounding-Box neu berechnen und Update-Regions registrieren.
		updateBoxes();

		++_version;

		// Änderungen Validieren
		validateObject();
	}

	// Dann muss der Objektstatus der Kinder aktualisiert werden.
	RENDEROBJECT_ITER it = _children.begin();
	for (; it != _children.end(); ++it)
		if (!(*it)->updateObjectState())
			return false;

	return true;
}

void RenderObject::updateBoxes() {
	_bbox = calcBoundingBox();
}

Common::Rect RenderObject::calcBoundingBox() const {
	Common::Rect bbox(0, 0, _width, _height);

	// Die absolute Position der Bounding-Box berechnen.
	bbox.translate(_absoluteX, _absoluteY);

	// Die Bounding-Box am Elternobjekt clippen.
	if (_parentPtr.isValid()) {
		bbox.clip(_parentPtr->getBbox());
	}

	return bbox;
}

void RenderObject::calcAbsolutePos(int32 &x, int32 &y, int32 &z) const {
	x = calcAbsoluteX();
	y = calcAbsoluteY();
	z = calcAbsoluteZ();
}

int32 RenderObject::calcAbsoluteX() const {
	if (_parentPtr.isValid())
		return _parentPtr->getAbsoluteX() + _x;
	else
		return _x;
}

int32 RenderObject::calcAbsoluteY() const {
	if (_parentPtr.isValid())
		return _parentPtr->getAbsoluteY() + _y;
	else
		return _y;
}

int32 RenderObject::calcAbsoluteZ() const {
	if (_parentPtr.isValid())
		return _parentPtr->getAbsoluteZ() + _z;
	else
		return _z;
}

void RenderObject::deleteAllChildren() {
	while (!_children.empty()) {
		RenderObjectPtr<RenderObject> curPtr = _children.back();
		curPtr.erase();
	}
}

bool RenderObject::addObject(RenderObjectPtr<RenderObject> pObject) {
	if (!pObject.isValid()) {
		error("Tried to add a null object to a renderobject.");
		return false;
	}

	// Insert Object in the children list.
	_children.push_back(pObject);

	// Make sure that before the next render the channel order is updated.
	if (_parentPtr.isValid())
		_parentPtr->signalChildChange();

	return true;
}

bool RenderObject::detatchChildren(RenderObjectPtr<RenderObject> pObject) {
	// Kinderliste durchgehen und Objekt entfernen falls vorhanden
	RENDEROBJECT_ITER it = _children.begin();
	for (; it != _children.end(); ++it)
		if (*it == pObject) {
			_children.erase(it);
			return true;
		}

	error("Tried to detach children from a render object that isn't its parent.");
	return false;
}

void RenderObject::sortRenderObjects() {
	Common::sort(_children.begin(), _children.end(), greater);
}

void RenderObject::updateAbsolutePos() {
	calcAbsolutePos(_absoluteX, _absoluteY, _absoluteZ);

	RENDEROBJECT_ITER it = _children.begin();
	for (; it != _children.end(); ++it)
		(*it)->updateAbsolutePos();
}

bool RenderObject::getObjectIntersection(RenderObjectPtr<RenderObject> pObject, Common::Rect &result) {
	result = pObject->getBbox();
	result.clip(_bbox);
	return result.isValidRect();
}

void RenderObject::setPos(int x, int y) {
	_x = x;
	_y = y;
	updateAbsolutePos();
}

void RenderObject::setX(int x) {
	_x = x;
	updateAbsolutePos();
}

void RenderObject::setY(int y) {
	_y = y;
	updateAbsolutePos();
}

void RenderObject::setZ(int z) {
	if (z < 0)
		error("Tried to set a negative Z value (%d).", z);
	else {
		_z = z;
		updateAbsolutePos();
	}
}

void RenderObject::setVisible(bool visible) {
	_visible = visible;
}

RenderObjectPtr<Animation> RenderObject::addAnimation(const Common::String &filename) {
	RenderObjectPtr<Animation> aniPtr((new Animation(this->getHandle(), filename))->getHandle());
	if (aniPtr.isValid() && aniPtr->getInitSuccess())
		return aniPtr;
	else {
		if (aniPtr.isValid())
			aniPtr.erase();
		return RenderObjectPtr<Animation>();
	}
}

RenderObjectPtr<Animation> RenderObject::addAnimation(const AnimationTemplate &animationTemplate) {
	Animation *aniPtr = new Animation(this->getHandle(), animationTemplate);
	if (aniPtr && aniPtr->getInitSuccess())
		return aniPtr->getHandle();
	else {
		delete aniPtr;
		return RenderObjectPtr<Animation>();
	}
}

RenderObjectPtr<Bitmap> RenderObject::addBitmap(const Common::String &filename) {
	RenderObjectPtr<Bitmap> bitmapPtr((new StaticBitmap(this->getHandle(), filename))->getHandle());
	if (bitmapPtr.isValid() && bitmapPtr->getInitSuccess())
		return RenderObjectPtr<Bitmap>(bitmapPtr);
	else {
		if (bitmapPtr.isValid())
			bitmapPtr.erase();
		return RenderObjectPtr<Bitmap>();
	}
}

RenderObjectPtr<Bitmap> RenderObject::addDynamicBitmap(uint width, uint height) {
	RenderObjectPtr<Bitmap> bitmapPtr((new DynamicBitmap(this->getHandle(), width, height))->getHandle());
	if (bitmapPtr.isValid() && bitmapPtr->getInitSuccess())
		return bitmapPtr;
	else {
		if (bitmapPtr.isValid())
			bitmapPtr.erase();
		return RenderObjectPtr<Bitmap>();
	}
}

RenderObjectPtr<Panel> RenderObject::addPanel(int width, int height, uint color) {
	RenderObjectPtr<Panel> panelPtr((new Panel(this->getHandle(), width, height, color))->getHandle());
	if (panelPtr.isValid() && panelPtr->getInitSuccess())
		return panelPtr;
	else {
		if (panelPtr.isValid())
			panelPtr.erase();
		return RenderObjectPtr<Panel>();
	}
}

RenderObjectPtr<Text> RenderObject::addText(const Common::String &font, const Common::String &text) {
	RenderObjectPtr<Text> textPtr((new Text(this->getHandle()))->getHandle());
	if (textPtr.isValid() && textPtr->getInitSuccess() && textPtr->setFont(font)) {
		textPtr->setText(text);
		return textPtr;
	} else {
		if (textPtr.isValid())
			textPtr.erase();
		return RenderObjectPtr<Text>();
	}
}

bool RenderObject::persist(OutputPersistenceBlock &writer) {
	// Typ und Handle werden als erstes gespeichert, damit beim Laden ein Objekt vom richtigen Typ mit dem richtigen Handle erzeugt werden kann.
	writer.write(static_cast<uint32>(_type));
	writer.write(_handle);

	// Restliche Objekteigenschaften speichern.
	writer.write(_x);
	writer.write(_y);
	writer.write(_absoluteX);
	writer.write(_absoluteY);
	writer.write(_z);
	writer.write(_width);
	writer.write(_height);
	writer.write(_visible);
	writer.write(_childChanged);
	writer.write(_initSuccess);
	writer.write((int32)_bbox.left);
	writer.write((int32)_bbox.top);
	writer.write((int32)_bbox.right);
	writer.write((int32)_bbox.bottom);
	writer.write((int32)_oldBbox.left);
	writer.write((int32)_oldBbox.top);
	writer.write((int32)_oldBbox.right);
	writer.write((int32)_oldBbox.bottom);
	writer.write(_oldX);
	writer.write(_oldY);
	writer.write(_oldZ);
	writer.write(_oldVisible);
	writer.write(_parentPtr.isValid() ? _parentPtr->getHandle() : 0);
	writer.write(_refreshForced);

	return true;
}

bool RenderObject::unpersist(InputPersistenceBlock &reader) {
	// Typ und Handle wurden schon von RecreatePersistedRenderObject() ausgelesen. Jetzt werden die restlichen Objekteigenschaften ausgelesen.
	reader.read(_x);
	reader.read(_y);
	reader.read(_absoluteX);
	reader.read(_absoluteY);
	reader.read(_z);
	reader.read(_width);
	reader.read(_height);
	reader.read(_visible);
	reader.read(_childChanged);
	reader.read(_initSuccess);
	reader.read(_bbox.left);
	reader.read(_bbox.top);
	reader.read(_bbox.right);
	reader.read(_bbox.bottom);
	reader.read(_oldBbox.left);
	reader.read(_oldBbox.top);
	reader.read(_oldBbox.right);
	reader.read(_oldBbox.bottom);
	reader.read(_oldX);
	reader.read(_oldY);
	reader.read(_oldZ);
	reader.read(_oldVisible);
	uint32 parentHandle;
	reader.read(parentHandle);
	_parentPtr = RenderObjectPtr<RenderObject>(parentHandle);
	reader.read(_refreshForced);

	updateAbsolutePos();
	updateObjectState();

	return reader.isGood();
}

bool RenderObject::persistChildren(OutputPersistenceBlock &writer) {
	bool result = true;

	// Kinderanzahl speichern.
	writer.write((uint32)_children.size());

	// Rekursiv alle Kinder speichern.
	RENDEROBJECT_LIST::iterator it = _children.begin();
	while (it != _children.end()) {
		result &= (*it)->persist(writer);
		++it;
	}

	return result;
}

bool RenderObject::unpersistChildren(InputPersistenceBlock &reader) {
	bool result = true;

	// Kinderanzahl einlesen.
	uint32 childrenCount;
	reader.read(childrenCount);
	if (!reader.isGood())
		return false;

	// Alle Kinder rekursiv wieder herstellen.
	for (uint32 i = 0; i < childrenCount; ++i) {
		if (!recreatePersistedRenderObject(reader).isValid())
			return false;
	}

	return result && reader.isGood();
}

RenderObjectPtr<RenderObject> RenderObject::recreatePersistedRenderObject(InputPersistenceBlock &reader) {
	RenderObjectPtr<RenderObject> result;

	// Typ und Handle auslesen.
	uint32 type;
	uint32 handle;
	reader.read(type);
	reader.read(handle);
	if (!reader.isGood())
		return result;

	switch (type) {
	case TYPE_PANEL:
		result = (new Panel(reader, this->getHandle(), handle))->getHandle();
		break;

	case TYPE_STATICBITMAP:
		result = (new StaticBitmap(reader, this->getHandle(), handle))->getHandle();
		break;

	case TYPE_DYNAMICBITMAP:
		// Videos are not normally saved: this probably indicates a bug, thus die here.
		//result = (new DynamicBitmap(reader, this->getHandle(), handle))->getHandle();
		error("Request to recreate a video. This is either a corrupted saved game, or a bug");
		break;

	case TYPE_TEXT:
		result = (new Text(reader, this->getHandle(), handle))->getHandle();
		break;

	case TYPE_ANIMATION:
		result = (new Animation(reader, this->getHandle(), handle))->getHandle();
		break;

	default:
		error("Cannot recreate render object of unknown type %d.", type);
	}

	return result;
}

bool RenderObject::greater(const RenderObjectPtr<RenderObject> lhs, const RenderObjectPtr<RenderObject> rhs) {
	// Das Objekt mit dem kleinem Z-Wert müssen zuerst gerendert werden.
	if (lhs->_z != rhs->_z)
		return lhs->_z < rhs->_z;
	// Falls der Z-Wert gleich ist, wird das weiter oben gelegenen Objekte zuerst gezeichnet.
	return lhs->_y < rhs->_y;
}

} // End of namespace Sword25
