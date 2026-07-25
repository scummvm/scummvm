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
#include "common/system.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/shared.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"
#include "scumm/insane/rebel2/psx/video.h"

namespace Scumm {

// The menus run one logic step per vertical blank.
const int kMenuFrameRate = 60;

// Sound effect ids the menus use.
enum {
	kMenuSfxMove = 0x1d,
	kMenuSfxAdjust = 0x1e,
	kMenuSfxEnter = 0x1f,
	kMenuSfxLeave = 0x20,
	kMenuSfxConfirm = 0x40,
	kMenuSfxLocked = 0x21,
	kMenuSfxStart = 0x44,
	kMenuSfxDefaults = 0x45
};

void RA2PSXSettings::reset() {
	difficulty = 1;
	sfx = 0x54;
	music = 0xc00;
	movies = 0xc00;
	mono = false;
	for (int i = 0; i < ARRAYSIZE(unlocked); ++i)
		unlocked[i] = 1;
	unlockAll = false;
}

void RA2PSXSettings::load() {
	reset();
	if (ConfMan.hasKey("rebel2_difficulty"))
		difficulty = CLIP(ConfMan.getInt("rebel2_difficulty"), 0, 2);
	if (ConfMan.hasKey("rebel2_mono"))
		mono = ConfMan.getBool("rebel2_mono");
	if (ConfMan.hasKey("sfx_volume"))
		sfx = CLIP(ConfMan.getInt("sfx_volume"), 0, 255) * kSFXMaximum / 255 /
				kSFXStep * kSFXStep;
	if (ConfMan.hasKey("music_volume"))
		music = CLIP(ConfMan.getInt("music_volume"), 0, 255) * kCDMaximum / 255 /
				kCDStep * kCDStep;
	if (ConfMan.hasKey("speech_volume"))
		movies = CLIP(ConfMan.getInt("speech_volume"), 0, 255) * kCDMaximum / 255 /
				kCDStep * kCDStep;
	unlockAll = ConfMan.getBool("rebel2_unlock_all");
	for (int i = 0; i < ARRAYSIZE(unlocked); ++i) {
		const Common::String key = Common::String::format("rebel2_psx_chapters%d", i);
		if (ConfMan.hasKey(key))
			unlocked[i] = CLIP(ConfMan.getInt(key), 1, 16);
	}
}

void RA2PSXSettings::save() const {
	ConfMan.setInt("rebel2_difficulty", difficulty);
	ConfMan.setBool("rebel2_mono", mono);
	ConfMan.setInt("sfx_volume", sfx * 255 / kSFXMaximum);
	ConfMan.setInt("music_volume", MIN<int>(255, music * 255 / kCDMaximum));
	ConfMan.setInt("speech_volume", MIN<int>(255, movies * 255 / kCDMaximum));
	for (int i = 0; i < ARRAYSIZE(unlocked); ++i)
		ConfMan.setInt(Common::String::format("rebel2_psx_chapters%d", i), unlocked[i]);
}

void RA2PSXSettings::apply(ScummEngine_v7 *vm) const {
	save();
	vm->syncSoundSettings();
}

int RA2PSXSettings::unlockedChapters() const {
	return unlockAll ? 16 : CLIP<int>(unlocked[CLIP(difficulty, 0, 2)], 1, 16);
}

byte RA2PSXSettings::videoVolume() const {
	return (byte)CLIP<int>(movies * 255 / kCDMaximum, 0, 255);
}

#ifdef USE_TINYGL

// A full screen quad in the GPU's subtract mode: level 0 is untouched, 0xff is black.
struct RA2PSXMenuFade {
	RA2PSXMenuFade() : level(0), step(0), active(false) {}

	void fadeIn(int amount) {
		level = 0xff;
		step = amount;
		active = true;
	}

	void fadeOut(int amount) {
		level = 0;
		step = -amount;
		active = true;
	}

	// Returns true on the step that completes a fade to black.
	bool update() {
		if (!active || !step)
			return false;
		level -= step;
		if (step > 0) {
			if (level <= 0) {
				level = 0;
				step = 0;
				active = false;
			}
			return false;
		}
		if (level < 0x100)
			return false;
		level = 0xff;
		step = 0;
		return true;
	}

	void apply(Graphics::Surface &surface) const {
		if (active && level > 0)
			subtractRA2PSXRect(surface, Common::Rect(surface.w, surface.h),
					level, level, level);
	}

