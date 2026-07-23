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
	RA2PSXLevel1Shot() : active(false), progress(0), targetX(0), targetY(0) {}

	bool active;
	int progress;
	float targetX;
	float targetY;
};

static const float kLevel1LaserStart[3][3] = {
	{ -350.0f, 200.0f, 400.0f },
	{ 350.0f, 200.0f, 400.0f },
	{ 0.0f, 500.0f, 400.0f }
};

static const float kLevel1LaserRoll[3] = { -45.0f, 45.0f, 0.0f };

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

static void spawnLevel1Shot(RA2PSXLevel1Shot *shots, int aimX, int aimY) {
	int slot = -1;
	for (int i = 0; i < 8; ++i) {
		if (!shots[i].active) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return;

	shots[slot].active = true;
	shots[slot].progress = 400;
	shots[slot].targetX = (aimX - 160) * 18000.0f / 640.0f;
	shots[slot].targetY = (aimY - 120) * 18000.0f / 640.0f;
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
			const float *start = kLevel1LaserStart[laserIndex];
			const float directionX = shot.targetX - start[0];
			const float directionY = shot.targetY - start[1];
			const float directionZ = 18000.0f - start[2];
			renderer.renderPerspectiveModel(laser,
					start[0] + directionX * progress,
					start[1] + directionY * progress,
					start[2] + directionZ * progress,
					directionX, directionY, directionZ, kLevel1LaserRoll[laserIndex], false);
		}
	}
}

static void drawLevel1Effects(Graphics::Surface &surface, const RA2PSXLevel1UI &ui,
		const RA2PSXLevel1Enemy *enemies, const RA2PSXLevel1Explosion *explosions) {
	const uint32 green = surface.format.RGBToColor(64, 255, 96);

	for (int i = 0; i < 3; ++i) {
		if (enemies[i].active && enemies[i].laserFrames > 0) {
			surface.drawLine((int)enemies[i].x - 2, (int)enemies[i].y,
					145, surface.h - 1, green);
			surface.drawLine((int)enemies[i].x + 2, (int)enemies[i].y,
					175, surface.h - 1, green);
		}
		if (explosions[i].frames > 0)
			ui.drawExplosion(surface, explosions[i].x, explosions[i].y,
					10 - explosions[i].frames);
	}
}
#endif

