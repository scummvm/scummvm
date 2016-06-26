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

#include "titanic/core/game_object.h"
#include "titanic/core/mail_man.h"
#include "titanic/core/resource_key.h"
#include "titanic/core/room_item.h"
#include "titanic/npcs/true_talk_npc.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/star_control/star_control.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGameObject, CNamedItem)
END_MESSAGE_MAP()

CCreditText *CGameObject::_credits;

void CGameObject::init() {
	_credits = nullptr;
}

void CGameObject::deinit() {
	if (_credits) {
		_credits->clear();
		delete _credits;
		_credits = nullptr;
	}
}

CGameObject::CGameObject(): CNamedItem() {
	_bounds = Rect(0, 0, 15, 15);
	_field34 = 0;
	_field38 = 0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0xF0;
	_field48 = 0xF0;
	_field4C = 0xFF;
	_isMail = false;
	_id = 0;
	_field58 = 0;
	_visible = true;
	_field60 = 0;
	_cursorId = CURSOR_ARROW;
	_initialFrame = 0;
	_frameNumber = -1;
	_field90 = 0;
	_field94 = 0;
	_field98 = 0;
	_field9C = 0;
	_surface = nullptr;
	_fieldB8 = 0;
}

void CGameObject::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(7, indent);
	error("TODO: CGameObject::save");

	CNamedItem::save(file, indent);
}

void CGameObject::load(SimpleFile *file) {
	int val = file->readNumber();
	CResourceKey resourceKey;

	switch (val) {
	case 7:
		_clipList2.load(file);
		_frameNumber = file->readNumber();
		// Deliberate fall-through

	case 6:
		_cursorId = (CursorId)file->readNumber();
		// Deliberate fall-through

	case 5:
		_clipList1.load(file);
		// Deliberate fall-through

	case 4:
		_field60 = file->readNumber();
		// Deliberate fall-through

	case 3:
		_field40 = file->readNumber();
		// Deliberate fall-through

	case 2:
		_resource = file->readString();
		// Deliberate fall-through

	case 1:
		_bounds = file->readBounds();
		_field34 = file->readFloat();
		_field38 = file->readFloat();
		_field3C = file->readFloat();
		_field44 = file->readNumber();
		_field48 = file->readNumber();
		_field4C = file->readNumber();
		_fieldB8 = file->readNumber();
		_visible = file->readNumber() != 0;
		_isMail = file->readNumber();
		_id = file->readNumber();
		_field58 = file->readNumber();

		resourceKey.load(file);		
		_surface = nullptr;
		val = file->readNumber();
		if (val) {
			_resource = resourceKey.getString();
		}
		break;

	default:
		break;
	}

	CNamedItem::load(file);
}

void CGameObject::stopMovie() {
	if (_surface)
		_surface->stopMovie();
}

bool CGameObject::checkPoint(const Point &pt, bool ignore40, bool visibleOnly) {
	if ((!_visible && visibleOnly) || !_bounds.contains(pt))
		return false;

	if (ignore40 || _field40)
		return true;

	if (!_surface) {
		if (_frameNumber == -1)
			return true;
		loadFrame(_frameNumber);
		if (!_surface)
			return true;
	}

	Common::Point pixelPos = pt - _bounds;
	if (_surface->_blitStyleFlag) {
		pixelPos.y = ((_bounds.height() - _bounds.top) / 2) * 2 - pixelPos.y;
	}

	uint transColor = _surface->getTransparencyColor();
	uint pixel = _surface->getPixel(pixelPos);
	return pixel != transColor;
}

bool CGameObject::clipRect(const Rect &rect1, Rect &rect2) const {
	if (!rect2.intersects(rect1))
		return false;

	rect2.clip(rect1);
	return true;
}

void CGameObject::draw(CScreenManager *screenManager, const Rect &destRect, const Rect &srcRect) {
	Rect tempRect = destRect;
	if (clipRect(srcRect, tempRect)) {
		if (!_surface && !_resource.empty()) {
			loadResource(_resource);
			_resource.clear();
		}

		if (_surface)
			screenManager->blitFrom(SURFACE_PRIMARY, &tempRect, _surface);
	}
}

