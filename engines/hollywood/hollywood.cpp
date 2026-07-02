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

#include "hollywood/hollywood.h"
#include "hollywood/console.h"
#include "hollywood/font.h"
#include "hollywood/gameplay/travel_screen.h"
#include "hollywood/scenes/intro/scene1000.h"
#include "hollywood/scenes/intro/scene9000.h"
#include "hollywood/scenes/intro/scene9010.h"
#include "hollywood/scenes/intro/scene9050.h"
#include "hollywood/scenes/intro/scene9100.h"
#include "hollywood/scenes/intro/scene9110.h"
#include "hollywood/scenes/intro/scene9120.h"
#include "hollywood/scenes/playable/scene1010.h"
#include "hollywood/scenes/playable/scene1020.h"
#include "hollywood/scenes/playable/scene1030.h"
#include "hollywood/scenes/playable/scene1040.h"
#include "hollywood/scenes/playable/scene1050.h"
#include "hollywood/scenes/playable/scene1060.h"
#include "hollywood/scenes/playable/scene1070.h"
#include "hollywood/scenes/playable/scene1080.h"
#include "hollywood/scenes/playable/scene1090.h"
#include "hollywood/scenes/playable/scene2000.h"
#include "hollywood/scenes/playable/scene2030.h"
#include "hollywood/scenes/playable/scene2040.h"
#include "hollywood/scenes/playable/scene3000.h"
#include "hollywood/scenes/playable/scene3010.h"
#include "hollywood/scenes/playable/scene3020.h"
#include "hollywood/scenes/playable/scene3030.h"
#include "hollywood/scenes/playable/scene3040.h"
#include "hollywood/scenes/playable/scene3050.h"
#include "hollywood/scenes/playable/scene3060.h"
#include "hollywood/scenes/playable/scene3070.h"
#include "hollywood/scenes/playable/scene3080.h"
#include "hollywood/scenes/playable/scene3090.h"
#include "hollywood/scenes/playable/scene3100.h"
#include "hollywood/scenes/playable/scene3110.h"
#include "hollywood/scenes/playable/scene4000.h"
#include "hollywood/scenes/playable/scene4010.h"
#include "hollywood/scenes/playable/scene4020.h"
#include "hollywood/scenes/playable/scene4030.h"
#include "hollywood/scenes/playable/scene4040.h"
#include "hollywood/scenes/playable/scene4050.h"
#include "hollywood/scenes/playable/scene4060.h"
#include "hollywood/scenes/playable/scene4070.h"
#include "hollywood/scenes/playable/scene4080.h"
#include "hollywood/scenes/playable/scene5000.h"
#include "hollywood/scenes/playable/scene5010.h"
#include "hollywood/scenes/playable/scene6000.h"
#include "hollywood/scenes/playable/scene6010.h"
#include "hollywood/scenes/playable/scene6020.h"
#include "hollywood/scenes/playable/scene6030.h"
#include "hollywood/scenes/playable/scene7000.h"
#include "hollywood/scenes/playable/scene7010.h"
#include "hollywood/scenes/playable/scene7020.h"
#include "hollywood/scenes/playable/scene7030.h"
#include "hollywood/scenes/playable/scene7040.h"
#include "hollywood/scenes/playable/scene7050.h"
#include "hollywood/scenes/playable/scene7060.h"
#include "hollywood/scenes/playable/scene7070.h"
#include "hollywood/scenes/playable/scene7080.h"
#include "hollywood/scenes/playable/scene7090.h"
#include "hollywood/scenes/playable/scene7100.h"
#include "hollywood/scenes/playable/scene8000.h"
#include "hollywood/scenes/playable/scene8010.h"
#include "hollywood/scenes/playable/scene8020.h"
#include "hollywood/resource.h"

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "graphics/thumbnail.h"

