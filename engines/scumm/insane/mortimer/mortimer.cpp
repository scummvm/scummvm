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
#include "scumm/insane/mortimer/dialogs.h"

#include "audio/audiostream.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
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
const char *const kDemoIntros[] = {
	"RESOURCE/BEGIN/INTRO1_H.SAN",
	"RESOURCE/BEGIN/INTRO2_H.SAN",
	"RESOURCE/BEGIN/INTRO3_H.SAN",
	"RESOURCE/BEGIN/INTRO4_H.SAN",
	"RESOURCE/BEGIN/INTRO5_H.SAN"
};

const char *const kDemoFlights[] = {
	"RESOURCE/DEMO/TITLE.SAN",
	"RESOURCE/DEMO/DEMO_PL.SAN",
	"RESOURCE/DEMO/DEMO_BT.SAN"
};

enum MortimerAction {
	kActionFire, kActionPause, kActionSkip, kActionRestart, kActionQuit,
	kActionMenu, kActionMouse, kActionJoystick
};

enum { kBootProgrammer = 1000 };

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
		{"RESTART", _s("Restart"), kActionRestart, {"C+r", "JOY_X", nullptr, nullptr}},
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

MortimerEngine::MortimerEngine(OSystem *system, bool isDemo) : Engine(system), _isDemo(isDemo),
	_restoredContent(isDemo && ConfMan.getBool("mortimer_restored_content")), _flightDifficulty(0), _riddleDifficulty(0),
	_pilotSlot(-1), _checkpoint(0), _score(0), _checkpointScore(0), _girl(false), _pilotLoadRequested(false),
	_highScoresActive(false), _highScoreEligible(false), _viewingHighScores(false),
	_random("mortimer-flight"), _flight(isDemo, _restoredContent), _interview(isDemo),
	_viewport(24, 20, 616, 360), _mouse(320, 240), _view(44, 60), _aim(320, 190), _shotWeapon(MortimerFlight::kRevitascope),
	_crashStage(kNoCrash), _nextFrameTime(0), _pauseTime(0), _introIndex(-1), _loadedRegion(-1), _creditedRescues(0),
	_segment(0), _carriedStones(0), _completedSpecies(0), _shellShocks(0), _shockFrame(0), _crashCount(0),
	_uiFrame(0), _shotFrame(3), _glowFrame(10), _glowStones(0), _pendingGlowStones(0), _finished(false),
	_controllerMode(false), _interviewActive(false), _castleActive(false), _resourcesLoaded(false), _playingCached(false), _needsRepair(false), _hitJolt(false),
	_cheats(ConfMan.getBool("mortimer_cheats")), _programmer(!isDemo && ConfMan.getBool("mortimer_programmer_easter_egg")), _invulnerable(false) {
	memset(_steering, 0, sizeof(_steering));
	memset(_iconFlash, 0, sizeof(_iconFlash));
}

MortimerEngine::~MortimerEngine() {
	stopSounds();
	_screen.free();
}

bool MortimerEngine::hasFeature(EngineFeature feature) const {
	return feature == kSupportsReturnToLauncher || (!_isDemo && feature == kSupportsLoadingDuringRuntime);
}

bool MortimerEngine::selectDifficulty() {
	if (_isDemo && !_restoredContent)
		return true;
	MortimerDifficultyDialog dialog(_isDemo, _flightDifficulty, _riddleDifficulty, _girl);
	if (runDialog(dialog) != 1 || shouldQuit())
		return false;
	setDifficulty(dialog.flightDifficulty(), dialog.riddleDifficulty());
	_girl = dialog.girl();
	_intro.context().girl = _flight.context().girl = _girl;
	return true;
}

void MortimerEngine::setDifficulty(uint flight, uint riddle) {
	_flightDifficulty = MIN(flight, 2U);
	_riddleDifficulty = MIN(riddle, 2U);
	_flight.setCollisionRadius(_isDemo && !_restoredContent ? 25 : 20 * (_flightDifficulty + 1));
	_interview.setDifficulty(_riddleDifficulty);
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
		{"RESOURCE/GENERAL/SFX/BURST1.SAD", &_balloonSound},
		{"RESOURCE/GENERAL/SFX/BUTTON7.SAD", &_buttonSound},
		{"RESOURCE/GENERAL/SFX/PAYOUT.SAD", &_payoutSound}
	};
	for (uint i = 0; i < ARRAYSIZE(effects); ++i) {
		if (!file.open(Common::Path(effects[i].name)) || !loadMortimerSound(file, *effects[i].sound)) {
			_loadError = Common::String::format("Missing or invalid %s", effects[i].name);
			return false;
		}
		file.close();
	}
	// The hit sound animation uses volume 50 on the original 0..127 scale.
	_collisionSound.volume = 50;
	if (_isDemo && !_interview.loadResources()) {
		_loadError = _interview.errorMessage();
		return false;
	}
	return true;
}

MortimerMovie &MortimerEngine::currentMovie() {
	if (_castleActive)
		return _castle;
	if (_crashStage != kNoCrash || _interviewActive || _introIndex >= 0)
		return _intro;
	return _flight;
}

const MortimerMovie &MortimerEngine::currentMovie() const {
	if (_castleActive)
		return _castle;
	if (_crashStage != kNoCrash || _interviewActive || _introIndex >= 0)
		return _intro;
	return _flight;
}

uint MortimerEngine::introCount() const {
	return ARRAYSIZE(kDemoIntros);
}