void CGameObject::draw(CScreenManager *screenManager, const Point &destPos) {
	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	if (_surface) {
		int xSize = _surface->getWidth();
		int ySize = _surface->getHeight();

		if (xSize > 0 && ySize > 0) {
			screenManager->blitFrom(SURFACE_BACKBUFFER, _surface, &destPos);
		}
	}
}

void CGameObject::draw(CScreenManager *screenManager, const Point &destPos, const Rect &srcRect) {
	draw(screenManager, Rect(destPos.x, destPos.y, destPos.x + 52, destPos.y + 52), srcRect);
}

void CGameObject::draw(CScreenManager *screenManager) {
	if (!_visible)
		return;
	if (_credits) {
		error("TODO: Block in CGameObject::draw");
	}

	if (_field40) {
		if (_field90) {
			if (_bounds.intersects(getGameManager()->_bounds))
				warning("TODO: _field90(screenManager);");
		}
	} else {
		if (!_surface) {
			if (!_resource.empty()) {
				loadResource(_resource);
				_resource = "";
			}
		}

		if (_surface) {
			_bounds.setWidth(_surface->getWidth());
			_bounds.setHeight(_surface->getHeight());

			if (!_bounds.width() || !_bounds.height())
				return;

			if (_frameNumber >= 0) {
				loadFrame(_frameNumber);
				_frameNumber = -1;
			}

			if (!_clipList2.empty())
				processClipList2();

			if (_bounds.intersects(getGameManager()->_bounds)) {
				if (_surface) {
					Point destPos(_bounds.left, _bounds.top);
					screenManager->blitFrom(SURFACE_BACKBUFFER, _surface, &destPos);
				}

				if (_field90)
					warning("TODO: sub_415f80(screenManager);");
			}
		}
	}
}

bool CGameObject::isPet() const {
	return isInstanceOf(CPetControl::_type);
}

void CGameObject::loadResource(const CString &name) {
	switch (name.fileTypeSuffix()) {
	case FILETYPE_IMAGE:
		loadImage(name);
		break;
	case FILETYPE_MOVIE:
		loadMovie(name);
		break;
	default:
		break;
	}
}

void CGameObject::loadMovie(const CString &name, bool pendingFlag) {
	g_vm->_filesManager->preload(name);

	// Create the surface if it doesn't already exist
	if (!_surface) {
		CGameManager *gameManager = getGameManager();
		_surface = new OSVideoSurface(gameManager->setScreenManager(), nullptr);		
	}

	// Load the new movie resource
	CResourceKey key(name);
	_surface->loadResource(key);

	if (_surface->hasSurface() && !pendingFlag) {
		_bounds.setWidth(_surface->getWidth());
		_bounds.setHeight(_surface->getHeight());
	}

	if (_initialFrame)
		loadFrame(_initialFrame);
}

void CGameObject::loadImage(const CString &name, bool pendingFlag) {
	// Get a refernce to the game and screen managers
	CGameManager *gameManager = getGameManager();
	CScreenManager *screenManager;

	if (gameManager && (screenManager = CScreenManager::setCurrent()) != nullptr) {
		// Destroy the object's surface if it already had one
		if (_surface) {
			delete _surface;
			_surface = nullptr;
		}

		g_vm->_filesManager->preload(name);

		if (!name.empty()) {
			_surface = new OSVideoSurface(screenManager, CResourceKey(name), pendingFlag);
		}

		if (_surface && !pendingFlag) {
			_bounds.right = _surface->getWidth();
			_bounds.bottom = _surface->getHeight();
		}

		// Mark the object's area as dirty, so that on the next frame rendering
		// this object will be redrawn
		makeDirty();
	}

	_initialFrame = 0;
}

void CGameObject::loadFrame(int frameNumber) {
	if (frameNumber != -1 && !_resource.empty())
		loadResource(_resource);
	
	if (_surface)
		_surface->setMovieFrame(frameNumber);

	makeDirty();
}

void CGameObject::processClipList2() {
	for (CMovieClipList::iterator i = _clipList2.begin(); i != _clipList2.end(); ++i)
		(*i)->process(this);

	_clipList2.destroyContents();
}

void CGameObject::makeDirty(const Rect &r) {
	CGameManager *gameManager = getGameManager();
	if (gameManager)
		gameManager->extendBounds(r);
}

