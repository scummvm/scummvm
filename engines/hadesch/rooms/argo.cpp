/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "common/translation.h"

namespace Hadesch {

static const char *kIslandNames = "islandnames";
static const char *kMastHeadAnim = "mastheadanim";

enum {
	kSkyZ = 10200,
	kCloudsZ = 10100,
	kWavesRightZ = 10050,
	kWavesLeftZ = 10050,
	kBackgroundZ = 10000,
	kFlagsZ = 9000,
	kMastHeadZ = 8000,
	kChessPieceZ = 701,
	kIslandNamesZ = 601
};

static const struct island {
	const char *hotname;
	const char *mouseoverAnim;
	TranscribedSound nameSound;
	const char *sfxSound;
	RoomId roomId;
	int zValue;
} islands[] = {
	{"Phils", "a1030bh0", {"a1030nf0", _s("Phil's") }, "a1030ef0", kWallOfFameRoom, 901},
	{"Medusa", "a1030bf0", {"a1030nc0", _s("Medusa Isle")}, "a1030ed0", kMedIsleRoom, 901},
	{"Troy", "a1030bd0", {"a1030na0", _s("Troy")}, "a1030eb0", kTroyRoom, 901},
	{"Seriphos", "a1030be0", {"a1030nd0", _s("Seriphos")}, "a1030ec0", kSeriphosRoom, 801},
	{"Crete", "a1030bc0", {"a1030nb0", _s("Crete")}, "a1030ea0", kCreteRoom, 801},
	{"Volcano", "a1030bg0", {"a1030ne0", _s("Volcano island")}, "a1030ee0", kVolcanoRoom, 801},
};

static const int nislands = ARRAYSIZE(islands);

static const TranscribedSound intros[] = {
	{ "a1150na0", _s("Aye, welcome onboard ladie") },
	{ "a1150nb0", _s("So, are you hero yet?") },
	{ "a1150nc0", _s("So, are you heroine yet?") },
	{ "a1150nd0", _s("So, made it back, you did? Frankly, I'm surprised") },
	{ "a1150ne0", _s("Glad I'm, you're still alive. I hate sailing alone") },
	{ "a1150nf0", _s("So where will we be headed now?") }
};

static const TranscribedSound defaultOutros[] = {
	{ "a1170na0", _s("Heave anchor") },
	{ "a1170nb0", _s("Hurry, hoist the main") },
	{ "a1170nc0", _s("All hands on deck. Man the sails") },
	{ "a1170nd0", _s("Pull her to starboard and bring her around") },
	{ "a1170ne0", _s("Pull back on that rudder. Hold her steady") }
};

enum {
	kPlayIntro2 = 27001,
	kPlayIntro3 = 27002,
	kReturnToIdleEvent = 27003,
	kIdleEvent = 27008,
	kOutroFinished = 27009,
	kOutroFinishedCounter = 1027001,
	kMastSoundFinished = 1027002
};

static const TranscribedSound
getOutroName(RoomId dest) {
	Persistent *persistent = g_vm->getPersistent();
	Quest quest = persistent->_quest;

	switch (dest) {
	case kWallOfFameRoom:
		if (!persistent->_argoSailedInQuest[dest][quest])
			return TranscribedSound::make("philsfirst", "That'd be where the grand heroes and heroines of the world go to train");
		break;
	case kSeriphosRoom:
		if ((quest == kTroyQuest || quest == kCreteQuest) && !persistent->_argoSailedInQuest[dest][quest])
			return TranscribedSound::make("seriphoscretetroy", "This place be ruled by the evil tyrant king Polydectes");
		if (quest == kMedusaQuest && !persistent->_argoSailedInQuest[dest][quest])
			return TranscribedSound::make("seriphosperseus", "Arr, Perseus be in trouble deep. Could use a hand");
		break;
	case kMedIsleRoom:
		if (quest == kMedusaQuest && !persistent->_argoSailedInQuest[dest][quest])
			return TranscribedSound::make("medusabeware", "Beware of Medusa. She be one scary looking lady. All her mirrors be made of shatter-proof glass");
		break;
	case kTroyRoom:
		if (!persistent->isRoomVisited(kTroyRoom))
			return TranscribedSound::make("troytenyears", "For ten years now trojan and greek soldiers have been fighting that trojan war. Talk about job security");
		if (quest == kTroyQuest && !persistent->_argoSailedInQuest[dest][quest])
			return TranscribedSound::make("troyregards", "Send me regards to Odysseus");
		if (quest > kTroyQuest && !persistent->_argoSaidTroyFinally) {
			persistent->_argoSaidTroyFinally = true;
			return TranscribedSound::make("troyfinally", "Finally, the trojan war be over and Helen be back with Menelaus. Now those two can fight without an interruption");
		}
		break;
	case kCreteRoom:
		if (!persistent->isRoomVisited(kCreteRoom))
			return TranscribedSound::make("cretedaedalus", "This be where Daedalus, the inventor, lives");

		if (quest != kCreteQuest && !persistent->_argoSaidCretePort)
			return TranscribedSound::make("creteport", "Crete, the famous international port of trade");
		break;
	case kVolcanoRoom:
		if (!persistent->isRoomVisited(kVolcanoRoom))
			return TranscribedSound::make("volcanotopfirst", "Know this: should you go down there, you may not come back");

		if (quest == kRescuePhilQuest && !!persistent->_argoSailedInQuest[dest][quest])
			return TranscribedSound::make("volcanotopyoufirst", "Hah, many are monsters down there. Very dangerous. You go first");
		break;

	default:
		assert(0);
	}
	int rnd = g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(defaultOutros) - 1);
	debug("rnd = %d", rnd);
	return defaultOutros[rnd];
}

