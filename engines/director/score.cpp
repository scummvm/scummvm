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

Score::Score(DirectorEngine *vm) {
	_vm = vm;
	_surface = nullptr;
	_maskSurface = nullptr;
	_backSurface = nullptr;
	_backSurface2 = nullptr;
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

	_window = nullptr;

	_stageColor = 0;
}

Score::~Score() {
	if (_maskSurface && _maskSurface->w)
		_maskSurface->free();

	if (_backSurface && _backSurface->w)
		_backSurface->free();

	if (_backSurface2 && _backSurface2->w)
		_backSurface2->free();

	delete _backSurface;
	delete _backSurface2;
	delete _maskSurface;

	if (_window)
		_vm->_wm->removeWindow(_window);

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

	_surface = _window->getWindowSurface();
	_maskSurface = new Graphics::ManagedSurface;
	_backSurface = new Graphics::ManagedSurface;
	_backSurface2 = new Graphics::ManagedSurface;

	_maskSurface->create(_movieRect.width(), _movieRect.height());
	_backSurface->create(_movieRect.width(), _movieRect.height());
	_backSurface2->create(_movieRect.width(), _movieRect.height());

	if (_vm->_backSurface.w > 0) {
		// Persist screen between the movies
		// TODO: this is a workaround until the rendering pipeline is reworked

		_backSurface2->copyFrom(g_director->_backSurface);
		_surface->copyFrom(g_director->_backSurface);

		_vm->_backSurface.free();
	}

	_vm->_backSurface.create(_movieRect.width(), _movieRect.height());

	_vm->_wm->setScreen(_surface);

	_surface->clear(_stageColor);

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

	_lingo->processEvent(kEventStartMovie);

	_maskSurface->clear(1);
	while (!_stopPlay) {
		if (_currentFrame >= _frames.size()) {
			if (debugChannelSet(-1, kDebugNoLoop))
				break;

			_currentFrame = 0;
		}

		update();
		_maskSurface->clear(0);

		if (_currentFrame < _frames.size())
			_vm->processEvents();

		if (debugChannelSet(-1, kDebugFewFramesOnly) || debugChannelSet(-1, kDebugScreenshot))
			_framesRan++;

		if (debugChannelSet(-1, kDebugFewFramesOnly) && _framesRan > 9) {
			warning("Score::startLoop(): exiting due to debug few frames only");
			break;
		}

		if (debugChannelSet(-1, kDebugScreenshot))
			screenShot();
	}

	_lingo->processEvent(kEventStopMovie);
}

