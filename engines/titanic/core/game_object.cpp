/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "titanic/core/game_object.h"
#include "titanic/core/mail_man.h"
#include "titanic/core/resource_key.h"
#include "titanic/core/room_item.h"
#include "titanic/core/project_item.h"
#include "titanic/debugger.h"
#include "titanic/events.h"
#include "titanic/game_manager.h"
#include "titanic/npcs/true_talk_npc.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/star_control/star_control.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "titanic/titanic.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CGameObject, CNamedItem);

CCreditText *CGameObject::_credits;
int CGameObject::_soundHandles[4];

void CGameObject::init() {
	_credits = nullptr;
	_soundHandles[0] = _soundHandles[1] = -1;
	_soundHandles[2] = _soundHandles[3] = -1;
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
	_unused1 = 0;
	_unused2 = 0;
	_unused3 = 0;
	_nonvisual = false;
	_toggleR = 0xF0;
	_toggleG = 0xF0;
	_toggleB = 0xFF;
	_isPendingMail = false;
	_destRoomFlags = 0;
	_roomFlags = 0;
	_visible = true;
	_handleMouseFlag = false;
	_cursorId = CURSOR_ARROW;
	_initialFrame = 0;
	_frameNumber = -1;
	_text = nullptr;
	_textBorder = _textBorderRight = 0;
	_surface = nullptr;
	_unused4 = 0;
}

CGameObject::~CGameObject() {
	delete _surface;
	delete _text;
}

void CGameObject::save(SimpleFile *file, int indent) {
	file->writeNumberLine(7, indent);
	_movieRangeInfoList.destroyContents();

	if (_surface) {
		const CMovieRangeInfoList *rangeList = _surface->getMovieRangeInfo();

		if (rangeList) {
			for (CMovieRangeInfoList::const_iterator i = rangeList->begin();
				i != rangeList->end(); ++i) {
				CMovieRangeInfo *rangeInfo = new CMovieRangeInfo(*i);
				rangeInfo->_initialFrame = (i == rangeList->begin()) ? getMovieFrame() : -1;
				_movieRangeInfoList.push_back(rangeInfo);
			}
		}
	}

	_movieRangeInfoList.save(file, indent);
	_movieRangeInfoList.destroyContents();

	file->writeNumberLine(getMovieFrame(), indent + 1);
	file->writeNumberLine(_cursorId, indent + 1);
	_movieClips.save(file, indent + 1);
	file->writeNumberLine(_handleMouseFlag, indent + 1);
	file->writeNumberLine(_nonvisual, indent + 1);
	file->writeQuotedLine(_resource, indent + 1);
	file->writeBounds(_bounds, indent + 1);

	file->writeFloatLine(_unused1, indent + 1);
	file->writeFloatLine(_unused2, indent + 1);
	file->writeFloatLine(_unused3, indent + 1);

	file->writeNumberLine(_toggleR, indent + 1);
	file->writeNumberLine(_toggleG, indent + 1);
	file->writeNumberLine(_toggleB, indent + 1);
	file->writeNumberLine(_unused4, indent + 1);
	file->writeNumberLine(_visible, indent + 1);
	file->writeNumberLine(_isPendingMail, indent + 1);
	file->writeNumberLine(_destRoomFlags, indent + 1);
	file->writeNumberLine(_roomFlags, indent + 1);

	if (_surface) {
		_surface->_resourceKey.save(file, indent);
	} else {
		CResourceKey resourceKey;
		resourceKey.save(file, indent);
	}
	file->writeNumberLine(_surface != nullptr, indent);

	CNamedItem::save(file, indent);
}

