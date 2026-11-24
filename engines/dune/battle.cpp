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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dune/dune.h"
#include "dune/actor_pool.h"
#include "dune/clock.h"
#include "dune/sprite_sheet.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Dune {

void DuneEngine::loadBattleAssets() {
	if (_battleAssetsReady)
		return;
	openSpriteSheet("ATTACK.HSQ", false);
	for (uint i = 4; i < 46; i++) {
		if (!currentSpriteSheet->size(i, &_battleSpriteWidth[i], &_battleSpriteHeight[i]))
			error("Dune: missing ATTACK sprite %u", i);
	}
	for (uint i = 0; i < 3; i++) {
		if (!currentSpriteSheet->read(53 + i, 6, _battlePalettes[i], 84))
			error("Dune: missing ATTACK flash palette %u", i);
	}
	_battleAssetsReady = true;
}

void DuneEngine::startBattleAnimation(bool restart) {
	loadBattleAssets();
	const uint16 place = READ_LE_UINT16(_state.begin() + 0x114e);
	if (_battleAnimationPlace != place) {
		clearMapUnits(); // A new battlefield takes ownership of ONMAP's C60B pool.
		_mapAnimationActive = false;
		_battleAnimationPlace = place;
		WRITE_LE_UINT16(_state.begin() + 0x3cbe, 0);
		memcpy(_battlePalette, _battlePalettes[0], 84);
	} else if (!restart) {
		return;
	}
	_battleAnimationElapsed = 0;
	// DA25 clears this callback's remainder, not D9D2's preceding clock.
	// ACD installs the ordered painter and clears the eight B45 counters.
	WRITE_LE_UINT16(_state.begin() + 0x2786, 0xc827);
	memset(_state.begin() + 0x4856, 0, 8);
	playSoundEffect(3); // B1C/AB15: SN3 loops for this registered battlefield.
}

void DuneEngine::stopBattleAnimation() {
	if (!_battleAnimationPlace)
		return;
	finishSoundEffectLoop(); // B21/AC30 releases before removing B45, not a hard stop.
	_battleAnimationPlace = 0;
	_battleAnimationElapsed = 0;
	// B21 keeps the pool only during its special negative-FB transition.
	if (_state[0x227d] || (int8)_state[0xfb] >= 0)
		WRITE_LE_UINT16(_state.begin() + 0x3cbe, 0);
}

/** C60B/C202: place an ATTACK sprite by its centre, retaining slot scratch. */
uint DuneEngine::createBattleActor(uint sprite, int x, int y, uint16 velocity) {
	if (sprite < 4 || sprite >= 46)
		return 0;
	x -= _battleSpriteWidth[sprite] / 2;
	y -= _battleSpriteHeight[sprite] / 2;
	return appendSpriteActor(_state.begin(), x, y, x + _battleSpriteWidth[sprite],
			y + _battleSpriteHeight[sprite], sprite, velocity);
}

/** C58A: compact without clearing the last slot, and relocate both handles. */
void DuneEngine::removeBattleActor(uint at) {
	removeSpriteActor(_state.begin(), at);
}

static uint16 rotateBattleWord(byte *word, uint bits) {
	const uint16 value = READ_LE_UINT16(word);
	const uint16 rotated = (value << bits) | (value >> (16 - bits));
	WRITE_LE_UINT16(word, rotated);
	return rotated;
}