void CGameObject::makeDirty() {
	makeDirty(_bounds);
}

bool CGameObject::soundFn1(int handle) {
	if (handle != 0 && handle != -1) {
		CGameManager *gameManager = getGameManager();
		if (gameManager)
			return gameManager->_sound.fn1(handle);
	}

	return false;
}

void CGameObject::soundFn3(int handle, int val2, int val3) {
	if (handle != 0 && handle != -1) {
		CGameManager *gameManager = getGameManager();
		if (gameManager)
			return gameManager->_sound.fn3(handle, val2, val3);
	}
}

void CGameObject::setVisible(bool val) {
	if (val != _visible) {
		_visible = val;
		makeDirty();
	}
}

void CGameObject::petHighlightGlyph(int val) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->highlightGlyph(val);
}

void CGameObject::petHideCursor() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->hideCursor();
}

void CGameObject::petShowCursor() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->showCursor();
}

void CGameObject::petSetRemoteTarget() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setRemoteTarget(this);
}

void CGameObject::playMovie(uint startFrame, uint endFrame, uint flags) {
	_frameNumber = -1;
	if (!_surface) {
		if (!_resource.empty())
			loadResource(_resource);
		_resource.clear();
	}

	if (_surface) {
		// TODO: Figure out where to do this legitimately
		OSMovie *movie = static_cast<OSMovie *>(_surface->_movie);
		if (movie)
			movie->_gameObject = this;

		_surface->playMovie(startFrame, endFrame, flags, flags != 0);

		if (flags & 0x10)
			getGameManager()->_gameState.addMovie(_surface->_movie);
	}
}

void CGameObject::playClip(const CString &name, uint flags) {
	_frameNumber = -1;
	CMovieClip *clip = _clipList1.findByName(name);
	if (clip)
		playMovie(clip->_startFrame, clip->_endFrame, flags);
}

void CGameObject::playClip(uint startFrame, uint endFrame) {
	CMovieClip *clip = new CMovieClip("", startFrame, endFrame);
	CGameManager *gameManager = getGameManager();
	CRoomItem *room = gameManager->getRoom();

	gameManager->playClip(clip, room, room);
}

void CGameObject::playRandomClip(const char **names, uint flags) {
	// Count size of array
	int count = 0;
	for (const char **p = names; *p; ++p)
		++count;

	// Play clip
	const char *name = names[g_vm->getRandomNumber(count - 1)];
	playClip(name, flags);
}

void CGameObject::playMovie(uint flags) {
	_frameNumber = -1;
	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	CVideoSurface *surface = (flags & 4) ? _surface : nullptr;
	if (_surface) {
		_surface->playMovie(flags, surface);
		
		// TODO: Figure out where to do this legitimately
		OSMovie *movie = static_cast<OSMovie *>(_surface->_movie);
		if (movie)
			movie->_gameObject = this;

		if (flags & 0x10) {
			getGameManager()->_gameState.addMovie(_surface->_movie);
		}
	}
}

void CGameObject::savePosition() {
	_savedPos = _bounds;
}

void CGameObject::resetPosition() {
	setPosition(_savedPos);
}

void CGameObject::setPosition(const Point &newPos) {
	makeDirty();
	_bounds.moveTo(newPos);
	makeDirty();
}

bool CGameObject::checkStartDragging(CMouseDragStartMsg *msg) {
	if (_visible && checkPoint(msg->_mousePos, msg->_handled, 1)) {
		savePosition();
		msg->_dragItem = this;
		return true;
	} else {
		return false;
	}
}

bool CGameObject::hasActiveMovie() const {
	if (_surface && _surface->_movie)
		return _surface->_movie->isActive();
	return false;
}

int CGameObject::getMovieFrame() const {
	if (_surface && _surface->_movie)
		return _surface->_movie->getFrame();
	return _initialFrame;
}

int CGameObject::getSurface45() const {
	return _surface ? _surface->proc45() : 0;
}

void CGameObject::sound8(bool flag) const {
	getGameManager()->_sound.managerProc8(flag ? 3 : 0);
}

void CGameObject::loadSound(const CString &name) {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		g_vm->_filesManager->preload(name);
		if (!name.empty())
			gameManager->_sound.loadSound(name);
	}
}

