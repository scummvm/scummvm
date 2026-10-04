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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/whackitpuzzle.h"

namespace Nancy {
namespace Action {

// How the bees fly
static const double kMaxBeeSpeed = 200.0;
static const double kFallForce = 5.0;
static const int kGustChance = 50;		// one tick in this many changes the bee's gust
static const int kMaxGust = 20;

// How long the screen flashes red after a sting
static const uint32 kStingFlashTime = 500;

void WhackItPuzzle::Animation::readData(Common::SeekableReadStream &stream) {
	int32 framesPerSecond = stream.readSint32LE();
	frameTime = framesPerSecond > 0 ? 1000 / framesPerSecond : 1000;

	frames.resize(stream.readUint16LE());
	for (Common::Rect &frame : frames) {
		readRect(stream, frame);
	}
}

void WhackItPuzzle::AnimationPlayer::start(const Animation *newAnimation) {
	animation = newAnimation;
	frame = 0;
	frameRemainder = 0;
	isDone = false;
}

void WhackItPuzzle::AnimationPlayer::advance(uint32 time, bool loops) {
	time += frameRemainder;
	frame += time / animation->frameTime;
	frameRemainder = time % animation->frameTime;

	if (loops) {
		frame %= animation->frames.size();
	} else {
		isDone = frame >= animation->frames.size();
	}
}

Common::Rect WhackItPuzzle::AnimationPlayer::getDest(const Common::Point &pos) const {
	const Common::Rect &src = getSrc();
	int left = pos.x - src.width() / 2;
	int top = pos.y - src.height() / 2;
	return Common::Rect(left, top, left + src.width(), top + src.height());
}

void WhackItPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	readRect(stream, _canSrc);
	_cursor = stream.readUint16LE();
	_canOffset.x = stream.readSint32LE();
	_canOffset.y = stream.readSint32LE();
	_startPos.x = stream.readSint32LE();
	_startPos.y = stream.readSint32LE();
	readExclusiveRect(stream, _bounds);
	_minBees = stream.readSint32LE();
	_maxBees = stream.readSint32LE();

	// In seconds
	uint32 *times[] = { &_downTime[0], &_downTime[1], &_revivingTime[0], &_revivingTime[1],
						&_flyingTime[0], &_flyingTime[1], &_angryTime[0], &_angryTime[1] };
	for (uint32 *time : times) {
		*time = stream.readUint32LE() * 1000;
	}

	_allDownFlag.label = stream.readSint16LE();
	_stopsWhenAllDown = stream.readByte();
	_hitFrame = stream.readSint32LE();

	_puffAnimation.readData(stream);
	_flyAnimation.readData(stream);
	_downAnimation.readData(stream);

	_puffSound.readData(stream);
	_revivingSound.readData(stream);
	_flyingSound.readData(stream);
	_angrySound.readData(stream);
	_stingSound.readData(stream);

	_maxStings = stream.readSint32LE();
	_loseScene._sceneChange.sceneID = stream.readUint16LE();
	_loseScene._sceneChange.frameID = stream.readUint16LE();
	_loseScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_loseScene._flag.label = stream.readSint16LE();
	_loseScene._flag.flag = stream.readByte();
	_loseSound.readData(stream);

