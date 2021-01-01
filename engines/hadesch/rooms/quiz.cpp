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
#include "hadesch/ambient.h"
#include "common/translation.h"

namespace Hadesch {

static const char *kQuestionBackground = "OverlayAnim";
static const char *kHadesEyes = "HadesEyesAnim";
static const char *kCounter = "CounterAnim";

enum {
	kBackgroundZ = 10000,
	kZeusLightAnimZ = 900,
	kHadesEyesZ = 850,
	kHadesAndZeusAnimZ = 850,
	kBigItemZ = 800,
	kSmallItemZ = 800,
	kFlameAnimZ = 800,
	kFlameBurstAnimZ = 799,
	kOverlayAnimZ = 550,
	kQuestionZ = 500,
	kCounterAnimZ = 549
};

static const char *questNames[] = {
	"None",
	"Crete",
	"Troy",
	"Medusa",
	"RescuePhil",
	"EndGame"
};

static const char *statueNames[] = {
	"Bacchus",
	"Hermes",
	"Zeus",
	"Poseidon",
	"Ares",
	"Aphrodite",
	"Apollo",
	"Artemis",
	"Demeter",
	"Athena",
	"Hera",
	"Hephaestus",
};

static const char *hadesIntroVideos[] = {
	"H0020bA0",
	"H0020bG0",
	"H0020bH0",
	"H0020bD0",
	"H0020bE0",
	"H0020bF0"
};

static const TranscribedSound h0090_names[] = {
	{ "H0090wF0", _s("Congratulations. You've shown Mr Sour Grapes") },
	{ "H0090wA0", _s("The enveloppe, please. And the winner is ... you. Hey, good job. That's showing him") },
	{ "H0090wB0", _s("Way to go") },
	// Difficult to hear. Please someone check after me
	{ "H0090wE0", _s("You're amazing. Or Hades is hard under the gollar") }
};

static const int kNumQuestions = 4;
static const int kNumAnswers = 5;

enum {
	kZeusStingerFinished = 30006,
	kZeusGreatFinished = 30007,
	kHadesVideoFinished = 30008,
	kFirstQuestion = 30010,
	kHadesQuestionVideoFinished = 30011,
	kHadesJokeVideoFinished = 30012,
	kHadesNagging = 30013,
	kHadesNaggingCleanup = 30014,
	kNextQuestion = 30015,
	kNextQuestionAfterFail = 30016,
	kBuzzerFinished = 30017,
	kDingFinished = 30018,
	kHadesFirstLaugh = 30019,
	kHadesInstructions = 30020,
	kFinished = 30028,
	kBigItemSwitchToLoop = 1030001
};

class QuizHandler : public Handler {
public:
	QuizHandler() {
	}