int CGameObject::playSound(const CString &name, int val2, int val3, int val4) {
	CProximity prox;
	prox._field8 = val2;
	prox._fieldC = val3;
	prox._field20 = val4;
	return playSound(name, prox);
}

int CGameObject::playSound(const CString &name, CProximity &prox) {
	if (prox._field28 == 2) {
		// TODO
	}

	return 0;
}

void CGameObject::stopSound(int handle, int val2) {
	if (handle != 0 && handle != -1) {
		CGameManager *gameManager = getGameManager();
		if (gameManager) {
			if (val2)
				gameManager->_sound.fn3(handle, 0, val2);
			else
				gameManager->_sound.fn2(handle);
		}
	}
}

int CGameObject::addTimer(int endVal, uint firstDuration, uint duration) {
	CTimeEventInfo *timer = new CTimeEventInfo(g_vm->_events->getTicksCount(),
		duration != 0, firstDuration, duration, this, endVal, CString());

	getGameManager()->addTimer(timer);
	return timer->_id;
}

int CGameObject::addTimer(uint firstDuration, uint duration) {
	CTimeEventInfo *timer = new CTimeEventInfo(g_vm->_events->getTicksCount(),
		duration != 0, firstDuration, duration, this, 0, CString());

	getGameManager()->addTimer(timer);
	return timer->_id;
}

void CGameObject::stopTimer(int id) {
	getGameManager()->stopTimer(id);
}

void CGameObject::gotoView(const CString &viewName, const CString &clipName) {
	CViewItem *newView = parseView(viewName);
	CGameManager *gameManager = getGameManager();
	CViewItem *oldView = gameManager ? gameManager->getView() : newView;
	if (!oldView || !newView)
		return;

	CMovieClip *clip = nullptr;
	if (clipName.empty()) {
		CLinkItem *link = oldView->findLink(newView);
		if (link)
			clip = link->getClip();
	} else {
		clip = oldView->findNode()->findRoom()->findClip(clipName);
	}

	// Change the view
	gameManager->_gameState.changeView(newView, clip);
}

CViewItem *CGameObject::parseView(const CString &viewString) {
	int firstIndex = viewString.indexOf('.');
	int lastIndex = viewString.lastIndexOf('.');
	CString roomName, nodeName, viewName;

	if (firstIndex == -1) {
		roomName = viewString;
	} else {
		roomName = viewString.left(firstIndex);

		if (lastIndex > firstIndex) {
			nodeName = viewString.mid(firstIndex + 1, lastIndex - firstIndex - 1);
			viewName = viewString.mid(lastIndex + 1);
		} else {
			nodeName = viewString.mid(firstIndex + 1);
		}
	}

	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return nullptr;

	CRoomItem *room = gameManager->getRoom();
	CProjectItem *project = room->getRoot();
	
	// Ensure we have the specified room
	if (project) {
		if (room->getName() != roomName) {
			// Scan for the correct room
			for (room = project->findFirstRoom(); room && room->getName() != roomName;
					room = project->findNextRoom(room)) ;
		}			
	}
	if (!room)
		return nullptr;

	// Find the designated node within the room
	CNodeItem *node = static_cast<CNodeItem *>(room->findChildInstanceOf(CNodeItem::_type));
	while (node && node->getName() != nodeName)
		node = static_cast<CNodeItem *>(room->findNextInstanceOf(CNodeItem::_type, node));
	if (!node)
		return nullptr;

	CViewItem *view = static_cast<CViewItem *>(node->findChildInstanceOf(CViewItem::_type));
	while (view && view->getName() != viewName)
		view = static_cast<CViewItem *>(node->findNextInstanceOf(CViewItem::_type, view));
	if (!view)
		return nullptr;

	// Find the view, so return it
	return view;
}

CString CGameObject::getViewFullName() const {
	CGameManager *gameManager = getGameManager();
	CViewItem *view = gameManager->getView();
	CNodeItem *node = view->findNode();
	CRoomItem *room = node->findRoom();

	return CString::format("%s.%s.%s", room->getName().c_str(),
		node->getName().c_str(), view->getName().c_str());
}

void CGameObject::sleep(uint milli) {
	g_vm->_events->sleep(milli);
}

Point CGameObject::getMousePos() const {
	return getGameManager()->_gameState.getMousePos();
}