	int level;
	int step;
	bool active;
};

struct RA2PSXMenuEvents {
	RA2PSXMenuEvents() : up(false), down(false), left(false), right(false), accept(false),
			cancel(false), globalMenu(false), mouseMoved(false), mouseClicked(false),
			mouseX(-1), mouseY(-1) {}

	bool up;
	bool down;
	bool left;
	bool right;
	bool accept;
	bool cancel;
	bool globalMenu;
	bool mouseMoved;
	bool mouseClicked;
	int mouseX;
	int mouseY;
};

void pollMenuEvents(ScummEngine_v7 *vm, RA2PSXMenuEvents &events) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			vm->quitGame();
			return;
		case Common::EVENT_MAINMENU:
			events.globalMenu = true;
			break;
		case Common::EVENT_LBUTTONDOWN:
			events.mouseClicked = true;
			// fall through
		case Common::EVENT_MOUSEMOVE:
			events.mouseMoved = true;
			events.mouseX = event.mouse.x;
			events.mouseY = event.mouse.y;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbdRepeat)
				break;
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				events.globalMenu = true;
				break;
			case Common::KEYCODE_LEFT:
				events.left = true;
				break;
			case Common::KEYCODE_RIGHT:
				events.right = true;
				break;
			case Common::KEYCODE_SPACE:
				events.accept = true;
				break;
			case Common::KEYCODE_BACKSPACE:
				events.cancel = true;
				break;
			default:
				switch (getRebel2MenuCommand(event.kbd)) {
				case kRebel2MenuCommandUp:
					events.up = true;
					break;
				case kRebel2MenuCommandDown:
					events.down = true;
					break;
				case kRebel2MenuCommandAccept:
					events.accept = true;
					break;
				default:
					break;
				}
				break;
			}
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kScummActionInsaneUp:
				events.up = true;
				break;
			case kScummActionInsaneDown:
				events.down = true;
				break;
			case kScummActionInsaneLeft:
				events.left = true;
				break;
			case kScummActionInsaneRight:
				events.right = true;
				break;
			case kScummActionInsaneAttack:
				events.accept = true;
				break;
			case kScummActionInsaneBack:
			case kScummActionInsaneSkip:
				events.cancel = true;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

int hitTestMenu(Common::Rect (*itemRect)(int), int count, const Graphics::Surface &surface,
		int x, int y) {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	for (int item = 0; item < count; ++item) {
		Common::Rect rect = itemRect(item);
		rect.translate(xOffset, yOffset);
		if (rect.contains(x, y))
			return item;
	}
	return -1;
}

