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

#include "scumm/insane/mortimer/mortimer.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "backends/keymapper/action.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/paletteman.h"
#include "gui/message.h"

namespace Scumm {

// SHELL's demo scene list at 444220 plays all five introductions before
// TITLE.SAN. The title continues directly into the first playable scene.
static const char *const kDemoIntros[] = {
	"RESOURCE/BEGIN/INTRO1_H.SAN",
	"RESOURCE/BEGIN/INTRO2_H.SAN",
	"RESOURCE/BEGIN/INTRO3_H.SAN",
	"RESOURCE/BEGIN/INTRO4_H.SAN",
	"RESOURCE/BEGIN/INTRO5_H.SAN"
};

static const char *const kDemoFlights[] = {
	"RESOURCE/DEMO/TITLE.SAN",
	"RESOURCE/DEMO/DEMO_PL.SAN",
	"RESOURCE/DEMO/DEMO_BT.SAN"
};

enum MortimerAction {
	kActionFire, kActionPause, kActionSkip, kActionRestart, kActionQuit,
	kActionMenu, kActionMouse, kActionJoystick
};

Common::KeymapArray MortimerEngine::initKeymaps() {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "mortimer", "Mortimer");
	keymap->setPartialMatchAllowed(false);
	struct Binding {
		const char *id, *label;
		MortimerAction action;
		const char *inputs[4];
	};
	const Binding bindings[] = {
		{"FIRE", _s("Fire"), kActionFire, {"MOUSE_LEFT", "MOUSE_RIGHT", "JOY_A", "JOY_B"}},
		{"PAUSE", _s("Pause"), kActionPause, {"SPACE", "JOY_START", nullptr, nullptr}},
		{"SKIP", _s("Skip"), kActionSkip, {"ESCAPE", "JOY_Y", nullptr, nullptr}},
		{"RESTART", _s("Restart"), kActionRestart, {"r", "JOY_X", nullptr, nullptr}},
		{"QUIT", _s("Quit"), kActionQuit, {"q", "A+q", nullptr, nullptr}},
		{"MENU", _s("Game menu"), kActionMenu, {"F5", "JOY_LEFT_SHOULDER", nullptr, nullptr}},
		{"MOUSE", _s("Mouse control"), kActionMouse, {"A+m", nullptr, nullptr, nullptr}},
		{"JOYSTICK", _s("Joystick control"), kActionJoystick, {"A+j", nullptr, nullptr, nullptr}}
	};
	for (uint i = 0; i < ARRAYSIZE(bindings); ++i) {
		Common::Action *action = new Common::Action(bindings[i].id, _(bindings[i].label));
		action->setCustomEngineActionEvent(bindings[i].action);
		for (uint j = 0; j < ARRAYSIZE(bindings[i].inputs); ++j)
			if (bindings[i].inputs[j])
				action->addDefaultInputMapping(bindings[i].inputs[j]);
		if (bindings[i].action == kActionFire)
			action->addDefaultInputMapping("RETURN");
		keymap->addAction(action);
	}
	const char *const directions[] = {"LEFT", "RIGHT", "UP", "DOWN"};
	const char *const labels[] = {_s("Left"), _s("Right"), _s("Up"), _s("Down")};
	const char *const axes[] = {"JOY_LEFT_STICK_X-", "JOY_LEFT_STICK_X+", "JOY_LEFT_STICK_Y-", "JOY_LEFT_STICK_Y+"};
	const char *const keypad[] = {"KP4", "KP6", "KP8", "KP2"};
	for (uint i = 0; i < ARRAYSIZE(directions); ++i) {
		Common::Action *action = new Common::Action(directions[i], _(labels[i]));
		Common::Event event;
		event.type = Common::EVENT_JOYAXIS_MOTION;
		event.joystick.axis = i;
		event.joystick.position = 0;
		action->setEvent(event);
		action->addDefaultInputMapping(directions[i]);
		action->addDefaultInputMapping(keypad[i]);
		action->addDefaultInputMapping(Common::String("JOY_") + directions[i]);
		action->addDefaultInputMapping(axes[i]);
		keymap->addAction(action);
	}
	Common::KeymapArray keymaps;
	keymaps.push_back(keymap);
	return keymaps;
}

