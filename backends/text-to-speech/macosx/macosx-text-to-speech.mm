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

#include "backends/text-to-speech/macosx/macosx-text-to-speech.h"

#if defined(USE_TTS) && defined(MACOSX)
#include "common/translation.h"
#include <AppKit/NSSpeechSynthesizer.h>
#include <Foundation/NSString.h>
#include <CoreFoundation/CFString.h>

@interface MacOSXTextToSpeechManagerDelegate : NSObject<NSSpeechSynthesizerDelegate> {
	MacOSXTextToSpeechManager *_ttsManager;
	BOOL _ignoreNextFinishedSpeaking;
}
- (id)initWithManager:(MacOSXTextToSpeechManager*)ttsManager;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)finishedSpeaking;
- (void)ignoreNextFinishedSpeaking:(BOOL)ignore;
@end

@implementation MacOSXTextToSpeechManagerDelegate
- (id)initWithManager:(MacOSXTextToSpeechManager*)ttsManager {
	self = [super init];
	_ttsManager = ttsManager;
	_ignoreNextFinishedSpeaking = NO;
	return self;
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)finishedSpeaking {
	if (!_ignoreNextFinishedSpeaking)
		_ttsManager->startNextSpeech();
	_ignoreNextFinishedSpeaking = NO;
}

- (void)ignoreNextFinishedSpeaking:(BOOL)ignore {
	_ignoreNextFinishedSpeaking = ignore;
}
@end

NSSpeechSynthesizer *synthesizer;
MacOSXTextToSpeechManagerDelegate *synthesizerDelegate;

MacOSXTextToSpeechManager::MacOSXTextToSpeechManager() : Common::TextToSpeechManager(), _paused(false) {
	synthesizer = [[NSSpeechSynthesizer alloc] init];
	synthesizerDelegate = [[MacOSXTextToSpeechManagerDelegate alloc] initWithManager:this];
	[synthesizer setDelegate:synthesizerDelegate];

#ifdef USE_TRANSLATION
	setLanguage(TransMan.getCurrentLanguage());
#else
	setLanguage("en");
#endif
}

MacOSXTextToSpeechManager::~MacOSXTextToSpeechManager() {
	clearState();

	[synthesizer release];
	[synthesizerDelegate release];
}

bool MacOSXTextToSpeechManager::say(const Common::U32String &text, Action action) {
	Common::String textToSpeak = text.encode();
	if (isSpeaking()) {
		// Interruptions are done on word boundaries for nice transitions.
		// Should we interrupt immediately?
		if (action == DROP)
			return true;
		else if (action == INTERRUPT) {
			_messageQueue.clear();
			[synthesizer stopSpeakingAtBoundary:NSSpeechWordBoundary];
		} else if (action == INTERRUPT_NO_REPEAT) {
			// If the new speech is the one being currently said, continue that speech but clear the queue.
			// And otherwise both clear the queue and interrupt the current speech.
			_messageQueue.clear();
			if (_currentSpeech == textToSpeak)
				return true;
			[synthesizer stopSpeakingAtBoundary:NSSpeechWordBoundary];
		} else if (action == QUEUE_NO_REPEAT) {
			if (!_messageQueue.empty()) {
				if (_messageQueue.back() == textToSpeak)
					return true;
			} else if (_currentSpeech == textToSpeak)
				return true;
		}
	}

	_messageQueue.push(textToSpeak);
	if (!isSpeaking())
		startNextSpeech();
	return true;
}

bool MacOSXTextToSpeechManager::startNextSpeech() {
	_currentSpeech.clear();
	if (_messageQueue.empty())
		return false;

	Common::String textToSpeak = _messageQueue.pop();

	// Get current encoding
	CFStringEncoding stringEncoding = kCFStringEncodingUTF8;

	CFStringRef textNSString = CFStringCreateWithCString(NULL, textToSpeak.c_str(), stringEncoding);
	bool status = [synthesizer startSpeakingString:(NSString *)textNSString];
	CFRelease(textNSString);
	if (status)
		_currentSpeech = textToSpeak;

	return status;
}