namespace Hollywood {

const uint kOptionsMaximumLevel = 200;
const int kMaximumConfigVolume = Audio::Mixer::kMaxMixerVolume;
const int kMaximumTalkSpeed = 255;
const int kTitleFrontEndState = 1000;
const int kScene1010FirstState = 0x03f2;
const int kScene1010LastState = 0x03fb;
const int kScene1020FirstState = 0x03fc;
const int kScene1020LastState = 0x0405;
const int kScene1030FirstState = 0x0406;
const int kScene1030LastState = 0x0407;
const int kScene1040FirstState = 0x0410;
const int kScene1040LastState = 0x0419;
const int kScene1050FirstState = 0x041a;
const int kScene1050LastState = 0x0423;
const int kScene1060FirstState = 0x0424;
const int kScene1060LastState = 0x042d;
const int kScene1070FirstState = 0x042e;
const int kScene1070LastState = 0x0437;
const int kScene1080FirstState = 0x0438;
const int kScene1080LastState = 0x0441;
const int kScene1090FirstState = 0x0442;
const int kScene1090LastState = 0x044b;
const int kScene2000State = 2000;
const int kScene2030FirstState = 0x07ee;
const int kScene2030LastState = 0x07f7;
const int kScene2040FirstState = 0x07f8;
const int kScene2040LastState = 0x0801;
const int kScene3000State = 3000;
const int kScene3010FirstState = 0x0bc2;
const int kScene3010LastState = 0x0bcb;
const int kScene3020FirstState = 0x0bcc;
const int kScene3020LastState = 0x0bd5;
const int kScene3030FirstState = 0x0bd6;
const int kScene3030LastState = 0x0bdf;
const int kScene3040State = 0x0be0;
const int kScene3050FirstState = 0x0bea;
const int kScene3050LastState = 0x0bf3;
const int kScene3060FirstState = 0x0bf4;
const int kScene3060LastState = 0x0bf5;
const int kScene3070FirstState = 0x0bfe;
const int kScene3070LastState = 0x0c07;
const int kScene3080FirstState = 0x0c08;
const int kScene3080LastState = 0x0c11;
const int kScene3090FirstState = 0x0c12;
const int kScene3090LastState = 0x0c1b;
const int kScene3100FirstState = 0x0c1c;
const int kScene3100LastState = 0x0c25;
const int kScene3110FirstState = 0x0c26;
const int kScene3110LastState = 0x0c27;
const int kScene4000State = 4000;
const int kScene4010FirstState = 0x0faa;
const int kScene4010LastState = 0x0fb3;
const int kScene4020FirstState = 0x0fb4;
const int kScene4020LastState = 0x0fb5;
const int kScene4030FirstState = 0x0fbe;
const int kScene4030LastState = 0x0fc7;
const int kScene4040FirstState = 0x0fc8;
const int kScene4040LastState = 0x0fd1;
const int kScene4050FirstState = 0x0fd2;
const int kScene4050LastState = 0x0fdb;
const int kScene4060FirstState = 0x0fdc;
const int kScene4060LastState = 0x0fe5;
const int kScene4070FirstState = 0x0fe6;
const int kScene4070LastState = 0x0fef;
const int kScene4080FirstState = 0x0ff0;
const int kScene4080LastState = 0x0ff0;
const int kScene5000State = 5000;
const int kScene5010FirstState = 0x1392;
const int kScene5010LastState = 0x139b;
const int kScene6000State = 6000;
const int kScene6010FirstState = 0x177a;
const int kScene6010LastState = 0x1783;
const int kScene6020FirstState = 0x1784;
const int kScene6020LastState = 0x178d;
const int kScene6030FirstState = 0x178e;
const int kScene6030LastState = 0x1797;
const int kScene8000State = 8000;
const int kScene8010FirstState = 0x1f4a;
const int kScene8010LastState = 0x1f4b;
const int kScene8020State = 0x1f54;
const int kTravelScreenSelectionState = 0xffff;
const int kScene9101State = 0x238d;

bool isImplementedIntroSceneNumber(int sceneNumber) {
	return sceneNumber == 9000 || sceneNumber == 9010 || sceneNumber == 9050 ||
		sceneNumber == 9100 || sceneNumber == 9110 || sceneNumber == 9120;
}

bool isImplementedGameplayState(int stateId) {
	return stateId == kTitleFrontEndState ||
		(stateId >= kScene1010FirstState && stateId <= kScene1010LastState) ||
		(stateId >= kScene1020FirstState && stateId <= kScene1020LastState) ||
		(stateId >= kScene1030FirstState && stateId <= kScene1030LastState) ||
		(stateId >= kScene1040FirstState && stateId <= kScene1040LastState) ||
		(stateId >= kScene1050FirstState && stateId <= kScene1050LastState) ||
		(stateId >= kScene1060FirstState && stateId <= kScene1060LastState) ||
		(stateId >= kScene1070FirstState && stateId <= kScene1070LastState) ||
		(stateId >= kScene1080FirstState && stateId <= kScene1080LastState) ||
		(stateId >= kScene1090FirstState && stateId <= kScene1090LastState) ||
		stateId == kScene2000State ||
		(stateId >= kScene2030FirstState && stateId <= kScene2030LastState) ||
		(stateId >= kScene2040FirstState && stateId <= kScene2040LastState) ||
		stateId == kScene3000State ||
		(stateId >= kScene3010FirstState && stateId <= kScene3010LastState) ||
		(stateId >= kScene3020FirstState && stateId <= kScene3020LastState) ||
		(stateId >= kScene3030FirstState && stateId <= kScene3030LastState) ||
		stateId == kScene3040State ||
		(stateId >= kScene3050FirstState && stateId <= kScene3050LastState) ||
		(stateId >= kScene3060FirstState && stateId <= kScene3060LastState) ||
		(stateId >= kScene3070FirstState && stateId <= kScene3070LastState) ||
		(stateId >= kScene3080FirstState && stateId <= kScene3080LastState) ||
		(stateId >= kScene3090FirstState && stateId <= kScene3090LastState) ||
		(stateId >= kScene3100FirstState && stateId <= kScene3100LastState) ||
		(stateId >= kScene3110FirstState && stateId <= kScene3110LastState) ||
		stateId == kScene4000State ||
		(stateId >= kScene4010FirstState && stateId <= kScene4010LastState) ||
		(stateId >= kScene4020FirstState && stateId <= kScene4020LastState) ||
		(stateId >= kScene4030FirstState && stateId <= kScene4030LastState) ||
		(stateId >= kScene4040FirstState && stateId <= kScene4040LastState) ||
		(stateId >= kScene4050FirstState && stateId <= kScene4050LastState) ||
		(stateId >= kScene4060FirstState && stateId <= kScene4060LastState) ||
		(stateId >= kScene4070FirstState && stateId <= kScene4070LastState) ||
		(stateId >= kScene4080FirstState && stateId <= kScene4080LastState) ||
		stateId == kScene5000State ||
		(stateId >= kScene5010FirstState && stateId <= kScene5010LastState) ||
		stateId == kScene6000State ||
		(stateId >= kScene6010FirstState && stateId <= kScene6010LastState) ||
		(stateId >= kScene6020FirstState && stateId <= kScene6020LastState) ||
		(stateId >= kScene6030FirstState && stateId <= kScene6030LastState) ||
		stateId == kScene8000State ||
		(stateId >= kScene8010FirstState && stateId <= kScene8010LastState) ||
		stateId == kScene8020State ||
		stateId == kTravelScreenSelectionState ||
		stateId == 7000 ||
		(stateId >= 0x1b62 && stateId <= 0x1b6b) ||
		stateId == 0x1b6c ||
		stateId == kScene9101State ||
		(stateId >= 0x1b76 && stateId <= 0x1b7f) ||
		(stateId >= 0x1b80 && stateId <= 0x1b89) ||
		stateId == 0x1b8a ||
		(stateId >= 0x1b94 && stateId <= 0x1b9d) ||
		(stateId >= 0x1b9e && stateId <= 0x1ba0) ||
		(stateId >= 0x1ba8 && stateId <= 0x1bb1) ||
		(stateId >= 0x1bb2 && stateId <= 0x1bbb) ||
		(stateId >= 0x1bbc && stateId <= 0x1bc5);
}

bool shouldRunSceneForBootParam(int bootParam, int sceneNumber, bool &bootSceneReached) {
	if (bootParam == 0 || bootSceneReached)
		return true;

	if (bootParam == sceneNumber) {
		bootSceneReached = true;
		return true;
	}

	return false;
}

int gameplayStateForBootParam(int bootParam) {
	if (bootParam == kScene1010FirstState)
		return 0x03f4;
	if (bootParam == 3070)
		return kScene3070FirstState;
	if (bootParam == 2040)
		return kScene2040FirstState;
	if (bootParam == 3040)
		return kScene3040State;
	if (bootParam == 3090)
		return kScene3090FirstState;
	if (bootParam == 3100)
		return kScene3100FirstState;
	if (bootParam == 3110)
		return kScene3110FirstState;
	if (bootParam == 4020)
		return kScene4020FirstState;
	if (bootParam == 4030)
		return kScene4030FirstState;
	if (bootParam == 4040)
		return kScene4040FirstState;
	if (bootParam == 4050)
		return kScene4050FirstState;
	if (bootParam == 4060)
		return kScene4060FirstState;
	if (bootParam == 4070)
		return kScene4070FirstState;
	if (bootParam == 4080)
		return kScene4080FirstState;
	if (bootParam == 8010)
		return kScene8010FirstState;
	if (bootParam == 8020)
		return kScene8020State;

	return bootParam;
}

byte configVolumeToOptionsLevel(int volume) {
	const int clippedVolume = CLIP<int>(volume, 0, kMaximumConfigVolume);
	return (byte)((clippedVolume * kOptionsMaximumLevel +
		kMaximumConfigVolume / 2) / kMaximumConfigVolume);
}

int optionsLevelToConfigVolume(byte level) {
	const uint clippedLevel = MIN<uint>(level, kOptionsMaximumLevel);
	return (clippedLevel * kMaximumConfigVolume +
		kOptionsMaximumLevel / 2) / kOptionsMaximumLevel;
}

byte configTalkSpeedToOptionsLevel(int talkSpeed) {
	const int clippedTalkSpeed = CLIP<int>(talkSpeed, 0, kMaximumTalkSpeed);
	return (byte)((clippedTalkSpeed * kOptionsMaximumLevel +
		kMaximumTalkSpeed / 2) / kMaximumTalkSpeed);
}

int optionsLevelToConfigTalkSpeed(byte level) {
	const uint clippedLevel = MIN<uint>(level, kOptionsMaximumLevel);
	return (clippedLevel * kMaximumTalkSpeed +
		kOptionsMaximumLevel / 2) / kOptionsMaximumLevel;
}

HollywoodEngine::HollywoodEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst),
		_gameDescription(gameDesc),
		_resources(new ResourceManager()),
		_font(new HollywoodFont()),
		_introMusic(),
		_gameplayMusic(),
		_gameState(),
		_lastGameplayThumbnail(),
		_lastGameplayThumbnailValid(false),
		_sceneRestartRequested(false),
		_canSave(false) {
}

