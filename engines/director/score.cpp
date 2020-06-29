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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"

#include "engines/util.h"
#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/surface.h"

#ifdef USE_PNG
#include "image/png.h"
#endif

#include "director/director.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/frame.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/stxt.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

Channel::Channel(Sprite *sp) {
	_sprite = sp;
	_currentPoint = sp->_startPoint;
	_delta = Common::Point(0, 0);
	_constraint = 0;

	_visible = true;

	if (_sprite && _sprite->_castType != kCastTypeNull) {
		_sprite->updateCast();
	}
}

Graphics::ManagedSurface *Channel::getSurface() {
	if (_sprite->_cast && _sprite->_cast->_widget) {
		return  _sprite->_cast->_widget->getSurface();
	} else {
		return nullptr;
	}
}

Common::Rect Channel::getBbox() {
	Common::Rect bbox = _sprite->getDims();
	bbox.moveTo(getPosition());

	return bbox;
}

void Channel::updateLocation() {
	_currentPoint += _delta;
	_delta = Common::Point(0, 0);

	if (_sprite->_cast && _sprite->_cast->_widget) {
		Common::Point p(getPosition());
		_sprite->_cast->_widget->_dims.moveTo(p.x, p.y);
	}
}

void Channel::addDelta(Common::Point pos) {
	if (_constraint > g_director->getCurrentScore()->_channels.size() - 1) {
		warning("Channel::addDelta: Received out-of-bounds constraint: %d", _constraint);
		_constraint = 0;
	} else if (_sprite->_moveable && _constraint > 0) {
		Common::Rect constraintBbox = g_director->getCurrentScore()->_channels[_constraint]->getBbox();

		Common::Rect currentBbox = getBbox();
		currentBbox.translate(pos.x, pos.y);

		// TODO: Snap to the nearest point on the rectangle.
		if (constraintBbox.findIntersectingRect(currentBbox) != currentBbox)
			return;
	}

	_delta += pos;
}

Common::Point Channel::getPosition() {
	Common::Point res = _currentPoint;

	if (_sprite->_castType == kCastBitmap && _sprite->_cast) {
		BitmapCast *bc = (BitmapCast *)(_sprite->_cast);

		res += Common::Point(bc->_initialRect.left - bc->_regX,
												 bc->_initialRect.top - bc->_regY);
	}

	return res;
}

void Channel::resetPosition() {
	_delta = _sprite->_startPoint;
}

MacShape *Channel::getShape() {
	MacShape *shape = new MacShape();

	if (_sprite->_castType != kCastShape)
		return nullptr;

	shape->ink = _sprite->_ink;
	shape->spriteType = _sprite->_spriteType;
	shape->foreColor = _sprite->_foreColor;
	shape->backColor = _sprite->_backColor;
	shape->lineSize = _sprite->_thickness & 0x3;

	if (g_director->getVersion() >= 3 && shape->spriteType == kCastMemberSprite) {
		if (!_sprite->_cast) {
			warning("Channel::getShape(): kCastMemberSprite has no cast defined");
			return nullptr;
		}
		switch (_sprite->_cast->_type) {
		case kCastShape:
			{
				ShapeCast *sc = (ShapeCast *)_sprite->_cast;
				switch (sc->_shapeType) {
				case kShapeRectangle:
					shape->spriteType = sc->_fillType ? kRectangleSprite : kOutlinedRectangleSprite;
					break;
				case kShapeRoundRect:
					shape->spriteType = sc->_fillType ? kRoundedRectangleSprite : kOutlinedRoundedRectangleSprite;
					break;
				case kShapeOval:
					shape->spriteType = sc->_fillType ? kOvalSprite : kOutlinedOvalSprite;
					break;
				case kShapeLine:
					shape->spriteType = sc->_lineDirection == 6 ? kLineBottomTopSprite : kLineTopBottomSprite;
					break;
				default:
					break;
				}
				if (g_director->getVersion() > 3) {
					shape->foreColor = sc->_fgCol;
					shape->backColor = sc->_bgCol;
					shape->lineSize = sc->_lineThickness;
					shape->ink = sc->_ink;
				}
			}
			break;
		default:
			warning("Channel::getShape(): Unhandled cast type: %d", _sprite->_cast->_type);
			break;
		}
	}

	// for outlined shapes, line thickness of 1 means invisible.
	shape->lineSize -= 1;

	return shape;
}