Rebel2PSX::Level1Result Rebel2PSX::playLevel1(const RA2PSXModel &model,
		const RA2PSXModel &crosshair, const RA2PSXModel &laser,
		const RA2PSXLevel1UI &ui, int lives, int &score) {
#ifndef USE_TINYGL
	(void)model;
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
	int lastFrame = -1;
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
	Level1Result result = kLevel1Complete;
	const int joystickDeadzone = MIN<int>(Common::JOYAXIS_MAX,
			MAX(0, ConfMan.getInt("joystick_deadzone")) * 1000);

	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(false);
	g_system->warpMouse(aimX, aimY);
	decoder.start();

	while (!_vm->shouldQuit() && !decoder.endOfVideo()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				aimX = CLIP<int>(event.mouse.x, 30, 290);
				aimY = CLIP<int>(event.mouse.y, 48, 178);
				aimVelocityX = aimVelocityY = 0;
				aimDirectionX = aimDirectionY = 0;
				break;
			case Common::EVENT_LBUTTONDOWN:
				aimX = CLIP<int>(event.mouse.x, 30, 290);
				aimY = CLIP<int>(event.mouse.y, 48, 178);
				mouseFire = true;
				fireRequested = true;
				break;
			case Common::EVENT_LBUTTONUP:
				mouseFire = false;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					result = kLevel1Quit;
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
		if (result == kLevel1Quit)
			break;

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (!frame) {
				result = kLevel1Error;
				break;
			}

			const int currentFrame = decoder.getCurFrame();
			while (lastFrame < currentFrame && shield > 0) {
				++lastFrame;
				updateLevel1Aim(aimX, aimY, aimVelocityX, aimVelocityY,
						aimDirectionX, aimDirectionY,
						moveLeft || actionLeft || joystickAxisX < -joystickDeadzone,
						moveRight || actionRight || joystickAxisX > joystickDeadzone,
						moveUp || actionUp || joystickAxisY < -joystickDeadzone,
						moveDown || actionDown || joystickAxisY > joystickDeadzone);

				if (lastFrame >= nextSpawnAdjustment) {
					nextSpawnAdjustment = lastFrame + 20;
					spawnRange = MAX(40, spawnRange - 1);
					spawnBase = MAX(20, spawnBase - 1);
				}

				int activeEnemies = 0;
				for (int i = 0; i < 3; ++i)
					activeEnemies += enemies[i].active ? 1 : 0;
				--spawnDelay;
				if (lastFrame < 1599 && activeEnemies < 3 && spawnDelay <= 0) {
					for (int i = 0; i < 3; ++i) {
						if (!enemies[i].active) {
							spawnLevel1Enemy(enemies[i], _vm->_rnd);
							break;
						}
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
					if (enemies[i].age == enemies[i].fireFrame) {
						enemies[i].laserFrames = 4;
						if (_vm->_rnd.getRandomNumber(99) < 38)
							shield = MAX(0, shield - (int)_vm->_rnd.getRandomNumberRng(6, 10));
					}
					if (enemies[i].age >= enemies[i].lifetime) {
						enemies[i].active = false;
						if (_vm->_rnd.getRandomNumber(99) < 18)
							shield = MAX(0, shield - 12);
					}
				}
			}

			if (shield <= 0) {
				result = kLevel1Death;
				break;
			}

			const bool heldFire = mouseFire || keyFire || actionFire;
			const bool shootRequested = fireRequested ||
					(heldFire && currentFrame - lastShotFrame >= 12);
			fireRequested = false;
			if (shootRequested && currentFrame - lastShotFrame >= 4) {
				lastShotFrame = currentFrame;
				spawnLevel1Shot(shots, aimX, aimY);
				int hitEnemy = -1;
				float hitDistance = 1000000.0f;
				for (int i = 0; i < 3; ++i) {
					if (!enemies[i].active)
						continue;
					const float dx = enemies[i].x - aimX;
					const float dy = enemies[i].y - aimY;
					const float distance = dx * dx + dy * dy;
					const float radius = MAX(10.0f, enemies[i].size * 0.72f);
					if (distance <= radius * radius && distance < hitDistance) {
						hitEnemy = i;
						hitDistance = distance;
					}
				}
				if (hitEnemy >= 0) {
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

			renderer.beginFrame(*frame);
			for (int i = 0; i < 3; ++i) {
				if (enemies[i].active)
					renderer.renderModel(model, enemies[i].x, enemies[i].y, enemies[i].size,
							enemies[i].pitch, enemies[i].yaw, enemies[i].roll);
			}
			renderLevel1Shots(renderer, laser, shots);
			const float crosshairX = (aimX - 160) * 620.0f / 640.0f;
			const float crosshairY = (aimY - 120) * 620.0f / 640.0f;
			renderer.renderModel(crosshair, aimX, aimY, 31.0f,
					crosshairY * 720.0f / 4096.0f,
					-crosshairX * 720.0f / 4096.0f, 0.0f, false);
			Graphics::Surface output;
			renderer.finishFrame(output);
			drawLevel1Effects(output, ui, enemies, explosions);
			ui.drawCockpit(output);
			ui.drawHUD(output, score, lives, shield, currentFrame);
			g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0, output.w, output.h);
			g_system->updateScreen();
		}
		g_system->delayMillis(5);
	}

	decoder.close();
	CursorMan.showMouse(cursorWasVisible);
	return _vm->shouldQuit() ? kLevel1Quit : result;
#endif
}

} // End of namespace Scumm
