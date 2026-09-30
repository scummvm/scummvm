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

static const char *const kDemoFlights[] = {
	"RESOURCE/DEMO/DEMO_S1.SAN",
	"RESOURCE/DEMO/DEMO_PL.SAN",
	"RESOURCE/DEMO/DEMO_BT.SAN"
};

MortimerEngine::MortimerEngine(OSystem *system) : Engine(system),
	_mouse(320, 180), _view(32, 24), _nextFrameTime(0), _pauseTime(0),
	_segment(0), _completedRescues(0), _completedSpecies(0), _shellShocks(0), _finished(false) {
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
		{"RESOURCE/GENERAL/COCKPT_H.NUT", &_cockpit, 3},
		{"RESOURCE/GENERAL/DASH_H.NUT", &_dashboard, 36},
		{"RESOURCE/GENERAL/RETICLEH.NUT", &_reticles, 2},
		{"RESOURCE/SAVANNA/S_ICON_H.NUT", &_icons, 14}
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
		{"RESOURCE/GENERAL/SFX/MUCKHIT1.SAD", &_slimeHitSound}
	};
	for (uint i = 0; i < ARRAYSIZE(effects); ++i) {
		if (!file.open(Common::Path(effects[i].name)) || !loadMortimerSound(file, *effects[i].sound)) {
			_loadError = Common::String::format("Missing or invalid %s", effects[i].name);
			return false;
		}
		file.close();
	}
	return loadFlight(0);
}

bool MortimerEngine::loadFlight(uint segment) {
	// The mixer borrows the samples owned by the flight being replaced.
	stopSounds();
	_flightHandles.clear();
	Common::File file;
	if (!file.open(Common::Path(kDemoFlights[segment]))) {
		_loadError = Common::String::format("Missing %s", kDemoFlights[segment]);
		return false;
	}
	if (!_flight.load(file)) {
		_loadError = Common::String::format("%s: %s", kDemoFlights[segment], _flight.errorMessage().c_str());
		return false;
	}
	_segment = segment;
	_flightHandles.resize(_flight.sounds().size());
	return true;
}