bool CGameObject::compareViewNameTo(const CString &name) const {
	return getViewFullName().compareToIgnoreCase(name);
}

int CGameObject::compareRoomNameTo(const CString &name) {
	CRoomItem *room = getGameManager()->getRoom();
	return room->getName().compareToIgnoreCase(name);
}

CGameObject *CGameObject::getMailManFirstObject() const {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->getFirstObject() : nullptr;
}

CGameObject *CGameObject::getMailManNextObject(CGameObject *prior) const {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->getNextObject(prior) : nullptr;
}

CGameObject *CGameObject::findRoomObject(const CString &name) const {
	return static_cast<CGameObject *>(findRoom()->findByName(name));
}

CGameObject *CGameObject::findUnder(CTreeItem *parent, const CString &name) {
	if (!parent)
		return nullptr;

	for (CTreeItem *treeItem = parent->getFirstChild(); treeItem;
	treeItem = treeItem->scan(parent)) {
		if (!treeItem->getName().compareTo(name)) {
			return dynamic_cast<CGameObject *>(treeItem);
		}
	}

	return nullptr;
}

Found CGameObject::find(const CString &name, CGameObject **item, int findAreas) {
	CGameObject *go;
	*item = nullptr;

	// Scan under PET if flagged
	if (findAreas & FIND_PET) {
		for (go = getPetControl()->getFirstObject(); go; go = getPetControl()->getNextObject(go)) {
			if (go->getName() == name) {
				*item = go;
				return FOUND_PET;
			}
		}
	}

	if (findAreas & FIND_MAILMAN) {
		for (go = getMailManFirstObject(); go; go = getMailManNextObject(go)) {
			if (go->getName() == name) {
				*item = go;
				return FOUND_MAILMAN;
			}
		}
	}

	if (findAreas & FIND_GLOBAL) {
		go = static_cast<CGameObject *>(getRoot()->findByName(name));
		if (go) {
			*item = go;
			return FOUND_GLOBAL;
		}
	}

	if (findAreas & FIND_ROOM) {
		go = findRoomObject(name);
		if (go) {
			*item = go;
			return FOUND_ROOM;
		}
	}

	return FOUND_NONE;
}

void CGameObject::moveToView() {
	CViewItem *view = getGameManager()->getView();
	detach();
	view->addUnder(this);
}

void CGameObject::incState38() {
	getGameManager()->_gameState.inc38();
}

void CGameObject::inc54() {
	getGameManager()->inc54();
}

void CGameObject::dec54() {
	getGameManager()->dec54();
}

void CGameObject::lockMouse() {
	CGameManager *gameMan = getGameManager();
	gameMan->lockInputHandler();

	if (CScreenManager::_screenManagerPtr->_mouseCursor)
		CScreenManager::_screenManagerPtr->_mouseCursor->hide();
}

void CGameObject::unlockMouse() {
	if (CScreenManager::_screenManagerPtr->_mouseCursor)
		CScreenManager::_screenManagerPtr->_mouseCursor->show();

	CGameManager *gameMan = getGameManager();
	gameMan->unlockInputHandler();
}

void CGameObject::loadSurface() {
	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	if (_surface)
		_surface->loadIfReady();
}

bool CGameObject::changeView(const CString &viewName, const CString &clipName) {
	CViewItem *newView = parseView(viewName);
	CGameManager *gameManager = getGameManager();
	CViewItem *oldView = gameManager->getView();

	if (!oldView || !newView)
		return false;
	
	CMovieClip *clip = nullptr;
	if (!clipName.empty()) {
		clip = oldView->findNode()->findRoom()->findClip(clipName);
	} else {
		CLinkItem *link = oldView->findLink(newView);
		if (link)
			clip = link->getClip();
	}

	gameManager->_gameState.changeView(newView, clip);
	return true;
}

void CGameObject::dragMove(const Point &pt) {
	if (_surface) {
		_bounds.setWidth(_surface->getWidth());
		_bounds.setHeight(_surface->getHeight());
	}

	setPosition(Point(pt.x - _bounds.width() / 2, pt.y - _bounds.height() / 2));
}

Point CGameObject::getControid() const {
	return Point(_bounds.left + _bounds.width() / 2,
		_bounds.top + _bounds.height() / 2);
}

