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
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/random.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"
#include "scumm/insane/rebel2/psx/video.h"

#include <math.h>

namespace Scumm {

#ifdef USE_TINYGL
static const int kLevel1FrameRate = 30;

static int getLevel1SoundPan(float screenX) {
	return CLIP<int>((int)(screenX * 127.0f / 320.0f), 0, 127);
}

struct RA2PSXLevel1Enemy {
	RA2PSXLevel1Enemy() : active(false), pattern(0), age(0), lifetime(0), fireFrame(0),
			laserFrames(0), startX(0), startY(0), controlX(0), controlY(0), endX(0), endY(0),
			x(0), y(0), size(0), pitch(0), yaw(0), roll(0) {}

	bool active;
	int pattern;
	int age;
	int lifetime;
	int fireFrame;
	int laserFrames;
	float startX;
	float startY;
	float controlX;
	float controlY;
	float endX;
	float endY;
	float x;
	float y;
	float size;
	float pitch;
	float yaw;
	float roll;
};

struct RA2PSXLevel1Explosion {
	RA2PSXLevel1Explosion() : frames(0), x(0), y(0) {}

	int frames;
	int x;
	int y;
};

struct RA2PSXLevel1Shot {
	RA2PSXLevel1Shot() : active(false), progress(0), targetX(0), targetY(0), targetZ(0) {}

	bool active;
	int progress;
	float start[3][3];
	float roll[3];
	float targetX;
	float targetY;
	float targetZ;
};

struct RA2PSXLevel1Ship {
	RA2PSXLevel1Ship() : x(0), y(0), z(1000), velocityX(0), velocityY(0) {}