// The options list and its "adjust sound" page; the other four screens stay dimmed.
void Rebel2PSX::runOptionsMenu(const RA2PSXOptionsUI &ui, RA2PSXSoundPlayer &sound,
		RA2PSXTinyGLRenderer &renderer) {
	enum Screen {
		kScreenMain,
		kScreenSound
	};
	enum Dialog {
		kDialogNone,
		kDialogResetConfirm,
		kDialogDefaultsRestored
	};

	Graphics::Surface background;
	background.create(_vm->_screenWidth, _vm->_screenHeight, g_system->getScreenFormat());
	background.fillRect(Common::Rect(background.w, background.h), 0);

	Screen screen = kScreenMain;
	Dialog dialog = kDialogNone;
	int selection = RA2PSXOptionsUI::kItemDifficulty;
	int soundSelection = RA2PSXOptionsUI::kSoundItemMode;
	int confirmSelection = 0;
	int messageFrames = 0;
	int cloakAngle = 0x800;
	int logicFrame = -1;
	bool leaving = false;
	RA2PSXMenuFade fade;
	fade.fadeIn(8);

	// Accumulates across iterations; a press between logic steps must not be lost.
	RA2PSXMenuEvents events;
	const uint32 startTime = g_system->getMillis();
	while (!_vm->shouldQuit()) {
		pollMenuEvents(_vm, events);
		if (_vm->shouldQuit())
			break;
		if (events.globalMenu) {
			_vm->openMainMenuDialog();
			_settings.load();
			events = RA2PSXMenuEvents();
			continue;
		}

		const uint32 elapsed = g_system->getMillis() - startTime;
		const int targetFrame = (int)((uint64)elapsed * kMenuFrameRate / 1000);
		if (logicFrame >= targetFrame) {
			g_system->delayMillis(5);
			continue;
		}

		bool fadedOut = false;
		while (logicFrame < targetFrame) {
			++logicFrame;
			cloakAngle = (cloakAngle + 6) & 0xfff;
			fadedOut |= fade.update();
			if (dialog == kDialogDefaultsRestored && --messageFrames <= 0)
				dialog = kDialogNone;
		}
		if (fadedOut && leaving)
			break;

		sound.update();
		if (!fade.active && dialog == kDialogResetConfirm) {
			if (events.left || events.right || events.up || events.down) {
				confirmSelection ^= 1;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (events.cancel) {
				dialog = kDialogNone;
				sound.play(kMenuSfxLeave, 0x7f, 0x40);
			} else if (events.accept) {
				if (confirmSelection == 1) {
					_settings.reset();
					_settings.apply(_vm);
					selection = RA2PSXOptionsUI::kItemDifficulty;
					dialog = kDialogDefaultsRestored;
					messageFrames = 0x78;
					sound.play(kMenuSfxDefaults, 0x7f, 0x40);
				} else {
					dialog = kDialogNone;
					sound.play(kMenuSfxLeave, 0x7f, 0x40);
				}
			}
		} else if (!fade.active && dialog == kDialogNone && screen == kScreenMain) {
			const int hit = hitTestMenu(RA2PSXOptionsUI::mainItemRect,
					RA2PSXOptionsUI::kItemCount, background, events.mouseX, events.mouseY);
			if (events.mouseMoved && hit >= 0 && hit != selection &&
					RA2PSXOptionsUI::isItemAvailable(hit)) {
				selection = hit;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (events.up || events.down) {
				const int step = events.down ? 1 : RA2PSXOptionsUI::kItemCount - 1;
				do {
					selection = (selection + step) % RA2PSXOptionsUI::kItemCount;
				} while (!RA2PSXOptionsUI::isItemAvailable(selection));
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (selection == RA2PSXOptionsUI::kItemDifficulty && (events.left || events.right)) {
				_settings.difficulty = (_settings.difficulty + (events.right ? 1 : 2)) % 3;
				_settings.save();
				sound.play(kMenuSfxAdjust, 0x7f, 0x40);
			}
			if (events.cancel) {
				leaving = true;
				sound.play(kMenuSfxLeave, 0x7f, 0x40);
				fade.fadeOut(8);
			} else if (events.accept || (events.mouseClicked && hit == selection)) {
				switch (selection) {
				case RA2PSXOptionsUI::kItemAdjustSound:
					screen = kScreenSound;
					soundSelection = RA2PSXOptionsUI::kSoundItemMode;
					sound.play(kMenuSfxEnter, 0x7f, 0x40);
					break;
				case RA2PSXOptionsUI::kItemResetSettings:
					dialog = kDialogResetConfirm;
					confirmSelection = 0;
					sound.play(kMenuSfxConfirm, 0x7f, 0x40);
					break;
				case RA2PSXOptionsUI::kItemExit:
					leaving = true;
					sound.play(kMenuSfxLeave, 0x7f, 0x40);
					fade.fadeOut(8);
					break;
				default:
					break;
				}
			}
		} else if (!fade.active && dialog == kDialogNone) {
			const int hit = hitTestMenu(RA2PSXOptionsUI::soundItemRect,
					RA2PSXOptionsUI::kSoundItemCount, background,
					events.mouseX, events.mouseY);
			if (events.mouseMoved && hit >= 0 && hit != soundSelection) {
				soundSelection = hit;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (events.up || events.down) {
				soundSelection = (soundSelection + (events.down ? 1 :
						RA2PSXOptionsUI::kSoundItemCount - 1)) %
						RA2PSXOptionsUI::kSoundItemCount;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (events.left || events.right) {
				const int direction = events.right ? 1 : -1;
				bool changed = true;
				switch (soundSelection) {
				case RA2PSXOptionsUI::kSoundItemMode:
					_settings.mono = !_settings.mono;
					break;
				case RA2PSXOptionsUI::kSoundItemEffects:
					_settings.sfx = CLIP<int>(_settings.sfx +
							direction * RA2PSXSettings::kSFXStep,
							0, RA2PSXSettings::kSFXMaximum);
					break;
				case RA2PSXOptionsUI::kSoundItemMusic:
					_settings.music = CLIP<int>(_settings.music +
							direction * RA2PSXSettings::kCDStep,
							0, RA2PSXSettings::kCDMaximum);
					break;
				case RA2PSXOptionsUI::kSoundItemMovies:
					_settings.movies = CLIP<int>(_settings.movies +
							direction * RA2PSXSettings::kCDStep,
							0, RA2PSXSettings::kCDMaximum);
					break;
				default:
					changed = false;
					break;
				}
				if (changed) {
					_settings.apply(_vm);
					sound.play(kMenuSfxAdjust, 0x7f, 0x40);
				}
			}
			if (events.cancel || ((events.accept || (events.mouseClicked && hit == soundSelection)) &&
					soundSelection == RA2PSXOptionsUI::kSoundItemExit)) {
				screen = kScreenMain;
				sound.play(kMenuSfxLeave, 0x7f, 0x40);
			}
		}

		events = RA2PSXMenuEvents();
		renderer.beginFrame(background);
		RA2PSXMatrix transform;
		transform.setRotationZ(-0x100);
		transform.preRotateY(cloakAngle);
		transform.setTranslation(0, 0, 0x604);
		renderer.renderTransformedModel(_cloakModel, transform, false);

		Graphics::Surface output;
		renderer.finishFrame(output);
		if (screen == kScreenMain)
			ui.drawMain(output, selection, _settings);
		else
			ui.drawSound(output, soundSelection, _settings);
		if (dialog == kDialogResetConfirm)
			ui.drawDialog(output, "reset settings", "are you sure?", nullptr, confirmSelection);
		else if (dialog == kDialogDefaultsRestored)
			ui.drawDialog(output, nullptr, "default values", "restored", -1);
		fade.apply(output);
		g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0, output.w, output.h);
		g_system->updateScreen();
	}

	background.free();
	_settings.save();
	ConfMan.flushToDisk();
}

int Rebel2PSX::runChapterSelect(const RA2PSXChapterSelectUI &ui) {
	RA2PSXTinyGLRenderer renderer;
	if (!renderer.init(_vm->_screenWidth, _vm->_screenHeight))
		return 0;

	// The tile previews are frames of LEVELSEL.STR, looped for as long as the
	// screen is up; without them the tiles stay empty.
	Common::SeekableReadStream *stream = openRawFile("LEVELSEL.STR", 1);
	RA2PSXStreamDecoder decoder(RA2PSXStreamDecoder::kVersion2);
	bool previewsReady = stream && decoder.loadStream(stream) &&
			decoder.setOutputPixelFormat(g_system->getScreenFormat());
	if (previewsReady) {
		decoder.setVolume(_settings.videoVolume());
		decoder.start();
	} else {
		decoder.close();
	}

	Graphics::Surface background;
	background.create(_vm->_screenWidth, _vm->_screenHeight, g_system->getScreenFormat());
	background.fillRect(Common::Rect(background.w, background.h), 0);

	RA2PSXSoundPlayer sound(_vm, _soundBank);
	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(true);

	const int unlocked = _settings.unlockedChapters();
	// Kept in our own surface so it survives restarting the stream.
	Graphics::Surface previews;
	bool havePreviews = false;
	int selection = 0;
	int chosen = 0;
	int scroll = 0;
	int target = 0;
	int velocity = 0;
	int direction = 1;
	int cloakAngle = 0x800;
	int crestAngle = 0;
	int logicFrame = -1;
	bool leaving = false;
	RA2PSXMenuFade fade;
	fade.fadeIn(8);

	// Accumulates across iterations; a press between logic steps must not be lost.
	RA2PSXMenuEvents events;
	const uint32 startTime = g_system->getMillis();
	while (!_vm->shouldQuit()) {
		pollMenuEvents(_vm, events);
		if (_vm->shouldQuit())
			break;
		if (events.globalMenu) {
			_vm->openMainMenuDialog();
			events = RA2PSXMenuEvents();
			continue;
		}

		if (previewsReady) {
			while (decoder.needsUpdate()) {
				const Graphics::Surface *frame = decoder.decodeNextFrame();
				if (!frame || frame->w < 320 || frame->h < 240)
					break;
				if (!previews.getPixels())
					previews.create(320, 240, frame->format);
				previews.copyRectToSurface(*frame, 0, 0, Common::Rect(320, 240));
				havePreviews = true;
			}
			// The clip only runs for about twelve seconds, so start it over.
			if (decoder.endOfVideo()) {
				decoder.close();
				stream = openRawFile("LEVELSEL.STR", 1);
				previewsReady = stream && decoder.loadStream(stream) &&
						decoder.setOutputPixelFormat(g_system->getScreenFormat());
				if (previewsReady) {
					decoder.setVolume(_settings.videoVolume());
					decoder.start();
				} else {
					decoder.close();
				}
			}
		}

		const uint32 elapsed = g_system->getMillis() - startTime;
		const int targetFrame = (int)((uint64)elapsed * kMenuFrameRate / 1000);
		if (logicFrame >= targetFrame) {
			g_system->delayMillis(5);
			continue;
		}

		bool fadedOut = false;
		while (logicFrame < targetFrame) {
			++logicFrame;
			cloakAngle = (cloakAngle + 6) & 0xfff;
			crestAngle = (crestAngle + 0x14) & 0xfff;
			fadedOut |= fade.update();

			// The list eases towards the selected row and never overshoots.
			if (scroll != target) {
				scroll += (MIN(velocity, 0x8000) >> 12) * direction;
				if (direction > 0 ? scroll >= target : scroll <= target) {
					scroll = target;
					velocity = 0;
				}
			}
			if (velocity) {
				velocity = velocity * 0xe80 >> 12;
				if (velocity < 0x1000)
					velocity = 0x1000;
			}
		}
		if (fadedOut && leaving)
			break;

		sound.update();
		if (!fade.active) {
			int mouseHit = -1;
			if (events.mouseMoved) {
				const int xOffset = (background.w - 320) / 2;
				const int yOffset = (background.h - 240) / 2;
				for (int chapter = 0; chapter < RA2PSXChapterSelectUI::kChapterCount; ++chapter) {
					Common::Rect rect = RA2PSXChapterSelectUI::tileRect(chapter, scroll);
					rect.translate(xOffset, yOffset);
					if (rect.contains(events.mouseX, events.mouseY)) {
						mouseHit = chapter;
						break;
					}
				}
			}

			int step = 0;
			if (events.down && selection < RA2PSXChapterSelectUI::kChapterCount - 1)
				step = 1;
			else if (events.up && selection > 0)
				step = -1;
			else if (mouseHit >= 0 && mouseHit != selection)
				step = mouseHit > selection ? 1 : -1;
			if (step) {
				const int rows = mouseHit >= 0 && !events.up && !events.down ?
						ABS(mouseHit - selection) : 1;
				selection += step * rows;
				target += step * rows * RA2PSXChapterSelectUI::kRowPitch;
				direction = step;
				velocity += 0x8000;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}

			const bool activate = events.accept ||
					(events.mouseClicked && mouseHit == selection);
			if (events.cancel) {
				leaving = true;
				sound.play(kMenuSfxLeave, 0x7f, 0x40);
				fade.fadeOut(8);
			} else if (activate) {
				if (selection >= unlocked) {
					// The original just refuses a locked chapter with a buzz.
					sound.play(kMenuSfxLocked, 0x7f, 0x40);
				} else {
					chosen = selection + 1;
					leaving = true;
					sound.play(kMenuSfxStart, 0x7f, 0x40);
					fade.fadeOut(8);
				}
			}
		}

		events = RA2PSXMenuEvents();
		renderer.beginFrame(background);
		RA2PSXMatrix transform;
		transform.setRotationZ(-0x100);
		transform.preRotateY(cloakAngle);
		transform.setTranslation(0, 0, 0x604);
		renderer.renderTransformedModel(_cloakModel, transform, false);
		transform.setScale(0x6f5, 0x6f5, 0x6f5);
		transform.preRotateY(crestAngle);
		transform.setTranslation(-0x120, -0xd6, 0x604);
		renderer.renderTransformedModel(_crestModel, transform, false);

		Graphics::Surface output;
		renderer.finishFrame(output);
		ui.draw(output, havePreviews ? &previews : nullptr, scroll, selection, unlocked);
		fade.apply(output);
		g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0, output.w, output.h);
		g_system->updateScreen();
	}

	sound.stopAll();
	decoder.close();
	previews.free();
	background.free();
	CursorMan.showMouse(cursorWasVisible);
	return _vm->shouldQuit() ? 0 : chosen;
}

Rebel2PSX::MenuResult Rebel2PSX::runMainMenu(const RA2PSXMainMenuUI &ui,
		const RA2PSXOptionsUI &options) {
	enum Action {
		kActionNone,
		kActionStart,
		kActionOptions
	};

	RA2PSXTinyGLRenderer renderer;
	if (!renderer.init(_vm->_screenWidth, _vm->_screenHeight))
		return kMenuQuit;

	Graphics::Surface background;
	background.create(_vm->_screenWidth, _vm->_screenHeight, g_system->getScreenFormat());
	background.fillRect(Common::Rect(background.w, background.h), 0);
	ui.drawBackground(background);

	RA2PSXSoundPlayer sound(_vm, _soundBank);
	const bool cursorWasVisible = CursorMan.isVisible();
	CursorMan.showMouse(true);

	MenuResult result = kMenuQuit;
	Action action = kActionNone;
	int selection = 0;
	int logoAngle = 0;
	int logicFrame = -1;
	int inputDelay = 10;
	RA2PSXMenuFade fade;
	fade.fadeIn(0x10);
	uint32 startTime = g_system->getMillis();

	// Accumulates across iterations; a press between logic steps must not be lost.
	RA2PSXMenuEvents events;
	while (!_vm->shouldQuit()) {
		pollMenuEvents(_vm, events);
		if (_vm->shouldQuit())
			break;
		if (events.globalMenu) {
			_vm->openMainMenuDialog();
			_settings.load();
			events = RA2PSXMenuEvents();
			continue;
		}

		const uint32 elapsed = g_system->getMillis() - startTime;
		const int targetFrame = (int)((uint64)elapsed * kMenuFrameRate / 1000);
		if (logicFrame >= targetFrame) {
			g_system->delayMillis(5);
			continue;
		}

		bool fadedOut = false;
		while (logicFrame < targetFrame) {
			++logicFrame;
			// A flat plate: sweeping edge on to edge on and snapping back reads as a spin.
			logoAngle = (logoAngle + 0x14) & 0xffff;
			if (inputDelay)
				--inputDelay;
			fadedOut |= fade.update();
		}

		sound.update();
		if (fadedOut && action == kActionStart) {
			result = kMenuStart;
			break;
		}
		if (fadedOut && action == kActionOptions) {
			runOptionsMenu(options, sound, renderer);
			if (_vm->shouldQuit())
				break;
			action = kActionNone;
			inputDelay = 0x1e;
			logicFrame = -1;
			startTime = g_system->getMillis();
			fade.fadeIn(0x10);
		}

		if (action == kActionNone && !inputDelay) {
			const int hit = hitTestMenu(RA2PSXMainMenuUI::itemRect, 2, background,
					events.mouseX, events.mouseY);
			if (events.mouseMoved && hit >= 0 && hit != selection) {
				selection = hit;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (events.up || events.down) {
				selection ^= 1;
				sound.play(kMenuSfxMove, 0x7f, 0x40);
			}
			if (events.accept || (events.mouseClicked && hit == selection)) {
				action = selection == 0 ? kActionStart : kActionOptions;
				sound.play(selection == 0 ? kMenuSfxStart : kMenuSfxEnter, 0x7f, 0x40);
				fade.fadeOut(0x10);
			}
		}

		events = RA2PSXMenuEvents();
		renderer.beginFrame(background);
		RA2PSXMatrix transform;
		transform.setScale(0x1800, 0x1800, 0x1000);
		transform.preRotateY((int16)((logoAngle & 0x7ff) - 0x400));
		transform.setTranslation(0, 0, 0x604);
		renderer.renderTransformedModel(_logoModel, transform, false);

		Graphics::Surface output;
		renderer.finishFrame(output);
		ui.drawForeground(output, selection);
		fade.apply(output);
		g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0, output.w, output.h);
		g_system->updateScreen();
	}

	sound.stopAll();
	background.free();
	CursorMan.showMouse(cursorWasVisible);
	return _vm->shouldQuit() ? kMenuQuit : result;
}

#else

Rebel2PSX::MenuResult Rebel2PSX::runMainMenu(const RA2PSXMainMenuUI &ui,
		const RA2PSXOptionsUI &options) {
	(void)ui;
	(void)options;
	return kMenuQuit;
}

int Rebel2PSX::runChapterSelect(const RA2PSXChapterSelectUI &ui) {
	(void)ui;
	return 0;
}

#endif

} // End of namespace Scumm