Score::Score(DirectorEngine *vm) {
	_vm = vm;
	_lingo = _vm->getLingo();
	_lingoArchive = kArchMain;
	_soundManager = _vm->getSoundManager();
	_currentMouseDownSpriteId = 0;
	_currentClickOnSpriteId = 0;
	_lastEventTime = _vm->getMacTicks();
	_lastKeyTime = _lastEventTime;
	_lastClickTime = _lastEventTime;
	_lastRollTime = _lastEventTime;
	_lastTimerReset = _lastEventTime;
	_puppetTempo = 0x00;

	// FIXME: TODO: Check whether the original truely does it
	if (_vm->getVersion() <= 3) {
		_lingo->executeScript(kMovieScript, 0);
	}
	_movieScriptCount = 0;
	_labels = nullptr;
	_font = nullptr;

	_versionMinor = _versionMajor = 0;
	_currentFrameRate = 20;
	_castArrayStart = _castArrayEnd = 0;
	_currentFrame = 0;
	_nextFrame = 0;
	_currentLabel = 0;
	_nextFrameTime = 0;
	_flags = 0;
	_stopPlay = false;

	_castIDoffset = 0;

	_movieArchive = nullptr;

	_loadedStxts = nullptr;
	_loadedCast = nullptr;

	_numChannelsDisplayed = 0;

	_framesRan = 0; // used by kDebugFewFramesOnly and kDebugScreenshot
}

Score::~Score() {
	for (uint i = 0; i < _frames.size(); i++)
		delete _frames[i];

	for (uint i = 0; i < _channels.size(); i++)
		delete _channels[i];

	if (_loadedStxts)
		for (Common::HashMap<int, const Stxt *>::iterator it = _loadedStxts->begin(); it != _loadedStxts->end(); ++it)
			delete it->_value;

	if (_movieArchive) {
		_movieArchive->close();
		delete _movieArchive;
		_movieArchive = nullptr;
	}

	if (_loadedCast)
		for (Common::HashMap<int, Cast *>::iterator it = _loadedCast->begin(); it != _loadedCast->end(); ++it)
			delete it->_value;

	if (_labels)
		for (Common::SortedArray<Label *>::iterator it = _labels->begin(); it != _labels->end(); ++it)
			delete *it;

	delete _font;
	delete _labels;
	delete _loadedStxts;
	delete _loadedCast;
}

Common::Rect Score::getCastMemberInitialRect(int castId) {
	Cast *cast = _loadedCast->getVal(castId);

	if (!cast) {
		warning("Score::getCastMemberInitialRect(%d): empty cast", castId);
		return Common::Rect(0, 0);
	}

	return cast->_initialRect;
}

void Score::setCastMemberModified(int castId) {
	Cast *cast = _loadedCast->getVal(castId);

	if (!cast) {
		warning("Score::setCastMemberModified(%d): empty cast", castId);
		return;
	}

	cast->_modified = 1;
}

bool Score::processImmediateFrameScript(Common::String s, int id) {
	s.trim();

	// In D2/D3 this specifies immediately the sprite/field properties
	if (!s.compareToIgnoreCase("moveableSprite") || !s.compareToIgnoreCase("editableText")) {
		_immediateActions[id] = true;
	}

	return false;
}

void Score::setStartToLabel(Common::String label) {
	if (!_labels) {
		warning("setStartToLabel: No labels set");
		return;
	}

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name.equalsIgnoreCase(label)) {
			_nextFrame = (*i)->number;
			return;
		}
	}
	warning("Label %s not found", label.c_str());
}

void Score::gotoLoop() {
	// This command has the playback head contonuously return to the first marker to to the left and then loop back.
	// If no marker are to the left of the playback head, the playback head continues to the right.
	if (_labels == NULL) {
		_nextFrame = 1;
		return;
	} else {
		_nextFrame = _currentLabel;
	}

	_vm->_skipFrameAdvance = true;
}

int Score::getCurrentLabelNumber() {
	Common::SortedArray<Label *>::iterator i;

	if (!_labels)
		return 0;

	int frame = 0;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number <= _currentFrame)
			frame = (*i)->number;
	}

	return frame;
}

void Score::gotoNext() {
	// we can just try to use the current frame and get the next label
	_nextFrame = getNextLabelNumber(_currentFrame);
}