void Score::update() {
	if (g_system->getMillis() < _nextFrameTime && !debugChannelSet(-1, kDebugFast)) {
		_vm->_wm->renderZoomBox(true);

		if (!_vm->_newMovieStarted)
			_vm->_wm->draw();

		return;
	}

	// For previous frame
	if (_currentFrame > 0) {
		// TODO: Confirm when the perFrameHook is called if
		// there's no transition.
		// If there is a transition, the perFrameHook is called
		// after each transition subframe instead.
		if (_frames[_currentFrame]->_transType == 0) {
			_lingo->executePerFrameHook(_currentFrame, 0);
		}

		// When Lingo::func_goto* is called, _nextFrame is set
		// and _skipFrameAdvance is set to true.
		// However, the exitFrame event can overwrite the value
		// for _nextFrame before it can be used.
		// Because we still want to call exitFrame, we check if
		// a goto call has been made and if so, cache the value
		// of _nextFrame so it doesn't get wiped.
		if (_vm->_skipFrameAdvance) {
			uint16 nextFrameCache = _nextFrame;
			_lingo->processEvent(kEventExitFrame);
			_nextFrame = nextFrameCache;
		} else {
			_lingo->processEvent(kEventExitFrame);
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

	if (_frames[_currentFrame]->_transType != 0 && !_vm->_newMovieStarted)	// Store screen, so we could draw a nice transition
		_backSurface2->copyFrom(*_surface);

	_vm->_newMovieStarted = false;

	_lingo->executeImmediateScripts(_frames[_currentFrame]);

	if (_vm->getVersion() >= 6) {
		// _lingo->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		// _lingo->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	renderFrame(_currentFrame);
	// Stage is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell, p.100)

	// Enter and exit from previous frame (Director 4)
	_lingo->processEvent(kEventEnterFrame);
	_lingo->processEvent(kEventNone);
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

void Score::renderFrame(uint16 frameId, bool forceUpdate, bool updateStageOnly) {

	Frame *currentFrame = _frames[frameId];

	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		Sprite *currentSprite = channel->_sprite;
		Sprite *nextSprite;

		if (currentSprite->_puppet)
			nextSprite = currentSprite;
		else
			nextSprite = currentFrame->_sprites[i];

		// A sprite needs to be updated if one of the following happens:
		// - The dimensions/bounding box of the sprite has changed (_dirty flag set)
		// - The cast member ID of the sprite has changed (_dirty flag set)
		// - The sprite slot from the current frame is different (cast member ID or bounding box) from the cached sprite slot
		// (maybe we have to compare all the sprite attributes, not just these two?)
		bool needsUpdate = currentSprite->isDirty() ||
			currentSprite->_castId != nextSprite->_castId ||
			channel->_delta != Common::Point(0, 0) ||
			currentSprite->getDims() != nextSprite->getDims() ||
			channel->_currentPoint != nextSprite->_startPoint;

		if ((needsUpdate || forceUpdate) && !currentSprite->_trails)
			markDirtyRect(channel->getBbox());

		channel->_sprite = nextSprite;
		channel->_sprite->updateCast();

		// Sprites marked moveable are constrained to the same bounding box until
		// the moveable is disabled
		if (!nextSprite->_puppet && !nextSprite->_moveable)
			channel->_currentPoint = nextSprite->_startPoint;

		channel->updateLocation();

		// TODO: Understand why conditioning this causes so many problems
		markDirtyRect(channel->getBbox());
	}

	for (uint id = 0; id < _channels.size(); id++) {
		Channel *channel = _channels[id];
		Sprite *sprite = channel->_sprite;

		if (!sprite || !sprite->_enabled || !sprite->_castId)
			continue;

		Common::Rect currentBbox = channel->getBbox();

		debugC(1, kDebugImages, "Score::renderFrame(): channel: %d,  castType: %d,  castId: %d", id, sprite->_castType, sprite->_castId);

		if (sprite->_castType == kCastShape) {
			renderShape(id);
		} else {
			Cast *cast = sprite->_cast;
			if (cast && cast->_widget) {
				cast->_widget->_priority = id;
				cast->_widget->draw();
				inkBasedBlit(cast->_widget->getMask(), cast->_widget->getSurface()->rawSurface(), channel->_sprite->_ink, currentBbox, id);
			} else {
				warning("Score::renderFrame(): No widget for channel ID %d", id);
			}
		}

		sprite->setClean();
	}

	if (!updateStageOnly) {
		_vm->_wm->renderZoomBox();

		_vm->_wm->draw();

		if (currentFrame->_transType != 0) {
			// TODO Handle changing area case
			playTransition(currentFrame->_transDuration, currentFrame->_transArea, currentFrame->_transChunkSize, currentFrame->_transType);
		}

		if (currentFrame->_sound1 != 0 || currentFrame->_sound2 != 0) {
			playSoundChannel(frameId);
		}

	}

	g_system->copyRectToScreen(_surface->getPixels(), _surface->pitch, 0, 0, _surface->getBounds().width(), _surface->getBounds().height());

}

void Score::markDirtyRect(Common::Rect dirty) {
	_maskSurface->fillRect(dirty, 1);
	_surface->fillRect(dirty, _stageColor);
}

void Score::screenShot() {
	Graphics::Surface rawSurface = _surface->rawSurface();
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

void Score::renderShape(uint16 spriteId) {
	Sprite *sp = _channels[spriteId]->_sprite;

	InkType ink = sp->_ink;
	byte spriteType = sp->_spriteType;
	byte foreColor = sp->_foreColor;
	byte backColor = sp->_backColor;
	int lineSize = sp->_thickness & 0x3;

	if (_vm->getVersion() >= 3 && spriteType == kCastMemberSprite) {
		if (!sp->_cast) {
			warning("Score::renderShape(): kCastMemberSprite has no cast defined");
			return;
		}
		switch (sp->_cast->_type) {
		case kCastShape:
			{
				ShapeCast *sc = (ShapeCast *)sp->_cast;
				switch (sc->_shapeType) {
				case kShapeRectangle:
					spriteType = sc->_fillType ? kRectangleSprite : kOutlinedRectangleSprite;
					break;
				case kShapeRoundRect:
					spriteType = sc->_fillType ? kRoundedRectangleSprite : kOutlinedRoundedRectangleSprite;
					break;
				case kShapeOval:
					spriteType = sc->_fillType ? kOvalSprite : kOutlinedOvalSprite;
					break;
				case kShapeLine:
					spriteType = sc->_lineDirection == 6 ? kLineBottomTopSprite : kLineTopBottomSprite;
					break;
				default:
					break;
				}
				if (_vm->getVersion() > 3) {
					foreColor = sc->_fgCol;
					backColor = sc->_bgCol;
					lineSize = sc->_lineThickness;
					ink = sc->_ink;
				}
			}
			break;
		default:
			warning("Score::renderShape(): Unhandled cast type: %d", sp->_cast->_type);
			break;
		}
	}

	// for outlined shapes, line thickness of 1 means invisible.
	lineSize -= 1;

	Common::Rect shapeRect = _channels[spriteId]->getBbox();

	Graphics::ManagedSurface tmpSurface, maskSurface;
	tmpSurface.create(shapeRect.width(), shapeRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	tmpSurface.clear(backColor);

	maskSurface.create(shapeRect.width(), shapeRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	maskSurface.clear(0);

	// Draw fill
	Common::Rect fillRect((int)shapeRect.width(), (int)shapeRect.height());
	Graphics::MacPlotData plotFill(&tmpSurface, &maskSurface, &_vm->getPatterns(), sp->getPattern(), -shapeRect.left, -shapeRect.top, 1, backColor);
	switch (spriteType) {
	case kRectangleSprite:
		Graphics::drawFilledRect(fillRect, foreColor, Graphics::macDrawPixel, &plotFill);
		break;
	case kRoundedRectangleSprite:
		Graphics::drawRoundRect(fillRect, 12, foreColor, true, Graphics::macDrawPixel, &plotFill);
		break;
	case kOvalSprite:
		Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, foreColor, true, Graphics::macDrawPixel, &plotFill);
		break;
	case kCastMemberSprite: 		// Face kit D3
		Graphics::drawFilledRect(fillRect, foreColor, Graphics::macDrawPixel, &plotFill);
		break;
	default:
		break;
	}

	// Draw stroke
	Common::Rect strokeRect(MAX((int)shapeRect.width() - lineSize, 0), MAX((int)shapeRect.height() - lineSize, 0));
	Graphics::MacPlotData plotStroke(&tmpSurface, &maskSurface, &_vm->getPatterns(), 1, -shapeRect.left, -shapeRect.top, lineSize, backColor);
	switch (spriteType) {
	case kLineTopBottomSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kLineBottomTopSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.bottom, strokeRect.right, strokeRect.top, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kRectangleSprite:
		// fall through
	case kOutlinedRectangleSprite:	// this is actually a mouse-over shape? I don't think it's a real button.
		Graphics::drawRect(strokeRect, foreColor, Graphics::macDrawPixel, &plotStroke);
		break;
	case kRoundedRectangleSprite:
		// fall through
	case kOutlinedRoundedRectangleSprite:
		Graphics::drawRoundRect(strokeRect, 12, foreColor, false, Graphics::macDrawPixel, &plotStroke);
		break;
	case kOvalSprite:
		// fall through
	case kOutlinedOvalSprite:
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, foreColor, false, Graphics::macDrawPixel, &plotStroke);
		break;
	default:
		break;
	}

	inkBasedBlit(&maskSurface, tmpSurface, ink, shapeRect, spriteId);
}

Cast *Score::getCastMember(int castId) {
	Cast *result = nullptr;

	if (_loadedCast->contains(castId)) {
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