MortimerEngine::MortimerEngine(OSystem *system) : Engine(system),
	_viewport(24, 20, 616, 360), _mouse(320, 240), _view(44, 60), _aim(320, 190), _shotWeapon(MortimerFlight::kRevitascope),
	_nextFrameTime(0), _pauseTime(0), _introIndex(-1), _segment(0), _completedRescues(0), _completedSpecies(0),
	_shellShocks(0), _uiFrame(0), _shotFrame(3), _glowFrame(10), _glowStones(0),
	_pendingGlowStones(0), _finished(false), _controllerMode(false) {
	memset(_steering, 0, sizeof(_steering));
	memset(_iconFlash, 0, sizeof(_iconFlash));
}

MortimerEngine::~MortimerEngine() {
	stopSounds();
	_screen.free();
}

bool MortimerEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher;
}

bool MortimerEngine::loadResources() {
	Common::File file;

	struct SpriteBank {
		const char *name;
		Common::Array<MortimerSprite> *sprites;
		uint minFrames;
	};
	const SpriteBank banks[] = {
		{"RESOURCE/GENERAL/COCKPT_H.NUT", &_cockpit, 14},
		{"RESOURCE/GENERAL/DASH_H.NUT", &_dashboard, 36},
		{"RESOURCE/GENERAL/RETICLEH.NUT", &_reticles, 2},
		{"RESOURCE/SAVANNA/S_ICON_H.NUT", &_icons, 14},
		{"RESOURCE/GENERAL/MHEAD_H.NUT", &_head, 4},
		{"RESOURCE/GENERAL/REVITE.NUT", &_scopeHit, 11},
		{"RESOURCE/GENERAL/SLIME_H.NUT", &_slimeHit, 7},
		{"RESOURCE/GENERAL/LINE_BLU.NUT", &_scopeBeam, 1},
		{"RESOURCE/GENERAL/SLMBALL.NUT", &_slimeBall, 1},
		{"RESOURCE/GENERAL/NUMBER_H.NUT", &_numbers, 11},
		{"RESOURCE/GENERAL/CORNER_H.NUT", &_corners, 2}
	};
	for (uint i = 0; i < ARRAYSIZE(banks); ++i) {
		if (!file.open(Common::Path(banks[i].name)) || !loadMortimerSprites(file, *banks[i].sprites) || banks[i].sprites->size() < banks[i].minFrames) {
			_loadError = Common::String::format("Missing or invalid %s", banks[i].name);
			return false;
		}
		file.close();
	}
	struct Effect {
		const char *name;
		MortimerSound *sound;
	};
	const Effect effects[] = {
		{"RESOURCE/GENERAL/SFX/REVITAL2.SAD", &_scopeSound},
		{"RESOURCE/GENERAL/SFX/SCOPHIT3.SAD", &_rescueSound},
		{"RESOURCE/GENERAL/SFX/SHELHIT1.SAD", &_collisionSound},
		{"RESOURCE/GENERAL/SFX/SLMGUN2.SAD", &_slimeSound},
		{"RESOURCE/GENERAL/SFX/MUCKHIT1.SAD", &_slimeHitSound},
		{"RESOURCE/GENERAL/SFX/GLOWIN1.SAD", &_glowSound},
		{"RESOURCE/GENERAL/SFX/BURST1.SAD", &_balloonSound}
	};
	for (uint i = 0; i < ARRAYSIZE(effects); ++i) {
		if (!file.open(Common::Path(effects[i].name)) || !loadMortimerSound(file, *effects[i].sound)) {
			_loadError = Common::String::format("Missing or invalid %s", effects[i].name);
			return false;
		}
		file.close();
	}
	return true;
}

MortimerMovie &MortimerEngine::currentMovie() {
	return _introIndex >= 0 ? _intro : _flight;
}

const MortimerMovie &MortimerEngine::currentMovie() const {
	return _introIndex >= 0 ? _intro : _flight;
}