void CGameObject::load(SimpleFile *file) {
	int val = file->readNumber();
	CResourceKey resourceKey;

	switch (val) {
	case 7:
		_movieRangeInfoList.load(file);
		_frameNumber = file->readNumber();
		// Intentional fall-through

	case 6:
		_cursorId = (CursorId)file->readNumber();
		// Intentional fall-through

	case 5:
		_movieClips.load(file);
		// Intentional fall-through

	case 4:
		_handleMouseFlag = file->readNumber();
		// Intentional fall-through

	case 3:
		_nonvisual = file->readNumber();
		// Intentional fall-through

	case 2:
		_resource = file->readString();
		// Intentional fall-through

	case 1:
		_bounds = file->readBounds();
		_unused1 = file->readFloat();
		_unused2 = file->readFloat();
		_unused3 = file->readFloat();
		_toggleR = file->readNumber();
		_toggleG = file->readNumber();
		_toggleB = file->readNumber();
		_unused4 = file->readNumber();
		_visible = file->readNumber() != 0;
		_isPendingMail = file->readNumber();
		_destRoomFlags = file->readNumber();
		_roomFlags = file->readNumber();

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

void CGameObject::draw(CScreenManager *screenManager) {
	if (!_visible)
		return;
	if (_credits && _credits->_objectP == this) {
		if (!_credits->draw())
			CGameObject::deinit();

		return;
	}

	if (_nonvisual) {
		// If a text is defined, handle drawing it
		if (_text && _bounds.intersects(getGameManager()->_bounds))
			_text->draw(screenManager);

		return;
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

			if (!_movieRangeInfoList.empty())
				processMoveRangeInfo();

			if (_bounds.intersects(getGameManager()->_bounds)) {
				if (_surface) {
					Point destPos(_bounds.left, _bounds.top);
					screenManager->blitFrom(SURFACE_BACKBUFFER, _surface, &destPos);
				}

				if (_text)
					_text->draw(screenManager);
			}
		}
	}
}

Rect CGameObject::getBounds() const {
	return (_surface && _surface->hasFrame()) ? _bounds : Rect();
}

void CGameObject::freeSurface() {
	// Handle freeing the surfaces of objects when their view is left
	if (_surface) {
		_resource = _surface->_resourceKey.getString();
		_initialFrame = getMovieFrame();

		delete _surface;
		_surface = nullptr;
	}
}

void CGameObject::stopMovie() {
	if (_surface)
		_surface->stopMovie();
}

bool CGameObject::checkPoint(const Point &pt, bool ignoreSurface, bool visibleOnly) {
	if ((!_visible && visibleOnly) || !_bounds.contains(pt))
		return false;

	if (ignoreSurface || _nonvisual)
		return true;

	if (!_surface) {
		if (_frameNumber == -1)
			return true;
		loadFrame(_frameNumber);
		if (!_surface)
			return true;
	}

	Common::Point pixelPos = pt - _bounds;
	if (_surface->_flipVertically) {
		pixelPos.y = ((_bounds.height() - _bounds.top) / 2) * 2 - pixelPos.y;
	}

	uint transColor = _surface->getTransparencyColor();
	uint pixel = _surface->getPixel(pixelPos);
	return pixel != transColor;
}

bool CGameObject::findPoint(Quadrant quadrant, Point &pt) {
	// Start by checking a centroid point in the bounds
	if (!_bounds.isEmpty()) {
		pt = _bounds.getPoint(quadrant);
		if (checkPoint(pt, false, true))
			return true;
	}

	// Scan through all the area of the bounds to find a valid point
	for (; pt.y < _bounds.bottom; ++pt.y, pt.x = _bounds.left) {
		for (; pt.x < _bounds.right; ++pt.x) {
			if (checkPoint(pt, false, true))
				return true;
		}
	}

	pt = Point(0, 0);
	return false;
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
	// Get a reference to the game and screen managers
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
	_frameNumber = -1;

	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	if (_surface)
		_surface->setMovieFrame(frameNumber);

	makeDirty();
}

void CGameObject::processMoveRangeInfo() {
	for (CMovieRangeInfoList::iterator i = _movieRangeInfoList.begin(); i != _movieRangeInfoList.end(); ++i)
		(*i)->process(this);

	_movieRangeInfoList.destroyContents();
}

void CGameObject::makeDirty(const Rect &r) {
	CGameManager *gameManager = getGameManager();
	if (gameManager)
		gameManager->addDirtyRect(r);
}

void CGameObject::makeDirty() {
	makeDirty(_bounds);
}

bool CGameObject::isSoundActive(int handle) const {
	if (handle != 0 && handle != -1) {
		CGameManager *gameManager = getGameManager();
		if (gameManager)
			return gameManager->_sound.isActive(handle);
	}

	return false;
}

void CGameObject::playAmbientSound(const CString &resName, VolumeMode mode, bool initialMute, bool repeated,
		int handleIndex, Audio::Mixer::SoundType soundType) {
	if (handleIndex < 0 || handleIndex > 3)
		return;
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return;

	// Preload the file, and stop any existing sound using the given slot
	CSound &sound = gameManager->_sound;
	g_vm->_filesManager->preload(resName);
	if (_soundHandles[handleIndex] != -1) {
		sound.stopSound(_soundHandles[handleIndex]);
		_soundHandles[handleIndex] = -1;
	}

	// If no new name specified, then exit
	if (resName.empty())
		return;

	uint newVolume = sound._soundManager.getModeVolume(mode);
	uint volume = initialMute ? 0 : newVolume;

	CProximity prox;
	prox._channelVolume = volume;
	prox._repeated = repeated;
	prox._soundType = soundType;

	switch (handleIndex) {
	case 0:
		prox._channelMode = 6;
		break;
	case 1:
		prox._channelMode = 7;
		break;
	case 2:
		prox._channelMode = 8;
		break;
	case 3:
		prox._channelMode = 9;
		break;
	default:
		break;
	}

	_soundHandles[handleIndex] = sound.playSound(resName, prox);

	if (_soundHandles[handleIndex])
		sound.setVolume(_soundHandles[handleIndex], newVolume, 2);
}

void CGameObject::setSoundVolume(int handle, uint percent, uint seconds) {
	if (handle != 0 && handle != -1) {
		CGameManager *gameManager = getGameManager();
		if (gameManager)
			return gameManager->_sound.setVolume(handle, percent, seconds);
	}
}

void CGameObject::stopAmbientSound(bool transition, int handleIndex) {
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return;
	CSound &sound = gameManager->_sound;

	if (handleIndex == -1) {
		for (int idx = 0; idx < 4; ++idx) {
			if (_soundHandles[idx] != -1) {
				sound.setVolume(_soundHandles[idx], 0, transition ? 1 : 0);
				sound.setCanFree(_soundHandles[idx]);
				_soundHandles[idx] = -1;
			}
		}
	} else if (handleIndex >= 0 && handleIndex <= 2 && _soundHandles[handleIndex] != -1) {
		if (transition) {
			// Transitioning to silent over 1 second
			sound.setVolume(_soundHandles[handleIndex], 0, 1);
			sleep(1000);
		}

		sound.stopSound(_soundHandles[handleIndex]);
		_soundHandles[handleIndex] = -1;
	}
}

void CGameObject::setAmbientSoundVolume(VolumeMode mode, uint seconds, int handleIndex) {
	CGameManager *gameManager = getGameManager();
	if (!gameManager)
		return;
	CSound &sound = gameManager->_sound;

	if (handleIndex == -1) {
		// Iterate through calling the method for each handle
		for (int idx = 0; idx < 3; ++idx)
			setAmbientSoundVolume(mode, seconds, idx);
	} else if (handleIndex >= 0 && handleIndex <= 3 && _soundHandles[handleIndex] != -1) {
		uint newVolume = sound._soundManager.getModeVolume(mode);
		sound.setVolume(_soundHandles[handleIndex], newVolume, seconds);
	}
}

void CGameObject::stopSoundChannel(bool channel3) {
	getGameManager()->_sound.stopChannel(channel3 ? 3 : 0);
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

void CGameObject::petShow() {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		gameManager->_gameState._petActive = true;
		gameManager->_gameState.setMode(GSMODE_INTERACTIVE);
		gameManager->markAllDirty();
	}
}

void CGameObject::petHide() {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		gameManager->_gameState._petActive = false;
		gameManager->_gameState.setMode(GSMODE_INTERACTIVE);
		gameManager->markAllDirty();
	}
}

