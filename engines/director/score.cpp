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
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

Score::Score(Movie *movie) {
	_movie = movie;
	_window = movie->getWindow();
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_soundManager = _vm->getSoundManager();

	_puppetTempo = 0x00;
	_puppetPalette = false;
	_lastPalette = 0;

	_labels = nullptr;
	_currentCursor = nullptr;

	_currentFrameRate = 20;
	_currentFrame = 0;
	_nextFrame = 0;
	_currentLabel = 0;
	_nextFrameTime = 0;
	_waitForChannel = 0;
	_cursorDirty = false;
	_waitForClick = false;
	_waitForClickCursor = false;
	_activeFade = 0;
	_playState = kPlayNotStarted;

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

int Score::getCurrentPalette() {
	return _frames[_currentFrame]->_palette.paletteId;
}

int Score::resolvePaletteId(int id) {
	if (id > 0) {
		CastMember *member = _movie->getCastMember(id);
		id = (member && member->_type == kCastPalette) ? ((PaletteCastMember *)member)->getPaletteId() : 0;
	}

	return id;
}

bool Score::processImmediateFrameScript(Common::String s, int id) {
	s.trim();

	// In D2/D3 this specifies immediately the sprite/field properties
	if (!s.compareToIgnoreCase("moveableSprite") || !s.compareToIgnoreCase("editableText")) {
		_immediateActions[id] = true;
	}

	return false;
}

uint16 Score::getLabel(Common::String &label) {
	if (!_labels) {
		warning("Score::getLabel: No labels set");
		return 0;
	}

	for (Common::SortedArray<Label *>::iterator i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name.equalsIgnoreCase(label)) {
			return (*i)->number;
		}
	}

	return 0;
}

Common::String *Score::getLabelList() {
	Common::String *res = new Common::String;

	for (Common::SortedArray<Label *>::iterator i = _labels->begin(); i != _labels->end(); ++i) {
		*res += (*i)->name;
		*res += '\n';
	}

	return res;
}

Common::String *Score::getFrameLabel(uint id) {
	for (Common::SortedArray<Label *>::iterator i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number == id) {
			return new Common::String((*i)->name);
			break;
		}
	}

	return new Common::String;
}