void CGameObject::performAction(int actionNum, CViewItem *view) {
	// TODO
}

bool CGameObject::clipExistsByStart(const CString &name, int startFrame) const {
	return _clipList1.existsByStart(name, startFrame);
}

bool CGameObject::clipExistsByEnd(const CString &name, int endFrame) const {
	return _clipList1.existsByEnd(name, endFrame);
}

void CGameObject::checkPlayMovie(const CString &name, int flags) {
	if (!_surface && !_resource.empty())
		loadResource(_resource);

	if (_surface ) {
		_surface->proc35(name, flags, (flags & CLIPFLAG_4) ? this : nullptr);
		if (flags & CLIPFLAG_PLAY)
			getGameManager()->_gameState.addMovie(_surface->_movie);
	}
}

void CGameObject::petClear() const {
	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->resetActiveNPC();
}

CPetControl *CGameObject::getPetControl() const {
	return static_cast<CPetControl *>(getDontSaveChild(CPetControl::_type));
}

CMailMan *CGameObject::getMailMan() const {
	return dynamic_cast<CMailMan *>(getDontSaveChild(CMailMan::_type));
}

CStarControl *CGameObject::getStarControl() const {
	CStarControl *starControl = static_cast<CStarControl *>(getDontSaveChild(CStarControl::_type));
	if (!starControl) {
		CViewItem *view = getGameManager()->getView();
		if (view)
			starControl = starControl = static_cast<CStarControl *>(view->findChildInstanceOf(CStarControl::_type));
	}

	return starControl;
}

CTreeItem *CGameObject::getDontSaveChild(ClassDef *classDef) const {
	CProjectItem *root = getRoot();
	if (!root)
		return nullptr;

	CDontSaveFileItem *dontSave = root->getDontSaveFileItem();
	if (!dontSave)
		return nullptr;

	return dontSave->findChildInstanceOf(classDef);
}

CRoomItem *CGameObject::getRoom() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->getRoom() : nullptr;
}

CRoomItem *CGameObject::getHiddenRoom() const {
	CProjectItem *root = getRoot();
	return root ? root->findHiddenRoom() : nullptr;
}

CMusicRoom *CGameObject::getMusicRoom() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? &gameManager->_musicRoom : nullptr;
}

int CGameObject::getPassengerClass() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->_gameState._passengerClass : 3;
}

int CGameObject::getPriorClass() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->_gameState._priorClass : 3;
}

void CGameObject::setPassengerClass(int newClass) {
	if (newClass >= 1 && newClass <= 4) {
		// Change the passenger class
		CGameManager *gameMan = getGameManager();
		gameMan->_gameState._priorClass = gameMan->_gameState._passengerClass;
		gameMan->_gameState._passengerClass = newClass;

		// Setup the PET again, so the new class's PET background can take effect
		CPetControl *petControl = getPetControl();
		if (petControl)
			petControl->setup();
	}
}

void CGameObject::createCredits() {
	_credits = new CCreditText();
	CScreenManager *screenManager = getGameManager()->setScreenManager();
	_credits->load(this, screenManager, _bounds);
}

void CGameObject::fn10(int v1, int v2, int v3) {
	makeDirty();
	_field44 = v1;
	_field48 = v2;
	_field4C = v3;
}

void CGameObject::setMovie14(int v) {
	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	if (_surface && _surface->_movie)
		_surface->_movie->_field14 = v;
}

void CGameObject::movie38(int v1, int v2) {
	if (_surface)
		_surface->proc38(v1, v2);
}

void CGameObject::movie38(int v1) {
	if (_surface)
		_surface->proc38(-1, v1);
}

int CGameObject::getClipDuration(const CString &name, int frameRate) const {
	CMovieClip *clip = _clipList1.findByName(name);
	return clip ? (clip->_endFrame - clip->_startFrame) * 1000 / frameRate : 0;
}

bool CGameObject::compareRoomFlags(int mode, uint flags1, uint flags2) {
	switch (mode) {
	case 1:
		return CRoomFlags::compareLocation(flags1, flags2);

	case 2:
		return CRoomFlags::compareClassElevator(flags1, flags2);

	case 3:
		return CRoomFlags::isTitania(flags1, flags2);

	default:
		return false;
	}
}