void CGameObject::petIncAreaLocks() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->incAreaLocks();
}

void CGameObject::petDecAreaLocks() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->decAreaLocks();
}

void CGameObject::petSetRemoteTarget() {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setRemoteTarget(this);
}

void CGameObject::playMovie(uint flags) {
	_frameNumber = -1;

	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	CGameObject *obj = (flags & MOVIE_NOTIFY_OBJECT) ? this : nullptr;
	if (_surface) {
		_surface->playMovie(flags, obj);
		if (flags & MOVIE_WAIT_FOR_FINISH)
			getGameManager()->_gameState.addMovie(_surface->_movie);
	}
}

void CGameObject::playMovie(int startFrame, int endFrame, uint flags) {
	_frameNumber = -1;

	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	CGameObject *obj = (flags & MOVIE_NOTIFY_OBJECT) ? this : nullptr;
	if (_surface) {
		_surface->playMovie(startFrame, endFrame, flags, obj);
		if (flags & MOVIE_WAIT_FOR_FINISH)
			getGameManager()->_gameState.addMovie(_surface->_movie);
	}
}


void CGameObject::playMovie(int startFrame, int endFrame, int initialFrame, uint flags) {
	_frameNumber = -1;

	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	CGameObject *obj = (flags & MOVIE_NOTIFY_OBJECT) ? this : nullptr;
	if (_surface) {
		_surface->playMovie(startFrame, endFrame, initialFrame, flags, obj);
		if (flags & MOVIE_WAIT_FOR_FINISH)
			getGameManager()->_gameState.addMovie(_surface->_movie);
	}
}

void CGameObject::playClip(const CString &name, uint flags) {
	debugC(DEBUG_DETAILED, kDebugScripts, "playClip - %s", name.c_str());

	_frameNumber = -1;
	CMovieClip *clip = _movieClips.findByName(name);
	if (clip)
		playMovie(clip->_startFrame, clip->_endFrame, flags);
}

void CGameObject::playClip(uint startFrame, uint endFrame) {
	debugC(DEBUG_DETAILED, kDebugScripts, "playClip - %d to %d", startFrame, endFrame);

	CMovieClip *clip = new CMovieClip("", startFrame, endFrame);
	CGameManager *gameManager = getGameManager();
	CRoomItem *room = gameManager->getRoom();

	gameManager->playClip(clip, room, room);
	delete clip;
}