bool MortimerEngine::loadIntro(uint index) {
	stopSounds();
	_movieHandles.clear();
	const char *name = kDemoIntros[index];
	Common::File file;
	if (!file.open(Common::Path(name))) {
		_loadError = Common::String::format("Missing %s", name);
		return false;
	}
	// The fifth introduction starts with native 640 x 480 animation.
	if (!_intro.load(file, index != 4)) {
		_loadError = Common::String::format("%s: %s", name, _intro.errorMessage().c_str());
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

bool MortimerEngine::loadSubtitles(const char *name) {
	if (!name || _subtitleName == name)
		return true;
	Common::File file;
	if (!file.open(Common::Path(name)) || !_subtitles.loadText(file)) {
		_loadError = Common::String::format("Missing or invalid %s", name);
		return false;
	}
	_subtitleName = name;
	return true;
}

bool MortimerEngine::loadRegion(uint region) {
	if (_loadedRegion == int(region))
		return true;
	stopSounds();
	if (!_resourcesLoaded) {
		if (!loadResources())
			return false;
		_resourcesLoaded = true;
	}
	const MortimerRegion &resources = mortimerRegion(region);
	const Common::String name = Common::String::format("%s/%c_ICON_H.NUT", resources.directory, resources.prefix);
	Common::File file;
	if (!file.open(Common::Path(name)) || !loadMortimerSprites(file, _icons) || _icons.size() < 14) {
		_loadError = Common::String::format("Missing or invalid %s", name.c_str());
		return false;
	}
	if (!_interview.loadResources(region, _programmer)) {
		_loadError = _interview.errorMessage();
		return false;
	}
	_loadedRegion = region;
	_flight.clearProgress();
	_interview.resetRiddle();
	_carriedStones = _completedSpecies = _creditedRescues = _shellShocks = 0;
	_shockFrame = _crashCount = 0;
	return true;
}

bool MortimerEngine::loadRetailFlight(const Common::String &name, bool cached) {
	stopSounds();
	_movieHandles.clear();
	bool loaded;
	if (cached) {
		// Loading a movie releases its LOAD buffer, so retain our own copy.
		const Common::Array<byte> data = _flight.cachedMovie();
		if (data.size() < 8 || READ_BE_UINT32(data.begin() + 4) != data.size() - 8) {
			_loadError = "Incomplete Mortimer cached flight transition";
			return false;
		}
		Common::MemoryReadStream stream(data.begin(), data.size());
		loaded = _flight.load(stream, true, true);
	} else {
		Common::File file;
		if (!file.open(Common::Path(name))) {
			_loadError = Common::String::format("Missing %s", name.c_str());
			return false;
		}
		loaded = _flight.load(file);
	}
	if (!loaded) {
		_loadError = Common::String::format("%s: %s", name.c_str(), _flight.errorMessage().c_str());
		return false;
	}
	_playingCached = cached;
	_flightName = name;
	_introIndex = -1;
	_movieHandles.resize(_flight.sounds().size());
	return true;
}

bool MortimerEngine::followFlightBranch() {
	const Common::String next = _flight.nextFlight();
	const bool cached = !_flight.cachedMovie().empty();
	_pendingFlight = cached ? next : Common::String();
	return loadRetailFlight(next, cached) && startMovie();
}

void MortimerEngine::updateMovieContext() {
	if (_castleActive) {
		_castle.setSteering(moviePoint(_aim));
		_castle.context().conditions[MortimerMovieContext::kDamaged] = _castle.damage() != 0;
		_intro.context() = _castle.context();
		return;
	}
	_flight.setSteering(moviePoint(_aim));
	if (_isDemo)
		return;
	MortimerMovieContext &context = _flight.context();
	context.conditions[MortimerMovieContext::kHasStones] = _interviewActive ? _interview.stones() != 0 :
		_carriedStones + _flight.rescuedCount() - _creditedRescues != 0;
	context.conditions[MortimerMovieContext::kDamaged] = _shellShocks != 0;
	context.conditions[MortimerMovieContext::kOutOfGuesses] = _interview.outOfGuesses();
	context.riddle = _interview.riddleId();
	_intro.context() = context;
}

bool MortimerEngine::enterCampaignScene() {
	stopSounds();
	if (_highScoresActive) {
		setScoreNameInput(false);
		_highScoresActive = false;
	}
	const MortimerCampaign::Scene &scene = _campaign.scene();
	_interviewActive = false;
	_castleActive = false;
	_viewport = Common::Rect(24, 20, 616, 360);
	_pendingFlight.clear();
	_playingCached = false;
	if (!loadSubtitles(scene.subtitles))
		return false;
	if (scene.kind == MortimerCampaign::kFlight) {
		if (!loadRegion(scene.region))
			return false;
		_flight.context().conditions[MortimerMovieContext::kAtGate] = false;
		return loadRetailFlight(scene.movie) && startFlight();
	}
	if (scene.kind == MortimerCampaign::kInterview) {
		if (!loadRegion(scene.region))
			return false;
		_interviewActive = true;
		_flight.context().conditions[MortimerMovieContext::kAtGate] = true;
		_interview.start(_completedSpecies, _carriedStones, scene.id);
		_glowStones = _carriedStones;
		return processInterview();
	}
	if (scene.kind == MortimerCampaign::kCastle || scene.kind == MortimerCampaign::kCastleRiddle) {
		if (!_resourcesLoaded) {
			if (!loadResources())
				return false;
			_resourcesLoaded = true;
		}
		_castleActive = true;
		_viewport = Common::Rect(8, 104, 632, 448);
		_castle.setProgrammer(_programmer);
		if (!_castle.start(scene.id, _flightDifficulty, _riddleDifficulty, _girl)) {
			_loadError = _castle.errorMessage();
			return false;
		}
		return processCastle();
	}
	_movieHandles.clear();
	const Common::String name = _campaign.movieName(_girl);
	Common::File file;
	if (!file.open(Common::Path(name))) {
		_loadError = Common::String::format("Missing %s", name.c_str());
		return false;
	}
	if (!_intro.load(file, scene.doubled)) {
		_loadError = Common::String::format("%s: %s", name.c_str(), _intro.errorMessage().c_str());
		return false;
	}
	_introIndex = scene.id;
	_movieHandles.resize(_intro.sounds().size());
	if (scene.kind == MortimerCampaign::kCredits) {
		if (!_highScores.load(*_saveFileMan, _targetName + ".hsf")) {
			_loadError = "Missing or invalid Mortimer high-score resources";
			return false;
		}
		_highScores.start(_score, _highScoreEligible);
		_highScoreEligible = false;
		_highScoresActive = true;
		setScoreNameInput(_highScores.enteringName());
	}
	return startMovie();
}

void MortimerEngine::setScoreNameInput(bool enabled) {
	_eventMan->getKeymapper()->setGameKeymapState("mortimer", !enabled);
	if (_system->hasFeature(OSystem::kFeatureVirtualKeyboard))
		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, enabled);
}

bool MortimerEngine::finishHighScores() {
	setScoreNameInput(false);
	_highScoresActive = false;
	if (_viewingHighScores) {
		_finished = true;
		return true;
	}
	_nextFrameTime = _system->getMillis() + 1000 / currentMovie().frameRate();
	return loadSubtitles("RESOURCE/DATA/CREDITS.TRS");
}

void MortimerEngine::playSound(const MortimerSound &sound, Audio::SoundHandle &handle, Audio::Mixer::SoundType type, uint64 skipFrames) {
	_mixer->stopHandle(handle);
	SaudStream *stream = new SaudStream(sound.samples, sound.script, sound.rate, _soundState, sound.volume, sound.balance);
	stream->skip(skipFrames);
	if (stream->endOfData()) {
		delete stream;
		return;
	}
	_mixer->playStream(type, &handle, stream);
}

void MortimerEngine::playMovieSounds(bool seek) {
	if (seek)
		stopSounds();
	const MortimerMovie &movie = currentMovie();
	for (uint i = 0; i < movie.sounds().size(); ++i) {
		const MortimerSound &sound = movie.sounds()[i];
		if (!movie.soundEnabled(i) || sound.frame > movie.frame() || (!seek && sound.frame != movie.frame()))
			continue;
		const uint64 elapsed = uint64(movie.frame() - sound.frame) * SaudStream::kRate / movie.frameRate();
		playSound(sound, _movieHandles[i], sound.music ? Audio::Mixer::kMusicSoundType :
			(sound.speech ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType), elapsed);
	}
}

void MortimerEngine::playCollisionSound() {
	// Each hit queues a new sample; earlier impacts can still be playing.
	uint channel = 0;
	while (channel < _collisionHandles.size() && _mixer->isSoundHandleActive(_collisionHandles[channel]))
		++channel;
	if (channel == _collisionHandles.size())
		_collisionHandles.push_back(Audio::SoundHandle());
	playSound(_collisionSound, _collisionHandles[channel], Audio::Mixer::kSFXSoundType);
}

void MortimerEngine::stopSounds() {
	for (uint i = 0; i < _movieHandles.size(); ++i)
		_mixer->stopHandle(_movieHandles[i]);
	for (uint i = 0; i < _collisionHandles.size(); ++i)
		_mixer->stopHandle(_collisionHandles[i]);
	_mixer->stopHandle(_scopeHandle);
	_mixer->stopHandle(_rescueHandle);
	_mixer->stopHandle(_glowHandle);
	_mixer->stopHandle(_castleHandle);
	_soundState.reset();
}

bool MortimerEngine::startMovie(bool preserveScreen) {
	stopSounds();
	MortimerMovie &movie = currentMovie();
	if (movie.started())
		movie.reset();
	if (preserveScreen)
		movie.setBackground((const byte *)_screen.getPixels());
	updateMovieContext();
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
	_flight.resetWeapon();
	_hitJolt = false;
	_uiFrame = 0;
	_shotFrame = 3;
	_glowFrame = 10;
	_glowStones = MIN(_carriedStones + (_isDemo && !_restoredContent ? 0 : _flight.rescuedCount() - _creditedRescues), 999U);
	_pendingGlowStones = 0;
	memset(_iconFlash, 0, sizeof(_iconFlash));
	for (uint i = 0; i < ARRAYSIZE(_steeringHistory); ++i)
		_steeringHistory[i] = Common::Point();
	setMouse(Common::Point(320, 240));
	updateSteering();
	_system->warpMouse(_mouse.x, _mouse.y);
	return startMovie();
}

bool MortimerEngine::startCrash() {
	// SHELL's difficulty table permits these crashes before a full repair.
	const uint allowances[] = {4, 3, 1};
	_needsRepair = ++_crashCount > allowances[_flightDifficulty];
	if (_needsRepair)
		_crashCount = 0;
	_hitJolt = false;
	_flight.resetWeapon();
	return loadCrashMovie(false);
}

bool MortimerEngine::loadCrashMovie(bool repair) {
	stopSounds();
	_movieHandles.clear();
	const char *name = repair ? "RESOURCE/GENERAL/REPAIR.SAN" :
		(!_isDemo && _campaign.scene().region == 3 ? "RESOURCE/GENERAL/D_CRASH.SAN" : "RESOURCE/GENERAL/CRASH.SAN");
	Common::File file;
	if (!file.open(Common::Path(name))) {
		_loadError = Common::String::format("Missing %s", name);
		return false;
	}
	if (!_intro.load(file)) {
		_loadError = Common::String::format("%s: %s", name, _intro.errorMessage().c_str());
		return false;
	}
	_crashStage = repair ? kRepairMovie : kCrashMovie;
	_movieHandles.resize(_intro.sounds().size());
	return startMovie();
}

bool MortimerEngine::finishCrash() {
	_crashStage = kNoCrash;
	_shellShocks = _shockFrame = 0;
	_pendingFlight.clear();
	if (_needsRepair) {
		_needsRepair = false;
		// Easy flight keeps rescues and stones even after a full repair.
		if (_flightDifficulty != 0) {
			_flight.clearProgress();
			_carriedStones = _completedSpecies = _creditedRescues = 0;
			_interview.resetAttempts();
		}
		if (_isDemo)
			return loadFlight(0) && startFlight();
		return _campaign.returnToFlight() && enterCampaignScene();
	}
	if (_flightDifficulty == 2)
		_flight.clearHostiles();
	if (_isDemo)
		return loadFlight(_segment) && startFlight();
	// Ordinary crashes replay the current branch, including a cached branch.
	return loadRetailFlight(_flightName) && startFlight();
}

void MortimerEngine::creditRescues() {
	// Restored demo flights retain target IDs across retries and repairs.
	const uint credited = _isDemo && !_restoredContent ? 0 : _creditedRescues;
	_carriedStones = MIN(999U, _carriedStones + _flight.rescuedCount() - credited);
	_creditedRescues = _flight.rescuedCount();
	_completedSpecies |= _flight.rescuedSpecies();
}

bool MortimerEngine::restart(bool all) {
	_crashStage = kNoCrash;
	_needsRepair = _hitJolt = false;
	_shockFrame = _crashCount = 0;
	if (!_isDemo) {
		_finished = false;
		_score = _checkpointScore;
		if (all) {
			_score = _checkpointScore = _checkpoint = 0;
			_highScoreEligible = false;
			_loadedRegion = -1;
			_campaign.start();
			return enterCampaignScene();
		}
		if (_castleActive)
			return _campaign.start(21) && enterCampaignScene();
		if (!_interviewActive && _introIndex >= 0)
			return startMovie();
		const uint region = _campaign.scene().region;
		_flight.clearProgress();
		_carriedStones = _completedSpecies = _creditedRescues = 0;
		_interview.resetRiddle();
		_shellShocks = 0;
		return _campaign.start(region) && enterCampaignScene();
	}
	if (_interviewActive || (_restoredContent && _introIndex < 0))
		all = true;
	if (_introIndex >= 0 && !all)
		return startMovie();
	_shellShocks = 0;
	if (all) {
		_interviewActive = false;
		_interview.resetRiddle();
		_flight.clearProgress();
		_carriedStones = _completedSpecies = _creditedRescues = 0;
		if (!loadFlight(0))
			return false;
	}
	return startFlight();
}

bool MortimerEngine::nextScene(bool skip) {
	stopSounds();
	if (_castleActive) {
		if (!_castle.skip()) {
			_loadError = _castle.errorMessage();
			return false;
		}
		if (!_castle.needsMovie() && !_castle.finished()) {
			_system->getPaletteManager()->setPalette(_castle.palette(), 0, 256);
			playMovieSounds(true);
		}
		return processCastle();
	}
	if (_crashStage != kNoCrash) {
		if (_crashStage == kCrashMovie && _needsRepair)
			return loadCrashMovie(true);
		return finishCrash();
	}
	if (_interviewActive) {
		_interview.skip();
		return processInterview();
	}
	if (!_isDemo) {
		if (_introIndex < 0) {
			if (!skip && !_flight.nextFlight().empty())
				return followFlightBranch();
			if (!skip && _playingCached) {
				const Common::String next = _pendingFlight;
				_pendingFlight.clear();
				return loadRetailFlight(next) && startMovie();
			}
			creditRescues();
			_flight.context().conditions[MortimerMovieContext::kAtGate] = true;
		}
		if (!_campaign.advance()) {
			_finished = true;
			return true;
		}
		return enterCampaignScene();
	}
	if (_introIndex >= 0) {
		if (uint(_introIndex + 1) < introCount())
			return loadIntro(_introIndex + 1) && startMovie();
		return loadFlight(0) && startFlight();
	}
	if (skip && !_flight.interactive()) {
		// Keep TITLE's music at the matching position when skipping its
		// opening, without compositing images that will never be displayed.
		while (!_flight.interactive()) {
			if (!_flight.advance(false)) {
				_loadError = _flight.errorMessage().empty() ? "Mortimer title has no gameplay transition" : _flight.errorMessage();
				return false;
			}
		}
		// The frame following the mode switch is a complete keyframe of
		// the playable scene, so it does not need the skipped title image.
		if (!_flight.advance()) {
			_loadError = "Mortimer title has no gameplay frame";
			return false;
		}
		_system->getPaletteManager()->setPalette(_flight.palette(), 0, 256);
		_nextFrameTime = _system->getMillis() + 1000 / _flight.frameRate();
		playMovieSounds(true);
		return true;
	}
	if (_segment + 1 == ARRAYSIZE(kDemoFlights)) {
		creditRescues();
		_glowStones = MIN(_carriedStones, 999U);
		_pendingGlowStones = 0;
		_glowFrame = 10;
		_interviewActive = true;
		_interview.start(_completedSpecies, _glowStones);
		return processInterview();
	}
	// The demo's flight movies must follow their expected SAN branches.
	if (!skip && !_flight.nextFlight().equalsIgnoreCase(kDemoFlights[_segment + 1])) {
		_loadError = Common::String::format("Unexpected flight transition in %s at frame %u/%u: expected '%s', got '%s'",
			kDemoFlights[_segment], _flight.frame() + 1, _flight.frameCount(), kDemoFlights[_segment + 1],
			_flight.nextFlight().empty() ? "<none>" : _flight.nextFlight().c_str());
		return false;
	}
	debug(1, "Mortimer: demo flight transition %s -> %s at frame %u/%u%s",
		kDemoFlights[_segment], kDemoFlights[_segment + 1], _flight.frame() + 1, _flight.frameCount(), skip ? " (skipped)" : "");
	creditRescues();
	return loadFlight(_segment + 1) && startFlight();
}

bool MortimerEngine::processInterview() {
	const uint bonusStones = _interview.takeBonusStones();
	if (!_isDemo && _interview.solved() && (int)_checkpoint <= _loadedRegion) {
		const MortimerScoring &scoring = mortimerScoring(_flightDifficulty, _riddleDifficulty);
		if (!_score)
			_score = scoring.initial;
		_score += bonusStones * scoring.stone;
		if (_interview.bonusFinished())
			saveCheckpoint(_loadedRegion + 1);
	}
	if (_interview.wantsFlight()) {
		_carriedStones = _interview.stones();
		_completedSpecies = _interview.species();
		_interviewActive = false;
		_shellShocks = 0;
		_shockFrame = 0;
		if (!_isDemo) {
			if (_interview.restartsRegion()) {
				_flight.clearProgress();
				_carriedStones = _completedSpecies = _creditedRescues = 0;
				_crashCount = 0;
			}
			return _campaign.returnToFlight() && enterCampaignScene();
		}
		if (_restoredContent) {
			// A new demo flight after an interview must offer rescues again.
			_flight.clearProgress();
			_creditedRescues = _crashCount = 0;
		}
		return loadFlight(0) && startFlight();
	}
	if (_interview.finished()) {
		stopSounds();
		if (!_isDemo) {
			_interviewActive = false;
			_carriedStones = 0;
			_campaign.start(8 + _loadedRegion);
			_campaign.advance();
			return enterCampaignScene();
		}
		_finished = true;
		return true;
	}
	if (!_isDemo) {
		_campaign.start(_interview.sceneId());
		const char *subtitles = _interview.fullScreen() ? mortimerRegion(_loadedRegion).subtitles : "RESOURCE/DATA/MORTIMER.TRS";
		if (!loadSubtitles(subtitles))
			return false;
		updateMovieContext();
	}
	if (_interview.needsMovie()) {
		stopSounds();
		_movieHandles.clear();
		Common::File file;
		if (!file.open(Common::Path(_interview.movieName()))) {
			_loadError = Common::String::format("Missing %s", _interview.movieName().c_str());
			return false;
		}
		if (!_intro.load(file, _interview.movieDoubled())) {
			_loadError = Common::String::format("%s: %s", _interview.movieName().c_str(), _intro.errorMessage().c_str());
			return false;
		}
		_movieHandles.resize(_intro.sounds().size());
		_interview.movieLoaded();
		if (!startMovie(_interview.fullScreen()))
			return false;
	}
	const MortimerInterview::Effect effect = _interview.takeEffect();
	if (effect != MortimerInterview::kNoEffect)
		playSound(effect == MortimerInterview::kPayoutEffect ? _payoutSound :
			(effect == MortimerInterview::kSwitchEffect ? _balloonSound : _buttonSound), _scopeHandle, Audio::Mixer::kSFXSoundType);
	_glowStones = _interview.stones();
	_system->getPaletteManager()->setPalette(_interview.palette(_intro), 0, 256);
	return true;
}

bool MortimerEngine::processCastle() {
	if (!_castle.errorMessage().empty()) {
		_loadError = _castle.errorMessage();
		return false;
	}
	if (_castle.solved() && _checkpoint < 5) {
		_score += mortimerScoring(_flightDifficulty, _riddleDifficulty).castle;
		saveCheckpoint(5);
		_highScoreEligible = true;
	}
	if (_castle.finished()) {
		stopSounds();
		_castleActive = false;
		return _campaign.start(36) && enterCampaignScene();
	}
	_campaign.start(_castle.sceneId());
	if (!loadSubtitles(_castle.subtitleName()))
		return false;
	if (_castle.needsMovie()) {
		stopSounds();
		_movieHandles.clear();
		if (!_castle.loadMovie()) {
			_loadError = _castle.errorMessage();
			return false;
		}
		_introIndex = -1;
		_movieHandles.resize(_castle.sounds().size());
		_hitJolt = false;
		_shotFrame = 3;
		for (uint i = 0; i < ARRAYSIZE(_steeringHistory); ++i)
			_steeringHistory[i] = Common::Point();
		setMouse(Common::Point(320, 240));
		updateSteering();
		_system->warpMouse(_mouse.x, _mouse.y);
		if (!startMovie())
			return false;
	}
	const MortimerSound *speech = _castle.takeSpeech(_mixer->isSoundHandleActive(_castleHandle));
	if (speech)
		playSound(*speech, _castleHandle, Audio::Mixer::kSpeechSoundType);
	return true;
}

bool MortimerEngine::updateCastle() {
	if (_controllerMode && (_castle.choosing() || !_castle.courseView()))
		setMouse(Common::Point(_mouse.x + (_steering[1] - _steering[0]) * 18 / Common::JOYAXIS_MAX,
			_mouse.y + (_steering[3] - _steering[2]) * 18 / Common::JOYAXIS_MAX));
	_castle.update();
	updateInterface();
	if (_castle.needsMovie() || _castle.finished())
		return processCastle();
	if (_castle.courseView()) {
		if (_hitJolt) {
			const int amount = 64 * (_flightDifficulty + 1);
			updateView(_random.getRandomBit() ? amount : -amount, _random.getRandomBit() ? amount : -amount);
			_hitJolt = false;
		}
		updateSteering(!_castle.acceptsShots());
	}
	updateMovieContext();
	if (_castle.paused())
		return processCastle();
	if (!_castle.advance()) {
		if (!_castle.errorMessage().empty()) {
			_loadError = _castle.errorMessage();
			return false;
		}
		if (_castle.hasSpeech() || _mixer->isSoundHandleActive(_castleHandle))
			return processCastle();
		_castle.movieFinished();
		return processCastle();
	}
	playMovieSounds();
	_system->getPaletteManager()->setPalette(_castle.palette(), 0, 256);
	if (_castle.courseView() && _castle.acceptsShots() && _castle.hitsObstacle(moviePoint(_aim)) && !addShellShock())
		return false;
	return processCastle();
}

bool MortimerEngine::updateScene() {
	if (_highScoresActive)
		return true;
	if (_castleActive)
		return updateCastle();
	if (_interviewActive) {
		if (_controllerMode) {
			setMouse(Common::Point(_mouse.x + (_steering[1] - _steering[0]) * 18 / Common::JOYAXIS_MAX,
				_mouse.y + (_steering[3] - _steering[2]) * 18 / Common::JOYAXIS_MAX));
		}
		updateInterface();
		_interview.update();
		const bool changed = _interview.needsMovie();
		if (!processInterview())
			return false;
		if (changed || !_interview.playingMovie())
			return true;
	} else if (interactive()) {
		if (_hitJolt) {
			const int amount = 64 * (_flightDifficulty + 1);
			const int y = _random.getRandomBit() ? amount : -amount;
			const int x = _random.getRandomBit() ? amount : -amount;
			updateView(x, y);
			_hitJolt = false;
		}
		updateSteering();
		updateInterface();
	}
	updateMovieContext();
	MortimerMovie &movie = currentMovie();
	if (!movie.advance()) {
		if (!movie.errorMessage().empty()) {
			_loadError = movie.errorMessage();
			return false;
		}
		if (_interviewActive) {
			stopSounds();
			_interview.movieFinished();
			return processInterview();
		}
		return nextScene();
	}
	if (!_isDemo && _crashStage == kNoCrash && !_interviewActive && _introIndex < 0 && !_flight.nextFlight().empty())
		return followFlightBranch();
	playMovieSounds();
	_system->getPaletteManager()->setPalette(_interviewActive ? _interview.palette(movie) : movie.palette(), 0, 256);
	if (interactive() && _flight.hitsObstacle(moviePoint(_aim)) && !addShellShock())
		return false;
	return true;
}

bool MortimerEngine::addShellShock() {
	if (_invulnerable || isPaused() || !interactive())
		return true;
	if (_castleActive) {
		if (!_castle.acceptsShots())
			return true;
		_castle.hit();
		_hitJolt = true;
		if (!processCastle())
			return false;
	} else if (_shellShocks + 1 >= (_isDemo ? 18U : 15U)) {
		// Retail crashes on the fifteenth hit; the demo allows eighteen.
		if (!(_isDemo && !_restoredContent ? restart() : startCrash()))
			return false;
	} else {
		++_shellShocks;
		_shockFrame = 0;
		_hitJolt = !_isDemo || _restoredContent;
	}
	// Movie changes stop audio, so enqueue the fatal impact afterwards.
	playCollisionSound();
	return true;
}

void MortimerEngine::clearShellShock() {
	if (_castleActive)
		_castle.clearDamage();
	else
		_shellShocks = _shockFrame = 0;
	_hitJolt = false;
	updateMovieContext();
}

bool MortimerEngine::skipRegion() {
	const uint region = _campaign.scene().region;
	const uint next = region < 5 ? (region + 1) % 5 : 0;
	_loadedRegion = -1;
	_crashStage = kNoCrash;
	_needsRepair = _hitJolt = false;
	_flight.clearProgress();
	_interview.resetRiddle();
	_carriedStones = _completedSpecies = _creditedRescues = _shellShocks = 0;
	_shockFrame = _crashCount = 0;
	return _campaign.start(next == 4 ? 34 : next) && enterCampaignScene();
}

bool MortimerEngine::handleCheatKey(const Common::KeyState &key) {
	if (!_cheats || _finished || (key.flags & (Common::KBD_CTRL | Common::KBD_META)))
		return false;
	const bool alt = key.flags & Common::KBD_ALT;
	const bool plus = key.keycode == Common::KEYCODE_EQUALS || key.keycode == Common::KEYCODE_PLUS ||
		key.keycode == Common::KEYCODE_KP_PLUS;
	const bool minus = key.keycode == Common::KEYCODE_MINUS || key.keycode == Common::KEYCODE_UNDERSCORE ||
		key.keycode == Common::KEYCODE_KP_MINUS;
	if ((plus || minus) && alt == !_isDemo) {
		if (interactive()) {
			if (plus)
				clearShellShock();
			else
				addShellShock();
		}
		return true;
	}
	if (alt)
		return false;
	if (key.keycode == Common::KEYCODE_a) {
		_completedSpecies = 0x7f;
		if (_interviewActive)
			_interview.rescueAll();
		return true;
	}
	if (key.keycode == Common::KEYCODE_i) {
		_invulnerable = !_invulnerable;
		return true;
	}
	if (_isDemo)
		return false;
	switch (key.keycode) {
	case Common::KEYCODE_s:
		if (!isPaused())
			skipRegion();
		break;
	case Common::KEYCODE_r:
	case Common::KEYCODE_w:
		if (_castleActive && _castle.sceneId() == 21)
			_castle.forceChoice(key.keycode == Common::KEYCODE_r);
		break;
	default:
		return false;
	}
	return true;
}

void MortimerEngine::setMouse(const Common::Point &position) {
	_mouse.x = CLIP<int>(position.x, 0, 639);
	_mouse.y = CLIP<int>(position.y, 0, 479);
}

void MortimerEngine::updateSteering(bool centerView) {
	int x = (_mouse.x - 320) / 2, y = 240 - _mouse.y;
	if (_controllerMode) {
		x = (_steering[1] - _steering[0]) * 128 / Common::JOYAXIS_MAX;
		y = (_steering[2] - _steering[3]) * 128 / Common::JOYAXIS_MAX;
	}
	updateView(centerView ? 0 : x, centerView ? 0 : y);
	if (ABS(x) < 3)
		x = 0;
	if (ABS(y) < 3)
		y = 0;
	const int halfWidth = _viewport.width() / 2, halfHeight = _viewport.height() / 2;
	_aim.x = _viewport.left + halfWidth + CLIP(x * halfWidth / 109, -halfWidth, halfWidth - 1);
	_aim.y = _viewport.top + halfHeight - CLIP(y * halfHeight / 128, 1 - halfHeight, halfHeight);
}

void MortimerEngine::updateView(int x, int y) {
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
	const int horizontal = _castleActive ? 72 : 88, vertical = _castleActive ? 116 : 118;
	_view.x = ((horizontal + CLIP(averageX * horizontal / 128, -horizontal, horizontal) + 3) & ~3) / 2;
	_view.y = ((vertical - CLIP(averageY * vertical / 128, -vertical, vertical) + 3) & ~3) / 2;
}

Common::Point MortimerEngine::moviePoint(const Common::Point &point) const {
	return Common::Point(_view.x + (point.x - _viewport.left) / 2, _view.y + (point.y - _viewport.top) / 2);
}

Common::Point MortimerEngine::screenPoint(const Common::Point &point) const {
	return Common::Point(_viewport.left + (point.x - _view.x) * 2, _viewport.top + (point.y - _view.y) * 2);
}

void MortimerEngine::fire() {
	MortimerFlight &flight = currentFlight();
	if (_shotFrame >= 3) {
		_shot = _aim;
		_shotWeapon = flight.weapon();
		_shotFrame = 0;
		playSound(_shotWeapon == MortimerFlight::kSlimeGun ? _slimeSound : _scopeSound,
			_scopeHandle, Audio::Mixer::kSFXSoundType);
	}
	const MortimerFlight::ShotResult result = flight.shoot(moviePoint(_aim));
	if (result == MortimerFlight::kAnimalRescued) {
		playSound(_rescueSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
		++_pendingGlowStones;
		if (_glowFrame >= 10) {
			_glowFrame = 0;
			playSound(_glowSound, _glowHandle, Audio::Mixer::kSFXSoundType);
		}
		for (uint i = 0; i < flight.targets().size(); ++i) {
			const MortimerFlight::Target &target = flight.targets()[i];
			if (target.hit && target.rescuedAt == flight.frame() && target.id / 100 < 7)
				_iconFlash[target.id / 100] = 6;
		}
	} else if (result == MortimerFlight::kHornetHit) {
		playSound(_slimeHitSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
	} else if (result == MortimerFlight::kShellRestored) {
		_shellShocks = _shockFrame = 0;
		playSound(_balloonSound, _rescueHandle, Audio::Mixer::kSFXSoundType);
	}
}

void MortimerEngine::updateInterface() {
	++_uiFrame;
	_shockFrame ^= 1;
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
	if (!_soundState.variable(2))
		return false;
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
	const int boot = ConfMan.hasKey("boot_param") ? ConfMan.getInt("boot_param") : -1;
	if (_isDemo && boot == kBootProgrammer)
		return Common::Error(Common::kUnsupportedGameidError, "The Programmer interview requires the full Mortimer game data");
	if (_isDemo && !(_resourcesLoaded = loadResources()))
		return Common::Error(Common::kReadingFailed, _loadError);
	if (!_isDemo && !_subtitles.load("RESOURCE/DATA/BEGIN.TRS", "RESOURCE/GENERAL/ESTRO20.NUT", "RESOURCE/GENERAL/MORTFNTH.NUT"))
		return Common::Error(Common::kReadingFailed, "Missing or invalid Mortimer subtitle resources");
	initGraphics(640, 480);
	_screen.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_screen.fillRect(Common::Rect(640, 480), 10);
	syncSoundSettings();
	if (!_isDemo && ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0) {
		const Common::Error result = loadGameState(ConfMan.getInt("save_slot"));
		if (result.getCode() != Common::kNoError)
			return result;
	} else if (!_isDemo && boot < 0) {
		if (!selectPilot())
			return Common::kNoError;
	} else if (!selectDifficulty()) {
		return Common::kNoError;
	}
	const bool oldCursor = CursorMan.showMouse(false);
	PauseToken pause;
	bool exit;
	// 1000 opens the hidden interview; other values select original scene IDs.
	if (_pilotLoadRequested) {
		exit = !startPilot();
	} else if (boot == kBootProgrammer) {
		_programmer = true;
		exit = !loadRegion(0);
		if (!exit) {
			_completedSpecies = 0x7f;
			_carriedStones = 30;
			_campaign.start(8);
			exit = !enterCampaignScene();
		}
	} else if (!_isDemo) {
		_subtitleName = "RESOURCE/DATA/BEGIN.TRS";
		if (!_campaign.start(boot < 0 ? 24 : boot))
			_campaign.start();
		exit = !enterCampaignScene();
	} else if (boot == 4 || boot == 10 || boot == 18 || boot == 8 || boot == 25) {
		stopSounds();
		_introIndex = -1;
		_interviewActive = true;
		_interview.start(0x7f, 30, boot);
		exit = !processInterview();
		_nextFrameTime = _system->getMillis() + 1000 / currentMovie().frameRate();
	} else if (boot == 0) {
		exit = !(loadFlight(0) && startFlight());
	} else {
		exit = !(loadIntro(boot >= 19 && boot <= 23 ? boot - 19 : 0) && startMovie());
	}
	while (!shouldQuit() && !exit) {
		if (_pilotLoadRequested) {
			if (pause.isActive())
				pause.clear();
			if (!startPilot())
				break;
		}
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if (_highScoresActive && !isPaused()) {
				if (_highScores.enteringName()) {
					if (event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat && _highScores.handleKey(event.kbd)) {
						if (_highScores.save(*_saveFileMan, _targetName + ".hsf")) {
							setScoreNameInput(false);
						} else {
							GUI::MessageDialog message(_("Could not save the high scores. Please try again."));
							runDialog(message);
						}
					}
				} else if ((event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat) ||
					event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN ||
					event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
					exit = !finishHighScores();
				}
				continue;
			}
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbdRepeat)
					continue;
				if (handleCheatKey(event.kbd)) {
					if (!_loadError.empty()) {
						exit = true;
						break;
					}
					continue;
				}
			}
			if (event.type == Common::EVENT_MOUSEMOVE) {
				if (!isPaused()) {
					setMouse(event.mouse);
					_controllerMode = false;
				}
			} else if (event.type == Common::EVENT_JOYAXIS_MOTION && event.joystick.axis < ARRAYSIZE(_steering)) {
				_steering[event.joystick.axis] = CLIP<int>(event.joystick.position, 0, Common::JOYAXIS_MAX);
				if (event.joystick.position && !isPaused())
					_controllerMode = true;
			} else if (_interviewActive && !isPaused() &&
				(event.type == Common::EVENT_WHEELUP || event.type == Common::EVENT_WHEELDOWN)) {
				_interview.scroll(event.type == Common::EVENT_WHEELUP ? -1 : 1);
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				switch (event.customType) {
				case kActionFire:
					if (!isPaused() && !_finished) {
						if (_castleActive) {
							if (_castle.courseView() && _castle.acceptsShots())
								fire();
							else
								_castle.click(_mouse);
							exit = !processCastle();
						} else if (_interviewActive) {
							const bool playing = _interview.playingMovie();
							_interview.click(_mouse);
							if (playing && !_interview.playingMovie())
								stopSounds();
							exit = !processInterview();
						} else if (interactive()) {
							fire();
						}
					}
					break;
				case kActionSkip:
					if (pause.isActive())
						pause.clear();
					else if (!interactive() || _cheats)
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
			if (_pilotLoadRequested)
				break;
		}
		if (exit)
			break;
		if (_pilotLoadRequested)
			continue;
		if (!isPaused() && !_finished && int32(_system->getMillis() - _nextFrameTime) >= 0) {
			_nextFrameTime += 1000 / currentMovie().frameRate();
			if (!updateScene())
				break;
			// Avoid racing through queued frames following a slow backend call.
			if (int32(_system->getMillis() - _nextFrameTime) > 250)
				_nextFrameTime = _system->getMillis() + 1000 / currentMovie().frameRate();
		}
		draw();
		if (_finished) {
			if (!_viewingHighScores) {
				GUI::MessageDialog dialog(_("Play again?"), _("Restart"), _("Quit"));
				if (runDialog(dialog) != GUI::kMessageOK)
					break;
			}
			_viewingHighScores = false;
			if (_isDemo) {
				if (!selectDifficulty() || !restart(true))
					break;
			} else if (!selectPilot()) {
				break;
			}
		}
		_system->delayMillis(10);
	}
	stopSounds();
	if (_highScoresActive)
		setScoreNameInput(false);
	CursorMan.showMouse(oldCursor);
	if (!_loadError.empty())
		warning("Mortimer: %s", _loadError.c_str());
	return _loadError.empty() ? savePilot() : Common::Error(Common::kReadingFailed, _loadError);
}

void MortimerEngine::drawSprite(const MortimerSprite &sprite, int x, int y, bool centered, const Common::Rect *bounds) {
	sprite.draw(_screen, x, y, centered, bounds);
}

void MortimerEngine::drawTargets() {
	MortimerFlight &flight = currentFlight();
	for (uint i = 0; i < flight.targets().size(); ++i) {
		const MortimerFlight::Target &target = flight.targets()[i];
		const bool hostile = (target.id >= 700 && target.id < 800) || (target.id >= 9000 && target.id < 9400);
		if (!target.active || !target.id || (target.id >= 800 && !hostile))
			continue;
		const Common::Point top = screenPoint(Common::Point(target.rect.left, target.rect.top));
		const Common::Point bottom = screenPoint(Common::Point(target.rect.right, target.rect.bottom));
		if (target.hit) {
			const Common::Array<MortimerSprite> &effect = target.hitWeapon == MortimerFlight::kSlimeGun ? _slimeHit : _scopeHit;
			const uint frame = flight.frame() - target.rescuedAt;
			if (frame < effect.size()) {
				// Average after scaling to retain the midpoint of odd-sized targets.
				const Common::Point center((top.x + bottom.x) / 2, (top.y + bottom.y) / 2);
				drawSprite(effect[frame], center.x, center.y, true, &_viewport);
			}
		} else if ((target.id < 700 && flight.weapon() == MortimerFlight::kRevitascope) ||
			(hostile && flight.weapon() == MortimerFlight::kSlimeGun)) {
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
	// Retail advances every hit after ten shocks and flashes from thirteen.
	const uint level = _isDemo || _shellShocks <= 10 ? _shellShocks / 2 : _shellShocks - 5;
	const uint gauge = level < 8 ? 19 + level : 27 + _shockFrame;
	drawSprite(_dashboard[gauge], 180, 442);
	const Common::Point demoIconOrigins[] = {{158, 364}, {203, 358}, {248, 358}, {293, 354}, {339, 355}, {383, 360}, {428, 361}};
	// Retail uses centered icons (SHELL 43f12c, 44304c); the demo uses sprite origins.
	const Common::Point iconCenters[] = {{176, 384}, {222, 378}, {265, 378}, {310, 376}, {355, 379}, {399, 380}, {444, 384}};
	const Common::Point flashOrigins[] = {{154, 345}, {198, 338}, {243, 334}, {288, 332}, {333, 334}, {378, 338}, {422, 345}};
	for (uint species = 0; species < 7; ++species) {
		if ((_iconFlash[species] & 1) || (_interviewActive && int(species) == _interview.highlightedAnimal()))
			drawSprite(_dashboard[species], flashOrigins[species].x, flashOrigins[species].y);
		const uint available = _interviewActive ? _interview.species() : (_completedSpecies | _flight.rescuedSpecies());
		const bool rescued = available & (1 << species);
		const Common::Point &origin = _isDemo ? demoIconOrigins[species] : iconCenters[species];
		drawSprite(_icons[species * 2 + (rescued ? 0 : 1)], origin.x, origin.y, !_isDemo);
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
	if (_interviewActive) {
		_interview.draw(_screen, _intro, _soundState.variable(1));
		if (_interview.showDashboard())
			drawDashboard();
		_interview.drawCursor(_screen, _mouse);
	} else if (interactive()) {
		_screen.fillRect(Common::Rect(640, 480), 10);
		for (int y = _viewport.top; y < _viewport.bottom; ++y) {
			const byte *src = currentFlight().pixels() + (_view.y + (y - _viewport.top) / 2) * MortimerMovie::kScreenWidth + _view.x;
			byte *dst = (byte *)_screen.getBasePtr(_viewport.left, y);
			for (int x = 0; x < _viewport.width(); ++x)
				dst[x] = src[x / 2];
		}
		drawTargets();
		if (_castleActive) {
			_castle.drawOverlay(_screen, _mouse, screenPoint(Common::Point()), _soundState.variable(1));
			drawSprite(_reticles[0], _castle.choosing() ? _mouse.x : _aim.x,
				_castle.choosing() ? _mouse.y : _aim.y, true, &_viewport);
		} else {
			drawSprite(_reticles[_flight.weapon() == MortimerFlight::kSlimeGun ? 0 : 1], _aim.x, _aim.y, true, &_viewport);
			drawSprite(_head[_uiFrame % 4], 270, 200);
			drawSprite(_cockpit[0], 0, 0);
			drawSprite(_cockpit[1], 567, 0);
			drawSprite(_cockpit[2], 0, 0);
			drawDashboard();
		}
		drawProjectile();
	} else {
		_screen.copyRectToSurface(currentMovie().pixels(), MortimerMovie::kScreenWidth, 0, 0, 640, 480);
		if (_castleActive)
			_castle.drawOverlay(_screen, _mouse, Common::Point(), _soundState.variable(1));
	}
	if (_highScoresActive) {
		_highScores.draw(_screen, _system->getMillis());
	} else if (!_isDemo) {
		_subtitles.draw(_screen, currentMovie().captions(), ConfMan.getBool("subtitles") ||
			(_interviewActive && _interview.forceSubtitles()) || (_castleActive && _castle.forceSubtitles()), &_intro.context());
		if (_interviewActive && _interview.payingBonus())
			_subtitles.drawScore(_screen, _score);
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