/** C3B: a burst from one of the four original DS:1592 launch points. */
void DuneEngine::spawnBattleActor() {
	byte *s = _state.begin();
	if ((int8)--s[0x4857] < 0) {
		const uint16 bits = READ_LE_UINT16(s);
		if (!(bits & 3))
			s[0x485d] = (bits & 12) ? 11 : 17;
		uint16 selector = troopRandom();
		if (s[0x473a])
			selector &= ~0x10;
		s[0x4857] = troopRandom(true) & 7;
		if (s[0x4857] >= 4)
			selector |= 0x4000;
		WRITE_LE_UINT16(s + 0x4858, selector);
	}
	s[0x4856] = 8;
	uint sprite;
	uint16 velocity;
	if (!(s[0x4858] & 0x10)) {
		const uint direction = s[0x4859] & 6;
		velocity = READ_LE_UINT16(s + 0x15a2 + direction);
		sprite = 4 + 2 * direction;
	} else {
		byte angle = s[0x4859] & 0x3f;
		byte flags = s[0x4859] & 0xc0;
		if ((flags & 0x40) && (rotateBattleWord(s, 1) & 1)) {
			angle += (flags & 0x80) ? -10 : 10;
			if ((int8)angle < 0) {
				flags ^= 0x80;
				angle = 0;
			}
			if (angle >= 0x40) {
				angle = 0x3f;
				flags ^= 0x80;
			}
			s[0x4859] = flags | angle;
		}
		// 5198's square bearing vector, not a trigonometric approximation.
		angle += 0xe0;
		const byte quadrant = angle + 0x20;
		int dx, dy;
		if ((quadrant & 0x7f) >= 0x40) {
			dx = 32;
			angle -= 0x40;
			if (quadrant & 0x80) {
				dx = -32;
				angle = -(byte)(angle - 0x80);
			}
			dy = (int8)angle;
		} else {
			dy = -32;
			if (quadrant & 0x80) {
				dy = 32;
				angle = -(byte)(angle - 0x80);
			}
			dx = (int8)angle;
		}
		velocity = (byte)dx | ((uint16)(byte)dy << 8);
		sprite = 20;
	}
	const uint origin = s[0x4858] & 12;
	const uint at = createBattleActor(sprite, READ_LE_UINT16(s + 0x1592 + origin),
		READ_LE_UINT16(s + 0x1594 + origin), velocity);
	if (at)
		WRITE_LE_UINT16(s + at + 13, 0);
}

/** B45, including D0D's original six-bit palette interpolation. */
DuneEngine::BattleAnimationUpdate DuneEngine::stepBattleAnimation(bool paused) {
	loadBattleAssets();
	byte *s = _state.begin();
	const byte flash = --s[0x485d];
	const bool updatePalette = (int8)s[0xea] <= 0 && flash <= 16;
	if (updatePalette) {
		if (flash == 16 || flash == 10) {
			memcpy(_battlePalette, _battlePalettes[flash == 16 ? 2 : 1], 84);
		} else {
			const int divisor = flash ? flash : 1;
			for (uint i = 0; i < 84; i++)
				_battlePalette[i] += ((int)_battlePalettes[0][i] - _battlePalette[i]) / divisor;
		}
	}
	// DOS gates movement after the palette callback. Native modal flags are
	// passed separately; borrowing DS:46EB would corrupt its other consumers.
	if (paused || READ_LE_UINT16(s + 0x479e) || s[0x46eb])
		return updatePalette ? kBattlePaletteUpdate : kBattleNoUpdate;
	const uint16 wait = READ_LE_UINT16(s + 0x485a) - 1;
	WRITE_LE_UINT16(s + 0x485a, wait);
	if ((int16)wait < 0) {
		if ((int8)--s[0x485c] < 0) {
			const uint16 random = troopRandom();
			s[0x485c] = random & 0x7f;
			WRITE_LE_UINT16(s + 0x485a, random >> 8);
		} else {
			const uint16 random = troopRandom();
			const uint y = random & 0x7f;
			const uint x = (random >> 8) | ((random & 0x80) << 1);
			if (y >= 0x30 && y < 0x60 && x < 320)
				createBattleActor(28 + (y & 7), x, y, 0);
		}
	}
	if ((int8)--s[0x4856] < 0)
		spawnBattleActor();
	// B45 snapshots the count and advances DI even after C58A compacts the
	// array. The shifted neighbour is skipped; the stale tail may be visited.
	const uint count = MIN<uint>(READ_LE_UINT16(s + 0x3cbe), kMaxBattleActors);
	for (uint i = 0; i < count; i++) {
		const uint at = 0x3cc0 + i * 17;
		uint16 sprite = READ_LE_UINT16(s + at + 8);
		int dx = (int8)s[at + 10], dy = (int8)s[at + 11];
		if ((byte)sprite >= 28) {
			if ((byte)++sprite > 45) {
				removeBattleActor(at);
				continue;
			}
		} else if ((byte)sprite >= 20) {
			// CEA runs once for each axis, carrying the fractional remainder.
			const byte x = ABS(dx) + s[at + 13];
			const byte y = ABS(dy) + s[at + 14];
			s[at + 13] = x & 31;
			s[at + 14] = y & 31;
			dx = (dx < 0 ? -1 : 1) * (x >> 5);
			dy = (dy < 0 ? -1 : 1) * (y >> 5);
			sprite = 20 + (rotateBattleWord(s, 3) & 7);
		} else {
			sprite >>= 2;
			for (uint bit = 0; bit < 2; bit++)
				sprite = (sprite << 1) | (rotateBattleWord(s, 1) & 1);
		}
		WRITE_LE_UINT16(s + at + 8, sprite);
		for (uint coord = 0; coord < 8; coord += 2)
			WRITE_LE_UINT16(s + at + coord, READ_LE_UINT16(s + at + coord) + ((coord & 2) ? dy : dx));
		if (READ_LE_UINT16(s + at) >= 320 || (int16)READ_LE_UINT16(s + at + 4) < 0 ||
				(int16)READ_LE_UINT16(s + at + 6) < 0)
			removeBattleActor(at);
	}
	return kBattleActorUpdate;
}