void Score::setStartToLabel(Common::String &label) {
	uint16 num = getLabel(label);

	if (num == 0)
		warning("Label %s not found", label.c_str());
	else
		_nextFrame = num;
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

void Score::startPlay() {
	_currentFrame = 0;
	_playState = kPlayStarted;
	_nextFrameTime = 0;

	_lastPalette = _movie->getCast()->_defaultPalette;
	_vm->setPalette(resolvePaletteId(_lastPalette));

	if (_frames.size() <= 1) {	// We added one empty sprite
		warning("Score::startLoop(): Movie has no frames");
		_playState = kPlayStopped;
	}

	// All frames in the same movie have the same number of channels
	if (_playState != kPlayStopped)
		for (uint i = 0; i < _frames[1]->_sprites.size(); i++)
			_channels.push_back(new Channel(_frames[1]->_sprites[i], i));

	if (_vm->getVersion() >= 300)
		_movie->processEvent(kEventStartMovie);
}

void Score::step() {
	if (_playState == kPlayStopped)
		return;

	_lingo->processEvents();

	update();

	if (debugChannelSet(-1, kDebugFewFramesOnly) || debugChannelSet(-1, kDebugScreenshot)) {
		warning("Score::startLoop(): ran frame %0d", _framesRan);
		_framesRan++;
	}

	if (debugChannelSet(-1, kDebugFewFramesOnly) && _framesRan > 9) {
		warning("Score::startLoop(): exiting due to debug few frames only");
		_playState = kPlayStopped;
		return;
	}

	if (debugChannelSet(-1, kDebugScreenshot))
		screenShot();
}

void Score::stopPlay() {
	if (_vm->getVersion() >= 300)
		_movie->processEvent(kEventStopMovie);
	_lingo->executePerFrameHook(-1, 0);
}

void Score::update() {
	if (_activeFade) {
		if (!_soundManager->fadeChannel(_activeFade))
			_activeFade = 0;
	}

	if (_movie->_videoPlayback) {
		renderFrame(_currentFrame);
	}

	if (!debugChannelSet(-1, kDebugFast)) {
		if (_waitForChannel) {
			if (_soundManager->isChannelActive(_waitForChannel))
				return;

			_waitForChannel = 0;
		}

		if (_waitForClick) {
			if (g_system->getMillis() >= _nextFrameTime + 1000) {
				_waitForClickCursor = !_waitForClickCursor;
				_vm->setCursor(kCursorDefault);
				_vm->setCursor(_waitForClickCursor ? kCursorMouseDown : kCursorMouseUp);
				_nextFrameTime = g_system->getMillis();
			}
			return;
		}

		if (g_system->getMillis() < _nextFrameTime && !_nextFrame)
			return;
	}

	// For previous frame
	if (_currentFrame > 0 && !_vm->_playbackPaused) {
		// When Lingo::func_goto* is called, _nextFrame is set
		// and _skipFrameAdvance is set to true.
		// exitFrame is not called in this case.
		if (!_vm->_skipFrameAdvance && _vm->getVersion() >= 400) {
			_movie->processEvent(kEventExitFrame);
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

	if (_currentFrame >= _frames.size()) {
		if (debugChannelSet(-1, kDebugNoLoop)) {
			_playState = kPlayStopped;
			return;
		}

		_currentFrame = 1;
	}

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

	if (_vm->getVersion() >= 600) {
		// _movie->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		// _movie->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	// Window is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell, p.100)
	renderFrame(_currentFrame);
	_window->_newMovieStarted = false;

	// Enter and exit from previous frame
	if (!_vm->_playbackPaused) {
		_movie->processEvent(kEventEnterFrame); // Triggers the frame script in D2-3, explicit enterFrame handlers in D4+
		if (_vm->getVersion() == 300) {
			// Movie version of enterFrame, for D3 only. The Lingo Dictionary claims
			// "This handler executes before anything else when the playback head moves."
			// but this is incorrect. The frame script is executed first.
			_movie->processEvent(kEventStepMovie);
		}
	}
	// TODO Director 6 - another order

	byte tempo = _frames[_currentFrame]->_tempo;
	if (tempo) {
		_puppetTempo = 0;
	} else if (_puppetTempo) {
		tempo = _puppetTempo;
	}

	if (tempo) {
		if (tempo > 161) {
			// Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;
		} else if (tempo <= 60) {
			// FPS
			_currentFrameRate = tempo;
			_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;
		} else {
			if (tempo >= 136) {
				// TODO Wait for channel tempo - 135
				warning("STUB: tempo >= 136");
			} else if (tempo == 128) {
				_waitForClick = true;
				_waitForClickCursor = false;
				_vm->setCursor(kCursorMouseUp);
			} else if (tempo == 135) {
				// Wait for sound channel 1
				_waitForChannel = 1;
			} else if (tempo == 134) {
				// Wait for sound channel 2
				_waitForChannel = 2;
			}
			_nextFrameTime = g_system->getMillis();
		}
	} else {
		_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;
	}

	if (debugChannelSet(-1, kDebugSlow))
		_nextFrameTime += 1000;
}

void Score::renderFrame(uint16 frameId, RenderMode mode) {
	if (!renderTransition(frameId))
		renderSprites(frameId, mode);

	int currentPalette = _frames[frameId]->_palette.paletteId;
	if (!_puppetPalette && currentPalette != 0 && currentPalette != _lastPalette) {
		_lastPalette = currentPalette;
		g_director->setPalette(resolvePaletteId(currentPalette));
	}

	if (mode != kRenderNoWindowRender)
		_window->render();

	if (_frames[frameId]->_sound1 || _frames[frameId]->_sound2)
		playSoundChannel(frameId);

	if (_cursorDirty) {
		renderCursor(_movie->getWindow()->getMousePos());
		_cursorDirty = false;
	}
}

bool Score::renderTransition(uint16 frameId) {
	Frame *currentFrame = _frames[frameId];
	TransParams *tp = _window->_puppetTransition;

	if (tp) {
		_window->playTransition(tp->duration, tp->area, tp->chunkSize, tp->type, frameId);

		delete _window->_puppetTransition;
		_window->_puppetTransition = nullptr;
		return true;
	} else if (currentFrame->_transType) {
		_window->playTransition(currentFrame->_transDuration, currentFrame->_transArea, currentFrame->_transChunkSize, currentFrame->_transType, frameId);
		return true;
	} else {
		return false;
 }
}

void Score::renderSprites(uint16 frameId, RenderMode mode) {
	if (_window->_newMovieStarted)
		mode = kRenderForceUpdate;

	_movie->_videoPlayback = false;

	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		Sprite *currentSprite = channel->_sprite;
		Sprite *nextSprite = _frames[frameId]->_sprites[i];

		// widget content has changed and needs a redraw.
		// this doesn't include changes in dimension or position!
		bool widgetRedrawn = channel->updateWidget();

		if (channel->isActiveText())
			_movie->_currentEditableTextChannel = i;

		if (channel->isActiveVideo())
			_movie->_videoPlayback = true;

		if (channel->isDirty(nextSprite) || widgetRedrawn || mode == kRenderForceUpdate) {
			if (!currentSprite->_trails)
				_window->addDirtyRect(channel->getBbox());

			channel->setClean(nextSprite, i);
			_window->addDirtyRect(channel->getBbox());
			debugC(2, kDebugImages, "Score::renderSprites(): CH: %-3d castId: %03d(%s) [ink: %d, puppet: %d, moveable: %d, visible: %d] [bbox: %d,%d,%d,%d] [type: %d fg: %d bg: %d] [script: %d]", i, currentSprite->_castId, numToCastNum(currentSprite->_castId), currentSprite->_ink, currentSprite->_puppet, currentSprite->_moveable, channel->_visible, PRINT_RECT(channel->getBbox()), currentSprite->_spriteType, currentSprite->_foreColor, currentSprite->_backColor, currentSprite->_scriptId);
		} else {
			channel->setClean(nextSprite, i, true);
		}
	}
}

void Score::renderCursor(Common::Point pos) {
	uint spriteId = 0;

	if (_channels.empty())
		return;

	for (int i = _channels.size() - 1; i >=0; i--)
		if (_channels[i]->isMouseIn(pos) && !_channels[i]->_cursor.isEmpty())
			spriteId = i;

	if (_channels[spriteId]->_cursor.isEmpty()) {
		if (_currentCursor) {
			_vm->_wm->popCursor();
			_currentCursor = nullptr;
		}
	} else {
		if (_currentCursor) {
			if (*_currentCursor == _channels[spriteId]->_cursor)
				return;

			_vm->_wm->popCursor();
		}

		_currentCursor = &_channels[spriteId]->_cursor;
		_vm->_wm->pushCursor(_currentCursor->_cursorType, _currentCursor);
	}
}

void Score::screenShot() {
	Graphics::Surface rawSurface = _window->getSurface()->rawSurface();
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

uint16 Score::getSpriteIDFromPos(Common::Point pos) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->isMouseIn(pos))
			return i;

	return 0;
}

uint16 Score::getMouseSpriteIDFromPos(Common::Point pos) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->isMouseIn(pos) && _channels[i]->_sprite->respondsToMouse())
			return i;

	return 0;
}

