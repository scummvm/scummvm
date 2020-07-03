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
#include "common/memstream.h"
#include "common/substream.h"

#include "engines/util.h"
#include "graphics/macgui/mactext.h"

#ifdef USE_PNG
#include "image/png.h"
#endif

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/score.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/stage.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

Channel::Channel(Sprite *sp) {
	_sprite = sp;
	_currentPoint = sp->_startPoint;
	_delta = Common::Point(0, 0);
	_constraint = 0;

	_visible = true;
	_dirty = true;

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

const Graphics::Surface *Channel::getMask() {
	switch (_sprite->_ink) {
	case kInkTypeMatte:
		// Mattes are only supported in bitmaps for now. Shapes don't need mattes,
		// as they already have all non-enclosed white pixels transparent.
		// Matte on text has a trivial enough effect to not worry about implementing.
		if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap) {
			return ((BitmapCastMember *)_sprite->_cast)->getMatte();
		} else {
			return nullptr;
		}

	case kInkTypeMask: {
		CastMember *member = g_director->getCurrentMovie()->getCastMember(_sprite->_castId + 1);

		if (_sprite->_cast && member->_initialRect == _sprite->_cast->_initialRect) {
			return &member->_widget->getSurface()->rawSurface();
		} else {
			warning("Channel::getMask(): Requested cast mask, but no matching mask was found");
			return nullptr;
		}

		// Silence warning
		break;
	}

	default:
		return nullptr;
	}
}

bool Channel::isDirty(Sprite *nextSprite) {
	// When a sprite is puppeted setTheSprite ensures that the dirty flag here is
	// set. Otherwise, we need to rerender when the position, bounding box, or
	// cast of the sprite changes.
	bool isDirty = _dirty ||
		_delta != Common::Point(0, 0) ||
		((_sprite->_castType != kCastTypeNull) &&
		 (_sprite->_cast && _sprite->_cast->isModified()));

	if (nextSprite) {
		isDirty |= _sprite->_castId != nextSprite->_castId ||
			_sprite->_ink != nextSprite->_ink ||
			_sprite->getDims() != nextSprite->getDims() ||
			(_currentPoint != nextSprite->_startPoint &&
			 !_sprite->_puppet && !_sprite->_moveable);
	}

	return isDirty;
}

Common::Rect Channel::getBbox() {
	Common::Rect bbox = _sprite->getDims();
	bbox.moveTo(getPosition());

	return bbox;
}

void Channel::setClean(Sprite *nextSprite, int spriteId) {
	_dirty = false;

	if (!_sprite->_puppet) {
		_sprite = nextSprite;
		_sprite->updateCast();

		// Sprites marked moveable are constrained to the same bounding box until
		// the moveable is disabled
		if (!_sprite->_moveable)
			_currentPoint = _sprite->_startPoint;
	}

	_currentPoint += _delta;
	_delta = Common::Point(0, 0);

	if (_sprite->_cast && _sprite->_cast->_widget) {
		Common::Point p(getPosition());
		_sprite->_cast->_modified = false;
		_sprite->_cast->_widget->_dims.moveTo(p.x, p.y);

		_sprite->_cast->_widget->_priority = spriteId;
		_sprite->_cast->_widget->draw();
		_sprite->_cast->_widget->_contentIsDirty = false;
	}
}