void Score::gotoPrevious() {
	// we actually need the frame of the label prior to the most recent label.
	_nextFrame = getPreviousLabelNumber(getCurrentLabelNumber());
}

int Score::getNextLabelNumber(int referenceFrame) {
	if (_labels == NULL || _labels->size() == 0)
		return 0;

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number >= referenceFrame) {
			int n = (*i)->number;
			++i;
			if (i != _labels->end()) {
				// return to the first marker to to the right
				return (*i)->number;
			} else {
				// if no markers are to the right of the playback head,
				// the playback head goes to the first marker to the left
				return n;
			}
		}
	}

	// If there are not markers to the left,
	// the playback head goes to frame 1, (Director frame array start from 1, engine from 0)
	return 0;
}

int Score::getPreviousLabelNumber(int referenceFrame) {
	if (_labels == NULL || _labels->size() == 0)
		return 0;

	// One label
	if (_labels->begin() == _labels->end())
		return (*_labels->begin())->number;

	Common::SortedArray<Label *>::iterator previous = _labels->begin();
	Common::SortedArray<Label *>::iterator i;

	for (i = (previous + 1); i != _labels->end(); ++i, ++previous) {
		if ((*i)->number >= referenceFrame)
			return (*previous)->number;
	}

	return 0;
}

Common::String Score::getString(Common::String str) {
	if (str.size() == 0) {
		return str;
	}

	uint8 f = static_cast<uint8>(str.firstChar());

	if (f == 0) {
		return "";
	}

	//TODO: check if all versions need to cut off the first character.
	if (_vm->getVersion() > 3) {
		str.deleteChar(0);
	}

	if (str.lastChar() == '\x00') {
		str.deleteLastChar();
	}

	return str;
}

void Score::startLoop() {

	debugC(1, kDebugImages, "Score dims: %dx%d", _movieRect.width(), _movieRect.height());

	initGraphics(_movieRect.width(), _movieRect.height());

	_currentFrame = 0;
	_stopPlay = false;
	_nextFrameTime = 0;

	if (_frames.size() <= 1) {	// We added one empty sprite
		warning("Score::startLoop(): Movie has no frames");
		_stopPlay = true;
	}

	// All frames in the same movie have the same number of channels
	if (!_stopPlay)
		for (uint i = 0; i < _frames[1]->_sprites.size(); i++)
			_channels.push_back(new Channel(_frames[1]->_sprites[i]));

	if (_vm->getVersion() >= 3)
		_lingo->processEvent(kEventStartMovie);

	while (!_stopPlay) {
		if (_currentFrame >= _frames.size()) {
			if (debugChannelSet(-1, kDebugNoLoop))
				break;

			_currentFrame = 0;
		}

		update();

		if (_currentFrame < _frames.size())
			_vm->processEvents();

		if (debugChannelSet(-1, kDebugFewFramesOnly) || debugChannelSet(-1, kDebugScreenshot)) {
			warning("Score::startLoop(): ran frame %0d", _framesRan);
			_framesRan++;
		}

		if (debugChannelSet(-1, kDebugFewFramesOnly) && _framesRan > 9) {
			warning("Score::startLoop(): exiting due to debug few frames only");
			break;
		}

		if (debugChannelSet(-1, kDebugScreenshot))
			screenShot();
	}

	if (_vm->getVersion() >= 3)
		_lingo->processEvent(kEventStopMovie);
	_lingo->executePerFrameHook(-1, 0);
}