void CGameObject::playRandomClip(const char *const *names, uint flags) {
	// Count size of array
	int count = 0;
	for (const char *const *p = names; *p; ++p)
		++count;

	// Play clip
	const char *name = names[g_vm->getRandomNumber(count - 1)];
	playClip(name, flags);
}

bool CGameObject::playCutscene(uint startFrame, uint endFrame) {
	if (!_surface) {
		if (!_resource.empty())
			loadResource(_resource);
		_resource.clear();
	}

	bool result = true;
	if (_surface && _surface->loadIfReady() && _surface->_movie) {
		disableMouse();
		result = _surface->_movie->playCutscene(_bounds, startFrame, endFrame);
		enableMouse();
	}

	return result;
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
	else if (_frameNumber > 0)
		// WORKAROUND: If an object has a pending frame to be set to,
		// but the movie hasn't yet been loaded, return that frame
		return _frameNumber;

	return _initialFrame;
}

bool CGameObject::surfaceHasFrame() const {
	return _surface ? _surface->hasFrame() : false;
}

void CGameObject::loadSound(const CString &name) {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		g_vm->_filesManager->preload(name);
		if (!name.empty())
			gameManager->_sound.loadSound(name);
	}
}

int CGameObject::playSound(const CString &name, uint volume, int balance, bool repeated) {
	CProximity prox;
	prox._channelVolume = volume;
	prox._balance = balance;
	prox._repeated = repeated;
	return playSound(name, prox);
}

int CGameObject::playSound(const CString &name, CProximity &prox) {
	if (prox._positioningMode == POSMODE_VECTOR) {
		// If the proximity doesn't have a position defined, default it to
		// the position of the view to which the game object belongs
		if (prox._posX == 0.0 && prox._posY == 0.0 && prox._posZ == 0.0)
			findView()->getPosition(prox._posX, prox._posY, prox._posZ);
	}

	CGameManager *gameManager = getGameManager();
	if (gameManager && !name.empty()) {
		g_vm->_filesManager->preload(name);

		return gameManager->_sound.playSound(name, prox);
	}

	return -1;
}

int CGameObject::queueSound(const CString &name, uint priorHandle, uint volume, int balance, bool repeated,
		Audio::Mixer::SoundType soundType) {
	CProximity prox;
	prox._balance = balance;
	prox._repeated = repeated;
	prox._channelVolume = volume;
	prox._priorSoundHandle = priorHandle;
	prox._soundType = soundType;

	return playSound(name, prox);
}

void CGameObject::stopSound(int handle, uint seconds) {
	if (handle != 0 && handle != -1) {
		CGameManager *gameManager = getGameManager();
		if (gameManager) {
			if (seconds) {
				gameManager->_sound.setVolume(handle, 0, seconds);
				gameManager->_sound.setCanFree(handle);
			} else {
				gameManager->_sound.stopSound(handle);
			}
		}
	}
}

int CGameObject::addTimer(int endVal, uint firstDuration, uint repeatDuration) {
	CTimeEventInfo *timer = new CTimeEventInfo(getTicksCount(), repeatDuration != 0,
		firstDuration, repeatDuration, this, endVal, CString());

	getGameManager()->addTimer(timer);
	return timer->_id;
}

int CGameObject::addTimer(uint firstDuration, uint repeatDuration) {
	CTimeEventInfo *timer = new CTimeEventInfo(getTicksCount(), repeatDuration != 0,
		firstDuration, repeatDuration, this, 0, CString());

	CGameManager *gameMan = getGameManager();
	if (gameMan)
		gameMan->addTimer(timer);
	return timer->_id;
}

void CGameObject::stopTimer(int id) {
	CGameManager *gameMan = getGameManager();
	if (gameMan)
		gameMan->stopTimer(id);
}

int CGameObject::startAnimTimer(const CString &action, uint firstDuration, uint repeatDuration) {
	CGameManager *gameMan = getGameManager();
	if (gameMan) {
		CTimeEventInfo *timer = new CTimeEventInfo(getTicksCount(), repeatDuration > 0,
			firstDuration, repeatDuration, this, 0, action);
		gameMan->addTimer(timer);
		return timer->_id;
	}

	return -1;
}

void CGameObject::stopAnimTimer(int id) {
	CGameManager *gameMan = getGameManager();
	if (gameMan)
		gameMan->stopTimer(id);
}

void CGameObject::gotoView(const CString &viewName, const CString &clipName) {
	CViewItem *newView = parseView(viewName);
	CGameManager *gameManager = getGameManager();
	CViewItem *oldView = gameManager->getView();
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
	return getRoot()->parseView(viewString);
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
	// Use an empty event target so that standard scene drawing won't happen
	Events &events = *g_vm->_events;
	CEventTarget nullTarget;
	events.addTarget(&nullTarget);
	events.sleep(milli);
	events.removeTarget();
}

Point CGameObject::getMousePos() const {
	return getGameManager()->_gameState.getMousePos();
}

