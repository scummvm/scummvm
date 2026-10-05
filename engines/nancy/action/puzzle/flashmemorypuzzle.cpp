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
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/flashmemorypuzzle.h"

namespace Nancy {
namespace Action {

// The three kinds of lightning
static const double kSteadyChance = 0.2;		// one long, even flash
static const double kDoubleChance = 0.5;		// a strike, a dim afterglow, then dark
static const double kNoFlicker = 1000.0;
static const double kFlickerInterval = 1.0 / 15;
static const double kMaxFlicker = 0.04;

FlashMemoryPuzzle::~FlashMemoryPuzzle() {
	for (auto &image : _lightImages) {
		delete image._value;
	}
}

double FlashMemoryPuzzle::getRandom() {
	return g_nancy->_randomSource->getRandomNumber(32767) / 32767.0;
}

void FlashMemoryPuzzle::Flash::addKeyframe(double keyTime, double keyBrightness) {
	for (uint i = 0; i < keyframes.size(); ++i) {
		if (keyframes[i].time == keyTime) {
			keyframes[i].brightness = keyBrightness;
			return;
		}

		if (keyframes[i].time > keyTime) {
			Keyframe keyframe;
			keyframe.time = keyTime;
			keyframe.brightness = keyBrightness;
			keyframes.insert_at(i, keyframe);
			return;
		}
	}

	Keyframe keyframe;
	keyframe.time = keyTime;
	keyframe.brightness = keyBrightness;
	keyframes.push_back(keyframe);
}

bool FlashMemoryPuzzle::Flash::update(double seconds) {
	flickerTime += seconds;
	time += seconds;

	if (flickerTime > flickerInterval) {
		// The light wavers, less so at the start and at the end
		flickerTime = 0;
		double amount = getRandom() * 2 * kMaxFlicker - kMaxFlicker;
		double progress = time / duration;
		if (progress < 0.2) {
			flicker = progress * 5 * amount;
		} else if (progress > 0.8) {
			flicker = (1 - (progress * 5 - 4)) * amount;
		} else {
			flicker = amount;
		}
	}

	for (uint i = 1; i < keyframes.size(); ++i) {
		const Keyframe &from = keyframes[i - 1];
		const Keyframe &to = keyframes[i];
		if (time < to.time) {
			brightness = (to.brightness - from.brightness) * ((time - from.time) / (to.time - from.time)) + from.brightness + flicker;
			brightness = CLIP(brightness, 0.0, 1.0);
			return false;
		}
	}

	return true;
}

void FlashMemoryPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_lights.resize(stream.readUint16LE());
	for (Light &light : _lights) {
		readFilename(stream, light.backgroundName);
		readFilename(stream, light.picturesName);
		light.minWait = stream.readDoubleLE();
		light.maxWait = stream.readDoubleLE();
		light.thunder.readData(stream);
		light.nextFlash = light.minWait + (light.maxWait - light.minWait) * getRandom();
	}

	// The two pictures of each pair; the pairs are told apart by these numbers
	struct Pair {
		int id;
		Common::Rect pictures[2];
		bool isFirstTaken;
	};

	Common::Array<Pair> pairs;
	for (int remaining = stream.readUint16LE(); remaining > 0; remaining -= 2) {
		Pair pair;
		pair.id = remaining;
		readExclusiveRect(stream, pair.pictures[0]);
		readExclusiveRect(stream, pair.pictures[1]);
		pair.isFirstTaken = false;
		pairs.push_back(pair);
	}

	// The pictures are dealt out at random
	_cards.resize(stream.readUint16LE());
	for (Card &card : _cards) {
		readExclusiveRect(stream, card.dest);

		if (pairs.empty()) {
			continue;
		}

		uint pairIndex = g_nancy->_randomSource->getRandomNumber(pairs.size() - 1);
		Pair &pair = pairs[pairIndex];
		card.pairID = pair.id;

		if (!pair.isFirstTaken) {
			card.src = pair.pictures[0];
			pair.isFirstTaken = true;
		} else {
			card.src = pair.pictures[1];
			pairs.remove_at(pairIndex);
		}
	}