void MortimerEngine::playSound(const MortimerSound &sound, Audio::SoundHandle &handle, Audio::Mixer::SoundType type) {
	_mixer->stopHandle(handle);
	// The engine owns the samples until all its handles have been stopped.
	Audio::AudioStream *stream = Audio::makeRawStream(sound.samples.begin(), sound.samples.size(), sound.rate,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	_mixer->playStream(type, &handle, stream, -1, sound.volume, sound.balance);
}

void MortimerEngine::playFlightSounds() {
	for (uint i = 0; i < _flight.sounds().size(); ++i)
		if (_flight.sounds()[i].frame == _flight.frame())
			playSound(_flight.sounds()[i], _flightHandles[i],
				_flight.sounds()[i].music ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSpeechSoundType);
}

void MortimerEngine::stopSounds() {
	for (uint i = 0; i < _flightHandles.size(); ++i)
		_mixer->stopHandle(_flightHandles[i]);
	_mixer->stopHandle(_scopeHandle);
	_mixer->stopHandle(_rescueHandle);
	_mixer->stopHandle(_collisionHandle);
}

bool MortimerEngine::startFlight() {
	stopSounds();
	_flight.reset();
	if (!_flight.advance()) {
		_loadError = _flight.errorMessage();
		return false;
	}
	_finished = false;
	setMouse(Common::Point(320, 180));
	_system->warpMouse(_mouse.x, _mouse.y);
	_system->getPaletteManager()->setPalette(_flight.palette(), 0, 256);
	_nextFrameTime = _system->getMillis() + 1000 / _flight.frameRate();
	playFlightSounds();
	return true;
}

bool MortimerEngine::restart(bool all) {
	_shellShocks = 0;
	if (all) {
		_completedRescues = _completedSpecies = 0;
		if (!loadFlight(0))
			return false;
	}
	return startFlight();
}

bool MortimerEngine::nextFlight(bool skip) {
	stopSounds();
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
	// A bounded view into the overscanned movie supplies steering. Exact
	// acceleration, collision damage and scope behaviour remain POC rules.
	_view.x = CLIP<int>(32 + (_mouse.x - 320) / 8, 0, MortimerFlight::kWidth - 320);
	_view.y = CLIP<int>(24 + (_mouse.y - 180) / 4, 0, MortimerFlight::kHeight - 240);
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
	bool exit = !restart();
	while (!shouldQuit() && !exit) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_MOUSEMOVE) {
				if (!isPaused())
					setMouse(event.mouse);
			} else if (event.type == Common::EVENT_LBUTTONDOWN && !isPaused() && !_finished) {
				setMouse(event.mouse);
				if (_mouse.x >= 54 && _mouse.x < 567 && _mouse.y >= 37 && _mouse.y < 320) {
					playSound(_flight.weapon() == MortimerFlight::kSlimeGun ? _slimeSound : _scopeSound,
						_scopeHandle, Audio::Mixer::kSFXSoundType);
					const MortimerFlight::ShotResult result = _flight.shoot(Common::Point(_view.x + _mouse.x / 2, _view.y + _mouse.y / 2));
					if (result == MortimerFlight::kAnimalRescued || result == MortimerFlight::kShellRestored)
						playSound(_rescueSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
					else if (result == MortimerFlight::kHornetHit)
						playSound(_slimeHitSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
					if (result == MortimerFlight::kShellRestored)
						_shellShocks = 0;
				}
			} else if (event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat) {
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (pause.isActive())
						pause.clear();
					else
						exit = !nextFlight(true);
					break;
				case Common::KEYCODE_q:
					exit = true;
					break;
				case Common::KEYCODE_SPACE:
					if (pause.isActive())
						pause.clear();
					else
						pause = pauseEngine();
					break;
				case Common::KEYCODE_r:
					if (pause.isActive())
						pause.clear();
					exit = !restart();
					break;
				case Common::KEYCODE_F5:
					openMainMenuDialog();
					break;
				default:
					break;
				}
			}
		}
		if (exit)
			break;
		if (!isPaused() && !_finished && int32(_system->getMillis() - _nextFrameTime) >= 0) {
			_nextFrameTime += 1000 / _flight.frameRate();
			if (!_flight.advance()) {
				if (!_flight.errorMessage().empty()) {
					_loadError = _flight.errorMessage();
					break;
				}
				if (!nextFlight())
					break;
			} else {
				playFlightSounds();
				_system->getPaletteManager()->setPalette(_flight.palette(), 0, 256);
				if (_flight.hitsObstacle(Common::Point(_view.x + _mouse.x / 2, _view.y + _mouse.y / 2))) {
					++_shellShocks;
					playSound(_collisionSound, _collisionHandle, Audio::Mixer::kSFXSoundType);
					// SHELL 40be36 resets the flight after eighteen shell shocks.
					if (_shellShocks >= 18 && !restart())
						break;
				}
			}
			// Avoid racing through queued frames following a slow backend call.
			if (int32(_system->getMillis() - _nextFrameTime) > 250)
				_nextFrameTime = _system->getMillis() + 1000 / _flight.frameRate();
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

void MortimerEngine::drawSprite(const MortimerSprite &sprite, int x, int y) {
	Common::Rect clip(x, y, x + sprite.width, y + sprite.height);
	clip.clip(Common::Rect(_screen.w, _screen.h));
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

void MortimerEngine::draw() {
	for (int y = 0; y < 480; ++y) {
		const byte *src = _flight.pixels() + (_view.y + y / 2) * MortimerFlight::kWidth + _view.x;
		byte *dst = (byte *)_screen.getBasePtr(0, y);
		for (int x = 0; x < 640; ++x)
			dst[x] = src[x / 2];
	}
	drawSprite(_cockpit[0], 0, _cockpit[0].y);
	drawSprite(_cockpit[1], 640 - _cockpit[1].width, _cockpit[1].y);
	drawSprite(_cockpit[2], 0, 0);
	drawSprite(_dashboard[35], 0, 320);
	const int iconCenterY[] = {384, 379, 377, 376, 378, 380, 384};
	for (uint species = 0; species < 7; ++species) {
		if ((_completedSpecies | _flight.rescuedSpecies()) & (1 << species)) {
			const MortimerSprite &icon = _icons[species * 2];
			drawSprite(icon, 176 + species * 45 - icon.width / 2, iconCenterY[species] - icon.height / 2);
		}
	}
	if (_mouse.y < 320) {
		const MortimerSprite &reticle = _reticles[_flight.weapon() == MortimerFlight::kSlimeGun ? 0 : 1];
		drawSprite(reticle, _mouse.x - reticle.width / 2, _mouse.y - reticle.height / 2);
	}

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	_screen.fillRect(Common::Rect(0, 449, 640, 465), 10);
	font->drawString(&_screen, Common::String::format("Animals rescued: %u   Shell shocks: %u / 18   Flight: %u / 3",
		_completedRescues + _flight.rescuedCount(), _shellShocks, _segment + 1), 8, 451, 624, 20);
	if (isPaused()) {
		_screen.fillRect(Common::Rect(90, 142, 550, 182), 10);
		font->drawString(&_screen, "Paused - Space to continue", 90, 155, 460, 20, Graphics::kTextAlignCenter);
	}
	_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, 640, 480);
	_system->updateScreen();
}

} // End of namespace Scumm