bool CGameObject::compareViewNameTo(const CString &name) const {
	return !getViewFullName().compareToIgnoreCase(name);
}

bool CGameObject::compareRoomNameTo(const CString &name) {
	CRoomItem *room = getGameManager()->getRoom();
	return !room->getName().compareToIgnoreCase(name);
}

CString CGameObject::getRoomName() const {
	CRoomItem *room = getRoom();
	return room ? room->getName() : CString();
}

CString CGameObject::getRoomNodeName() const {
	CNodeItem *node = getNode();
	if (!node)
		return CString();

	CRoomItem *room = node->findRoom();

	return CString::format("%s.%s", room->getName().c_str(), node->getName().c_str());
}

CString CGameObject::getFullViewName() {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->getFullViewName() : CString();
}

CGameObject *CGameObject::getMailManFirstObject() const {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->getFirstObject() : nullptr;
}

CGameObject *CGameObject::getMailManNextObject(CGameObject *prior) const {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->getNextObject(prior) : nullptr;
}

CGameObject *CGameObject::findMailByFlags(RoomFlagsComparison compareType, uint roomFlags) {
	CMailMan *mailMan = getMailMan();
	if (!mailMan)
		return nullptr;

	for (CGameObject *obj = mailMan->getFirstObject(); obj;
			obj = mailMan->getNextObject(obj)) {
		if (compareRoomFlags(compareType, obj->_roomFlags, roomFlags))
			return obj;
	}

	return nullptr;
}

CGameObject *CGameObject::getNextMail(CGameObject *prior) {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->getNextObject(prior) : nullptr;
}

CGameObject *CGameObject::findRoomObject(const CString &name) const {
	return dynamic_cast<CGameObject *>(findRoom()->findByName(name));
}

CGameObject *CGameObject::findInRoom(const CString &name) {
	CRoomItem *room = getRoom();
	return room ? dynamic_cast<CGameObject *>(room->findByName(name)) : nullptr;
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
		go = dynamic_cast<CGameObject *>(getRoot()->findByName(name));
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
	addUnder(view);
}

void CGameObject::moveToView(const CString &name) {
	CViewItem *view = parseView(name);
	detach();
	addUnder(view);
}

void CGameObject::stateChangeSeason() {
	getGameManager()->_gameState.changeSeason();
}

Season CGameObject::stateGetSeason() const {
	return getGameManager()->_gameState._seasonNum;
}

void CGameObject::stateSetParrotMet() {
	getGameManager()->_gameState.setParrotMet(true);
}

bool CGameObject::stateGetParrotMet() const {
	return getGameManager()->_gameState.getParrotMet();
}

void CGameObject::incParrotResponse() {
	getGameManager()->_gameState.incParrotResponse();
}

int CGameObject::getParrotResponse() const {
	return getGameManager()->_gameState._parrotResponseIndex;
}

void CGameObject::quitGame() {
	getGameManager()->_gameState._quitGame = true;
}

void CGameObject::incTransitions() {
	getGameManager()->incTransitions();
}

void CGameObject::decTransitions() {
	getGameManager()->decTransitions();
}

void CGameObject::setMovieFrameRate(double rate) {
	if (_surface)
		_surface->setMovieFrameRate(rate);
}

void CGameObject::setText(const CString &str, int border, int borderRight) {
	if (!_text)
		_text = new CTextControl();
	_textBorder = border;
	_textBorderRight = borderRight;

	setTextBounds();
	_text->setText(str);
	CScreenManager *screenManager = getGameManager()->setScreenManager();
	_text->scrollToTop(screenManager);
}

void CGameObject::setTextHasBorders(bool hasBorders) {
	if (!_text)
		_text = new CTextControl();

	_text->setHasBorder(hasBorders);
}

void CGameObject::setTextBounds() {
	Rect rect = _bounds;
	rect.grow(_textBorder);
	rect.right -= _textBorderRight;

	_text->setBounds(rect);
	makeDirty();
}

void CGameObject::setTextColor(byte r, byte g, byte b) {
	if (!_text)
		_text = new CTextControl();

	_text->setColor(r, g, b);
}

void CGameObject::setTextFontNumber(int fontNumber) {
	if (!_text)
		_text = new CTextControl();

	_text->setFontNumber(fontNumber);
}

int CGameObject::getTextWidth() const {
	assert(_text);
	return _text->getTextWidth(CScreenManager::_screenManagerPtr);
}

CTextCursor *CGameObject::getTextCursor() const {
	return CScreenManager::_screenManagerPtr->_textCursor;
}

Movement CGameObject::getMovement() const {
	return CLinkItem::getMovementFromCursor(_cursorId);
}

void CGameObject::scrollTextUp() {
	if (_text)
		_text->scrollUp(CScreenManager::_screenManagerPtr);
	makeDirty();
}