uint16 Score::getActiveSpriteIDFromPos(Common::Point pos) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->isMouseIn(pos) && _channels[i]->_sprite->isActive())
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

	for (uint i = 0; i < _channels.size(); i++) {
		if (!_channels[i]->isEmpty() && !r.findIntersectingRect(_channels[i]->getBbox()).isEmpty())
			intersections.push_back(_channels[i]);
	}

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

void Score::loadFrames(Common::SeekableReadStreamEndian &stream) {
	debugC(1, kDebugLoading, "****** Loading frames VWSC");

	//stream.hexdump(stream.size());

	uint32 size = stream.readUint32();
	size -= 4;

	if (_vm->getVersion() < 400) {
		_numChannelsDisplayed = 30;
	} else if (_vm->getVersion() >= 400 && _vm->getVersion() < 500) {
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
	} else if (_vm->getVersion() >= 500) {
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

	Frame *initial = new Frame(this, _numChannelsDisplayed);
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
		debugC(3, kDebugLoading, "++++++++++ score frame %d (frameSize %d) size %d", _frames.size(), frameSize, size);

		if (frameSize > 0) {
			Frame *frame = new Frame(this, _numChannelsDisplayed);
			size -= frameSize;
			frameSize -= 2;

			while (frameSize != 0) {

				if (_vm->getVersion() < 400) {
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

void Score::loadLabels(Common::SeekableReadStreamEndian &stream) {
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

void Score::loadActions(Common::SeekableReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading Actions VWAC");

	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	byte id = stream.readByte();

	byte subId = stream.readByte(); // I couldn't find how it used in continuity (except print). Frame actionId = 1 byte.
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 1; i <= count; i++) {
		uint16 nextId = stream.readByte();
		byte nextSubId = stream.readByte();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);

		Common::String script;
		for (uint16 j = stringPos; j < nextStringPos; j++) {
			byte ch = stream.readByte();
			if (ch == 0x0d) {
				ch = '\n';
			}
			script += ch;
		}
		_actions[i] = script;

		debugC(3, kDebugLoading, "Action index: %d id: %d nextId: %d subId: %d, code: %s", i, id, nextId, subId, _actions[i].c_str());

		stream.seek(streamPos);

		id = nextId;
		subId = nextSubId;
		stringPos = nextStringPos;

		if ((int32)stringPos == stream.size())
			break;
	}

	bool *scriptRefs = (bool *)calloc(_actions.size() + 1, sizeof(bool));

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
			// Check if it is empty
			bool empty = true;
			for (const char *ptr = j->_value.c_str(); *ptr; ptr++)
				if (!(*ptr == ' ' || *ptr == '-' || *ptr == '\n' || *ptr == '\r' || *ptr == '\t' || *ptr == '\xc2')) {
					empty = false;
					break;
				}

			if (!empty)
				warning("Action id %d is not referenced, the code is:\n-----\n%s\n------", j->_key, j->_value.c_str());

			continue;
		}
		if (!j->_value.empty()) {
			_movie->getMainLingoArch()->addCode(j->_value.c_str(), kScoreScript, j->_key);

			processImmediateFrameScript(j->_value, j->_key);
		}
	}

	free(scriptRefs);
}

} // End of namespace Director
