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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/text-to-speech/avfaudio/avfaudio-text-to-speech.h"

#if defined(USE_TTS) && (defined(MACOSX) || defined(IPHONE))
#include "common/translation.h"
#include <Foundation/NSString.h>
#include <AVFoundation/AVFoundation.h>

@interface AVFAudioTextToSpeechManagerDelegate : NSObject<AVSpeechSynthesizerDelegate> {
	AVFAudioTextToSpeechManager *_ttsManager;
	BOOL _ignoreNextFinishedSpeaking;
}
- (id)initWithManager:(AVFAudioTextToSpeechManager*)ttsManager;
- (void)speechSynthesizer:(AVSpeechSynthesizer *)sender didFinishSpeechUtterance:(AVSpeechUtterance *)utterance;
- (void)speechSynthesizer:(AVSpeechSynthesizer *)sender didCancelSpeechUtterance:(AVSpeechUtterance *)utterance;
- (void)ignoreNextFinishedSpeaking:(BOOL)ignore;
@end

@implementation AVFAudioTextToSpeechManagerDelegate
- (id)initWithManager:(AVFAudioTextToSpeechManager*)ttsManager {
	self = [super init];
	_ttsManager = ttsManager;
	_ignoreNextFinishedSpeaking = NO;
	return self;
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)sender didFinishSpeechUtterance:(AVSpeechUtterance *) utterance {
	if (!_ignoreNextFinishedSpeaking)
		_ttsManager->startNextSpeech();
	_ignoreNextFinishedSpeaking = NO;
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)sender didCancelSpeechUtterance:(AVSpeechUtterance *) utterance {
	if (!_ignoreNextFinishedSpeaking)
		_ttsManager->startNextSpeech();
	_ignoreNextFinishedSpeaking = NO;
}

- (void)ignoreNextFinishedSpeaking:(BOOL)ignore {
	_ignoreNextFinishedSpeaking = ignore;
}
@end

AVSpeechSynthesizer *synthesizer;
AVFAudioTextToSpeechManagerDelegate *synthesizerDelegate;

AVFAudioTextToSpeechManager::AVFAudioTextToSpeechManager() : Common::TextToSpeechManager(), _paused(false), _interruptRequested(false) {
	synthesizer = [[AVSpeechSynthesizer alloc] init];
	synthesizerDelegate = [[AVFAudioTextToSpeechManagerDelegate alloc] initWithManager:this];
	[synthesizer setDelegate:synthesizerDelegate];

#ifdef USE_TRANSLATION
	setLanguage(TransMan.getCurrentLanguage());
#else
	setLanguage("en");
#endif
}

AVFAudioTextToSpeechManager::~AVFAudioTextToSpeechManager() {
	clearState();

	[synthesizer release];
	[synthesizerDelegate release];
}

bool AVFAudioTextToSpeechManager::say(const Common::U32String &text, Action action) {
	Common::String textToSpeak = text.encode();
	if (isSpeaking()) {
		// Interruptions are done on word boundaries for nice transitions.
		// Should we interrupt immediately?
		if (action == DROP)
			return true;
		else if (action == INTERRUPT) {
			_messageQueue.clear();
			// If an interrupt was already requested but not yet processed do not request it again as it may end up
			// interrpting the next speech.
			if (!_interruptRequested) {
				_interruptRequested = true;
				[synthesizer stopSpeakingAtBoundary:AVSpeechBoundaryWord];
			}
		} else if (action == INTERRUPT_NO_REPEAT) {
			// If the new speech is the one being currently said, continue that speech but clear the queue.
			// And otherwise both clear the queue and interrupt the current speech.
			_messageQueue.clear();
			if (_currentSpeech == textToSpeak)
				return true;
			// If an interrupt was already requested but not yet processed do not request it again as it may end up
			// interrpting the next speech.
			if (!_interruptRequested) {
				_interruptRequested = true;
				[synthesizer stopSpeakingAtBoundary:AVSpeechBoundaryWord];
			}
		} else if (action == QUEUE_NO_REPEAT) {
			if (!_messageQueue.empty()) {
				if (_messageQueue.back() == textToSpeak)
					return true;
			} else if (_currentSpeech == textToSpeak)
				return true;
		}
	}

	// We can queue utterances in the AVSpeechSynthesizer, however if we did that we could not
	// unqueue them without also stopping the current utterance. That means we could not implement
	// INTERRUPT_NO_REPEAT when the new text is the same as the current one, which we thus want to
	// continue, and the queue has additionaltext we want to drop.
	// So use our own queue and start each utterance manually.
	_messageQueue.push(textToSpeak);
	if (!isSpeaking())
		startNextSpeech();
	return true;
}

bool AVFAudioTextToSpeechManager::startNextSpeech() {
	_currentSpeech.clear();
	_interruptRequested = false;
	if (_messageQueue.empty())
		return false;

	Common::String textToSpeak;
	do {
		textToSpeak = _messageQueue.pop();
	} while (textToSpeak.empty() && !_messageQueue.empty());
	if (textToSpeak.empty())
		return false;

	CFStringRef textNSString = CFStringCreateWithCString(NULL, textToSpeak.c_str(), kCFStringEncodingUTF8);
	AVSpeechUtterance *utterance = [[AVSpeechUtterance alloc]initWithString:(NSString*)textNSString];
	CFRelease(textNSString);
	if (!_ttsState->_availableVoices.empty())
		utterance.voice = [AVSpeechSynthesisVoice voiceWithIdentifier:(NSString *)(getVoice().getData())];
	// The rate is a value between -100 and +100, with 0 being the default rate.
	// Convert this to a multiplier between 0.5 and 1.5.
	float ratehMultiplier = 1.0f + getRate() / 200.0f;
	utterance.rate = AVSpeechUtteranceDefaultSpeechRate * ratehMultiplier;
	// The pitch is a value between -100 and +100, with 0 being the default pitch.
	// Convert this to a multiplier between 0.5 and 1.5 on the default voice pitch.
	utterance.pitchMultiplier = 1.0f + getPitch() / 200.0f;
	utterance.volume = getVolume() / 100.0f;
	//utterance.postUtteranceDelay = 0.1f;

	[synthesizer speakUtterance:utterance];
	[utterance release];
	_currentSpeech = textToSpeak;

	return true;
}