void CGameObject::setState1C(bool flag) {
	getGameManager()->_gameState._field1C = flag;
}

void CGameObject::addMail(int mailId) {
	CMailMan *mailMan = getMailMan();
	if (mailMan) {
		makeDirty();
		mailMan->addMail(this, mailId);
	}
}

void CGameObject::setMailId(int mailId) {
	CMailMan *mailMan = getMailMan();
	if (mailMan) {
		makeDirty();
		mailMan->setMailId(this, mailId);
	}
}

bool CGameObject::mailExists(int id) const {
	return findMail(id) != nullptr;
}

CGameObject *CGameObject::findMail(int id) const {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->findMail(id) : nullptr;
}

void CGameObject::removeMail(int id, int v) {
	CMailMan *mailMan = getMailMan();
	if (mailMan)
		mailMan->removeMail(id, v);
}

void CGameObject::resetMail() {
	CMailMan *mailMan = getMailMan();
	if (mailMan)
		mailMan->resetValue();
}

void CGameObject::petAddToCarryParcel(CGameObject *obj) {
	CPetControl *pet = getPetControl();
	if (pet) {
		CGameObject *parcel = pet->getHiddenObject("CarryParcel");
		if (parcel)
			parcel->moveUnder(obj);
	}
}

void CGameObject::petAddToInventory() {
	CPetControl *pet = getPetControl();
	if (pet) {
		makeDirty();
		pet->addToInventory(this);
	}
}

CTreeItem *CGameObject::petContainerRemove(CGameObject *obj) {
	CPetControl *pet = getPetControl();
	if (!obj || !pet)
		return nullptr;
	if (!obj->compareRoomNameTo("CarryParcel"))
		return obj;

	CGameObject *item = static_cast<CGameObject *>(pet->getLastChild());
	if (item)
		item->detach();

	pet->moveToHiddenRoom(obj);
	pet->removeFromInventory(item, false, false);

	return item;
}

bool CGameObject::petDismissBot(const CString &name) {
	CPetControl *pet = getPetControl();
	return pet ? pet->dismissBot(name) : false;
}

bool CGameObject::petDoorOrBellbotPresent() const {
	CPetControl *pet = getPetControl();
	return pet ? pet->isDoorOrBellbotPresent() : false;
}

void CGameObject::petDisplayMessage(int unused, const CString &msg) {
	petDisplayMessage(msg);
}

void CGameObject::petDisplayMessage(const CString &msg) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->displayMessage(msg);
}

int CGameObject::petGetRooms1D0() const {
	CPetControl *petControl = getPetControl();
	return petControl ? petControl->getRooms1D0() : 0;
}

void CGameObject::petInvChange() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->invChange(this);
}

void CGameObject::petLockInput() {
	getPetControl()->incInputLocks();
}

void CGameObject::petMoveToHiddenRoom() {
	CPetControl *pet = getPetControl();
	if (pet) {
		makeDirty();
		pet->moveToHiddenRoom(this);
	}
}

void CGameObject::petReassignRoom(int passClassNum) {
	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->reassignRoom(passClassNum);
}

void CGameObject::petSetArea(PetArea newArea) const {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setArea(newArea);
}

void CGameObject::petSetRooms1D0(int val) {
	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->setRooms1D0(val);
}

void CGameObject::petOnSummonBot(const CString &name, int val) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->summonBot(name, val);
}

void CGameObject::petUnlockInput() {
	getPetControl()->decInputLocks();
}

/*------------------------------------------------------------------------*/

void CGameObject::startTalking(const CString &npcName, uint id, CViewItem *view) {
	CTrueTalkNPC *npc = static_cast<CTrueTalkNPC *>(getRoot()->findByName(npcName));
	startTalking(npc, id, view);
}

void CGameObject::startTalking(CTrueTalkNPC *npc, uint id, CViewItem *view) {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		CTrueTalkManager *talkManager = gameManager->getTalkManager();
		if (talkManager)
			talkManager->start(npc, id, view);
	}
}

void CGameObject::endTalking(CTrueTalkNPC *npc, bool viewFlag, CViewItem *view) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setActiveNPC(npc);

	if (viewFlag)
		npc->setView(view);

	if (pet)
		pet->refreshNPC();
}

} // End of namespace Titanic