bool MortimerEngine::loadIntro(uint index) {
	stopSounds();
	_movieHandles.clear();
	Common::File file;
	if (!file.open(Common::Path(kDemoIntros[index]))) {
		_loadError = Common::String::format("Missing %s", kDemoIntros[index]);
		return false;
	}
	// The fifth introduction starts with native 640 x 480 animation.
	if (!_intro.load(file, index != 4)) {
		_loadError = Common::String::format("%s: %s", kDemoIntros[index], _intro.errorMessage().c_str());
		return false;
	}
	_introIndex = index;
	_movieHandles.resize(_intro.sounds().size());
	return true;
}

bool MortimerEngine::loadFlight(uint segment) {
	// The mixer borrows the samples owned by the movie being replaced.
	stopSounds();
	_movieHandles.clear();
	Common::File file;
	if (!file.open(Common::Path(kDemoFlights[segment]))) {
		_loadError = Common::String::format("Missing %s", kDemoFlights[segment]);
		return false;
	}
	if (!_flight.load(file, segment != 0)) {
		_loadError = Common::String::format("%s: %s", kDemoFlights[segment], _flight.errorMessage().c_str());
		return false;
	}
	_segment = segment;
	_introIndex = -1;
	_movieHandles.resize(_flight.sounds().size());
	return true;
}