	void handleClick(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		for (int ansidx = 0; ansidx < kNumAnswers; ansidx++) {
			if (name == Common::String::format("A%d", ansidx + 1)) {
				nextQuestion(ansidx);
				return;
			}
		}
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		switch (eventId) {
		case kBigItemSwitchToLoop:
			room->playAnimWithSFX(
				_bigItem, "SpinningItemSnd", kBigItemZ, PlayAnimParams::loop().partial(4, -1));
			break;
		case kZeusStingerFinished:
			room->playVideo(_zeusGreat, 0, kZeusGreatFinished);
			break;
		case kZeusGreatFinished:
			room->stopAnim(_bigItem);
			room->playVideo(_hcQuest.get(_questName, "HadesBurst"), kHadesEyesZ,
					kHadesVideoFinished);
			break;
		case kHadesVideoFinished:
			room->selectFrame(kQuestionBackground, kOverlayAnimZ, 0);
			room->playAnimLoop("FlameAnim", kFlameAnimZ);
			room->playSFXLoop("FlameSnd");
			room->playMusicLoop("AmbientQuestionMusic");
			smallAnim();
			playHadesVideo(hadesIntroVideos[g_vm->getRnd().getRandomNumberRng(0, 5)],
					kFirstQuestion);
			break;
		case kNextQuestion:
		case kNextQuestionAfterFail:
			killQuestion();
			if (_currentQuestion == kNumQuestions - 1) {
				room->stopAnim("AmbientQuestionMusic");
				switch (_rightAnswerCount) {
				case 0:
					playHadesVideo(Common::String::format("H0090b%c0", 'E' + g_vm->getRnd().getRandomNumberRng(0, 2)),
						       kFinished);
					break;
				case 1:
				case 2:
					playHadesVideo(Common::String::format("H0090b%c0", 'B' + g_vm->getRnd().getRandomNumberRng(0, 2)),
						       kFinished);
					break;
				case 3:
				case 4:
					int v6 = g_vm->getRnd().getRandomNumberRng(0, 3);
					hadesAndZeus(h0090_names[v6], kFinished);
					break;
				}

				return;
			}
			_currentQuestion++;
			/* Fallthrough */
		case kFirstQuestion:
			_pauseMouseover = false;
			memset(_frames, 0, sizeof (_frames));
			smallAnim();
			renderQuestion();
			room->enableMouse();
			_hadesCancelableVideo = true;
			playHadesVideo(getQuestionAttribute("HQuestion"), kHadesQuestionVideoFinished);
			break;
		case kHadesQuestionVideoFinished:
			playHadesVideo(getQuestionAttribute("HJoke"), kHadesJokeVideoFinished);
			break;
		case kHadesJokeVideoFinished:
			room->selectFrame(kHadesEyes, kHadesEyesZ, 0);
			_hadesIsFree = true;
			_hadesCancelableVideo = false;
			break;
		case kDingFinished:
			playHadesVideo(Common::String::format("H0080b%c0", 'A' + (_hades_dislike_counter % 5)),
				       kNextQuestion);
			_hades_dislike_counter++;
			_rightAnswerCount++;
			break;

		case kHadesFirstLaugh:
			hadesAndZeus(TranscribedSound::make("ZeusNotFair", "Hold on, Hades. That's not fair. You've never explained the rules. That doesn't count"),
				     kHadesInstructions);
			_hadesIsFree = false;
			break;

		case kHadesInstructions:
			hadesAndZeusEnd();
			room->playAnimWithSFX("FlameBurstAnim", "FlameBurstSnd", kFlameBurstAnimZ,
					      PlayAnimParams::disappear(), 30021);
			_shrinkLevel--;
			break;

		case kHadesNagging:
			if (_hadesIsFree) {
				playHadesVideo(Common::String::format("H0050b%c0",
						       _naggingCounter + 'A'), kHadesNaggingCleanup);
				_naggingCounter = (_naggingCounter + 1) % 8;
			}
			break;

		case 30021:
			playHadesVideo("HadesInstructions", 30022);
			break;

		case 30022:
			room->selectFrame(kHadesEyes, kHadesEyesZ, 0);
			room->enableMouse();
			_hadesIsFree = true;
			memset(_frames, 0, sizeof (_frames));
			renderQuestion();
			break;

		case kHadesNaggingCleanup:
			room->selectFrame(kHadesEyes, kHadesEyesZ, 0);
			_hadesIsFree = true;
			break;

		case kBuzzerFinished:
			room->playAnimWithSFX("FlameBurstAnim", "FlameBurstSnd", kFlameBurstAnimZ,
					      PlayAnimParams::disappear());
			_shrinkLevel++;
			if (_wrongAnswerCount == 0) {
				playHadesVideo("HadesLaugh", kHadesFirstLaugh);
			} else {
				playHadesVideo(Common::String::format("H0040b%c0", 'A' + (_hades_like_counter % 5)),
					       kNextQuestionAfterFail);
				_hades_like_counter++;
			}
			_wrongAnswerCount++;
			break;
		case kFinished:
			persistent->_powerLevel[persistent->_quest-kCreteQuest] = countLevel();
			g_vm->moveToRoom(kWallOfFameRoom);
			persistent->_quest = (Quest) (persistent->_quest + 1);
			persistent->clearInventory();
			persistent->_doQuestIntro = true;
			break;
		}
	}

