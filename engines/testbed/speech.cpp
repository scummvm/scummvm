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
 */

#include "common/text-to-speech.h"
#include "common/system.h"
#include "common/events.h"
#include "common/array.h"
#include "engines/testbed/speech.h"

namespace Testbed {

void Speechtests::waitForSpeechEnd(Common::TextToSpeechManager *ttsMan) {
	Common::Event event;
	while (ttsMan->isSpeaking()) {
		g_system->delayMillis(100);
		g_system->getEventManager()->pollEvent(event);
	}
}

TestExitStatus Speechtests::testMale() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	Testsuite::clearScreen();
	Common::String info = "Male voice test. You should expect a male voice to say \"Testing text to speech with male voice.\"";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing male TTS voice", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testMale\n");
		return kTestSkipped;
	}

	Common::Array<int> maleVoices = ttsMan->getVoiceIndicesByGender(Common::TTSVoice::MALE);
	if (maleVoices.size() == 0) {
		Testsuite::displayMessage("No male voice available");
		return kTestFailed;
	}
	ttsMan->setVoice(maleVoices[0]);
	ttsMan->say("Testing text to speech with male voice.");
	if (!ttsMan->isSpeaking()) {
		Testsuite::logDetailedPrintf("Male TTS failed\n");
		return kTestFailed;
	}
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear male voice saying: \"Testing text to speech with male voice.\" ?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("Male TTS failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testFemale() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	Testsuite::clearScreen();
	Common::String info = "Female voice test. You should expect a female voice to say \"Testing text to speech with female voice.\"";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing female TTS voice", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testFemale\n");
		return kTestSkipped;
	}

	Common::Array<int> femaleVoices = ttsMan->getVoiceIndicesByGender(Common::TTSVoice::FEMALE);
	if (femaleVoices.size() == 0) {
		Testsuite::logDetailedPrintf("Female TTS failed\n");
		return kTestFailed;
	}
	ttsMan->setVoice(femaleVoices[0]);
	ttsMan->say("Testing text to speech with female voice.");
	if (!ttsMan->isSpeaking()) {
		Testsuite::logDetailedPrintf("Female TTS failed\n");
		return kTestFailed;
	}
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear female voice saying: \"Testing text to speech with female voice.\" ?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("Female TTS failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testStop() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech stop test. You should expect a voice to start speaking and after approximately a second it should stop the speech";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS stop", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testStop\n");
		return kTestSkipped;
	}

	ttsMan->say("Testing text to speech, the speech should stop after approximately a second after it started, so it shouldn't have the time to read this.");
	g_system->delayMillis(1000);
	ttsMan->stop();
	// It is allright if the voice isn't available right away, but a second should be
	// enough for the TTS to recover and get ready.
	g_system->delayMillis(1000);
	if (!ttsMan->isReady()) {
		Testsuite::logDetailedPrintf("TTS stop failed\n");
		return kTestFailed;
	}
	Common::String prompt = "Did you hear a voice saying: \"Testing text to speech, the speech should stop after approximately a second after it started, so it shouldn't have the time to read this.\" but stopping in the middle?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS stop failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testPauseResume() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech pause test. You should expect a voice to start speaking, then after approximately a second of speech, it should pause and then continue from where it left.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS pause", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testPauseResume\n");
		return kTestSkipped;
	}

	ttsMan->say("Testing text to speech, the speech should pause after a second");
	g_system->delayMillis(1000);
	ttsMan->pause();
	if (!ttsMan->isPaused()) {
		Testsuite::logDetailedPrintf("TTS pause failed\n");
		return kTestFailed;
	}
	ttsMan->say("and then resume again", Common::TextToSpeechManager::QUEUE);
	g_system->delayMillis(3000);
	if (!ttsMan->isPaused()) {
		Testsuite::logDetailedPrintf("TTS pause failed\n");
		return kTestFailed;
	}
	ttsMan->resume();
	if (!ttsMan->isSpeaking()) {
		Testsuite::logDetailedPrintf("TTS pause failed\n");
		return kTestFailed;
	}
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice saying: \"Testing text to speech, the speech should pause after a second and then resume again.\" but with a pause in the middle?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS pauseResume failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testRate() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech rate test. You should expect a voice to say: \"Text to speech slow rate.\" really slowly and then \"Text to speech fast rate\" really fast";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS rate", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testRate\n");
		return kTestSkipped;
	}

	ttsMan->setRate(-100);
	ttsMan->say("Text to speech slow rate.");
	waitForSpeechEnd(ttsMan);
	ttsMan->setRate(100);
	ttsMan->say("Text to speech fast rate.");
	waitForSpeechEnd(ttsMan);

	Common::String prompt = "Did you hear a voice saying: \"Text to speech slow rate.\" slowly and then \"Text to speech fast rate.\" fast?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS rate failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testVolume() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech volume test. You should expect a voice to say: \"Text to speech low volume.\" quietly and then \"Text to speech max volume\" at a higher volume";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS volume", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testVolume\n");
		return kTestSkipped;
	}

	ttsMan->setVolume(20);
	ttsMan->say("Text to speech low volume.");
	waitForSpeechEnd(ttsMan);
	ttsMan->setVolume(100);
	ttsMan->say("Text to speech max volume.");
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice saying: \"Text to speech low volume.\" quietly and then \"Text to speech max volume.\" at a higher volume?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS volume failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testPitch() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech pitch test. You should expect a high pitched voice to say: \"Text to speech high pitch.\" and then a low pitched voice: \"Text to speech low pitch\"";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS pitch", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testPitch\n");
		return kTestSkipped;
	}

	ttsMan->setPitch(100);
	ttsMan->say("Text to speech high pitch.");
	waitForSpeechEnd(ttsMan);
	ttsMan->setPitch(-100);
	ttsMan->say("Text to speech low pitch.");
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a high pitched voice saying: \"Text to speech high pitch.\" and then a low pitched voice: \"Text to speech low pitch.\" ?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS pitch failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testStateStacking() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech state stacking test. You should expect a speech from three different voices (different pitch, gender, volume and speech rate), each voice will say: \"Voice number X is speaking.\", the voices will speak in this order: 1, 2, 3, 2, 1. A voice with the same number should sound the same every time";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS state stacking", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testStateStacking\n");
		return kTestSkipped;
	}

	ttsMan->say("Voice number 1 is speaking");
	waitForSpeechEnd(ttsMan);
	ttsMan->pushState();
	Common::Array<int> femaleVoices = ttsMan->getVoiceIndicesByGender(Common::TTSVoice::FEMALE);
	Common::Array<Common::TTSVoice> allVoices = ttsMan->getVoicesArray();
	if (femaleVoices.size() == 0)
		ttsMan->setVoice(1 % allVoices.size());
	else
		ttsMan->setVoice(femaleVoices[0]);
	ttsMan->setVolume(80);
	ttsMan->setPitch(40);
	ttsMan->setRate(-30);
	ttsMan->say("Voice number 2 is speaking");
	waitForSpeechEnd(ttsMan);
	ttsMan->pushState();
	ttsMan->setVoice(2 % allVoices.size());
	ttsMan->setVolume(90);
	ttsMan->setPitch(-80);
	ttsMan->setRate(-50);
	ttsMan->say("Voice number 3 is speaking");
	waitForSpeechEnd(ttsMan);
	ttsMan->popState();
	ttsMan->say("Voice number 2 is speaking");
	waitForSpeechEnd(ttsMan);
	ttsMan->popState();
	ttsMan->say("Voice number 1 is speaking");
	waitForSpeechEnd(ttsMan);

	Common::String prompt = "Did you hear three different voices speaking in this order: 1, 2, 3, 2, 1 and each time the same voice spoke, it sounded the same?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS state stacking\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testQueueing() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech queue test. You should expect a voice to say: \"This is first speech. This is  second speech\"";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS queue", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testQueueing\n");
		return kTestSkipped;
	}

	ttsMan->say("This is first speech.");
	ttsMan->say("This is second speech.", Common::TextToSpeechManager::QUEUE);
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice saying: \"This is first speech. This is second speech\" ?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS queue failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testInterrupting() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech interrupt test. You should expect a voice to start saying english alphabet and after about a second it should get interrupted and say: \"Speech interrupted\" instead.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS interrupt", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testInterrupting\n");
		return kTestSkipped;
	}

	ttsMan->say("A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z");
	g_system->delayMillis(1000);
	ttsMan->say("Speech interrupted", Common::TextToSpeechManager::INTERRUPT);
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice saying the engilsh alphabet, but it got interrupted and said: \"Speech interrupted\" instead?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS interrupt failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testDroping() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech drop test. You should expect a voice to start say:\"Today is a really nice weather, perfect day to use ScummVM, don't you think?\" and nothing else.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS drop", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testDroping\n");
		return kTestSkipped;
	}

	ttsMan->say("Today is a really nice weather, perfect day to use ScummVM, don't you think?");
	ttsMan->say("Speech interrupted, fail", Common::TextToSpeechManager::DROP);
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice say: \"Today is a really nice weather, perfect day to use ScummVM, don't you think?\" and nothing else?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS drop failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testInterruptNoRepeat() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech interrupt no repeat test. You should expect a voice to start saying:\"This is the first sentence, this should get interrupted\", but the speech gets interrupted and \"This is the second sentence, it should play only once\" is said instead.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS Interrupt No Repeat", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testInterruptNoRepeat\n");
		return kTestSkipped;
	}

	ttsMan->say("This is the first sentence, this should get interrupted");
	ttsMan->say("Failure", Common::TextToSpeechManager::QUEUE);
	g_system->delayMillis(1000);
	ttsMan->say("This is the second sentence, it should play only once", Common::TextToSpeechManager::INTERRUPT_NO_REPEAT);
	ttsMan->say("Failure", Common::TextToSpeechManager::QUEUE);
	g_system->delayMillis(1000);
	ttsMan->say("This is the second sentence, it should play only once", Common::TextToSpeechManager::INTERRUPT_NO_REPEAT);
	ttsMan->say("Failure", Common::TextToSpeechManager::QUEUE);
	g_system->delayMillis(1000);
	ttsMan->say("This is the second sentence, it should play only once", Common::TextToSpeechManager::INTERRUPT_NO_REPEAT);
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice say: \"This is the first sentence, this should get interrupted\", but it got interrupted and \"This is the second sentence, it should play only once.\" got said instead?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS interruptNoRepeat failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus Speechtests::testQueueNoRepeat() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	ttsMan->setLanguage("en");
	ttsMan->setVolume(100);
	ttsMan->setRate(0);
	ttsMan->setPitch(0);
	ttsMan->setVoice(0);
	Testsuite::clearScreen();
	Common::String info = "Text to speech queue no repeat test. You should expect a voice to start say:\"This is the first sentence. This is the second sentence\" and nothing else";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing TTS Queue No Repeat", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testQueueNoRepeat\n");
		return kTestSkipped;
	}

	ttsMan->say("This is the first sentence.");
	ttsMan->say("This is the first sentence.", Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	g_system->delayMillis(1000);
	ttsMan->say("This is the first sentence.", Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	ttsMan->say("This is the second sentence.", Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	ttsMan->say("This is the second sentence.", Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	g_system->delayMillis(1000);
	ttsMan->say("This is the second sentence.", Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	waitForSpeechEnd(ttsMan);
	Common::String prompt = "Did you hear a voice say: \"This is the first sentence. This the second sentence\" and nothing else?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("TTS QueueNoRepeat failed\n");
		return kTestFailed;
	}
	return kTestPassed;
}

SpeechTestSuite::SpeechTestSuite() {
	_isTsEnabled = true;
	if (!g_system->getTextToSpeechManager())
		_isTsEnabled = false;
	addTest("testMale", &Speechtests::testMale, true);
	addTest("testFemale", &Speechtests::testFemale, true);
	addTest("testStop", &Speechtests::testStop, true);
	addTest("testPauseResume", &Speechtests::testPauseResume, true);
	addTest("testRate", &Speechtests::testRate, true);
	addTest("testVolume", &Speechtests::testVolume, true);
	addTest("testPitch", &Speechtests::testPitch, true);
	addTest("testStateStacking", &Speechtests::testStateStacking, true);
	addTest("testQueueing", &Speechtests::testQueueing, true);
	addTest("testInterrupting", &Speechtests::testInterrupting, true);
	addTest("testDroping", &Speechtests::testDroping, true);
	addTest("testInterruptNoRepeat", &Speechtests::testInterruptNoRepeat, true);
	addTest("testQueueNoRepeat", &Speechtests::testQueueNoRepeat, true);
}

} // End of namespace Testbed