void MortimerEngine::playSound(const MortimerSound &sound, Audio::SoundHandle &handle, Audio::Mixer::SoundType type, uint offset) {
	_mixer->stopHandle(handle);
	// The engine owns the samples until all its handles have been stopped.
	Audio::AudioStream *stream = Audio::makeRawStream(sound.samples.begin() + offset, sound.samples.size() - offset, sound.rate,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	_mixer->playStream(type, &handle, stream, -1, sound.volume, sound.balance);
}

void MortimerEngine::playMovieSounds(bool seek) {
	const MortimerMovie &movie = currentMovie();
	for (uint i = 0; i < movie.sounds().size(); ++i) {
		const MortimerSound &sound = movie.sounds()[i];
		if (sound.frame > movie.frame() || (!seek && sound.frame != movie.frame()))
			continue;
		const uint64 offset = uint64(movie.frame() - sound.frame) * sound.rate / movie.frameRate();
		if (offset < sound.samples.size())
			playSound(sound, _movieHandles[i], sound.music ? Audio::Mixer::kMusicSoundType :
				(sound.speech ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType), offset);
	}
}

void MortimerEngine::stopSounds() {
	for (uint i = 0; i < _movieHandles.size(); ++i)
		_mixer->stopHandle(_movieHandles[i]);
	_mixer->stopHandle(_scopeHandle);
	_mixer->stopHandle(_rescueHandle);
	_mixer->stopHandle(_collisionHandle);
	_mixer->stopHandle(_glowHandle);
}

bool MortimerEngine::startMovie() {
	stopSounds();
	MortimerMovie &movie = currentMovie();
	movie.reset();
	if (!movie.advance()) {
		_loadError = movie.errorMessage();
		return false;
	}
	_finished = false;
	_system->getPaletteManager()->setPalette(movie.palette(), 0, 256);
	_nextFrameTime = _system->getMillis() + 1000 / movie.frameRate();
	playMovieSounds();
	return true;
}

bool MortimerEngine::startFlight() {
	_uiFrame = 0;
	_shotFrame = 3;
	_glowFrame = 10;
	_glowStones = MIN(_completedRescues, 999U);
	_pendingGlowStones = 0;
	memset(_iconFlash, 0, sizeof(_iconFlash));
	for (uint i = 0; i < ARRAYSIZE(_steeringHistory); ++i)
		_steeringHistory[i] = Common::Point();
	setMouse(Common::Point(320, 240));
	updateSteering();
	_system->warpMouse(_mouse.x, _mouse.y);
	return startMovie();
}

bool MortimerEngine::restart(bool all) {
	if (_introIndex >= 0 && !all)
		return startMovie();
	_shellShocks = 0;
	if (all) {
		_completedRescues = _completedSpecies = 0;
		if (!loadFlight(0))
			return false;
	}
	return startFlight();
}

bool MortimerEngine::nextScene(bool skip) {
	stopSounds();
	if (_introIndex >= 0) {
		if (uint(_introIndex + 1) < ARRAYSIZE(kDemoIntros))
			return loadIntro(_introIndex + 1) && startMovie();
		return loadFlight(0) && startFlight();
	}
	if (skip && !_flight.interactive()) {
		// Keep TITLE's music at the matching position when skipping its
		// opening; the same track continues after the cockpit appears.
		while (!_flight.interactive()) {
			if (!_flight.advance()) {
				_loadError = _flight.errorMessage().empty() ? "Mortimer title has no gameplay transition" : _flight.errorMessage();
				return false;
			}
		}
		_system->getPaletteManager()->setPalette(_flight.palette(), 0, 256);
		_nextFrameTime = _system->getMillis() + 1000 / _flight.frameRate();
		playMovieSounds(true);
		return true;
	}
	if (_segment + 1 == ARRAYSIZE(kDemoFlights)) {
		_finished = true;
		return true;
	}
	// Only follow the demo's two flight branches, never an interview or a
	// full-game resource named by another SAN file.
	if (!skip && !_flight.nextFlight().equalsIgnoreCase(kDemoFlights[_segment + 1])) {
		_loadError = Common::String::format("Unexpected flight transition in %s", kDemoFlights[_segment]);
		return false;
	}
	_completedRescues += _flight.rescuedCount();
	_completedSpecies |= _flight.rescuedSpecies();
	return loadFlight(_segment + 1) && startFlight();
}

void MortimerEngine::setMouse(const Common::Point &position) {
	_mouse.x = CLIP<int>(position.x, 0, 639);
	_mouse.y = CLIP<int>(position.y, 0, 479);
}

void MortimerEngine::updateSteering() {
	int x = (_mouse.x - 320) / 2, y = 240 - _mouse.y;
	if (_controllerMode) {
		x = (_steering[1] - _steering[0]) * 128 / Common::JOYAXIS_MAX;
		y = (_steering[2] - _steering[3]) * 128 / Common::JOYAXIS_MAX;
	}
	// SHELL 40db90 averages ten input samples before moving the viewport.
	const int weights[] = {1, 1, 3, 3, 8, 8, 14, 16, 16, 14};
	int averageX = 0, averageY = 0;
	for (uint i = 0; i < ARRAYSIZE(weights); ++i) {
		_steeringHistory[i] = i + 1 < ARRAYSIZE(weights) ? _steeringHistory[i + 1] : Common::Point(x, y);
		averageX += _steeringHistory[i].x * weights[i];
		averageY += _steeringHistory[i].y * weights[i];
	}
	averageX /= 84;
	averageY /= 84;
	// The movie is doubled, and its translation is aligned to four pixels.
	_view.x = ((88 + CLIP(averageX * 88 / 128, -88, 88) + 3) & ~3) / 2;
	_view.y = ((118 - CLIP(averageY * 118 / 128, -118, 118) + 3) & ~3) / 2;
	if (ABS(x) < 3)
		x = 0;
	if (ABS(y) < 3)
		y = 0;
	_aim.x = 320 + CLIP(x * 296 / 109, -296, 295);
	_aim.y = 190 - CLIP(y * 170 / 128, -169, 170);
}

Common::Point MortimerEngine::moviePoint(const Common::Point &point) const {
	return Common::Point(_view.x + (point.x - _viewport.left) / 2, _view.y + (point.y - _viewport.top) / 2);
}

Common::Point MortimerEngine::screenPoint(const Common::Point &point) const {
	return Common::Point(_viewport.left + (point.x - _view.x) * 2, _viewport.top + (point.y - _view.y) * 2);
}

void MortimerEngine::fire() {
	if (_shotFrame >= 3) {
		_shot = _aim;
		_shotWeapon = _flight.weapon();
		_shotFrame = 0;
		playSound(_shotWeapon == MortimerFlight::kSlimeGun ? _slimeSound : _scopeSound,
			_scopeHandle, Audio::Mixer::kSFXSoundType);
	}
	const MortimerFlight::ShotResult result = _flight.shoot(moviePoint(_aim));
	if (result == MortimerFlight::kAnimalRescued) {
		playSound(_rescueSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
		++_pendingGlowStones;
		if (_glowFrame >= 10) {
			_glowFrame = 0;
			playSound(_glowSound, _glowHandle, Audio::Mixer::kSFXSoundType);
		}
		for (uint i = 0; i < _flight.targets().size(); ++i) {
			const MortimerFlight::Target &target = _flight.targets()[i];
			if (target.hit && target.rescuedAt == _flight.frame() && target.id / 100 < 7)
				_iconFlash[target.id / 100] = 6;
		}
	} else if (result == MortimerFlight::kHornetHit) {
		playSound(_slimeHitSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
	} else if (result == MortimerFlight::kShellRestored) {
		_shellShocks = 0;
		playSound(_balloonSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
	}
}

void MortimerEngine::updateInterface() {
	++_uiFrame;
	if (_shotFrame < 3)
		++_shotFrame;
	for (uint i = 0; i < ARRAYSIZE(_iconFlash); ++i)
		if (_iconFlash[i])
			--_iconFlash[i];
	if (_glowFrame < 10 && ++_glowFrame == 10) {
		_glowStones = MIN(_glowStones + _pendingGlowStones, 999U);
		_pendingGlowStones = 0;
	}
}

bool MortimerEngine::speechPlaying() const {
	for (uint i = 0; i < _movieHandles.size(); ++i)
		if (currentMovie().sounds()[i].speech && _mixer->isSoundHandleActive(_movieHandles[i]))
			return true;
	return false;
}

void MortimerEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause)
		_pauseTime = _system->getMillis();
	else
		_nextFrameTime += _system->getMillis() - _pauseTime;
}

Common::Error MortimerEngine::run() {
	if (!loadResources())
		return Common::Error(Common::kReadingFailed, _loadError);
	initGraphics(640, 480);
	_screen.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	syncSoundSettings();
	const bool oldCursor = CursorMan.showMouse(false);
	PauseToken pause;
	bool exit = !(loadIntro(0) && startMovie());
	while (!shouldQuit() && !exit) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_MOUSEMOVE) {
				if (!isPaused()) {
					setMouse(event.mouse);
					_controllerMode = false;
				}
			} else if (event.type == Common::EVENT_JOYAXIS_MOTION && event.joystick.axis < ARRAYSIZE(_steering)) {
				_steering[event.joystick.axis] = CLIP<int>(event.joystick.position, 0, Common::JOYAXIS_MAX);
				if (event.joystick.position && !isPaused())
					_controllerMode = true;
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				switch (event.customType) {
				case kActionFire:
					if (!isPaused() && !_finished && interactive())
						fire();
					break;
				case kActionSkip:
					if (pause.isActive())
						pause.clear();
					else
						exit = !nextScene(true);
					break;
				case kActionQuit:
					exit = true;
					break;
				case kActionPause:
					if (pause.isActive())
						pause.clear();
					else
						pause = pauseEngine();
					break;
				case kActionRestart:
					if (pause.isActive())
						pause.clear();
					exit = !restart();
					break;
				case kActionMenu:
					openMainMenuDialog();
					break;
				case kActionMouse:
					_controllerMode = false;
					setMouse(_eventMan->getMousePos());
					break;
				case kActionJoystick:
					_controllerMode = true;
					break;
				default:
					break;
				}
			}
		}
		if (exit)
			break;
		if (!isPaused() && !_finished && int32(_system->getMillis() - _nextFrameTime) >= 0) {
			MortimerMovie &movie = currentMovie();
			_nextFrameTime += 1000 / movie.frameRate();
			if (interactive()) {
				updateSteering();
				updateInterface();
			}
			if (!movie.advance()) {
				if (!movie.errorMessage().empty()) {
					_loadError = movie.errorMessage();
					break;
				}
				if (!nextScene())
					break;
			} else {
				playMovieSounds();
				_system->getPaletteManager()->setPalette(movie.palette(), 0, 256);
				if (interactive() && _flight.hitsObstacle(moviePoint(_aim))) {
					++_shellShocks;
					playSound(_collisionSound, _collisionHandle, Audio::Mixer::kSFXSoundType);
					// SHELL 40be36 resets the flight after eighteen shell shocks.
					if (_shellShocks >= 18 && !restart())
						break;
				}
			}
			// Avoid racing through queued frames following a slow backend call.
			if (int32(_system->getMillis() - _nextFrameTime) > 250)
				_nextFrameTime = _system->getMillis() + 1000 / currentMovie().frameRate();
		}
		draw();
		if (_finished) {
			GUI::MessageDialog dialog(_("Play again?"), _("Restart"), _("Quit"));
			if (dialog.runModal() != GUI::kMessageOK || !restart(true))
				break;
		}
		_system->delayMillis(10);
	}
	stopSounds();
	CursorMan.showMouse(oldCursor);
	return _loadError.empty() ? Common::kNoError : Common::Error(Common::kReadingFailed, _loadError);
}