void Score::update() {
	if (g_system->getMillis() < _nextFrameTime && !debugChannelSet(-1, kDebugFast)) {
		_vm->_wm->renderZoomBox(true);

		if (!_vm->_newMovieStarted)
			_vm->_wm->draw();

		return;
	}

	// For previous frame
	if (_currentFrame > 0 && !_vm->_playbackPaused) {
		// When Lingo::func_goto* is called, _nextFrame is set
		// and _skipFrameAdvance is set to true.
		// However, the exitFrame event can overwrite the value
		// for _nextFrame before it can be used.
		// Because we still want to call exitFrame, we check if
		// a goto call has been made and if so, cache the value
		// of _nextFrame so it doesn't get wiped.
		if (_vm->_skipFrameAdvance) {
			uint16 nextFrameCache = _nextFrame;
			if (_vm->getVersion() >= 4)
				_lingo->processEvent(kEventExitFrame);
			_nextFrame = nextFrameCache;
		} else {
			if (_vm->getVersion() >= 4)
				_lingo->processEvent(kEventExitFrame);
		}

		// If there is a transition, the perFrameHook is called
		// after each transition subframe instead.
		if (_frames[_currentFrame]->_transType == 0) {
			_lingo->executePerFrameHook(_currentFrame, 0);
		}
	}

	if (!_vm->_playbackPaused) {
		if (_nextFrame)
			_currentFrame = _nextFrame;
		else
			_currentFrame++;
	}

	_nextFrame = 0;

	_vm->_skipFrameAdvance = false;

	if (_currentFrame >= _frames.size())
		return;

	Common::SortedArray<Label *>::iterator i;
	if (_labels != NULL) {
		for (i = _labels->begin(); i != _labels->end(); ++i) {
			if ((*i)->number == _currentFrame) {
				_currentLabel = _currentFrame;
			}
		}
	}

	debugC(1, kDebugImages, "******************************  Current frame: %d", _currentFrame);

	_lingo->executeImmediateScripts(_frames[_currentFrame]);

	if (_vm->getVersion() >= 6) {
		// _lingo->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		// _lingo->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	// Stage is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell, p.100)
	renderFrame(_currentFrame);
	_vm->_newMovieStarted = false;

	// Enter and exit from previous frame
	if (!_vm->_playbackPaused) {
		_lingo->processEvent(kEventEnterFrame); // Triggers the frame script in D2-3, explicit enterFrame handlers in D4+
		if (_vm->getVersion() == 3) {
			// Movie version of enterFrame, for D3 only. The Lingo Dictionary claims
			// "This handler executes before anything else when the playback head moves."
			// but this is incorrect. The frame script is executed first.
			_lingo->processEvent(kEventStepMovie);
		}
	}
	// TODO Director 6 - another order

	byte tempo = _puppetTempo ? _puppetTempo : _frames[_currentFrame]->_tempo;

	if (tempo) {
		if (tempo > 161) {
			// Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;

			return;
		} else if (tempo <= 60) {
			// FPS
			_nextFrameTime = g_system->getMillis() + (float)tempo / 60 * 1000;
			_currentFrameRate = tempo;
		} else if (tempo >= 136) {
			// TODO Wait for channel tempo - 135
			warning("STUB: tempo >= 136");
		} else if (tempo == 128) {
			_vm->waitForClick();
		} else if (tempo == 135) {
			// Wait for sound channel 1
			while (_soundManager->isChannelActive(1)) {
				_vm->processEvents();
			}
		} else if (tempo == 134) {
			// Wait for sound channel 2
			while (_soundManager->isChannelActive(2)) {
				_vm->processEvents();
			}
		}
	}

	_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;

	if (debugChannelSet(-1, kDebugSlow))
		_nextFrameTime += 1000;
}

void Score::renderFrame(uint16 frameId, RenderMode mode) {
	Frame *currentFrame = _frames[frameId];

	if (currentFrame->_transType != 0 && mode != kRenderUpdateStageOnly) {
		// TODO Handle changing area case
		g_director->getStage()->playTransition(currentFrame->_transDuration, currentFrame->_transArea, currentFrame->_transChunkSize, currentFrame->_transType, frameId);
	} else {
		renderSprites(frameId, mode);
	}

	_vm->_wm->renderZoomBox();
	g_director->getStage()->render();
	_vm->_wm->draw();

	if (currentFrame->_sound1 != 0 || currentFrame->_sound2 != 0)
		playSoundChannel(frameId);
}

void Score::renderSprites(uint16 frameId, RenderMode mode) {
	if (_vm->_newMovieStarted) {
		// g_director->getStage()->reset();
		mode = kRenderForceUpdate;
	}

	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		Sprite *currentSprite = channel->_sprite;
		Sprite *nextSprite = _frames[frameId]->_sprites[i];

		// A sprite needs to be updated if one of the following happens:
		// - The dimensions/bounding box of the sprite has changed (_dirty flag set)
		// - The cast member ID of the sprite has changed (_dirty flag set)
		// - The sprite slot from the current frame is different (cast member ID or bounding box) from the cached sprite slot
		// (maybe we have to compare all the sprite attributes, not just these two?)
		bool needsUpdate = currentSprite->isDirty() ||
			currentSprite->_castId != nextSprite->_castId ||
			channel->_delta != Common::Point(0, 0) ||
			currentSprite->getDims() != nextSprite->getDims() ||
			currentSprite->_ink != nextSprite->_ink ||
			(channel->_currentPoint != nextSprite->_startPoint &&
			 !currentSprite->_puppet && !currentSprite->_moveable);

		if ((needsUpdate || mode == kRenderForceUpdate) && !currentSprite->_trails)
			g_director->getStage()->addDirtyRect(channel->getBbox());

		currentSprite->setClean();

		if (!currentSprite->_puppet) {
			channel->_sprite = nextSprite;
			channel->_sprite->updateCast();

			// Sprites marked moveable are constrained to the same bounding box until
			// the moveable is disabled
			if (!channel->_sprite->_moveable)
				channel->_currentPoint = channel->_sprite->_startPoint;
		}

		channel->updateLocation();

		if (channel->_sprite->_cast && channel->_sprite->_cast->_widget) {
			channel->_sprite->_cast->_widget->_priority = i;
			channel->_sprite->_cast->_widget->draw();
			channel->_sprite->_cast->_widget->_contentIsDirty = false;
		}

		if (needsUpdate || mode == kRenderForceUpdate)
			g_director->getStage()->addDirtyRect(channel->getBbox());
	}
}

