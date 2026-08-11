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
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/ambient.h"

namespace Hadesch {

static const char *caTxtNames[] = {
	"CaLeft.txt",
	"CaCenter.txt",
	"CaRight.txt"
};

static const char *skullHotzones[] = {
	"LSkull",
	"CSkull",
	"RSkull"
};

static const char *torchHotzones[] = {
	"LTorch",
	"CTorch",
	"RTorch"
};

static const char *signNames[] = {
	"SignToHelen",
	"SignToGuards",
	"SignToPainPanic"
};

static const char *musicNames[] = {
	"MusicHelen",
	"MusicGuard",
	"MusicPainPanic"
};

static const TranscribedSound painSounds[] = {
	{"SndPainBedtime", _hs("It's bed time")},
	{"SndPanicBoneHead", _hs("Hey there, bonehead")},
	{"SndPainRecognize", _hs("Recognize the jewel?")} // Unclear
};

static const TranscribedSound painSounds2[] = {
	{"SndPanicLightsOut", _hs("He-he. Lights out") },
	{"SndPainByeBye", _hs("Bye-Bye")},
	{"SndPanicMaybeHit", _hs("Maybe it will hit ya")}
};

static const TranscribedSound guardSpeeches[] = {
	{"T3220wA0", _hs("Do you think we were going to let you just walk into Troy?")},
	// FIXME: Spelling incorrect. noone should be no one. Fixing changes game data and thus may cause issues
	{"T3220wB0", _hs("So sorry, noone is allowed in. So beat it")},
	{"T3220wC0", _hs("Hey, Troy is closed to all visitors. Take a hike")}
};

enum {
	kBackgroundCenterZ = 10001,
	kBackgroundZ = 10000
};

enum {
	// Originally 22014 for all 3 but I'd rather avoid passing extra args around
	kL1TrochLitLeft = 1022001,
	kL1TrochLitCenter = 1022002,
	kL1TrochLitRight = 1022003,
	kBonkVideoFinished = 1022004
};

class CatacombsHandler : public Handler {
public:
	CatacombsHandler() {
		_philWarnedTorch = false;
		_philBangPlayed = false;
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;

		if (name == "LExit") {
			handleExit(kCatacombsLeft);
			return;
		}
		if (name == "CExit") {
			handleExit(kCatacombsCenter);
			return;
		}
		if (name == "RExit") {
			handleExit(kCatacombsRight);
			return;
		}
		if (level == 0 && (name == "LTorch" || name == "CTorch" || name == "RTorch")) {
			g_vm->getHeroBelt()->placeToInventory(kTorch);
			room->stopAnim(caVariantGet(_torchPosition, "TorchNormal"));
			return;
		}

		if (level == 1 && name == "LTorch") {
			lightTorchL1(kCatacombsLeft);
			return;
		}

		if (level == 1 && name == "CTorch") {
			lightTorchL1(kCatacombsCenter);
			return;
		}

		if (level == 1 && name == "RTorch") {
			lightTorchL1(kCatacombsRight);
			return;
		}

		if (name == "LSkull" || name == "CSkull" || name == "RSkull") {
			_decoderPosition = 0;
			renderDecoder();
			if (!_philBangPlayed) {
				_philBangPlayed = true;
				room->playSFX("SndBigBang", 22012);
			}
			return;
		}

		if (name == "DecoderDown" && _decoderPosition < 6) {
			_decoderPosition++;
			renderDecoder();
			room->playAnim("AnimDecoderArrows", 149, PlayAnimParams::disappear().partial(0, 0));
			return;
		}

		if (name == "DecoderUp" && _decoderPosition > 0) {
			_decoderPosition--;
			renderDecoder();
			room->playAnim("AnimDecoderArrows", 149, PlayAnimParams::disappear().partial(1, 1));
			return;
		}

		if (name == "DecoderDone") {
			removeDecoder();
			return;
		}
	}