void CGameObject::scrollTextDown() {
	if (_text)
		_text->scrollDown(CScreenManager::_screenManagerPtr);
	makeDirty();
}

void CGameObject::lockMouse() {
	CGameManager *gameMan = getGameManager();
	gameMan->lockInputHandler();

	if (CScreenManager::_screenManagerPtr->_mouseCursor)
		CScreenManager::_screenManagerPtr->_mouseCursor->incBusyCount();
}

void CGameObject::unlockMouse() {
	if (CScreenManager::_screenManagerPtr->_mouseCursor)
		CScreenManager::_screenManagerPtr->_mouseCursor->decBusyCount();

	CGameManager *gameMan = getGameManager();
	gameMan->unlockInputHandler();
}

void CGameObject::hideMouse() {
	CScreenManager::_screenManagerPtr->_mouseCursor->incHideCounter();
}

void CGameObject::showMouse() {
	CScreenManager::_screenManagerPtr->_mouseCursor->decHideCounter();
}

void CGameObject::disableMouse() {
	lockInputHandler();
	hideMouse();
}

void CGameObject::enableMouse() {
	unlockInputHandler();
	showMouse();
}

void CGameObject::mouseDisableControl() {
	CScreenManager::_screenManagerPtr->_mouseCursor->disableControl();
}

void CGameObject::mouseEnableControl() {
	CScreenManager::_screenManagerPtr->_mouseCursor->enableControl();
}

void CGameObject::mouseSetPosition(const Point &pt, double rate) {
	CScreenManager::_screenManagerPtr->_mouseCursor->setPosition(pt, rate);
}

void CGameObject::lockInputHandler() {
	getGameManager()->lockInputHandler();
}

void CGameObject::unlockInputHandler() {
	getGameManager()->unlockInputHandler();
}

void CGameObject::loadSurface() {
	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	if (_surface)
		_surface->loadIfReady();
}

bool CGameObject::changeView(const CString &viewName) {
	return getRoot()->changeView(viewName, "");
}

bool CGameObject::changeView(const CString &viewName, const CString &clipName) {
	return getRoot()->changeView(viewName, clipName);
}

void CGameObject::dragMove(const Point &pt) {
	if (_surface) {
		_bounds.setWidth(_surface->getWidth());
		_bounds.setHeight(_surface->getHeight());
	}

	setPosition(Point(pt.x - _bounds.width() / 2, pt.y - _bounds.height() / 2));
}

CGameObject *CGameObject::getDraggingObject() const {
	CTreeItem *item = getGameManager()->_dragItem;
	return dynamic_cast<CGameObject *>(item);
}

Point CGameObject::getControid() const {
	return Point(_bounds.left + _bounds.width() / 2,
		_bounds.top + _bounds.height() / 2);
}

bool CGameObject::clipExistsByStart(const CString &name, int startFrame) const {
	return _movieClips.existsByStart(name, startFrame);
}

bool CGameObject::clipExistsByEnd(const CString &name, int endFrame) const {
	return _movieClips.existsByEnd(name, endFrame);
}

void CGameObject::petClear() const {
	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->resetRemoteTarget();
}

CDontSaveFileItem *CGameObject::getDontSave() const {
	CProjectItem *project = getRoot();
	return project ? project->getDontSaveFileItem() : nullptr;
}

CPetControl *CGameObject::getPetControl() const {
	return dynamic_cast<CPetControl *>(getDontSaveChild(CPetControl::_type));
}

CMailMan *CGameObject::getMailMan() const {
	return dynamic_cast<CMailMan *>(getDontSaveChild(CMailMan::_type));
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

CRoomItem *CGameObject::getHiddenRoom() const {
	CProjectItem *root = getRoot();
	return root ? root->findHiddenRoom() : nullptr;
}

CRoomItem *CGameObject::locateRoom(const CString &name) const {
	if (name.empty())
		return nullptr;

	CProjectItem *project = getRoot();
	for (CRoomItem *room = project->findFirstRoom(); room; room = project->findNextRoom(room)) {
		if (!room->getName().compareToIgnoreCase(name))
			return room;
	}

	return nullptr;
}

CGameObject *CGameObject::getHiddenObject(const CString &name) const {
	CRoomItem *room = getHiddenRoom();
	return room ? dynamic_cast<CGameObject *>(findUnder(room, name)) : nullptr;
}

CTreeItem *CGameObject::findUnder(CTreeItem *parent, const CString &name) const {
	if (!parent)
		return nullptr;

	for (CTreeItem *item = parent->getFirstChild(); item; item = item->scan(parent)) {
		if (item->getName() == name)
			return item;
	}

	return nullptr;
}

CRoomItem *CGameObject::findRoomByName(const CString &name) {
	CProjectItem *project = getRoot();
	for (CRoomItem *room = project->findFirstRoom(); room; room = project->findNextRoom(room)) {
		if (!room->getName().compareToIgnoreCase(name))
			return room;
	}

	return nullptr;
}

CMusicRoom *CGameObject::getMusicRoom() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? &gameManager->_musicRoom : nullptr;
}