void Score::screenShot() {
	Graphics::Surface rawSurface = g_director->getStage()->getSurface()->rawSurface();
	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::Surface *newSurface = rawSurface.convertTo(requiredFormat_4byte, _vm->getPalette());
	Common::String currentPath = _vm->getCurrentPath().c_str();
	Common::replace(currentPath, "/", "-"); // exclude '/' from screenshot filename prefix
	Common::String prefix = Common::String::format("%s%s", currentPath.c_str(), _macName.c_str());
	Common::String filename = dumpScriptName(prefix.c_str(), kMovieScript, _framesRan, "png");

	Common::DumpFile screenshotFile;
	if (screenshotFile.open(filename)) {
#ifdef USE_PNG
		Image::writePNG(screenshotFile, *newSurface);
#else
		warning("Screenshot requested, but PNG support is not compiled in");
#endif
	}

	newSurface->free();
}

Cast *Score::getCastMember(int castId) {
	Cast *result = nullptr;

	if (_loadedCast && _loadedCast->contains(castId)) {
		result = _loadedCast->getVal(castId);
	}
	return result;
}

const Stxt *Score::getStxt(int castId) {
	const Stxt *result = nullptr;

	if (_loadedStxts->contains(castId)) {
		result = _loadedStxts->getVal(castId);
	}
	return result;
}

uint16 Score::getSpriteIDFromPos(Common::Point pos, bool onlyActive) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->getBbox().contains(pos) && (!onlyActive || _channels[i]->_sprite->isActive()))
			return i;

	return 0;
}

bool Score::checkSpriteIntersection(uint16 spriteId, Common::Point pos) {
	if (_channels[spriteId]->getBbox().contains(pos))
		return true;

	return false;
}

Common::List<Channel *> Score::getSpriteIntersections(const Common::Rect &r) {
	Common::List<Channel *>intersections;

	for (uint i = 0; i < _channels.size(); i++)
		if (!r.findIntersectingRect(_channels[i]->getBbox()).isEmpty())
			intersections.push_back(_channels[i]);

	return intersections;
}

Sprite *Score::getSpriteById(uint16 id) {
	Channel *channel = getChannelById(id);

	if (channel) {
		return channel->_sprite;
	} else {
		warning("Score::getSpriteById(): sprite on frame %d with id %d not found", _currentFrame, id);
		return nullptr;
	}
}

Channel *Score::getChannelById(uint16 id) {
	if (id >= _channels.size()) {
		warning("Score::getChannelById(%d): out of bounds", id);
		return nullptr;
	}

	return _channels[id];
}

void Score::playSoundChannel(uint16 frameId) {
	Frame *frame = _frames[frameId];

	debugC(5, kDebugLoading, "playSoundChannel(): Sound1 %d Sound2 %d", frame->_sound1, frame->_sound2);
	DirectorSound *sound = _vm->getSoundManager();
	sound->playCastMember(frame->_sound1, 1, false);
	sound->playCastMember(frame->_sound2, 2, false);
}

} // End of namespace Director