	bool handleClickWithItem(const Common::String &name, InventoryItem item) override {
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;
		if (item == kTorch && level == 1) {
			if (name == "LTorch") {
				lightTorchL1(kCatacombsLeft);
				return true;
			}

			if (name == "CTorch") {
				lightTorchL1(kCatacombsCenter);
				return true;
			}

			if (name == "RTorch") {
				lightTorchL1(kCatacombsRight);
				return true;
			}
		}
		return false;
	}


	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;
		switch(eventId) {
		case kL1TrochLitLeft:
		case kL1TrochLitCenter:
		case kL1TrochLitRight: {
			CatacombsPosition side = (CatacombsPosition) (eventId - kL1TrochLitLeft + kCatacombsLeft);
			bool isHelen = persistent->_catacombPaths[1][side] == kCatacombsHelen;
			room->playAnimLoop(
				caVariantGet(side, isHelen ? "TorchLong" : "TorchNormal"),
				caVariantGet(side, "TorchZ").asUint64());
			break;
		}
		case 22009:
			room->playVideo("PhilQuickNameThatTune", 0);
			break;
		case 22012:
			room->playVideo("PhilWowLowOnTroops", 0);
			break;
		case 22016:
			room->playSFX("SndGuardTrapDoorOpen", 22017);
			break;
		case 22017:
			room->playSpeech(TranscribedSound::make("SndGuardLaugh", "[laughter]"), 22018);
			break;
		case 22018:
			room->playSpeech(
				guardSpeeches[g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(guardSpeeches) - 1)],
				22019);
			break;
		case 22019:
			room->playSFX("SndGuardTrapDoorClose", 22020);
			break;
		case 22020:
			persistent->_catacombLevel = kCatacombLevelSign;
			g_vm->moveToRoom(kTroyRoom);
			break;
		case 22022:
			room->playSpeech(painSounds2[level], 22023);
			persistent->_catacombLevel = kCatacombLevelSign;
			break;
		case 22023:
			room->playVideo("MovPainPanicBonk", 103, kBonkVideoFinished);
			break;
		case kBonkVideoFinished:
			g_vm->moveToRoom(kTroyRoom);
			break;
		}
	}

	void handleMouseOver(const Common::String &name) override {
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;

		if (level == 2) {
			if (name == "LExit") {
				playTune(kCatacombsLeft);
				return;
			}
			if (name == "CExit") {
				playTune(kCatacombsCenter);
				return;
			}
			if (name == "RExit") {
				playTune(kCatacombsRight);
				return;
			}
		}
	}

	void handleMouseOut(const Common::String &name) override {
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;

		if (level == 2 && (name == "LExit" || name == "CExit" || name == "RExit"))
			stopTune();
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		CatacombsLevel level = persistent->_catacombLevel;

		persistent->_catacombLastLevel = level;

		if (persistent->_catacombPainAndPanic) {
			persistent->_catacombPainAndPanic = false;
			room->addStaticLayer("DeadEndBackground", 10001);
			room->playMusic("SndPainPanicStinger", 22022);
			room->playSpeech(painSounds[persistent->_catacombLevel]);
			return;
		}

		if (level == 0)
			room->loadHotZones("CaDecode.HOT", false);
		room->playMusicLoop("T3010eA0");
		// TODO: tremmors
		// TODO: handle timer
		g_vm->addTimer(22007, level == 2 ? 30000 : 40000, -1);

		for (int i = 0; i < 3; i++)
			_caMapTxt[i] = TextTable(
				Common::SharedPtr<Common::SeekableReadStream>(room->openFile(caTxtNames[i])), 13);

		if (persistent->_catacombPaths[0][0] == kCatacombsHelen
		    && persistent->_catacombPaths[1][0] == kCatacombsHelen) {
			for (int i = 0; i < 3; i++) {
				Common::Array<int> p3 = permute3();
				persistent->_catacombVariants[0][i] = p3[0];
				persistent->_catacombVariants[1][i] = p3[1];
				persistent->_catacombVariants[2][i] = p3[2];
			}
			for (int i = 0; i < 3; i++) {
				Common::Array<int> p3 = permute3();
				persistent->_catacombPaths[i][0] = (CatacombsPath) p3[0];
				persistent->_catacombPaths[i][1] = (CatacombsPath) p3[1];
				persistent->_catacombPaths[i][2] = (CatacombsPath) p3[2];
			}
			persistent->_catacombDecoderSkullPosition = (CatacombsPosition) g_vm->getRnd().getRandomNumberRng(0, 2);
		}

		for (CatacombsPosition i = kCatacombsLeft; i <= kCatacombsRight; i = (CatacombsPosition) (i + 1)) {
			room->loadHotZones(
				caVariantGet(i, "Hotspots"), false);
			room->addStaticLayer(
				caVariantGet(i, "Background"),
				i == kCatacombsCenter ? kBackgroundCenterZ : kBackgroundZ);
		}

		if (persistent->_catacombVariants[level][0] == 2) {
			room->playAnimLoop("GlowingEyes", 900);
		}

		room->enableHotzone("LExit");
		room->enableHotzone("CExit");
		room->enableHotzone("RExit");

		switch (level) {
		case 0:
			room->playMusic("IntroMusic");
			room->enableHotzone(skullHotzones[persistent->_catacombDecoderSkullPosition]);
			room->selectFrame(
				caVariantGet(persistent->_catacombDecoderSkullPosition, "SkullDecoder"), 450, 1);
			for (CatacombsPosition i = kCatacombsLeft; i <= kCatacombsRight; i = (CatacombsPosition) (i + 1)) {
				room->selectFrame(
					caVariantGet(i, "SignBoard"), 501, 0);
				room->selectFrame(
					caVariantGet(i, signNames[persistent->_catacombPaths[level][i]]), 500, 0);
			}
			if (!persistent->isInInventory(kTorch)) {
				_torchPosition = (CatacombsPosition) g_vm->getRnd().getRandomNumberRng(0, 2);
				room->enableHotzone(torchHotzones[_torchPosition]);
				room->playAnimLoop(
					caVariantGet(_torchPosition, "TorchNormal"),
					caVariantGet(_torchPosition, "TorchZ").asUint64());
			}
			break;
		case 1:
			room->enableHotzone("LTorch");
			room->enableHotzone("CTorch");
			room->enableHotzone("RTorch");
			for (CatacombsPosition side = kCatacombsLeft; side <= kCatacombsRight; side = (CatacombsPosition) (side + 1)) {
				room->selectFrame(
					caVariantGet(side, "TorchNormalBurst"),
					caVariantGet(side, "TorchZ").asUint64(), 0);
			}
			break;
		case 2:
			room->playSFX("CollapseSnd", 22009);
			for (CatacombsPosition side = kCatacombsLeft; side <= kCatacombsRight; side = (CatacombsPosition) (side + 1)) {
				room->playAnimLoop(
					caVariantGet(side, "TorchNormal"),
					caVariantGet(side, "TorchZ").asUint64());
			}
			break;
		}

		g_vm->getHeroBelt()->setColour(HeroBelt::kCool);
	}