	readExitHotspot(stream);
}

void WhackItPuzzle::init() {
	initViewportSurface();
	loadImage();

	int numBees = _minBees;
	if (_maxBees > _minBees) {
		numBees += g_nancy->_randomSource->getRandomNumber(_maxBees - _minBees - 1);
	}

	_bees.resize(numBees);
	for (Bee &bee : _bees) {
		spawnBee(bee);
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	_flash._drawSurface.create(viewportBounds.width(), viewportBounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_flash.moveTo(viewportBounds);
	_flash.setVisible(false);

	_lastUpdateTime = g_nancy->getTotalPlayTime();
	redraw();
}

void WhackItPuzzle::registerGraphics() {
	PuzzleRecord::registerGraphics();
	_flash.registerGraphics();
}

uint32 WhackItPuzzle::getRandomTime(uint32 minTime, uint32 maxTime) const {
	uint32 range = MAX<int32>((int32)(maxTime - minTime), 1);
	return g_nancy->getTotalPlayTime() + g_nancy->_randomSource->getRandomNumber(range - 1) + minTime;
}

void WhackItPuzzle::spawnBee(Bee &bee) {
	bee.animation.start(&_flyAnimation);
	bee.state = kFlying;

	const Common::Rect &frame = _flyAnimation.frames[0];
	bee.body.setSize(frame.width(), frame.height());
	bee.body.setBounds(_bounds);
	bee.body.setBoundsMode(PhysicalObject::kBounceOffBounds);
	bee.body.setMaxSpeed(kMaxBeeSpeed);

	int x = _startPos.x;
	int y = _startPos.y;
	if (x < 0) {
		x = _bounds.left + g_nancy->_randomSource->getRandomNumber(MAX<int>(_bounds.width(), 1) - 1);
	}

	if (y < 0) {
		y = _bounds.top + g_nancy->_randomSource->getRandomNumber(MAX<int>(_bounds.height(), 1) - 1);
	}

	bee.body.moveTo(x, y);
	bee.body.setSpeed(50 + g_nancy->_randomSource->getRandomNumber(99));
	bee.body.setHeading(g_nancy->_randomSource->getRandomNumber(359) * M_PI / 180);
	bee.stateEndTime = getRandomTime(_flyingTime[0], _flyingTime[1]);
}

void WhackItPuzzle::flyAgain(Bee &bee) {
	bee.animation.start(&_flyAnimation);
	bee.body.setSpeed(kMaxBeeSpeed);
	bee.body.setHeading(3 * M_PI / 2);
	bee.body.setForce(0, 0);
	bee.body.setBoundsMode(PhysicalObject::kBounceOffBounds);
	bee.state = kFlying;
	bee.stateEndTime = getRandomTime(_flyingTime[0], _flyingTime[1]);
}

void WhackItPuzzle::knockDown(Bee &bee) {
	// The bee drops to the ground
	bee.animation.start(&_downAnimation);
	bee.body.setSpeed(0);
	bee.body.setHeading(M_PI / 2);
	bee.body.setForce(0, kFallForce);
	bee.body.setBoundsMode(PhysicalObject::kStopAtBounds);
	bee.state = kDown;
	bee.stateEndTime = getRandomTime(_downTime[0], _downTime[1]);
}

void WhackItPuzzle::sting() {
	++_numStings;
	playSoundBlock(_stingSound);
	_lastStingTime = g_nancy->getTotalPlayTime();
	_hasStung = true;

	if (_numStings > _maxStings) {
		_isLost = true;
	}
}

void WhackItPuzzle::updateBee(Bee &bee, uint32 numTicks) {
	bee.body.update(numTicks);
	const uint32 now = g_nancy->getTotalPlayTime();

	switch (bee.state) {
	case kDown:
		// A bee that is down doesn't move its wings
		if (_isActive && now > bee.stateEndTime) {
			bee.state = kReviving;
			bee.stateEndTime = getRandomTime(_revivingTime[0], _revivingTime[1]);
		}

		return;
	case kReviving:
		if (!_isActive) {
			return;
		}

		if (now > bee.stateEndTime) {
			flyAgain(bee);
		}

		break;
	case kFlying:
	case kAngry:
		if (now > bee.stateEndTime) {
			if (bee.state == kFlying) {
				bee.state = kAngry;
				bee.stateEndTime = getRandomTime(_angryTime[0], _angryTime[1]);
			} else {
				sting();
				flyAgain(bee);
			}
		}

		// Now and then a gust changes where the bee drifts
		if (g_nancy->_randomSource->getRandomNumber(kGustChance - 1) == 0) {
			bee.body.setForce((int)g_nancy->_randomSource->getRandomNumber(2 * kMaxGust - 1) - kMaxGust,
							(int)g_nancy->_randomSource->getRandomNumber(2 * kMaxGust - 1) - kMaxGust);
		}

		break;
	}

	bee.animation.advance(numTicks * PhysicalObject::kTickLength, true);
}

void WhackItPuzzle::updateSounds(uint numReviving, uint numFlying, uint numAngry) {
	// The more bees there are of a kind, the louder they get
	const double numBees = MAX<uint>(_bees.size(), 1);
	const RandomSoundBlock *sounds[] = { &_revivingSound, &_flyingSound, &_angrySound };
	uint16 volumes[] = {
		(uint16)(numReviving ? (0.5 + numReviving / numBees * 0.5) * _revivingSound.volume : 0),
		(uint16)(numFlying ? (0.5 + numFlying / numBees * 0.5) * _flyingSound.volume : 0),
		(uint16)(numAngry ? _angrySound.volume : 0)
	};

	for (uint i = 0; i < ARRAYSIZE(sounds); ++i) {
		if (sounds[i]->names.empty()) {
			continue;
		}

		if (!g_nancy->_sound->isSoundPlaying(sounds[i]->channel)) {
			playSoundBlock(*sounds[i]);
		}

		g_nancy->_sound->setVolume(sounds[i]->channel, volumes[i]);
	}
}

void WhackItPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());
	const uint32 now = g_nancy->getTotalPlayTime();

	for (const Bee &bee : _bees) {
		const Common::Rect &src = bee.animation.getSrc();
		Common::Rect dest = bee.animation.getDest(bee.getPos());

		if (bee.state != kAngry) {
			_drawSurface.blitFrom(_image, src, dest.origin());
			continue;
		}

		// An angry bee flickers red
		uint8 amount = (now - bee.stateEndTime) & 0xFF;
		for (int y = MAX<int>(dest.top, 0); y < MIN<int>(dest.bottom, _drawSurface.h); ++y) {
			for (int x = MAX<int>(dest.left, 0); x < MIN<int>(dest.right, _drawSurface.w); ++x) {
				uint32 color = _image.getPixel(src.left + x - dest.left, src.top + y - dest.top);
				if (color == _image.getTransparentColor()) {
					continue;
				}

				uint8 a, r, g, b;
				_image.format.colorToARGB(color, a, r, g, b);
				_drawSurface.setPixel(x, y, _drawSurface.format.ARGBToColor(a, r, g * amount / 255, b * amount / 255));
			}
		}
	}

	for (const Puff &puff : _puffs) {
		_drawSurface.blitFrom(_image, puff.animation.getSrc(), puff.animation.getDest(puff.pos).origin());
	}

	if (_isActive && _isMouseInside) {
		_drawSurface.blitFrom(_image, _canSrc, _mousePos + _canOffset);
	}

	// A sting turns everything red for a moment
	if (_hasStung && now < _lastStingTime + kStingFlashTime) {
		uint8 alpha = (uint8)((1.0 - (now - _lastStingTime) / (double)kStingFlashTime) * 255);
		_flash._drawSurface.clear(_flash._drawSurface.format.ARGBToColor(alpha, 255, 0, 0));
		_flash.setVisible(true);
		_flash.setNeedsRedraw(true);
	} else {
		_flash.setVisible(false);
	}

	_needsRedraw = true;
}

void WhackItPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		uint32 now = g_nancy->getTotalPlayTime();
		uint32 numTicks = (now - _lastUpdateTime) / PhysicalObject::kTickLength;
		_lastUpdateTime += numTicks * PhysicalObject::kTickLength;