void Channel::addDelta(Common::Point pos) {
	// TODO: Channel should have a pointer to its score
	if (_constraint > g_director->getCurrentMovie()->getScore()->_channels.size() - 1) {
		warning("Channel::addDelta: Received out-of-bounds constraint: %d", _constraint);
		_constraint = 0;
	} else if (_sprite->_moveable && _constraint > 0) {
		Common::Rect constraintBbox = g_director->getCurrentMovie()->getScore()->_channels[_constraint]->getBbox();

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
		BitmapCastMember *bc = (BitmapCastMember *)(_sprite->_cast);

		res += Common::Point(bc->_initialRect.left - bc->_regX,
												 bc->_initialRect.top - bc->_regY);
	}

	return res;
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
				ShapeCastMember *sc = (ShapeCastMember *)_sprite->_cast;
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

Score::Score(Movie *movie) {
	_movie = movie;
	_stage = movie->getStage();
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_soundManager = _vm->getSoundManager();
	_puppetTempo = 0x00;

	_labels = nullptr;

	_currentFrameRate = 20;
	_currentFrame = 0;
	_nextFrame = 0;
	_currentLabel = 0;
	_nextFrameTime = 0;
	_stopPlay = false;

	_numChannelsDisplayed = 0;

	_framesRan = 0; // used by kDebugFewFramesOnly and kDebugScreenshot
}

Score::~Score() {
	for (uint i = 0; i < _frames.size(); i++)
		delete _frames[i];

	for (uint i = 0; i < _channels.size(); i++)
		delete _channels[i];

	if (_labels)
		for (Common::SortedArray<Label *>::iterator it = _labels->begin(); it != _labels->end(); ++it)
			delete *it;

	delete _labels;
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

void Score::startLoop() {
	// TODO: Should the dims be set by the movie?
	debugC(1, kDebugImages, "Score dims: %dx%d", _movie->_movieRect.width(), _movie->_movieRect.height());
	initGraphics(_vm->_surface->w, _vm->_surface->h);

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

		if (!_stage->_newMovieStarted)
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
	_stage->_newMovieStarted = false;

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
	if (!renderTransition(frameId))
		renderSprites(frameId, mode);

	_vm->_wm->renderZoomBox();
	_stage->render();
	_vm->_wm->draw();

	if (_frames[frameId]->_sound1 || _frames[frameId]->_sound2)
		playSoundChannel(frameId);
}

bool Score::renderTransition(uint16 frameId) {
	Frame *currentFrame = _frames[frameId];
	TransParams *tp = _stage->_puppetTransition;

	if (tp) {
		_stage->playTransition(tp->duration, tp->area, tp->chunkSize, tp->type, frameId);

		delete _stage->_puppetTransition;;
		_stage->_puppetTransition = nullptr;
		return true;
	} else if (currentFrame->_transType) {
		_stage->playTransition(currentFrame->_transDuration, currentFrame->_transArea, currentFrame->_transChunkSize, currentFrame->_transType, frameId);
		return true;
	} else {
		return false;
 }
}

void Score::renderSprites(uint16 frameId, RenderMode mode) {
	if (_stage->_newMovieStarted)
		mode = kRenderForceUpdate;

	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		Sprite *currentSprite = channel->_sprite;
		Sprite *nextSprite = _frames[frameId]->_sprites[i];

		bool needsUpdate = channel->isDirty(nextSprite) || mode == kRenderForceUpdate;

		if (needsUpdate && !currentSprite->_trails)
			_stage->addDirtyRect(channel->getBbox());

		channel->setClean(nextSprite, i);

		if (needsUpdate)
			_stage->addDirtyRect(channel->getBbox());
	}
}

void Score::screenShot() {
	Graphics::Surface rawSurface = _stage->getSurface()->rawSurface();
	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::Surface *newSurface = rawSurface.convertTo(requiredFormat_4byte, _vm->getPalette());
	Common::String currentPath = _vm->getCurrentPath().c_str();
	Common::replace(currentPath, "/", "-"); // exclude '/' from screenshot filename prefix
	Common::String prefix = Common::String::format("%s%s", currentPath.c_str(), _movie->getMacName().c_str());
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

void Score::loadFrames(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading frames VWSC");

	//stream.hexdump(stream.size());

	uint32 size = stream.readUint32();
	size -= 4;

	if (_vm->getVersion() < 4) {
		_numChannelsDisplayed = 30;
	} else if (_vm->getVersion() == 4) {
		uint32 frame1Offset = stream.readUint32();
		uint32 numFrames = stream.readUint32();
		uint16 version = stream.readUint16();
		uint16 spriteRecordSize = stream.readUint16();
		uint16 numChannels = stream.readUint16();
		size -= 14;

		if (version > 13) {
			_numChannelsDisplayed = stream.readUint16();
		} else {
			if (version <= 7)	// Director5
				_numChannelsDisplayed = 48;
			else
				_numChannelsDisplayed = 120;	// D6

			stream.readUint16(); // Skip
		}

		size -= 2;

		warning("STUB: Score::loadFrames. frame1Offset: %x numFrames: %x version: %x spriteRecordSize: %x numChannels: %x numChannelsDisplayed: %x",
			frame1Offset, numFrames, version, spriteRecordSize, numChannels, _numChannelsDisplayed);
		// Unknown, some bytes - constant (refer to contuinity).
	} else if (_vm->getVersion() > 4) {
		//what data is up the top of D5 VWSC?
		uint32 unk1 = stream.readUint32();
		uint32 unk2 = stream.readUint32();

		uint16 unk3, unk4, unk5, unk6;

		if (unk2 > 0) {
			uint32 blockSize = stream.readUint32() - 1;
			stream.readUint32();
			stream.readUint32();
			stream.readUint32();
			stream.readUint32();
			for (uint32 skip = 0; skip < blockSize * 4; skip++)
				stream.readByte();

			//header number two... this is our actual score entry point.
			unk1 = stream.readUint32();
			unk2 = stream.readUint32();
			stream.readUint32();
			unk3 = stream.readUint16();
			unk4 = stream.readUint16();
			unk5 = stream.readUint16();
			unk6 = stream.readUint16();
		} else {
			unk3 = stream.readUint16();
			unk4 = stream.readUint16();
			unk5 = stream.readUint16();
			unk6 = stream.readUint16();
			size -= 16;
		}
		warning("STUB: Score::loadFrames. unk1: %x unk2: %x unk3: %x unk4: %x unk5: %x unk6: %x", unk1, unk2, unk3, unk4, unk5, unk6);
	}

	uint16 channelSize;
	uint16 channelOffset;

	Frame *initial = new Frame(_vm, _numChannelsDisplayed);
	// Push a frame at frame#0 position.
	// This makes all indexing simpler
	_frames.push_back(initial);

	// This is a representation of the channelData. It gets overridden
	// partically by channels, hence we keep it and read the score from left to right
	//
	// TODO Merge it with shared cast
	byte channelData[kChannelDataSize];
	memset(channelData, 0, kChannelDataSize);

	while (size != 0 && !stream.eos()) {
		uint16 frameSize = stream.readUint16();
		debugC(8, kDebugLoading, "++++++++++ score frame %d (frameSize %d) size %d", _frames.size(), frameSize, size);

		if (frameSize > 0) {
			Frame *frame = new Frame(_vm, _numChannelsDisplayed);
			size -= frameSize;
			frameSize -= 2;

			while (frameSize != 0) {

				if (_vm->getVersion() < 4) {
					channelSize = stream.readByte() * 2;
					channelOffset = stream.readByte() * 2;
					frameSize -= channelSize + 2;
				} else {
					channelSize = stream.readUint16();
					channelOffset = stream.readUint16();
					frameSize -= channelSize + 4;
				}

				assert(channelOffset + channelSize < kChannelDataSize);
				stream.read(&channelData[channelOffset], channelSize);
			}

			Common::MemoryReadStreamEndian *str = new Common::MemoryReadStreamEndian(channelData, ARRAYSIZE(channelData), stream.isBE());
			// str->hexdump(str->size(), 32);
			frame->readChannels(str);
			delete str;

			debugC(8, kDebugLoading, "Score::loadFrames(): Frame %d actionId: %d", _frames.size(), frame->_actionId);

			_frames.push_back(frame);
		} else {
			warning("zero sized frame!? exiting loop until we know what to do with the tags that follow.");
			size = 0;
		}
	}
}

void Score::setSpriteCasts() {
	// Update sprite cache of cast pointers/info
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			_frames[i]->_sprites[j]->setCast(_frames[i]->_sprites[j]->_castId);

			debugC(1, kDebugImages, "Score::setSpriteCasts(): Frame: %d Channel: %d castId: %d type: %d", i, j, _frames[i]->_sprites[j]->_castId, _frames[i]->_sprites[j]->_spriteType);
		}
	}
}