bool AVFAudioTextToSpeechManager::stop() {
	_messageQueue.clear();
	if (isSpeaking()) {
		_currentSpeech.clear(); // so that it immediately reports that it is no longer speaking
		// Stop as soon as possible
		// Also tell the AVFAudioTextToSpeechManagerDelegate to ignore the next finishedSpeaking as
		// it has already been handled, but we might have started another speech by the time we
		// receive it, and we don't want to stop that one.
		[synthesizerDelegate ignoreNextFinishedSpeaking:YES];
		[synthesizer stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
	}
	return true;
}

bool AVFAudioTextToSpeechManager::pause() {
	// Pause on a word boundary as pausing/resuming in a middle of words is strange.
	[synthesizer pauseSpeakingAtBoundary:AVSpeechBoundaryWord];
	_paused = true;
	return true;
}

bool AVFAudioTextToSpeechManager::resume() {
	_paused = false;
	[synthesizer continueSpeaking];
	return true;
}

bool AVFAudioTextToSpeechManager::isSpeaking() {
	// Because the AVSpeechSynthesizer is asynchronous, it doesn't start speaking immediately
	// and thus using [synthesizer isSpeaking] just after [synthesizer startSpeakingString:]] is
	// likely to return NO. So instead we check the _currentSpeech string (set when calling
	// startSpeakingString, and cleared when we receive the didFinishSpeaking message).
	//return [synthesizer isSpeaking];
	return !_currentSpeech.empty();

}

bool AVFAudioTextToSpeechManager::isPaused() {
	// Because the AVSpeechSynthesizer is asynchronous, and because we pause at the end of a word
	// and not immediately, we cannot check the speech status as it is likely to not be paused yet
	// immediately after we requested the pause. So we keep our own flag.
	//return [synthesizer isPaused];
	return _paused;
}

bool AVFAudioTextToSpeechManager::isReady() {
	// See comments in isSpeaking() and isPaused()
	return _currentSpeech.empty() && !_paused;
}

void AVFAudioTextToSpeechManager::setLanguage(Common::String language) {
	Common::TextToSpeechManager::setLanguage(language);
	updateVoices();
}

void AVFAudioTextToSpeechManager::setVoice(unsigned index) {
	if (_ttsState->_availableVoices.empty())
		return;
	assert(index < _ttsState->_availableVoices.size());
	_ttsState->_activeVoice = index;
}

int AVFAudioTextToSpeechManager::getDefaultVoice() {
	if (_ttsState->_availableVoices.size() < 2)
		return 0;

	Common::String lang = getLanguage();
	CFStringRef langNSString = CFStringCreateWithCString(NULL, lang.c_str(), kCFStringEncodingUTF8);
	AVSpeechSynthesisVoice *defaultVoice = [AVSpeechSynthesisVoice voiceWithLanguage:(NSString *)langNSString];
	CFRelease(langNSString);

	if (defaultVoice == nil)
		return 0;
	for (unsigned int i = 0 ; i < _ttsState->_availableVoices.size() ; ++i) {
		if ([defaultVoice.identifier isEqualToString:(NSString*)(_ttsState->_availableVoices[i].getData())])
			return i;
	}
	return 0;
}

void AVFAudioTextToSpeechManager::freeVoiceData(void *data) {
	NSString* voiceId = (NSString*)data;
	[voiceId release];
}

void AVFAudioTextToSpeechManager::updateVoices() {
	Common::String currentVoice;
	if (!_ttsState->_availableVoices.empty())
		currentVoice = _ttsState->_availableVoices[_ttsState->_activeVoice].getDescription();
	_ttsState->_availableVoices.clear();
	int activeVoiceIndex = -1;

	Common::String lang = getLanguage();
	NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
	int voiceIndex = 0;
	for (AVSpeechSynthesisVoice *voice in voices) {
		Common::String voiceLocale([voice.language UTF8String]);
		if (voiceLocale.hasPrefix(lang)) {
			NSString *data = [[NSString alloc] initWithString:voice.identifier];
			Common::String name([voice.name UTF8String]);
			Common::TTSVoice::Gender gender = Common::TTSVoice::UNKNOWN_GENDER;
#if defined(__IPHONE_13_0) || defined(MACOSX)
			if (@available(iOS 13, macOS 10.15, *)) {
				switch (voice.gender) {
				case AVSpeechSynthesisVoiceGenderMale:
					gender = Common::TTSVoice::MALE;
					break;
				case AVSpeechSynthesisVoiceGenderFemale:
					gender = Common::TTSVoice::FEMALE;
					break;
				case AVSpeechSynthesisVoiceGenderUnspecified:
					gender = Common::TTSVoice::UNKNOWN_GENDER;
					break;
				}
			}
#endif
			Common::TTSVoice::Age age = Common::TTSVoice::UNKNOWN_AGE;
			Common::TTSVoice ttsVoice(gender, age, data, name);
			_ttsState->_availableVoices.push_back(ttsVoice);
			if (name == currentVoice)
				activeVoiceIndex = voiceIndex;
			++voiceIndex;
		}
	}

	if (activeVoiceIndex == -1)
		activeVoiceIndex = getDefaultVoice();
	setVoice(activeVoiceIndex);
}

#endif