void MortimerEngine::drawSprite(const MortimerSprite &sprite, int x, int y, bool centered, const Common::Rect *bounds) {
	// NUT offsets are relative to the animation's origin, including when its
	// frames have different bounding boxes (SHELL 402c83/4141bc).
	if (centered) {
		x -= (sprite.x + sprite.width) / 2;
		y -= (sprite.y + sprite.height) / 2;
	}
	x += sprite.x;
	y += sprite.y;
	Common::Rect clip(x, y, x + sprite.width, y + sprite.height);
	clip.clip(Common::Rect(_screen.w, _screen.h));
	if (bounds)
		clip.clip(*bounds);
	if (clip.isEmpty())
		return;
	for (int row = clip.top; row < clip.bottom; ++row) {
		const byte *src = sprite.pixels.begin() + (row - y) * sprite.width + clip.left - x;
		byte *dst = (byte *)_screen.getBasePtr(clip.left, row);
		for (int col = clip.left; col < clip.right; ++col, ++src, ++dst)
			if (*src)
				*dst = *src;
	}
}

void MortimerEngine::drawTargets() {
	for (uint i = 0; i < _flight.targets().size(); ++i) {
		const MortimerFlight::Target &target = _flight.targets()[i];
		if (!target.active || !target.id || target.id >= 800)
			continue;
		if (target.hit) {
			const Common::Array<MortimerSprite> &effect = target.hitWeapon == MortimerFlight::kSlimeGun ? _slimeHit : _scopeHit;
			const uint frame = _flight.frame() - target.rescuedAt;
			if (frame < effect.size()) {
				const Common::Point center = screenPoint(Common::Point((target.rect.left + target.rect.right) / 2,
					(target.rect.top + target.rect.bottom) / 2));
				drawSprite(effect[frame], center.x, center.y, true, &_viewport);
			}
		} else if ((target.id < 700 && _flight.weapon() == MortimerFlight::kRevitascope) ||
			(target.id >= 700 && _flight.weapon() == MortimerFlight::kSlimeGun)) {
			const Common::Point top = screenPoint(Common::Point(target.rect.left, target.rect.top));
			const Common::Point bottom = screenPoint(Common::Point(target.rect.right, target.rect.bottom));
			drawSprite(_corners[0], top.x, top.y, true, &_viewport);
			drawSprite(_corners[1], bottom.x, bottom.y, true, &_viewport);
		}
	}
}