void Score::loadLabels(Common::SeekableSubReadStreamEndian &stream) {
	if (debugChannelSet(5, kDebugLoading)) {
		debug("Score::loadLabels()");
		stream.hexdump(stream.size());
	}

	_labels = new Common::SortedArray<Label *>(compareLabels);
	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	uint16 frame = stream.readUint16();
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 1; i < count; i++) {
		uint16 nextFrame = stream.readUint16();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);
		Common::String label;

		for (uint32 j = stringPos; j < nextStringPos; j++) {
			label += stream.readByte();
		}

		_labels->insert(new Label(label, frame));
		stream.seek(streamPos);

		frame = nextFrame;
		stringPos = nextStringPos;
	}

	Common::SortedArray<Label *>::iterator j;

	debugC(2, kDebugLoading, "****** Loading labels");
	for (j = _labels->begin(); j != _labels->end(); ++j) {
		debugC(2, kDebugLoading, "Frame %d, Label '%s'", (*j)->number, Common::toPrintable((*j)->name).c_str());
	}
}

int Score::compareLabels(const void *a, const void *b) {
	return ((const Label *)a)->number - ((const Label *)b)->number;
}

void Score::loadActions(Common::SeekableSubReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading Actions VWAC");

	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	byte id = stream.readByte();

	byte subId = stream.readByte(); // I couldn't find how it used in continuity (except print). Frame actionId = 1 byte.
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 0; i < count; i++) {
		uint16 nextId = stream.readByte();
		byte nextSubId = stream.readByte();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);

		for (uint16 j = stringPos; j < nextStringPos; j++) {
			byte ch = stream.readByte();
			if (ch == 0x0d) {
				ch = '\n';
			}
			_actions[i + 1] += ch;
		}

		debugC(3, kDebugLoading, "Action id: %d nextId: %d subId: %d, code: %s", id, nextId, subId, _actions[id].c_str());

		stream.seek(streamPos);

		id = nextId;
		subId = nextSubId;
		stringPos = nextStringPos;

		if ((int32)stringPos == stream.size())
			break;
	}

	bool *scriptRefs = (bool *)calloc(_actions.size() + 1, sizeof(int));

	// Now let's scan which scripts are actually referenced
	for (uint i = 0; i < _frames.size(); i++) {
		if (_frames[i]->_actionId <= _actions.size())
			scriptRefs[_frames[i]->_actionId] = true;

		for (uint16 j = 0; j <= _frames[i]->_numChannels; j++) {
			if (_frames[i]->_sprites[j]->_scriptId <= _actions.size())
				scriptRefs[_frames[i]->_sprites[j]->_scriptId] = true;
		}
	}

	Common::HashMap<uint16, Common::String>::iterator j;

	if (ConfMan.getBool("dump_scripts"))
		for (j = _actions.begin(); j != _actions.end(); ++j) {
			if (!j->_value.empty())
				_movie->getCast()->dumpScript(j->_value.c_str(), kScoreScript, j->_key);
		}

	for (j = _actions.begin(); j != _actions.end(); ++j) {
		if (!scriptRefs[j->_key]) {
			warning("Action id %d is not referenced, the code is:\n-----\n%s\n------", j->_key, j->_value.c_str());
			// continue;
		}
		if (!j->_value.empty()) {
			_movie->getMainLingoArch()->addCode(j->_value.c_str(), kScoreScript, j->_key);

			processImmediateFrameScript(j->_value, j->_key);
		}
	}

	free(scriptRefs);
}

} // End of namespace Director