	_cardCursor = stream.readUint16LE();
	_turnBackTime = stream.readDoubleLE();
	_clickSound.readData(stream);
	_matchSound.readData(stream);
	_noMatchSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_solveSoundBlock.readData(stream);
}

void FlashMemoryPuzzle::init() {
	// The puzzle draws the whole room, lightning and all
	const Graphics::ManagedSurface &background = NancySceneState.getViewport().getBackground();
	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(viewportBounds.width(), viewportBounds.height(), background.format);
	setTransparent(false);
	setVisible(true);
	moveTo(viewportBounds);

	g_nancy->_resource->loadImage(_imageName, _image);

	for (const Light &light : _lights) {
		const Common::Path *names[] = { &light.backgroundName, &light.picturesName };
		for (const Common::Path *name : names) {
			Common::String key = name->toString();
			if (!_lightImages.contains(key)) {
				Graphics::ManagedSurface *image = new Graphics::ManagedSurface();
				g_nancy->_resource->loadImage(*name, *image);
				_lightImages[key] = image;
			}
		}
	}

	_lastUpdateTime = g_nancy->getTotalPlayTime();
	redraw();
}

void FlashMemoryPuzzle::startFlash(uint lightID) {
	Light &light = _lights[lightID];
	light.waited = 0;
	light.nextFlash = light.minWait + (light.maxWait - light.minWait) * getRandom();

	Flash flash;
	flash.lightID = lightID;

	double kind = getRandom();
	if (kind < kSteadyChance) {
		flash.flickerInterval = kNoFlicker;
		flash.duration = getRandom() * 0.25 + 0.5;
		flash.addKeyframe(0, 1.0);
		flash.addKeyframe(flash.duration, 1.0);
	} else if (kind < kDoubleChance) {
		flash.flickerInterval = kNoFlicker;
		flash.duration = getRandom() * 0.5 + 1.0;
		double strikeEnd = flash.duration * (getRandom() * 0.1 + 0.25);
		flash.addKeyframe(0, 1.0);
		flash.addKeyframe(strikeEnd, 1.0);
		flash.addKeyframe(strikeEnd + 0.001, 0.05);
		flash.addKeyframe(flash.duration, 0);
	} else {
		// A few flickers of random strength
		flash.flickerInterval = kFlickerInterval;
		flash.duration = getRandom() + 1.0;
		double peak = getRandom() * 0.25 + 0.5;
		flash.addKeyframe(0, peak);

		int numFlickers = g_nancy->_randomSource->getRandomNumber(2) + 1;
		for (int i = 0; i < numFlickers; ++i) {
			double keyTime = getRandom() * flash.duration;
			flash.addKeyframe(keyTime, getRandom() * peak);
		}

		flash.addKeyframe(flash.duration, 0);
	}

	_flashes.push_back(flash);
	playSoundBlock(light.thunder);
}

void FlashMemoryPuzzle::clickCard(int cardID) {
	Card &card = _cards[cardID];

	if (!card.isShown) {
		// Turning a picture back over takes it back
		_firstCard = -1;
	} else if (_firstCard == -1) {
		_firstCard = cardID;
	} else {
		_secondCard = cardID;
	}

	card.isShown = !card.isShown;
	playSoundBlock(_clickSound);
}

void FlashMemoryPuzzle::checkPair() {
	Card &first = _cards[_firstCard];
	Card &second = _cards[_secondCard];

	if (first.pairID == second.pairID) {
		first.isMatched = true;
		second.isMatched = true;
		playSoundBlock(_matchSound);

		bool isCleared = true;
		for (const Card &card : _cards) {
			if (!card.isMatched) {
				isCleared = false;
				break;
			}
		}

		_isSolved = isCleared;
	} else {
		first.isShown = !first.isShown;
		second.isShown = !second.isShown;
		playSoundBlock(_noMatchSound);
	}

	_firstCard = -1;
	_secondCard = -1;
	_pairTime = 0;
}

void FlashMemoryPuzzle::addLight(const Graphics::ManagedSurface &source, const Common::Rect &src, const Common::Point &dest, double brightness) {
	for (int y = 0; y < src.height(); ++y) {
		int destY = dest.y + y;
		if (destY < 0 || destY >= _drawSurface.h || src.top + y >= source.h) {
			continue;
		}

		for (int x = 0; x < src.width(); ++x) {
			int destX = dest.x + x;
			if (destX < 0 || destX >= _drawSurface.w || src.left + x >= source.w) {
				continue;
			}

			uint8 r, g, b, lightR, lightG, lightB;
			source.format.colorToRGB(source.getPixel(src.left + x, src.top + y), lightR, lightG, lightB);
			_drawSurface.format.colorToRGB(_drawSurface.getPixel(destX, destY), r, g, b);

			r = (uint8)MIN<int>(255, r + (int)(lightR * brightness));
			g = (uint8)MIN<int>(255, g + (int)(lightG * brightness));
			b = (uint8)MIN<int>(255, b + (int)(lightB * brightness));
			_drawSurface.setPixel(destX, destY, _drawSurface.format.RGBToColor(r, g, b));
		}
	}
}

void FlashMemoryPuzzle::redraw() {
	_drawSurface.blitFrom(NancySceneState.getViewport().getBackground());

	for (const Flash &flash : _flashes) {
		const Graphics::ManagedSurface &image = *_lightImages[_lights[flash.lightID].backgroundName.toString()];
		addLight(image, Common::Rect(image.w, image.h), Common::Point(), flash.brightness);
	}

	for (const Card &card : _cards) {
		if (card.isShown) {
			_drawSurface.blitFrom(_image, card.src, card.dest.origin());
		}
	}

	for (const Flash &flash : _flashes) {
		const Graphics::ManagedSurface &image = *_lightImages[_lights[flash.lightID].picturesName.toString()];
		for (const Card &card : _cards) {
			if (card.isShown) {
				addLight(image, card.src, card.dest.origin(), flash.brightness);
			}
		}
	}

	_needsRedraw = true;
}

void FlashMemoryPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();
		double seconds = (now - _lastUpdateTime) / 1000.0;
		_lastUpdateTime = now;

		for (uint i = 0; i < _lights.size(); ++i) {
			Light &light = _lights[i];
			light.waited += seconds;
			if (light.waited >= light.nextFlash) {
				startFlash(i);
			}
		}

		for (uint i = 0; i < _flashes.size(); ) {
			if (_flashes[i].update(seconds)) {
				_flashes.remove_at(i);
			} else {
				++i;
			}
		}

		// Two pictures stay up for a moment before they go away or turn back
		if (_firstCard != -1 && _secondCard != -1) {
			_pairTime += seconds;
			if (_pairTime >= _turnBackTime) {
				checkPair();
			}
		}

		if (_isSolved) {
			_state = kActionTrigger;
		}

		redraw();
		break;
	}
	case kActionTrigger:
		playSoundBlock(_solveSoundBlock);
		_solveScene.execute();
		finishExecution();
		break;
	}
}

void FlashMemoryPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isSolved || (_firstCard != -1 && _secondCard != -1)) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	for (int i = _cards.size() - 1; i >= 0; --i) {
		if (_cards[i].isMatched || !_cards[i].dest.contains(pos)) {
			continue;
		}

		setDataCursor(_cardCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			clickCard(i);
			input.eatMouseInput();
		}

		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