void MortimerEngine::drawProjectile() {
	if (_shotFrame >= 3)
		return;
	const bool slime = _shotWeapon == MortimerFlight::kSlimeGun;
	const MortimerSprite &texture = slime ? _slimeBall[0] : _scopeBeam[0];
	const Common::Point origin(slime ? 599 : 40, 400);
	// SHELL 403b60 projects a textured streak in three steps from the weapon
	// towards the sight. Its far endpoint is 7/6 of the distance to the aim.
	const int dx = (_shot.x - origin.x) * 7 / 6, dy = (_shot.y - origin.y) * 7 / 6;
	const int distance = 32 * _shotFrame + 16;
	const Common::Point tail(origin.x + dx - dx * 16 / distance, origin.y + dy - dy * 16 / distance);
	const Common::Point tip(origin.x + dx - dx * 16 / (distance + 25), origin.y + dy - dy * 16 / (distance + 25));
	const int lengthX = ABS(tip.x - tail.x), lengthY = ABS(tip.y - tail.y);
	const bool horizontal = lengthX > lengthY;
	const int length = MAX(lengthX, lengthY);
	const int thickness = MIN(horizontal ? 8 : texture.height, length * texture.height / texture.width + 2);
	for (int row = 0; row < thickness; ++row) {
		const byte *src = texture.pixels.begin() + (row * texture.height / thickness) * texture.width;
		for (int step = 0; step <= length; ++step) {
			const int x = tail.x + (tip.x - tail.x) * step / MAX(length, 1) + (horizontal ? 0 : row - thickness / 2);
			const int y = tail.y + (tip.y - tail.y) * step / MAX(length, 1) + (horizontal ? row - thickness / 2 : 0);
			const byte color = src[step * texture.width / (length + 1)];
			if (color && x >= 0 && x < _screen.w && y >= 0 && y < _screen.h)
				*(byte *)_screen.getBasePtr(x, y) = color;
		}
	}
}