PassengerClass CGameObject::getPassengerClass() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->_gameState._passengerClass : THIRD_CLASS;
}

PassengerClass CGameObject::getPriorClass() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->_gameState._priorClass : THIRD_CLASS;
}

void CGameObject::setPassengerClass(PassengerClass newClass) {
	if (newClass >= 1 && newClass <= 4) {
		// Change the passenger class
		CGameManager *gameMan = getGameManager();
		gameMan->_gameState._priorClass = gameMan->_gameState._passengerClass;
		gameMan->_gameState._passengerClass = newClass;

		// Setup the PET again, so the new class's PET background can take effect
		CPetControl *petControl = getPetControl();
		if (petControl)
			petControl->reset();
	}
}

void CGameObject::createCredits() {
	_credits = new CCreditText();
	CScreenManager *screenManager = getGameManager()->setScreenManager();
	_credits->load(this, screenManager, _bounds);
}

void CGameObject::setToggleColor(byte r, byte g, byte b) {
	makeDirty();
	_toggleR = r;
	_toggleG = g;
	_toggleB = b;
}

void CGameObject::movieSetPlaying(bool flag) {
	if (!_surface && !_resource.empty()) {
		loadResource(_resource);
		_resource.clear();
	}

	if (_surface && _surface->_movie)
		_surface->_movie->setPlaying(flag);
}

void CGameObject::movieEvent(int frameNumber) {
	if (_surface)
		_surface->addMovieEvent(frameNumber, this);
}

void CGameObject::movieEvent() {
	if (_surface)
		_surface->addMovieEvent(-1, this);
}

int CGameObject::getClipDuration(const CString &name, int frameRate) const {
	CMovieClip *clip = _movieClips.findByName(name);
	return clip ? (clip->_endFrame - clip->_startFrame) * 1000 / frameRate : 0;
}

uint32 CGameObject::getTicksCount() {
	return g_vm->_events->getTicksCount();
}

Common::SeekableReadStream *CGameObject::getResource(const CString &name) {
	return g_vm->_filesManager->getResource(name);
}

bool CGameObject::compareRoomFlags(RoomFlagsComparison compareType, uint flags1, uint flags2) {
	switch (compareType) {
	case RFC_LOCATION:
		return CRoomFlags::compareLocation(flags1, flags2);

	case RFC_CLASS_ELEVATOR:
		return CRoomFlags::compareClassElevator(flags1, flags2);

	case RFC_TITANIA:
		return CRoomFlags::isTitania(flags1, flags2);

	default:
		return false;
	}
}

void CGameObject::stateSetSoundMakerAllowed(bool flag) {
	getGameManager()->_gameState._soundMakerAllowed = flag;
}

void CGameObject::addMail(uint destRoomFlags) {
	CMailMan *mailMan = getMailMan();
	if (mailMan) {
		makeDirty();
		mailMan->addMail(this, destRoomFlags);
	}
}

void CGameObject::setMailDest(uint roomFlags) {
	CMailMan *mailMan = getMailMan();
	if (mailMan) {
		makeDirty();
		mailMan->setMailDest(this, roomFlags);
	}
}

bool CGameObject::mailExists(uint roomFlags) const {
	return findMail(roomFlags) != nullptr;
}

CGameObject *CGameObject::findMail(uint roomFlags) const {
	CMailMan *mailMan = getMailMan();
	return mailMan ? mailMan->findMail(roomFlags) : nullptr;
}

void CGameObject::sendMail(uint currRoomFlags, uint newRoomFlags) {
	CMailMan *mailMan = getMailMan();
	if (mailMan)
		mailMan->sendMail(currRoomFlags, newRoomFlags);
}

void CGameObject::resetMail() {
	CMailMan *mailMan = getMailMan();
	if (mailMan)
		mailMan->resetValue();
}

int CGameObject::getRandomNumber(int max, int *oldVal) {
	if (oldVal) {
		int startingVal = *oldVal;
		while (*oldVal == startingVal && max > 0)
			*oldVal = g_vm->getRandomNumber(max);

		return *oldVal;
	} else {
		return g_vm->getRandomNumber(max);
	}
}

bool CGameObject::isBotDisallowedLocation() {
	// 1) At the Embarkation SuccUBus
	// 2) in front of the Deskbot's desk
	// 3) When in the Gondola
	// 4) At the top of the well
	CString fullName = getFullViewName();
	return fullName == "EmbLobby.Node 2.W" || fullName == "EmbLobby.Node 4.E" ||
		fullName == "TopOfWell.Node 29.N" || fullName == "TopOfWell.Node 21.S";
}

/*------------------------------------------------------------------------*/