bool MacOSXTextToSpeechManager::stop() {
	_messageQueue.clear();
	if (isSpeaking()) {
		_currentSpeech.clear(); // so that it immediately reports that it is no longer speaking
		// Stop as soon as possible
		// Also tell the MacOSXTextToSpeechManagerDelegate to ignore the next finishedSpeaking as
		// it has already been handled, but we might have started another speech by the time we
		// receive it, and we don't want to stop that one.
		[synthesizerDelegate ignoreNextFinishedSpeaking:YES];
		[synthesizer stopSpeakingAtBoundary:NSSpeechImmediateBoundary];
	}
	return true;
}

bool MacOSXTextToSpeechManager::pause() {
	// Pause on a word boundary as pausing/resuming in a middle of words is strange.
	[synthesizer pauseSpeakingAtBoundary:NSSpeechWordBoundary];
	_paused = true;
	return true;
}

bool MacOSXTextToSpeechManager::resume() {
	_paused = false;
	[synthesizer continueSpeaking];
	return true;
}

bool MacOSXTextToSpeechManager::isSpeaking() {
	// Because the NSSpeechSynthesizer is asynchronous, it doesn't start speaking immediately
	// and thus using [synthesizer isSpeaking] just after [synthesizer startSpeakingString:]] is
	// likely to return NO. So instead we check the _currentSpeech string (set when calling
	// startSpeakingString, and cleared when we receive the didFinishSpeaking message).
	//return [synthesizer isSpeaking];
	return !_currentSpeech.empty();
}

bool MacOSXTextToSpeechManager::isPaused() {
	// Because the NSSpeechSynthesizer is asynchronous, and because we pause at the end of a word
	// and not immediately, we cannot check the speech status as it is likely to not be paused yet
	// immediately after we requested the pause. So we keep our own flag.
	//NSDictionary *statusDict = (NSDictionary*) [synthesizer objectForProperty:NSSpeechStatusProperty error:nil];
	//return [[statusDict objectForKey:NSSpeechStatusOutputBusy] boolValue] && [[statusDict objectForKey:NSSpeechStatusOutputPaused] boolValue];
	return _paused;
}

bool MacOSXTextToSpeechManager::isReady() {
	// See comments in isSpeaking() and isPaused()
	//NSDictionary *statusDict = (NSDictionary*) [synthesizer objectForProperty:NSSpeechStatusProperty error:nil];
	//return [[statusDict objectForKey:NSSpeechStatusOutputBusy] boolValue] == NO;
	return _currentSpeech.empty() && !_paused;
}

void MacOSXTextToSpeechManager::setVoice(unsigned index) {
	if (_ttsState->_availableVoices.empty())
		return;
	assert(index < _ttsState->_availableVoices.size());
	Common::TTSVoice voice = _ttsState->_availableVoices[index];
	_ttsState->_activeVoice = index;

	[synthesizer setVoice:(NSString*)voice.getData()];

	// Setting the voice reset the pitch and rate to the voice defaults.
	// Apply back the modifiers.
	int pitch = getPitch(), rate = getRate();
	Common::TextToSpeechManager::setPitch(0);
	Common::TextToSpeechManager::setRate(0);
	setPitch(pitch);
	setRate(rate);
}

void MacOSXTextToSpeechManager::setRate(int rate) {
	int oldRate = getRate();
	Common::TextToSpeechManager::setRate(rate);
	// The rate is a value between -100 and +100, with 0 being the default rate.
	// Convert this to a multiplier between 0.5 and 1.5.
	float oldRateMultiplier = 1.0f + oldRate / 200.0f;
	float ratehMultiplier = 1.0f + rate / 200.0f;
	synthesizer.rate = synthesizer.rate / oldRateMultiplier * ratehMultiplier;
}