void MortimerEngine::drawDashboard() {
	// Origins and frame ranges are from SHELL's animation/position tables
	// at 441240, 442d20 and 444958. drawSprite adds each frame's own offset.
	drawSprite(_dashboard[35], 0, 0);
	drawSprite(_dashboard[29 + _uiFrame % 6], 0, 439);
	drawSprite(_dashboard[speechPlaying() ? 16 + _uiFrame % 3 : 16], 21, 312);
	const uint gauge = _shellShocks < 16 ? 19 + _shellShocks / 2 : 27 + _uiFrame % 2;
	drawSprite(_dashboard[gauge], 180, 442);
	const Common::Point iconOrigins[] = {{158, 364}, {203, 358}, {248, 358}, {293, 354}, {339, 355}, {383, 360}, {428, 361}};
	const Common::Point flashOrigins[] = {{154, 345}, {198, 338}, {243, 334}, {288, 332}, {333, 334}, {378, 338}, {422, 345}};
	for (uint species = 0; species < 7; ++species) {
		if (_iconFlash[species] & 1)
			drawSprite(_dashboard[species], flashOrigins[species].x, flashOrigins[species].y);
		const bool rescued = (_completedSpecies | _flight.rescuedSpecies()) & (1 << species);
		drawSprite(_icons[species * 2 + (rescued ? 0 : 1)], iconOrigins[species].x, iconOrigins[species].y);
	}
	const uint thresholds[] = {1, 2, 3, 4, 7, 20, 30, 40, 50};
	for (int i = ARRAYSIZE(thresholds) - 1; i >= 0; --i) {
		if (_glowStones >= thresholds[i]) {
			drawSprite(_dashboard[7 + i], 468, 368);
			break;
		}
	}
	drawSprite(_numbers[_glowStones < 100 ? 10 : _glowStones / 100], 504, 360);
	drawSprite(_numbers[_glowStones < 10 ? 10 : (_glowStones / 10) % 10], 517, 361);
	drawSprite(_numbers[_glowStones % 10], 529, 363);
	if (_glowFrame < 10)
		drawSprite(_cockpit[4 + _glowFrame], 392, 272);
	if (_flight.weapon() == MortimerFlight::kSlimeGun)
		drawSprite(_cockpit[3], 306, 465, true);
}

void MortimerEngine::draw() {
	if (interactive()) {
		_screen.fillRect(Common::Rect(640, 480), 10);
		for (int y = _viewport.top; y < _viewport.bottom; ++y) {
			const byte *src = _flight.pixels() + (_view.y + (y - _viewport.top) / 2) * MortimerMovie::kScreenWidth + _view.x;
			byte *dst = (byte *)_screen.getBasePtr(_viewport.left, y);
			for (int x = 0; x < _viewport.width(); ++x)
				dst[x] = src[x / 2];
		}
		drawTargets();
		drawSprite(_head[_uiFrame % 4], 270, 200);
		drawSprite(_reticles[_flight.weapon() == MortimerFlight::kSlimeGun ? 0 : 1], _aim.x, _aim.y, true, &_viewport);
		drawSprite(_cockpit[0], 0, 0);
		drawSprite(_cockpit[1], 567, 0);
		drawSprite(_cockpit[2], 0, 0);
		drawDashboard();
		drawProjectile();
	} else {
		_screen.copyRectToSurface(currentMovie().pixels(), MortimerMovie::kScreenWidth, 0, 0, 640, 480);
	}
	if (isPaused()) {
		const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
		_screen.fillRect(Common::Rect(90, 142, 550, 182), 10);
		font->drawString(&_screen, _("Paused"), 90, 155, 460, 20, Graphics::kTextAlignCenter);
	}
	_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, 640, 480);
	_system->updateScreen();
}

} // End of namespace Scumm