	int x;
	int y;
	int z;
	int velocityX;
	int velocityY;
};

static const float kLevel1LaserStart[3][3] = {
	{ -350.0f, 200.0f, 400.0f },
	{ 350.0f, 200.0f, 400.0f },
	{ 0.0f, 500.0f, 400.0f }
};

static const float kLevel1LaserRoll[3] = { -45.0f, 45.0f, 0.0f };

static const float kLevel1ShipLaserStart[3][3] = {
	{ -93.0f, 11.0f, -139.0f },
	{ 93.0f, 11.0f, -139.0f },
	{ 4.0f, 210.0f, -111.0f }
};

static void updateLevel1Enemy(RA2PSXLevel1Enemy &enemy) {
	const float t = MIN(1.0f, (float)enemy.age / enemy.lifetime);
	const float inverse = 1.0f - t;
	enemy.x = inverse * inverse * enemy.startX + 2.0f * inverse * t * enemy.controlX +
			t * t * enemy.endX;
	enemy.y = inverse * inverse * enemy.startY + 2.0f * inverse * t * enemy.controlY +
			t * t * enemy.endY;
	enemy.size = 5.0f + 72.0f * t * t;
	enemy.yaw = (enemy.controlX - enemy.startX) * 0.18f + sinf(enemy.age * 0.09f) * 18.0f;
	enemy.pitch = -8.0f + sinf(enemy.age * 0.06f) * 10.0f;
	enemy.roll = (enemy.controlY - enemy.startY) * 0.12f + sinf(enemy.age * 0.12f) * 8.0f;
}

static void spawnLevel1Enemy(RA2PSXLevel1Enemy &enemy, Common::RandomSource &random) {
	enemy = RA2PSXLevel1Enemy();
	enemy.active = true;
	enemy.pattern = random.getRandomBit();
	enemy.lifetime = random.getRandomNumberRng(95, 150);
	enemy.fireFrame = enemy.lifetime * random.getRandomNumberRng(45, 70) / 100;

	const bool fromLeft = random.getRandomBit();
	if (enemy.pattern == 0) {
		enemy.startX = fromLeft ? -24.0f : 344.0f;
		enemy.startY = (float)random.getRandomNumberRng(25, 175);
		enemy.controlX = (float)random.getRandomNumberRng(95, 225);
		enemy.controlY = (float)random.getRandomNumberRng(35, 170);
		enemy.endX = fromLeft ? (float)random.getRandomNumberRng(210, 390) :
				(float)random.getRandomNumberRngSigned(-70, 110);
	} else {
		enemy.startX = (float)random.getRandomNumberRng(35, 285);
		enemy.startY = (float)random.getRandomNumberRng(20, 155);
		enemy.controlX = fromLeft ? (float)random.getRandomNumberRng(180, 310) :
				(float)random.getRandomNumberRng(10, 140);
		enemy.controlY = (float)random.getRandomNumberRng(20, 190);
		enemy.endX = fromLeft ? (float)random.getRandomNumberRngSigned(-80, 80) :
				(float)random.getRandomNumberRng(240, 400);
	}
	enemy.endY = (float)random.getRandomNumberRngSigned(-25, 245);
	updateLevel1Enemy(enemy);
}

static void updateLevel1Aim(int &x, int &y, int &velocityX, int &velocityY,
		int &directionX, int &directionY, bool left, bool right, bool up, bool down) {
	if (left && right)
		left = right = false;
	if (up && down)
		up = down = false;

	if (!left && !right) {
		directionX = 0;
		velocityX /= 2;
	} else {
		if (left) {
			if (velocityX > 0)
				velocityX = -velocityX / 4;
			velocityX = MAX(-4096, velocityX - 448);
			directionX = -1;
		}
		if (right) {
			if (velocityX < 0)
				velocityX = -velocityX / 4;
			velocityX = MIN(4096, velocityX + 448);
			directionX = 1;
		}
		if ((up || down) && ABS(velocityX) < ABS(velocityY) / 2)
			velocityX = ABS(velocityY) * directionX / 2;
	}
	x = CLIP<int>(x + velocityX / 512, 30, 290);

	if (!up && !down) {
		directionY = 0;
		velocityY /= 2;
	} else {
		if (up) {
			if (velocityY > 0)
				velocityY = -velocityY / 4;
			velocityY = MAX(-4096, velocityY - 448);
			directionY = -1;
		}
		if (down) {
			if (velocityY < 0)
				velocityY = -velocityY / 4;
			velocityY = MIN(4096, velocityY + 448);
			directionY = 1;
		}
		if ((left || right) && ABS(velocityX) / 2 > ABS(velocityY))
			velocityY = ABS(velocityX) * directionY / 2;
	}
	y = CLIP<int>(y + velocityY / 512, 48, 178);
}

static void updateLevel1Ship(RA2PSXLevel1Ship &ship,
		bool left, bool right, bool up, bool down) {
	if (left == right) {
		ship.velocityX = ship.velocityX * 3 / 4;
	} else if (left) {
		ship.velocityX = MAX(-4096, ship.velocityX - 400);
	} else {
		ship.velocityX = MIN(4096, ship.velocityX + 400);
	}

	if (up == down) {
		ship.velocityY = ship.velocityY * 3 / 4;
	} else if (up) {
		ship.velocityY = MAX(-4096, ship.velocityY - 400);
	} else {
		ship.velocityY = MIN(4096, ship.velocityY + 400);
	}

	ship.x = CLIP<int>(ship.x + ship.velocityX / 16 / 25, -282, 282);
	ship.y = CLIP<int>(ship.y + ship.velocityY * 10 / 4096, -142, 157);
}

static void getLevel1ShipOrientation(const RA2PSXLevel1Ship &ship,
		float &forwardX, float &forwardY, float &forwardZ, float &roll) {
	const float bank = (ship.velocityX / 16) * 360.0f / 4096.0f;
	const float yaw = -bank * 0.5f * 0.017453292519943295f;
	forwardX = sinf(yaw);
	forwardY = 0.0f;
	forwardZ = -cosf(yaw);
	roll = bank;
}

static void transformLevel1ShipPoint(const RA2PSXLevel1Ship &ship,
		float localX, float localY, float localZ,
		float &worldX, float &worldY, float &worldZ) {
	float forwardX;
	float forwardY;
	float forwardZ;
	float roll;
	getLevel1ShipOrientation(ship, forwardX, forwardY, forwardZ, roll);

	const float angle = roll * 0.017453292519943295f;
	const float cosine = cosf(angle);
	const float sine = sinf(angle);
	worldX = ship.x + forwardZ * cosine * localX - forwardZ * sine * localY +
			forwardX * localZ;
	worldY = ship.y + sine * localX + cosine * localY + forwardY * localZ;
	worldZ = ship.z - forwardX * cosine * localX + forwardX * sine * localY +
			forwardZ * localZ;
}

static bool spawnLevel1Shot(RA2PSXLevel1Shot *shots, int aimX, int aimY,
		const RA2PSXLevel1Ship *ship, int &targetScreenX, int &targetScreenY) {
	int slot = -1;
	for (int i = 0; i < 8; ++i) {
		if (!shots[i].active) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return false;

	RA2PSXLevel1Shot &shot = shots[slot];
	shot.active = true;
	shot.progress = 400;
	if (!ship) {
		for (int i = 0; i < 3; ++i) {
			for (int axis = 0; axis < 3; ++axis)
				shot.start[i][axis] = kLevel1LaserStart[i][axis];
			shot.roll[i] = kLevel1LaserRoll[i];
		}
		shot.targetX = (aimX - 160) * 18000.0f / 640.0f;
		shot.targetY = (aimY - 120) * 18000.0f / 640.0f;
		shot.targetZ = 18000.0f;
	} else {
		float forwardX;
		float forwardY;
		float forwardZ;
		float shipRoll;
		getLevel1ShipOrientation(*ship, forwardX, forwardY, forwardZ, shipRoll);
		for (int i = 0; i < 3; ++i) {
			transformLevel1ShipPoint(*ship, kLevel1ShipLaserStart[i][0],
					kLevel1ShipLaserStart[i][1], kLevel1ShipLaserStart[i][2],
					shot.start[i][0], shot.start[i][1], shot.start[i][2]);
			shot.roll[i] = kLevel1LaserRoll[i] + shipRoll;
		}
		transformLevel1ShipPoint(*ship, 0.0f, -70.0f, -100.0f,
				shot.targetX, shot.targetY, shot.targetZ);
		shot.targetX -= forwardX * 18000.0f;
		shot.targetY -= forwardY * 18000.0f;
		shot.targetZ -= forwardZ * 18000.0f;
	}

	targetScreenX = 160 + (int)(shot.targetX * 640.0f / shot.targetZ);
	targetScreenY = 120 + (int)(shot.targetY * 640.0f / shot.targetZ);
	return true;
}

static void updateLevel1Shots(RA2PSXLevel1Shot *shots) {
	for (int i = 0; i < 8; ++i) {
		if (!shots[i].active)
			continue;
		shots[i].progress += 200;
		if (shots[i].progress > 4399)
			shots[i].active = false;
	}
}

static void renderLevel1Shots(RA2PSXTinyGLRenderer &renderer, const RA2PSXModel &laser,
		const RA2PSXLevel1Shot *shots) {
	for (int shotIndex = 0; shotIndex < 8; ++shotIndex) {
		const RA2PSXLevel1Shot &shot = shots[shotIndex];
		if (!shot.active || shot.progress >= 4000)
			continue;
		const float progress = shot.progress / 4096.0f;
		for (int laserIndex = 0; laserIndex < 3; ++laserIndex) {
			const float *start = shot.start[laserIndex];
			const float directionX = shot.targetX - start[0];
			const float directionY = shot.targetY - start[1];
			const float directionZ = shot.targetZ - start[2];
			renderer.renderPerspectiveModel(laser,
					start[0] + directionX * progress,
					start[1] + directionY * progress,
					start[2] + directionZ * progress,
					directionX, directionY, directionZ, shot.roll[laserIndex], false);
		}
	}
}

static void drawLevel1Effects(Graphics::Surface &surface, const RA2PSXLevel1UI &ui,
		const RA2PSXLevel1Enemy *enemies, const RA2PSXLevel1Explosion *explosions,
		int laserTargetX, int laserTargetY) {
	const uint32 green = surface.format.RGBToColor(64, 255, 96);

	for (int i = 0; i < 3; ++i) {
		if (enemies[i].active && enemies[i].laserFrames > 0) {
			surface.drawLine((int)enemies[i].x - 2, (int)enemies[i].y,
					laserTargetX - 15, laserTargetY, green);
			surface.drawLine((int)enemies[i].x + 2, (int)enemies[i].y,
					laserTargetX + 15, laserTargetY, green);
		}
		if (explosions[i].frames > 0)
			ui.drawExplosion(surface, explosions[i].x, explosions[i].y,
					10 - explosions[i].frames);
	}
}
#endif

Rebel2PSX::Level1Result Rebel2PSX::playLevel1(const RA2PSXModel &enemyModel,
		const RA2PSXModel &shipModel, const RA2PSXModel &crosshair,
		const RA2PSXModel &laser, const RA2PSXLevel1UI &ui, int lives, int &score) {
#ifndef USE_TINYGL
	(void)enemyModel;
	(void)shipModel;
	(void)crosshair;
	(void)laser;
	(void)ui;
	(void)lives;
	(void)score;
	return kLevel1Error;
#else
	Common::SeekableReadStream *stream = openRawFile("S1/L01_PLAY.STR", 1);
	if (!stream)
		return kLevel1Error;

	RA2PSXStreamDecoder decoder(RA2PSXStreamDecoder::kVersion2);
	if (!decoder.loadStream(stream) || !decoder.setOutputPixelFormat(g_system->getScreenFormat())) {
		decoder.close();
		return kLevel1Error;
	}

	RA2PSXTinyGLRenderer renderer;
	if (!renderer.init(_vm->_screenWidth, _vm->_screenHeight)) {
		decoder.close();
		return kLevel1Error;
	}

	RA2PSXLevel1Enemy enemies[3];
	RA2PSXLevel1Explosion explosions[3];
	RA2PSXLevel1Shot shots[8];
	RA2PSXLevel1Ship ship;
	RA2PSXSoundPlayer soundPlayer(_vm, _soundBank);
	RA2PSXSoundPlayer::SoundId approachSounds[3] = {};
	int aimX = 160;
	int aimY = 113;
	int aimVelocityX = 0;
	int aimVelocityY = 0;
	int aimDirectionX = 0;
	int aimDirectionY = 0;
	int shield = 100;
	int spawnDelay = 0;
	int spawnRange = 80;
	int spawnBase = 60;
	int nextSpawnAdjustment = 0;
	int logicFrame = -1;
	int videoFrame = -1;
	int lastShotFrame = -4;
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool actionLeft = false;
	bool actionRight = false;
	bool actionUp = false;
	bool actionDown = false;
	int joystickAxisX = 0;
	int joystickAxisY = 0;
	bool mouseFire = false;
	bool keyFire = false;
	bool actionFire = false;
	bool fireRequested = false;
	bool thirdPersonView = true;
	Level1Result result = kLevel1Complete;
	const int joystickDeadzone = MIN<int>(Common::JOYAXIS_MAX,
			MAX(0, ConfMan.getInt("joystick_deadzone")) * 1000);

	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(false);
	g_system->warpMouse(160, 120);
	decoder.start();
	const uint32 gameplayStartTime = g_system->getMillis();
	const Graphics::Surface *background = nullptr;

	while (!_vm->shouldQuit() && !decoder.endOfVideo()) {
		bool redraw = false;
		bool toggleViewRequested = false;
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				if (thirdPersonView) {
					ship.x = CLIP<int>((event.mouse.x - 160) * ship.z / 640, -282, 282);
					ship.y = CLIP<int>((event.mouse.y - 120) * ship.z / 640, -142, 157);
					ship.velocityX = ship.velocityY = 0;
				} else {
					aimX = CLIP<int>(event.mouse.x, 30, 290);
					aimY = CLIP<int>(event.mouse.y, 48, 178);
					aimVelocityX = aimVelocityY = 0;
					aimDirectionX = aimDirectionY = 0;
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (!thirdPersonView) {
					aimX = CLIP<int>(event.mouse.x, 30, 290);
					aimY = CLIP<int>(event.mouse.y, 48, 178);
				}
				mouseFire = true;
				fireRequested = true;
				break;
			case Common::EVENT_LBUTTONUP:
				mouseFire = false;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					result = kLevel1Quit;
				} else if (event.kbd.keycode == Common::KEYCODE_TAB && !event.kbdRepeat) {
					toggleViewRequested = true;
				} else if (event.kbd.keycode == Common::KEYCODE_LEFT ||
						event.kbd.keycode == Common::KEYCODE_a) {
					moveLeft = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RIGHT ||
						event.kbd.keycode == Common::KEYCODE_d) {
					moveRight = true;
				} else if (event.kbd.keycode == Common::KEYCODE_UP ||
						event.kbd.keycode == Common::KEYCODE_w) {
					moveUp = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN ||
						event.kbd.keycode == Common::KEYCODE_s) {
					moveDown = true;
				} else if (event.kbd.keycode == Common::KEYCODE_SPACE ||
						event.kbd.keycode == Common::KEYCODE_RETURN) {
					keyFire = true;
					fireRequested = true;
				}
				break;
			case Common::EVENT_KEYUP:
				if (event.kbd.keycode == Common::KEYCODE_LEFT || event.kbd.keycode == Common::KEYCODE_a)
					moveLeft = false;
				else if (event.kbd.keycode == Common::KEYCODE_RIGHT || event.kbd.keycode == Common::KEYCODE_d)
					moveRight = false;
				else if (event.kbd.keycode == Common::KEYCODE_UP || event.kbd.keycode == Common::KEYCODE_w)
					moveUp = false;
				else if (event.kbd.keycode == Common::KEYCODE_DOWN || event.kbd.keycode == Common::KEYCODE_s)
					moveDown = false;
				else if (event.kbd.keycode == Common::KEYCODE_SPACE ||
						event.kbd.keycode == Common::KEYCODE_RETURN)
					keyFire = false;
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			case Common::EVENT_CUSTOM_ENGINE_ACTION_END: {
				const bool pressed = event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START;
				switch (event.customType) {
				case kScummActionInsaneLeft:
					actionLeft = pressed;
					break;
				case kScummActionInsaneRight:
					actionRight = pressed;
					break;
				case kScummActionInsaneUp:
					actionUp = pressed;
					break;
				case kScummActionInsaneDown:
					actionDown = pressed;
					break;
				case kScummActionInsaneAttack:
					actionFire = pressed;
					if (pressed)
						fireRequested = true;
					break;
				case kScummActionInsaneSwitch:
					if (pressed)
						toggleViewRequested = true;
					break;
				case kScummActionInsaneBack:
					if (pressed)
						result = kLevel1Quit;
					break;
				default:
					break;
				}
				break;
			}
			case Common::EVENT_CUSTOM_BACKEND_ACTION_AXIS: {
				const int axisPosition = event.joystick.position == Common::JOYAXIS_MIN ?
						Common::JOYAXIS_MAX : event.joystick.position;
				switch (event.customType) {
				case kScummBackendActionRebel2AxisUp:
					if (event.joystick.position != 0 || joystickAxisY <= 0)
						joystickAxisY = -axisPosition;
					break;
				case kScummBackendActionRebel2AxisDown:
					if (event.joystick.position != 0 || joystickAxisY >= 0)
						joystickAxisY = axisPosition;
					break;
				case kScummBackendActionRebel2AxisLeft:
					if (event.joystick.position != 0 || joystickAxisX <= 0)
						joystickAxisX = -axisPosition;
					break;
				case kScummBackendActionRebel2AxisRight:
					if (event.joystick.position != 0 || joystickAxisX >= 0)
						joystickAxisX = axisPosition;
					break;
				default:
					break;
				}
				break;
			}
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				_vm->quitGame();
				result = kLevel1Quit;
				break;
			default:
				break;
			}
		}
		if (toggleViewRequested) {
			thirdPersonView = !thirdPersonView;
			ship.velocityX = ship.velocityY = 0;
			aimX = 160;
			aimY = 113;
			aimVelocityX = aimVelocityY = 0;
			aimDirectionX = aimDirectionY = 0;
			g_system->warpMouse(160, thirdPersonView ? 120 : aimY);
			redraw = true;
		}
		if (result == kLevel1Quit)
			break;

		while (decoder.needsUpdate()) {
			background = decoder.decodeNextFrame();
			if (!background) {
				result = kLevel1Error;
				break;
			}
			videoFrame = decoder.getCurFrame();
		}
		if (result == kLevel1Error)
			break;
		soundPlayer.update();

		const uint32 elapsed = g_system->getMillis() - gameplayStartTime;
		const int targetLogicFrame = (int)((uint64)elapsed * kLevel1FrameRate / 1000);
		while (logicFrame < targetLogicFrame && shield > 0) {
			++logicFrame;
			redraw = true;
			const bool left = moveLeft || actionLeft || joystickAxisX < -joystickDeadzone;
			const bool right = moveRight || actionRight || joystickAxisX > joystickDeadzone;
			const bool up = moveUp || actionUp || joystickAxisY < -joystickDeadzone;
			const bool down = moveDown || actionDown || joystickAxisY > joystickDeadzone;
			if (thirdPersonView) {
				updateLevel1Ship(ship, left, right, up, down);
			} else {
				updateLevel1Aim(aimX, aimY, aimVelocityX, aimVelocityY,
						aimDirectionX, aimDirectionY,
						left, right, up, down);
			}

			if (logicFrame >= nextSpawnAdjustment) {
				nextSpawnAdjustment = logicFrame + 20;
				spawnRange = MAX(40, spawnRange - 1);
				spawnBase = MAX(20, spawnBase - 1);
			}

			int activeEnemies = 0;
			for (int i = 0; i < 3; ++i)
				activeEnemies += enemies[i].active ? 1 : 0;
			--spawnDelay;
			if (videoFrame < 1599 && activeEnemies < 3 && spawnDelay <= 0) {
				int spawnedEnemy = -1;
				for (int i = 0; i < 3; ++i) {
					if (!enemies[i].active) {
						spawnLevel1Enemy(enemies[i], _vm->_rnd);
						spawnedEnemy = i;
						break;
					}
				}
				if (spawnedEnemy >= 0) {
					const uint16 sfx = _vm->_rnd.getRandomNumber(999) < 800 ? 0x19 : 0x1a;
					const int rate = 0x1c18 + _vm->_rnd.getRandomNumber(1999);
					approachSounds[spawnedEnemy] = soundPlayer.play(sfx, 0x5e, 0x40, rate);
				}
				spawnDelay = spawnBase + _vm->_rnd.getRandomNumber(spawnRange - 1);
			}

			updateLevel1Shots(shots);
			for (int i = 0; i < 3; ++i) {
				if (explosions[i].frames > 0)
					--explosions[i].frames;
				if (!enemies[i].active)
					continue;

				if (enemies[i].laserFrames > 0)
					--enemies[i].laserFrames;
				++enemies[i].age;
				updateLevel1Enemy(enemies[i]);
				const int soundPan = getLevel1SoundPan(enemies[i].x);
				soundPlayer.setPan(approachSounds[i], soundPan);
				if (enemies[i].age == enemies[i].fireFrame) {
					enemies[i].laserFrames = 4;
					soundPlayer.play(0x17, 0x4e, soundPan);
					if (_vm->_rnd.getRandomNumber(99) < 38) {
						shield = MAX(0, shield - (int)_vm->_rnd.getRandomNumberRng(6, 10));
						soundPlayer.play(0x36, 0x7f, 0x40);
					}
				}
				if (enemies[i].age >= enemies[i].lifetime) {
					const int rate = _vm->_rnd.getRandomNumber(0x3fff);
					soundPlayer.play(0x1b, 0x5a, soundPan, rate);
					soundPlayer.stop(approachSounds[i]);
					approachSounds[i] = RA2PSXSoundPlayer::kInvalidSoundId;
					enemies[i].active = false;
					if (_vm->_rnd.getRandomNumber(99) < 18) {
						shield = MAX(0, shield - 12);
						soundPlayer.play(0x36, 0x7f, 0x40);
					}
				}
			}

			const bool heldFire = mouseFire || keyFire || actionFire;
			const bool shootRequested = fireRequested ||
					(heldFire && logicFrame - lastShotFrame >= 12);
			fireRequested = false;
			if (shootRequested && logicFrame - lastShotFrame >= 4) {
				int shotTargetX;
				int shotTargetY;
				if (!spawnLevel1Shot(shots, aimX, aimY,
						thirdPersonView ? &ship : nullptr, shotTargetX, shotTargetY))
					continue;
				soundPlayer.play(0x18, 0x3f, 0x40);
				lastShotFrame = logicFrame;
				int hitEnemy = -1;
				float hitDistance = 1000000.0f;
				for (int i = 0; i < 3; ++i) {
					if (!enemies[i].active)
						continue;
					const float dx = enemies[i].x - shotTargetX;
					const float dy = enemies[i].y - shotTargetY;
					const float distance = dx * dx + dy * dy;
					const float radius = MAX(10.0f, enemies[i].size * 0.72f);
					if (distance <= radius * radius && distance < hitDistance) {
						hitEnemy = i;
						hitDistance = distance;
					}
				}
				if (hitEnemy >= 0) {
					const int soundPan = getLevel1SoundPan(enemies[hitEnemy].x);
					const int rate = _vm->_rnd.getRandomNumber(0x3fff);
					soundPlayer.play(0x1b, 0x5a, soundPan, rate);
					soundPlayer.stop(approachSounds[hitEnemy]);
					approachSounds[hitEnemy] = RA2PSXSoundPlayer::kInvalidSoundId;
					enemies[hitEnemy].active = false;
					score = MIN(9999999, score + 100);
					for (int i = 0; i < 3; ++i) {
						if (explosions[i].frames == 0) {
							explosions[i].frames = 10;
							explosions[i].x = (int)enemies[hitEnemy].x;
							explosions[i].y = (int)enemies[hitEnemy].y;
							break;
						}
					}
				}
			}
		}

		if (shield <= 0) {
			result = kLevel1Death;
			break;
		}

		if (background && redraw) {
			renderer.beginFrame(*background);
			for (int i = 0; i < 3; ++i) {
				if (enemies[i].active)
					renderer.renderModel(enemyModel, enemies[i].x, enemies[i].y, enemies[i].size,
							enemies[i].pitch, enemies[i].yaw, enemies[i].roll);
			}
			renderLevel1Shots(renderer, laser, shots);
			if (thirdPersonView) {
				float forwardX;
				float forwardY;
				float forwardZ;
				float shipRoll;
				getLevel1ShipOrientation(ship, forwardX, forwardY, forwardZ, shipRoll);
				renderer.renderPerspectiveModel(shipModel, ship.x, ship.y, ship.z,
						forwardX, forwardY, forwardZ, shipRoll, false);
			} else {
				renderer.renderModel(crosshair, aimX, aimY, 31.0f,
						0.0f, 0.0f, 0.0f, false);
			}
			Graphics::Surface output;
			renderer.finishFrame(output);
			const int laserTargetX = thirdPersonView ? 160 + ship.x * 640 / ship.z : 160;
			const int laserTargetY = thirdPersonView ? 120 + ship.y * 640 / ship.z : output.h - 1;
			drawLevel1Effects(output, ui, enemies, explosions, laserTargetX, laserTargetY);
			if (!thirdPersonView)
				ui.drawCockpit(output);
			ui.drawHUD(output, score, lives, shield, logicFrame);
			g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0, output.w, output.h);
			g_system->updateScreen();
		}
		g_system->delayMillis(5);
	}

	soundPlayer.stopAll();
	decoder.close();
	CursorMan.showMouse(cursorWasVisible);
	return _vm->shouldQuit() ? kLevel1Quit : result;
#endif
}

} // End of namespace Scumm