	void handleMouseOver(const Common::String &name) override {
		if (_pauseMouseover)
			return;
		for (int ansidx = 0; ansidx < kNumAnswers; ansidx++) {
			_frames[ansidx] = (name == Common::String::format("A%d", ansidx + 1)) ? 1 : 0;
		}
		renderQuestion();
	}

	void handleMouseOut(const Common::String &name) override {
		if (_pauseMouseover)
			return;
		memset(_frames, 0, sizeof (_frames));
		renderQuestion();
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();
		Quest quest = persistent->_quest;
		room->loadHotZones("HadesCh.HOT", true);
		room->addStaticLayer("Background", kBackgroundZ);
		room->disableHeroBelt();
		room->disableMouse();
		_questName = questNames[quest];
		_hcQuest = TextTable(
			Common::SharedPtr<Common::SeekableReadStream>(room->openFile("HcQuest.txt")), 5);
		_hcQList = TextTable(
			Common::SharedPtr<Common::SeekableReadStream>(room->openFile("HcQList.txt")), 12);
		_hchadesx = TextTable(
			Common::SharedPtr<Common::SeekableReadStream>(room->openFile("HcHadesX.txt")), 2);
		_bigItem = _hcQuest.get(_questName, "BigItem");
		_smallItem = _hcQuest.get(_questName, "SmallItem");
		_zeusGreat = _hcQuest.get(_questName, "ZeusGreat");
		room->playAnim(_bigItem, kBigItemZ,
			       PlayAnimParams::keepLastFrame().partial(0, 4), kBigItemSwitchToLoop);
		room->playVideo("ZeusStingerSnd", 0, kZeusStingerFinished);
		Common::HashMap<Common::String, bool> touchedStatues;
		for (int statue = kBacchusStatue; statue < kNumStatues;
		     statue++)
			if (persistent->_statuesTouched[statue])
				touchedStatues[statueNames[statue]] = true;

		Common::Array<int> untouchedQuestions;
		Common::Array<int> touchedQuestions;

		for (int i = 0; i < _hcQList.size(); i++)
			if (_hcQList.get(i, "Quest") == _questName) {
				Common::String statue = _hcQList.get(i, "Statue");
				if (touchedStatues[statue])
					touchedQuestions.push_back(i);
				else
					untouchedQuestions.push_back(i);
			}

		Common::Array<int> candidates;
		candidates.push_back(untouchedQuestions);

		if (candidates.size() <= kNumQuestions) {
			candidates.push_back(touchedQuestions);
		}

		while (_chosenQuestions.size() <= kNumQuestions) {
			int x = candidates[g_vm->getRnd().getRandomNumberRng(0, candidates.size() - 1)];
			bool valid = true;
			for (unsigned i = 0; i < _chosenQuestions.size(); i++) {
				if (_chosenQuestions[i] == x) {
					valid = false;
					break;
				}
			}

			if (!valid)
				continue;
			_chosenQuestions.push_back(x);
		}
		_currentQuestion = 0;
		_shrinkLevel = 0;
		_hadesCancelableVideo = false;
		memset(_frames, 0, sizeof (_frames));
		_hades_dislike_counter = g_vm->getRnd().getRandomBit();
		_hades_like_counter = g_vm->getRnd().getRandomBit();
		_naggingCounter = g_vm->getRnd().getRandomBit();
		g_vm->addTimer(kHadesNagging, 5000, -1);
		_rightAnswerCount = 0;
		_wrongAnswerCount = 0;
		_pauseMouseover = false;
		_hadesIsFree = false;
	}

private:
	int countLevel() {
		switch (_rightAnswerCount) {
		case 0:
			return 1;
		case 1:
		case 2:
			return 2;
		case 3:
		case 4:
		default:
			return 3;
		}
	}
	void killQuestion() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->stopAnim(getQuestionAttribute("Question"));
		for (int ansidx = 0; ansidx < kNumAnswers; ansidx++) {
			room->stopAnim(getQuestionAttribute(Common::String::format("A%d", ansidx + 1)));
		}
	}
	void renderQuestion() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->selectFrame(kCounter, kCounterAnimZ, _currentQuestion);
		room->selectFrame(getQuestionAttribute("Question"), kQuestionZ, 0);
		Common::Point q0, qstep;
		if (getQuestionAttribute("PrePlaced") == "0") {
			q0 = Common::Point(0, 256);
			qstep = Common::Point(0, 22);
		}
		for (int ansidx = 0; ansidx < kNumAnswers; ansidx++) {
			room->selectFrame(getQuestionAttribute(Common::String::format("A%d", ansidx + 1)), kQuestionZ,
					  _frames[ansidx], q0 + ansidx * qstep);
		}
	}

	void smallAnim() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnim(_smallItem, kSmallItemZ,
			       PlayAnimParams::loop().partial(_shrinkLevel * 30, _shrinkLevel * 30 + 29),
			       EventHandlerWrapper());
	}

	Common::String getQuestionAttribute(const Common::String &name) {
		return _hcQList.get(_chosenQuestions[_currentQuestion], name);
	}

	void playHadesVideo(const Common::String &name, int eventId) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		int x = 0;
		if (name == "HadesInstructions" || name == "HadesLaugh") {
			x = 110;
		} else
			x = _hchadesx.get(name, "X").asUint64();
		room->stopAnim(kHadesEyes);
		room->stopAnim("HadesAndZeusAnim");
		room->playVideo(name, kHadesEyesZ, eventId, Common::Point(x, 0));
		_hadesIsFree = false;
	}

	int getRightAnswer() {
		return (int) getQuestionAttribute("RightAnswer").asUint64() - 1;
	}

	void nextQuestion(int selected) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->disableMouse();
		if (_hadesCancelableVideo)
			room->cancelVideo();
		_hadesCancelableVideo = false;
		_hadesIsFree = false;
		room->selectFrame(kHadesEyes, kHadesEyesZ, 0);
		if (selected == getRightAnswer())
			room->playSFX("DingSnd", kDingFinished);
		else
			room->playSFX("BuzzerSnd", kBuzzerFinished);
		memset(_frames, 0, sizeof (_frames));
		for (int ansidx = 0; ansidx < kNumAnswers; ansidx++) {
			_frames[ansidx] = 5;
		}
		if (selected == getRightAnswer() || _wrongAnswerCount != 0) {
			_frames[getRightAnswer()] = 1;
		}
		_pauseMouseover = true;
		renderQuestion();
	}

	void hadesAndZeus(const TranscribedSound &name, int event) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnimWithSpeech("HadesAndZeusAnim", name, kHadesAndZeusAnimZ,
					PlayAnimParams::keepLastFrame().partial(0, 5), event);
		room->playAnim("ZeusLightAnim", kZeusLightAnimZ, PlayAnimParams::keepLastFrame());
		_hadesIsFree = false;
	}

	void hadesAndZeusEnd() {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnim("HadesAndZeusAnim", kHadesAndZeusAnimZ,
			       PlayAnimParams::keepLastFrame().partial(6, 11));
		room->playAnim("ZeusLightAnim", kZeusLightAnimZ, PlayAnimParams::disappear().backwards());
	}

	TextTable _hcQuest;
	TextTable _hcQList;
	TextTable _hchadesx;
	Common::Array<int> _chosenQuestions;
	int _currentQuestion;
	int _shrinkLevel;
	int _frames[kNumAnswers];
	bool _pauseMouseover;
	int _hades_dislike_counter;
	int _hades_like_counter;
	int _rightAnswerCount;
	int _wrongAnswerCount;
	bool _hadesCancelableVideo;
	bool _hadesIsFree;
	int _naggingCounter;
	Common::String _questName;
	Common::String _bigItem, _smallItem, _zeusGreat;
};

Common::SharedPtr<Hadesch::Handler> makeQuizHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new QuizHandler());
}

}