void DuneEngine::drawBattleActors() {
	enum { kSceneHeight = 152 };
	openSpriteSheet("ATTACK.HSQ", false);
	const uint count = MIN<uint>(READ_LE_UINT16(_state.begin() + 0x3cbe), kMaxBattleActors);
	// Restore the bottom strip after clipping the original 152-row viewport.
	byte panel[kScreenWidth * (kScreenHeight - kSceneHeight)];
	memcpy(panel, _framebuffer1 + kScreenWidth * kSceneHeight, sizeof(panel));
	for (uint i = 0; i < count; i++) {
		const byte *actor = _state.begin() + 0x3cc0 + 17 * i;
		const uint sprite = READ_LE_UINT16(actor + 8);
		if (!(actor[12] & 0x80) && sprite >= 4 && sprite < 46)
			drawSprite(sprite, (int16)READ_LE_UINT16(actor), (int16)READ_LE_UINT16(actor + 2));
	}
	memcpy(_framebuffer1 + kScreenWidth * kSceneHeight, panel, sizeof(panel));
}

void DuneEngine::advanceBattleAnimation(uint32 elapsedMs) {
	const bool visible = !_onWorld && !_onGlobe && !_onMixer && !_inCabin &&
		!_inDesert && _menuKind != kMenuWormMap && _menuKind != kMenuTroopNotice;
	// DA25 registers B45 at three 200-Hz ticks. D9D2 invokes it at most once
	// per poll, retaining the remainder instead of replaying missed callbacks.
	const uint32 elapsed = animationTimerElapsedMs(elapsedMs, _battleAnimationElapsed);
	_battleAnimationElapsed = elapsed % 15;
	if (elapsed >= 15) {
		const BattleAnimationUpdate update = stepBattleAnimation(!visible);
		if (update == kBattleActorUpdate) {
			drawBattlefield(true);
		} else if (visible && update == kBattlePaletteUpdate) {
			// D0D updates the palette even when B45 leaves the scene pixels alone.
			applyPalette(128, 28, _battlePalette);
			_system->updateScreen();
		}
	}
}

/** D84B/D84E: refresh scratch AFTER all registered callbacks, not just B45. */
void DuneEngine::refreshInputRandom() {
	WRITE_LE_UINT16(_state.begin(), troopRandom());
}

/** DD63 refreshes DS:0 BEFORE D9D2, unlike the ordinary D83E poll. */
void DuneEngine::pollBattleWait(uint32 elapsedMs) {
	refreshInputRandom();
	updateGameMusic();
	// This API accepts an observed relative DD63 input, including no-delay
	// diagnostics. Retain it, and advance the shared sample for the next poll.
	const uint32 previous = _animationClock.sampled() ? _animationClock.lastMillis()
		: _battleAnimationClockSet ? _battleAnimationTime : _system->getMillis();
	const uint32 now = previous + elapsedMs;
	const AnimationClock::Poll poll(_animationClock, now, (uint16)(now / 5 - previous / 5));
	advanceBattleAnimation(elapsedMs);
	updateSkyFade();
}