HollywoodEngine::~HollywoodEngine() {
	_lastGameplayThumbnail.free();
	delete _font;
	delete _resources;
}

void HollywoodEngine::captureLastGameplayThumbnail() {
	Graphics::Surface thumbnail;
	if (!Graphics::createThumbnail(thumbnail))
		return;

	_lastGameplayThumbnail.copyFrom(thumbnail);
	_lastGameplayThumbnailValid = true;
	thumbnail.free();
}

bool HollywoodEngine::copyLastGameplayThumbnail(Graphics::Surface &thumbnail) const {
	if (!_lastGameplayThumbnailValid || !_lastGameplayThumbnail.getPixels())
		return false;

	thumbnail.copyFrom(_lastGameplayThumbnail);
	return true;
}

Common::Error HollywoodEngine::run() {
	setDebugger(new Console(this));

	const Graphics::PixelFormat screenFormat = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(kScreenWidth, kScreenHeight, &screenFormat);

	syncSoundSettings();
	_font->load();

	debugC(1, kDebugGeneral, "Hollywood Monsters engine initialized");

	const int startupLoadSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	const bool startupLoad = startupLoadSlot >= 0;
	if (startupLoad) {
		Common::Error loadError = loadGameState(startupLoadSlot);
		if (loadError.getCode() != Common::kNoError)
			return loadError;
		clearSceneRestartRequest();
		debugC(1, kDebugGeneral, "Loaded startup save slot %d", startupLoadSlot);
	}

	const int bootParam = startupLoad ? 0 : ConfMan.getInt("boot_param");
	const bool bootToIntroScene = isImplementedIntroSceneNumber(bootParam);
	const bool bootToGameplayScene = isImplementedGameplayState(bootParam);
	bool bootSceneReached = bootParam == 0;

	if (bootParam != 0) {
		if (bootToIntroScene || bootToGameplayScene) {
			debugC(1, kDebugGeneral, "Boot parameter requested scene/state %d", bootParam);
		} else {
			warning("Unsupported Hollywood boot parameter %d; starting normally", bootParam);
			bootSceneReached = true;
		}
	}

	if (!startupLoad && !bootToGameplayScene) {
		if (shouldRunSceneForBootParam(bootParam, 9000, bootSceneReached)) {
			Scene9000 scene9000(this);
			if (!scene9000.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9010, bootSceneReached)) {
			Scene9010 scene9010(this);
			if (!scene9010.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9100, bootSceneReached)) {
			Scene9100 scene9100(this);
			if (!scene9100.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9050, bootSceneReached)) {
			Scene9050 scene9050(this);
			if (!scene9050.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9120, bootSceneReached)) {
			Scene9120 scene9120(this);
			if (!scene9120.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9110, bootSceneReached)) {
			Scene9110 scene9110(this);
			if (!scene9110.play())
				return Common::kReadingFailed;
		}
	}

	if (!startupLoad) {
		Scene7000 scene7000(this);
		if (!scene7000.play())
			return Common::kReadingFailed;
		if (bootToGameplayScene && bootParam != 7000)
			gameState().mainFlowStateId = (uint16)gameplayStateForBootParam(bootParam);
	}

	bool handledState = true;
	while (!Engine::shouldQuit() && handledState) {
		handledState = false;
		clearSceneRestartRequest();
		const uint16 stateId = gameState().mainFlowStateId;

		if (stateId == kTravelScreenSelectionState) {
			handledState = true;
			TravelScreen travelScreen(this);
			uint16 selectedStateId = stateId;
			const byte currentChapterId = gameState().travelScreenCurrentChapterId != 0 ?
				gameState().travelScreenCurrentChapterId : 1;
			if (!travelScreen.runSelection(currentChapterId, selectedStateId))
				return Common::kReadingFailed;
			if (!Engine::shouldQuit() && !isSceneRestartRequested()) {
				gameState().mainFlowStateId = selectedStateId;
				gameState().travelScreenCurrentChapterId = 0;
			}
			continue;
		}

		if (stateId == kTitleFrontEndState) {
			handledState = true;
			Scene1000 scene1000(this);
			if (!scene1000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1010FirstState && stateId <= kScene1010LastState) {
			handledState = true;
			Scene1010 scene1010(this);
			if (!scene1010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1020FirstState && stateId <= kScene1020LastState) {
			handledState = true;
			Scene1020 scene1020(this);
			if (!scene1020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1030FirstState && stateId <= kScene1030LastState) {
			handledState = true;
			Scene1030 scene1030(this);
			if (!scene1030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1040FirstState && stateId <= kScene1040LastState) {
			handledState = true;
			Scene1040 scene1040(this);
			if (!scene1040.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1050FirstState && stateId <= kScene1050LastState) {
			handledState = true;
			Scene1050 scene1050(this);
			if (!scene1050.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1060FirstState && stateId <= kScene1060LastState) {
			handledState = true;
			Scene1060 scene1060(this);
			if (!scene1060.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1070FirstState && stateId <= kScene1070LastState) {
			handledState = true;
			Scene1070 scene1070(this);
			if (!scene1070.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1080FirstState && stateId <= kScene1080LastState) {
			handledState = true;
			Scene1080 scene1080(this);
			if (!scene1080.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene1090FirstState && stateId <= kScene1090LastState) {
			handledState = true;
			Scene1090 scene1090(this);
			if (!scene1090.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene2000State) {
			handledState = true;
			Scene2000 scene2000(this);
			if (!scene2000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene2030FirstState && stateId <= kScene2030LastState) {
			handledState = true;
			Scene2030 scene2030(this);
			if (!scene2030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene2040FirstState && stateId <= kScene2040LastState) {
			handledState = true;
			Scene2040 scene2040(this);
			if (!scene2040.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene3000State) {
			handledState = true;
			Scene3000 scene3000(this);
			if (!scene3000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3010FirstState && stateId <= kScene3010LastState) {
			handledState = true;
			Scene3010 scene3010(this);
			if (!scene3010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3020FirstState && stateId <= kScene3020LastState) {
			handledState = true;
			Scene3020 scene3020(this);
			if (!scene3020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3030FirstState && stateId <= kScene3030LastState) {
			handledState = true;
			Scene3030 scene3030(this);
			if (!scene3030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene3040State) {
			handledState = true;
			Scene3040 scene3040(this);
			if (!scene3040.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3050FirstState && stateId <= kScene3050LastState) {
			handledState = true;
			Scene3050 scene3050(this);
			if (!scene3050.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3060FirstState && stateId <= kScene3060LastState) {
			handledState = true;
			Scene3060 scene3060(this);
			if (!scene3060.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3070FirstState && stateId <= kScene3070LastState) {
			handledState = true;
			Scene3070 scene3070(this);
			if (!scene3070.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3080FirstState && stateId <= kScene3080LastState) {
			handledState = true;
			Scene3080 scene3080(this);
			if (!scene3080.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3090FirstState && stateId <= kScene3090LastState) {
			handledState = true;
			Scene3090 scene3090(this);
			if (!scene3090.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3100FirstState && stateId <= kScene3100LastState) {
			handledState = true;
			Scene3100 scene3100(this);
			if (!scene3100.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene3110FirstState && stateId <= kScene3110LastState) {
			handledState = true;
			Scene3110 scene3110(this);
			if (!scene3110.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene4000State) {
			handledState = true;
			Scene4000 scene4000(this);
			if (!scene4000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4010FirstState && stateId <= kScene4010LastState) {
			handledState = true;
			Scene4010 scene4010(this);
			if (!scene4010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4020FirstState && stateId <= kScene4020LastState) {
			handledState = true;
			Scene4020 scene4020(this);
			if (!scene4020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4030FirstState && stateId <= kScene4030LastState) {
			handledState = true;
			Scene4030 scene4030(this);
			if (!scene4030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4040FirstState && stateId <= kScene4040LastState) {
			handledState = true;
			Scene4040 scene4040(this);
			if (!scene4040.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4050FirstState && stateId <= kScene4050LastState) {
			handledState = true;
			Scene4050 scene4050(this);
			if (!scene4050.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4060FirstState && stateId <= kScene4060LastState) {
			handledState = true;
			Scene4060 scene4060(this);
			if (!scene4060.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4070FirstState && stateId <= kScene4070LastState) {
			handledState = true;
			Scene4070 scene4070(this);
			if (!scene4070.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene4080FirstState && stateId <= kScene4080LastState) {
			handledState = true;
			Scene4080 scene4080(this);
			if (!scene4080.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene5000State) {
			handledState = true;
			Scene5000 scene5000(this);
			if (!scene5000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene5010FirstState && stateId <= kScene5010LastState) {
			handledState = true;
			Scene5010 scene5010(this);
			if (!scene5010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene6000State) {
			handledState = true;
			Scene6000 scene6000(this);
			if (!scene6000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene6010FirstState && stateId <= kScene6010LastState) {
			handledState = true;
			Scene6010 scene6010(this);
			if (!scene6010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene6020FirstState && stateId <= kScene6020LastState) {
			handledState = true;
			Scene6020 scene6020(this);
			if (!scene6020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene6030FirstState && stateId <= kScene6030LastState) {
			handledState = true;
			Scene6030 scene6030(this);
			if (!scene6030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene8000State) {
			handledState = true;
			Scene8000 scene8000(this);
			if (!scene8000.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= kScene8010FirstState && stateId <= kScene8010LastState) {
			handledState = true;
			Scene8010 scene8010(this);
			if (!scene8010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene8020State) {
			handledState = true;
			Scene8020 scene8020(this);
			if (!scene8020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b62 && stateId <= 0x1b6b) {
			handledState = true;
			Scene7010 scene7010(this);
			if (!scene7010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == 0x1b6c) {
			handledState = true;
			Scene7020 scene7020(this);
			if (!scene7020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == kScene9101State) {
			handledState = true;
			Scene9100 scene9101(this);
			if (!scene9101.playDialogueBranch())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b76 && stateId <= 0x1b7f) {
			handledState = true;
			Scene7030 scene7030(this);
			if (!scene7030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b80 && stateId <= 0x1b89) {
			handledState = true;
			Scene7040 scene7040(this);
			if (!scene7040.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == 0x1b8a) {
			handledState = true;
			Scene7050 scene7050(this);
			if (!scene7050.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b94 && stateId <= 0x1b9d) {
			handledState = true;
			Scene7060 scene7060(this);
			if (!scene7060.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b9e && stateId <= 0x1ba0) {
			handledState = true;
			Scene7070 scene7070(this);
			if (!scene7070.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1ba8 && stateId <= 0x1bb1) {
			handledState = true;
			Scene7080 scene7080(this);
			if (!scene7080.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1bb2 && stateId <= 0x1bbb) {
			handledState = true;
			Scene7090 scene7090(this);
			if (!scene7090.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1bbc && stateId <= 0x1bc5) {
			handledState = true;
			Scene7100 scene7100(this);
			if (!scene7100.play())
				return Common::kReadingFailed;
			continue;
		}
	}

	debugC(1, kDebugGeneral, "Gameplay preview completed; current main-flow state is 0x%04x", gameState().mainFlowStateId);
	return Common::kNoError;
}

bool HollywoodEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher ||
		f == kSupportsSubtitleOptions ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime;
}

void HollywoodEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	const bool globalMute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	const bool musicMuted = ConfMan.getBool("music_mute");
	const bool sfxMuted = ConfMan.getBool("sfx_mute");
	const bool speechMuted = ConfMan.getBool("speech_mute");
	bool subtitlesEnabled = ConfMan.getBool("subtitles");
	if (speechMuted && !subtitlesEnabled) {
		ConfMan.setBool("subtitles", true);
		subtitlesEnabled = true;
	}

	_gameState.musicEnabled = !musicMuted;
	_gameState.soundEffectsEnabled = !sfxMuted;
	_gameState.musicVolumeLevel = configVolumeToOptionsLevel(ConfMan.getInt("music_volume"));
	_gameState.soundEffectsVolumeLevel = configVolumeToOptionsLevel(ConfMan.getInt("sfx_volume"));
	_gameState.voiceVolumeLevel = configVolumeToOptionsLevel(ConfMan.getInt("speech_volume"));
	_gameState.speechTextSpeedLevel = configTalkSpeedToOptionsLevel(ConfMan.getInt("talkspeed"));
	if (speechMuted)
		_gameState.actorSpeechTextMode = 2;
	else
		_gameState.actorSpeechTextMode = subtitlesEnabled ? 1 : 0;

	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, globalMute || musicMuted);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, globalMute || sfxMuted);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, globalMute || speechMuted);
}

void HollywoodEngine::syncSoundSettingsFromGameState() {
	const GameplayState &state = _gameState;
	ConfMan.setBool("music_mute", !state.musicEnabled);
	ConfMan.setBool("sfx_mute", !state.soundEffectsEnabled);
	ConfMan.setInt("music_volume", optionsLevelToConfigVolume(state.musicVolumeLevel));
	ConfMan.setInt("sfx_volume", optionsLevelToConfigVolume(state.soundEffectsVolumeLevel));
	ConfMan.setInt("speech_volume", optionsLevelToConfigVolume(state.voiceVolumeLevel));
	ConfMan.setInt("talkspeed", optionsLevelToConfigTalkSpeed(state.speechTextSpeedLevel));
	ConfMan.setBool("subtitles", state.actorSpeechTextMode != 0);
	ConfMan.setBool("speech_mute", state.actorSpeechTextMode == 2);

	syncSoundSettings();
}

const char *HollywoodEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Language HollywoodEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform HollywoodEngine::getPlatform() const {
	return _gameDescription->platform;
}

} // End of namespace Hollywood