void MacOSXTextToSpeechManager::setPitch(int pitch) {
	int oldPitch = getPitch();
	Common::TextToSpeechManager::setPitch(pitch);
	// The pitch is a value between -100 and +100, with 0 being the default pitch.
	// Convert this to a multiplier between 0.5 and 1.5 on the default voice pitch.
	float oldPitchMultiplier = 1.0f + oldPitch / 200.0f;
	float pitchMultiplier = 1.0f + pitch / 200.0f;
	NSNumber *basePitchNumber = [synthesizer objectForProperty:NSSpeechPitchBaseProperty error:nil];
	float basePitch = [basePitchNumber floatValue] / oldPitchMultiplier * pitchMultiplier;
	[synthesizer setObject:[NSNumber numberWithFloat:basePitch] forProperty:NSSpeechPitchBaseProperty error:nil];
}

void MacOSXTextToSpeechManager::setVolume(unsigned volume) {
	Common::TextToSpeechManager::setVolume(volume);
	synthesizer.volume = volume / 100.0f;
}

void MacOSXTextToSpeechManager::setLanguage(Common::String language) {
	Common::TextToSpeechManager::setLanguage(language);
	updateVoices();
}

int MacOSXTextToSpeechManager::getDefaultVoice() {
	if (_ttsState->_availableVoices.size() < 2)
		return 0;
	NSString *defaultVoice = [NSSpeechSynthesizer defaultVoice];
	if (defaultVoice == nil)
		return 0;
	for (unsigned int i = 0 ; i < _ttsState->_availableVoices.size() ; ++i) {
		if ([defaultVoice isEqualToString:(NSString*)(_ttsState->_availableVoices[i].getData())])
			return i;
	}
	return 0;
}

void MacOSXTextToSpeechManager::freeVoiceData(void *data) {
	NSString* voiceId = (NSString*)data;
	[voiceId release];
}

void MacOSXTextToSpeechManager::updateVoices() {
	Common::String currentVoice;
	if (!_ttsState->_availableVoices.empty())
		currentVoice = _ttsState->_availableVoices[_ttsState->_activeVoice].getDescription();
	_ttsState->_availableVoices.clear();
	int activeVoiceIndex = -1, defaultVoiceIndex = -1;

	Common::String lang = getLanguage();
	NSArray *voices = [NSSpeechSynthesizer availableVoices];
	NSString *defaultVoice = [NSSpeechSynthesizer defaultVoice];
	int voiceIndex = 0;
	for (NSString *voiceId in voices) {
		NSDictionary *voiceAttr = [NSSpeechSynthesizer attributesForVoice:voiceId];
		Common::String voiceLocale([[voiceAttr objectForKey:NSVoiceLocaleIdentifier] UTF8String]);
		if (voiceLocale.hasPrefix(lang)) {
			NSString *data = [[NSString alloc] initWithString:voiceId];
			Common::String name([[voiceAttr objectForKey:NSVoiceName] UTF8String]);
			Common::TTSVoice::Gender gender = Common::TTSVoice::UNKNOWN_GENDER;
			NSString *voiceGender = [voiceAttr objectForKey:NSVoiceGender];
			if (voiceGender != nil) {
				// This can be VoiceGenderMale, VoiceGenderFemale, VoiceGenderNeuter
				if ([voiceGender isEqualToString:@"VoiceGenderMale"])
					gender = Common::TTSVoice::MALE;
				else if ([voiceGender isEqualToString:@"VoiceGenderFemale"])
					gender = Common::TTSVoice::FEMALE;
			}
			Common::TTSVoice::Age age = Common::TTSVoice::UNKNOWN_AGE;
			NSNumber *voiceAge = [voiceAttr objectForKey:NSVoiceAge];
			if (voiceAge != nil) {
				if ([voiceAge integerValue] < 18)
					age = Common::TTSVoice::CHILD;
				else
					age = Common::TTSVoice::ADULT;
			}
			Common::TTSVoice voice(gender, age, data, name);
			_ttsState->_availableVoices.push_back(voice);
			if (name == currentVoice)
				activeVoiceIndex = voiceIndex;
			if (defaultVoice != nil && [defaultVoice isEqualToString:voiceId])
				defaultVoiceIndex = voiceIndex;
			++voiceIndex;
		}
	}

	if (activeVoiceIndex == -1)
		activeVoiceIndex = defaultVoiceIndex == -1 ? 0 : defaultVoiceIndex;
	setVoice(activeVoiceIndex);
}


#endif