bool DuneEngine::pollBattleWaitInput(byte &buttons) {
	Common::EventManager *events = _system->getEventManager();
	byte current = (events->getButtonState() & 3) | (buttons == 0xff ? 0 : buttons & 4);
	_state[0xcee9] = 0; // DE54: distinguish Escape from another skipped wait.
	bool keyPressed = false;
	Common::Event event;
	while (events->pollEvent(event)) {
		updatePointerClickKeys(event);
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			// F012 overwrites the key latch until DD63 polls it.
			keyPressed = true;
			_state[0xcee9] = event.kbd.keycode == Common::KEYCODE_ESCAPE;
			break;
		case Common::EVENT_LBUTTONDOWN: current |= 1; break;
		case Common::EVENT_LBUTTONUP: current &= ~1; break;
		case Common::EVENT_RBUTTONDOWN: current |= 2; break;
		case Common::EVENT_RBUTTONUP: current &= ~2; break;
		case Common::EVENT_MBUTTONDOWN: current |= 4; break;
		case Common::EVENT_MBUTTONUP: current &= ~4; break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return true;
		case Common::EVENT_MAINMENU:
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			return true;
		default:
			break;
		}
	}
	if (keyPressed)
		return true;
	const bool pressed = (current & ~buttons) != 0;
	buttons = current;
	return pressed || shouldQuit();
}

/** DDB0 waits until subtraction underflows, not merely until it reaches 0. */
void DuneEngine::waitBattleEffect(uint16 ticks, uint32 &clock, uint32 &lastPoll, bool noDelay) {
	byte buttons = 0xff; // SI=-1: ignore buttons already held on the first poll.
	_state[0xcee8] = 0;
	int remaining = ticks;
	while (!shouldQuit()) {
		const uint32 before = noDelay ? clock : _system->getMillis() / 5 * 5;
		if (pollBattleWaitInput(buttons))
			break;
		const uint32 now = noDelay ? clock : _system->getMillis() / 5 * 5;
		pollBattleWait(now - lastPoll);
		lastPoll = now;
		if (noDelay) {
			// Fast diagnostics still execute every poll and callback. Skipping
			// time here used to change both saved random streams and the effects.
			clock += 5;
		} else {
			// DDD8 waits for at least the next hardware tick after DD63. It
			// does not call the scheduler repeatedly inside that spin loop.
			while (_system->getMillis() / 5 * 5 == before && !shouldQuit())
				_system->delayMillis(1);
			clock = _system->getMillis() / 5 * 5;
		}
		remaining -= (int)((clock - before) / 5);
		if (remaining < 0)
			break;
	}
	_state[0xcee8] = 0; // DE4E clears the consumed key, never pass it to a menu.
}

void DuneEngine::massiveAttack() {
	if (!battleHere())
		return;
	startBattleAnimation();
	resolveMassiveAttack();
	_menuUnder = -1;
	const bool noDelay = ConfMan.getBool("dune_nodelay");
	uint32 clock = noDelay ? 0 : _system->getMillis() / 5 * 5;
	uint32 lastPoll = !noDelay && _battleAnimationClockSet ? _battleAnimationTime : clock;
	// 7373..7391: a skip returns to this loop, not to the room menu. Combat
	// has already been resolved even if the player interrupts every wait.
	for (uint effect = 0; effect < 20 && !shouldQuit(); effect++) {
		const uint16 choice = troopRandom(true) & 0x201;
		_state[0x485d] = (choice & 1) ? 17 : 11;
		waitBattleEffect(40 + (choice >> 8), clock, lastPoll, noDelay);
	}
	_state[0x473a] = 0;
	_battleAnimationTime = noDelay ? _system->getMillis() / 5 * 5 : lastPoll;
	_battleAnimationClockSet = true;
	_animationClock.rebase(_battleAnimationTime);
}

void DuneEngine::updateBattleAnimation() {
	// D9D2 samples time even with no B45 registration. Keep that preceding
	// poll across AFB/B21: the first new battle callback receives the whole
	// interval, including the part before its registration. Zero is a valid
	// clock sample, not a reason to drop that first interval.
	const uint32 now = animationClockTicks() * 5;
	const uint32 elapsed = _animationClock.active() ? _animationClock.elapsed() * 5u
		: _battleAnimationClockSet ? now - _battleAnimationTime : 0;
	_battleAnimationTime = now;
	_battleAnimationClockSet = true;
	if (_inDesert || !battleHere() || (_onWorld && !_inCabin) ||
			(_pendingBoardingPlace >= 0 && !_state[0x2b])) {
		stopBattleAnimation();
		// 5A1A's 18BA removes B45 before the strategic map replaces its
		// actors. Do not reinstall that callback merely because the covered
		// place is still a battlefield. The cabin and worm selector have
		// different entry paths and retain their paused battle callback.
		return;
	}
	startBattleAnimation();
	advanceBattleAnimation(elapsed);
}

} // End of namespace Dune