class ArgoHandler : public Handler {
public:
	ArgoHandler() {
		_prevId = kInvalidRoom;
		_destination = kInvalidRoom;
		_mastHeadIsBusy = false;
	}
	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_destination = kInvalidRoom;
		for (unsigned i = 0; i < nislands; i++) {
			if (name == islands[i].hotname) {
				_destination = islands[i].roomId;
				break;
			}
		}
		if (_destination != kInvalidRoom) {
		  	Persistent *persistent = g_vm->getPersistent();
			room->disableMouse();
			room->stopAnim("idlesound");
			if (_destination == _prevId) {
				playMastSound(TranscribedSound::make(
						  "currentlocation",
						  "Here be your current location, matie."),
					      kOutroFinished);
				return;
			}

			_outroCounter = 4;
			_cloudsMoving = true;
			_cloudsMoveStart = g_vm->getCurrentTime();
			playMastSound(getOutroName(_destination), kOutroFinishedCounter);
			room->playAnimWithSFX("wavesleft", "wavesleftSFX", kWavesLeftZ,
					      PlayAnimParams::disappear(),
					      kOutroFinishedCounter);
			room->playAnimWithSFX("wavesright", "wavesrightSFX", kWavesRightZ,
					      PlayAnimParams::disappear(),
					      kOutroFinishedCounter);
			room->playSFX("A1030eG0", kOutroFinishedCounter);
			persistent->_argoSailedInQuest[_destination][persistent->_quest] = true;
		}
	}
	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		switch (eventId) {
		case kPlayIntro2:
			playMastSound(TranscribedSound::make("intro2", "Navigate by clicking on the island you want to go to"), kPlayIntro3);
			break;
		case kPlayIntro3:
			playMastSound(TranscribedSound::make(
					      "intro3",
					      "The map shall always show the location of the Argo in relation to the other islands in the region"),
				      kReturnToIdleEvent);
			break;
		case kReturnToIdleEvent:
			_mastHeadIsBusy = false;
			room->enableMouse();
			break;
		case kOutroFinishedCounter:
			if (--_outroCounter > 0)
				break;
			// Fallthrough
		case kOutroFinished:
			room->selectFrame(kMastHeadAnim, kMastHeadZ, 0);
			g_vm->moveToRoom(_destination);
			break;
		case kIdleEvent:
			g_vm->addTimer(kIdleEvent, 30000);
			if (_mastHeadIsBusy)
				break;
			playMastSound(TranscribedSound::make("idlesound", "And what course lies ahead for you, matie?"), kMastSoundFinished);
			room->selectFrame(kMastHeadAnim, kMastHeadZ, 1);
			break;
		case 27301:
			room->playAnimWithSpeech(kMastHeadAnim, _mastSound, kMastHeadZ,
						 PlayAnimParams::keepLastFrame().partial(8, 21), 27303);
			break;
		// 27302 was for event chaining and frame keeping
		case 27303:
			room->playAnim(kMastHeadAnim, kMastHeadZ,
				       PlayAnimParams::keepLastFrame().partial(8, 0), _mastHeadEndEvent);
			break;
		case kMastSoundFinished:
			_mastHeadIsBusy = false;
			break;
		}
	}
	void handleMouseOver(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (unsigned i = 0; i < nislands; i++) {
			if (name == islands[i].hotname) {
				room->selectFrame(kIslandNames, kIslandNamesZ, i);
				room->playAnimKeepLastFrame(islands[i].mouseoverAnim, islands[i].zValue);
				playMastSound(islands[i].nameSound);
				room->playSFXLoop(islands[i].sfxSound);
				return;
			}
		}
	}
	
	void handleMouseOut(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (unsigned i = 0; i < nislands; i++)
			if (name == islands[i].hotname) {
				if (_destination != islands[i].roomId) {
					room->stopAnim(kIslandNames);
					room->stopAnim(islands[i].mouseoverAnim);
				}
				room->stopAnim(islands[i].nameSound.soundName);
				room->stopAnim(islands[i].sfxSound);
				return;
			}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		_prevId = g_vm->getPreviousRoomId();
		room->loadHotZones("argo.HOT");
		room->addStaticLayer("background", kBackgroundZ);
		Common::String sky;
		int chesspiece;
		Common::String bgsound;

		switch (_prevId) {
		default:
			sky = "bluesky";
			chesspiece = 0;
			bgsound = "a1180ea0";
			break;
		case kSeriphosRoom:
			sky = "pinksky";
			chesspiece = 3;
			bgsound = "A1070eA0";
			break;
		case kMedIsleRoom:
			sky = "mauvesky";
			chesspiece = 1;
			bgsound = "a1210ea0";
			break;
		case kTroyRoom:
			sky = "goldsky";
			chesspiece = 2;
			bgsound = "a1190eb0";
			break;
		case kCreteRoom:
			sky = "bluesky";
			chesspiece = 4;
			bgsound = "a1180ea0";
			break;
		case kVolcanoRoom:
			sky = "pinksky";
			chesspiece = 5;
			bgsound = "a1210ea0";
			break;
		}
		room->addStaticLayer(sky, kSkyZ);
		room->playMusicLoop(bgsound);

		room->selectFrame("chesspiece", kChessPieceZ, chesspiece);

		room->disableMouse();
		// Originally event 4015
		if (!persistent->isRoomVisited(kArgoRoom))
			playMastSound(TranscribedSound::make(
					      "intro1",
					      "Sharpen up now, matie. You'll be on the Argo now. It's a hero of ships. It used to belong to Jason and his crew, the argonauts. And now it'll be here for you"),
				      kPlayIntro2);
		else {
			int rnd = g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(intros) - 1);
			debug("rnd = %d", rnd);
			if (rnd == 1 || rnd == 2)
				rnd = persistent->_gender == kFemale ? 2 : 1;
			playMastSound(intros[rnd], kReturnToIdleEvent);
		}

		room->playAnimWithSFX("flags", "flagsSFX", kFlagsZ, PlayAnimParams::loop());
		g_vm->addTimer(kIdleEvent, 30000);
		g_vm->getHeroBelt()->setColour(HeroBelt::kCool);
		room->playMusic("intromusic");
		_cloudsMoving = false;
		cloudMove(0);
	}

	void frameCallback() override {
		if (_cloudsMoving) {
			cloudMove(g_vm->getCurrentTime() - _cloudsMoveStart);
		}
	}

	void cloudMove(int cloudMoveTime) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		double div = cloudMoveTime / 15000.0;
		room->selectFrame("cloudright", kCloudsZ, 0, Common::Point(450, 0) + Common::Point(650, -50) * div);
		room->selectFrame("cloudmiddle", kCloudsZ, 1, Common::Point(220, 0) + Common::Point(220, -50) * div);
		room->selectFrame("cloudleft", kCloudsZ, 2, Common::Point(0, 0) + Common::Point(-200, -50) * div);
	}
	
private:
	void playMastSound(const TranscribedSound &sound, int event = kMastSoundFinished) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_mastSound = sound;
		_mastHeadEndEvent = event;
		_mastHeadIsBusy = true;
		room->playAnim(kMastHeadAnim, kMastHeadZ, PlayAnimParams::keepLastFrame().partial(1, 8), 27301);
	}

	RoomId _prevId;
	RoomId _destination;
	int _outroCounter;
	int _cloudsMoveStart;
	bool _cloudsMoving;
	int _mastHeadEndEvent;
	bool _mastHeadIsBusy;
	TranscribedSound _mastSound;
};

Common::SharedPtr<Hadesch::Handler> makeArgoHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new ArgoHandler());
}

}