private:
	void stopTune() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (int i = 0; i < 3; i++)
			room->stopAnim(musicNames[i]);
	}

	// TODO: how can we make tune challenge accessible to deaf people?
	void playTune(CatacombsPosition side) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		stopTune();
		room->playMusicLoop(musicNames[persistent->_catacombPaths[2][side]]);
	}

	void renderDecoder() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		room->selectFrame("AnimDecoderScroll", 151, 0);
		room->selectFrame("AnimDecoderSymbols", 150, _decoderPosition);
		room->selectFrame(
			caVariantGet(persistent->_catacombDecoderSkullPosition, "SkullDecoder"), 450, 0);
		room->enableHotzone("DecoderDone");
		room->enableHotzone("DecoderDown");
		room->enableHotzone("DecoderUp");
	}

	void removeDecoder() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		room->stopAnim("AnimDecoderScroll");
		room->stopAnim("AnimDecoderSymbols");
		room->selectFrame(
			caVariantGet(persistent->_catacombDecoderSkullPosition, "SkullDecoder"), 450, 1);
		room->stopAnim("AnimDecoderArrows");
		room->disableHotzone("DecoderDone");
		room->disableHotzone("DecoderDown");
		room->disableHotzone("DecoderUp");
	}

	void handleExit(CatacombsPosition side) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;

		if (level == 0 && !_philWarnedTorch && !persistent->isInInventory(kTorch) && persistent->_hintsAreEnabled) {
			_philWarnedTorch = true;
			room->playVideo("PhilGrabTheTorch", 0, 22003);
			return;
		}

		switch (persistent->_catacombPaths[level][side]) {
		case kCatacombsHelen:
			room->disableMouse();
			if (persistent->_catacombLevel == kCatacombLevelMusic) {
				persistent->_catacombLevel = kCatacombLevelSign;
				g_vm->moveToRoom(kPriamRoom);
			} else {
				persistent->_catacombLevel = (CatacombsLevel) (persistent->_catacombLevel + 1);
				g_vm->moveToRoom(kCatacombsRoom);
			}
			break;
		case kCatacombsGuards:
			room->disableMouse();
			g_vm->cancelTimer(22007);
			room->fadeOut(1000, 22016);
			break;
		case kCatacombsPainAndPanic:
			room->disableMouse();
			g_vm->cancelTimer(22007);
			persistent->_catacombPainAndPanic = true;
			g_vm->moveToRoom(kCatacombsRoom);
			break;
		}
	}

	void lightTorchL1(CatacombsPosition side) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		bool isHelen = persistent->_catacombPaths[1][side] == kCatacombsHelen;
		room->playAnim(
			caVariantGet(side, isHelen ? "TorchLongBurst" : "TorchNormalBurst"),
			caVariantGet(side, "TorchZ").asUint64(),
			PlayAnimParams::disappear(), kL1TrochLitLeft + side - kCatacombsLeft);
		room->playSFX("SndTorchBurst");
		room->disableHotzone(torchHotzones[side]);
	}

	Common::String caVariantGet(CatacombsPosition side, const Common::String &property) {
		Persistent *persistent = g_vm->getPersistent();
		int level = persistent->_catacombLevel;
		int variant = persistent->_catacombVariants[level][side];
		Common::String ret = _caMapTxt[side].get(variant, property);
		if (ret == "") {
			debug("No attrinute for %d/%s", side, property.c_str());
		}
		return ret;
	}

	Common::Array<int> permute3() {
		Common::Array <int> ret;
		int x = g_vm->getRnd().getRandomNumberRng(0, 5);
		int a = x / 2;
		ret.push_back(a);
		int cand1 = a == 0 ? 1 : 0;
		int cand2 = 0;
		for (cand2 = 0; cand2 == a || cand2 == cand1; cand2++);
		if (x % 2) {
			ret.push_back(cand2);
			ret.push_back(cand1);
		} else {
			ret.push_back(cand1);
			ret.push_back(cand2);
		}
		return ret;
	}

	CatacombsPosition _torchPosition;
	TextTable _caMapTxt[3];
	bool _philWarnedTorch;
	bool _philBangPlayed;
	int _decoderPosition;
};

Common::SharedPtr<Hadesch::Handler> makeCatacombsHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new CatacombsHandler());
}

}