CRoomItem *CGameObject::getRoom() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->getRoom() : nullptr;
}

CNodeItem *CGameObject::getNode() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->getNode() : nullptr;
}

CViewItem *CGameObject::getView() const {
	CGameManager *gameManager = getGameManager();
	return gameManager ? gameManager->getView() : nullptr;
}

/*------------------------------------------------------------------------*/

void CGameObject::petAddToCarryParcel(CGameObject *obj) {
	CPetControl *pet = getPetControl();
	if (pet) {
		CGameObject *parcel = pet->getHiddenObject("CarryParcel");
		if (parcel)
			parcel->moveUnder(obj);
	}
}

void CGameObject::petAddToInventory() {
	assert(dynamic_cast<CCarry *>(this));

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

	CGameObject *item = dynamic_cast<CGameObject *>(pet->getLastChild());
	if (item)
		item->detach();

	pet->moveToHiddenRoom(obj);
	pet->removeFromInventory(item, false, false);

	return item;
}

bool CGameObject::petCheckNode(const CString &name) {
	CPetControl *pet = getPetControl();
	return pet ? pet->checkNode(name) : false;
}

bool CGameObject::petDismissBot(const CString &name) {
	CPetControl *pet = getPetControl();
	return pet ? pet->dismissBot(name) : false;
}

bool CGameObject::petDoorOrBellbotPresent() const {
	CPetControl *pet = getPetControl();
	return pet ? pet->isDoorOrBellbotPresent() : false;
}

void CGameObject::petDisplayMessage(int unused, StringId stringId) {
	petDisplayMessage(stringId);
}

void CGameObject::petDisplayMessage(StringId stringId, int param) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->displayMessage(stringId, param);
}

void CGameObject::petDisplayMessage(int unused, const CString &msg) {
	petDisplayMessage(msg);
}

void CGameObject::petDisplayMessage(const CString &msg, int param) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->displayMessage(msg, param);
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

void CGameObject::petReassignRoom(PassengerClass passClassNum) {
	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->reassignRoom(passClassNum);
}

void CGameObject::petSetArea(PetArea newArea) const {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setArea(newArea);
}

void CGameObject::petSetRoomsWellEntry(int entryNum) {
	CPetControl *petControl = getPetControl();
	if (petControl)
		petControl->setRoomsWellEntry(entryNum);
}

int CGameObject::petGetRoomsWellEntry() const {
	CPetControl *petControl = getPetControl();
	return petControl ? petControl->getRoomsWellEntry() : 0;
}

void CGameObject::petSetRoomsElevatorBroken(bool flag) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setRoomsElevatorBroken(flag);
}

void CGameObject::petOnSummonBot(const CString &name, int val) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->onSummonBot(name, val);
}

void CGameObject::petUnlockInput() {
	getPetControl()->decInputLocks();
}

/*------------------------------------------------------------------------*/

CStarControl *CGameObject::getStarControl() const {
	CStarControl *starControl = dynamic_cast<CStarControl *>(getDontSaveChild(CStarControl::_type));
	if (!starControl) {
		CViewItem *view = getGameManager()->getView();
		if (view)
			starControl = dynamic_cast<CStarControl *>(view->findChildInstanceOf(CStarControl::_type));
	}

	return starControl;
}

void CGameObject::starFn(StarControlAction action) {
	CStarControl *starControl = getStarControl();
	if (starControl)
		starControl->doAction(action);
}

bool CGameObject::starIsSolved() const {
	CStarControl *starControl = getStarControl();
	return starControl ? starControl->isSolved() : false;
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

void CGameObject::setTalking(CTrueTalkNPC *npc, bool viewFlag, CViewItem *view) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->setActiveNPC(npc);

	if (viewFlag)
		npc->setView(view);

	if (pet)
		pet->refreshNPC();
}

void CGameObject::talkSetDialRegion(const CString &name, int dialNum, int regionNum) {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		CTrueTalkManager *talkManager = gameManager->getTalkManager();
		if (talkManager) {
			TTnpcScript *npcScript = talkManager->getTalker(name);
			if (npcScript)
				npcScript->setDialRegion(dialNum, regionNum);
		}
	}
}

int CGameObject::talkGetDialRegion(const CString &name, int dialNum) {
	CGameManager *gameManager = getGameManager();
	if (gameManager) {
		CTrueTalkManager *talkManager = gameManager->getTalkManager();
		if (talkManager) {
			TTnpcScript *npcScript = talkManager->getTalker(name);
			if (npcScript)
				return npcScript->getDialRegion(dialNum);
		}
	}

	return 0;
}

/*------------------------------------------------------------------------*/

uint CGameObject::getNodeChangedCtr() const {
	return getGameManager()->_gameState.getNodeChangedCtr();
}

uint CGameObject::getNodeEnterTicks() const {
	return getGameManager()->_gameState.getNodeEnterTicks();
}


} // End of namespace Titanic