		if (numTicks > 0) {
			uint numReviving = 0;
			uint numFlying = 0;
			uint numAngry = 0;

			for (Bee &bee : _bees) {
				updateBee(bee, numTicks);

				switch (bee.state) {
				case kReviving:
					++numReviving;
					break;
				case kFlying:
					++numFlying;
					break;
				case kAngry:
					++numAngry;
					break;
				default:
					break;
				}
			}

			updateSounds(numReviving, numFlying, numAngry);

			bool isAllDown = numFlying + numAngry == 0;
			NancySceneState.setEventFlag(_allDownFlag.label, isAllDown ? g_nancy->_true : g_nancy->_false);
			if (isAllDown && _stopsWhenAllDown) {
				_isActive = false;
			}

			// A puff knocks down every flying bee it touches while it lasts
			for (uint i = 0; i < _puffs.size(); ) {
				Puff &puff = _puffs[i];
				puff.animation.advance(numTicks * PhysicalObject::kTickLength, false);
				if (puff.animation.isDone) {
					_puffs.remove_at(i);
					continue;
				}

				Common::Rect puffRect = puff.animation.getDest(puff.pos);
				for (Bee &bee : _bees) {
					if (bee.state != kFlying && bee.state != kAngry) {
						continue;
					}

					// The frame the bee is hit in, or its current one
					const Common::Array<Common::Rect> &frames = bee.animation.animation->frames;
					int hitFrame = _hitFrame < 0 ? (int)bee.animation.frame : _hitFrame;
					const Common::Rect &hitSrc = frames[CLIP<int>(hitFrame, 0, frames.size() - 1)];
					Common::Point pos = bee.getPos();
					Common::Rect beeRect(pos.x - hitSrc.width() / 2, pos.y - hitSrc.height() / 2,
										pos.x - hitSrc.width() / 2 + hitSrc.width(), pos.y - hitSrc.height() / 2 + hitSrc.height());

					if (beeRect.intersects(puffRect)) {
						knockDown(bee);
					}
				}

				++i;
			}
		}

		if (_isLost) {
			_state = kActionTrigger;
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_isLost) {
			playSoundBlock(_loseSound);
			_loseScene.execute();
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void WhackItPuzzle::handleInput(NancyInput &input) {
	_isMouseInside = false;

	if (_state != kRun || _isLost || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	const bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	if (hoverExitHotspot(input)) {
		if (clicked) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	_mousePos = mouseRect.origin();
	_isMouseInside = true;

	if (!_isActive) {
		return;
	}

	// The spray can takes the place of the cursor
	setDataCursor(_cursor);

	if (clicked) {
		playSoundBlock(_puffSound);

		Puff puff;
		puff.pos = _mousePos;
		puff.animation.start(&_puffAnimation);
		_puffs.push_back(puff);

		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
