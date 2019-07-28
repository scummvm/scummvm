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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/text-to-speech/macosx/macosx-text-to-speech.h"

#if defined(USE_MACOSX_TTS)
#include "common/translation.h"
#include <AppKit/NSSpeechSynthesizer.h>
#include <Foundation/NSString.h>
#include <CoreFoundation/CFString.h>

NSSpeechSynthesizer* synthesizer;

MacOSXTextToSpeechManager::MacOSXTextToSpeechManager() : Common::TextToSpeechManager() {
	synthesizer = [[NSSpeechSynthesizer alloc] init];

#ifdef USE_TRANSLATION
	setLanguage(TransMan.getCurrentLanguage());
#else
	setLanguage("en");
#endif
}

MacOSXTextToSpeechManager::~MacOSXTextToSpeechManager() {
	[synthesizer release];
}

bool MacOSXTextToSpeechManager::say(Common::String text, Common::String encoding) {
	if (encoding.empty()) {
#ifdef USE_TRANSLATION
		encoding = TransMan.getCurrentCharset();
#endif
	}

	// Get current encoding
	CFStringEncoding stringEncoding = kCFStringEncodingASCII;
	if (!encoding.empty()) {
		CFStringRef encStr = CFStringCreateWithCString(NULL, encoding.c_str(), kCFStringEncodingASCII);
		stringEncoding = CFStringConvertIANACharSetNameToEncoding(encStr);
		CFRelease(encStr);
	}

	CFStringRef textNSString = CFStringCreateWithCString(NULL, text.c_str(), stringEncoding);
	bool status = [synthesizer startSpeakingString:(NSString *)textNSString];
	CFRelease(textNSString);
	return status;
}

bool MacOSXTextToSpeechManager::stop() {
	[synthesizer stopSpeaking];
	return true;
}

bool MacOSXTextToSpeechManager::pause() {
	// Should we use NSSpeechWordBoundary, or even NSSpeechSentenceBoundary?
	[synthesizer pauseSpeakingAtBoundary:NSSpeechImmediateBoundary];
	return true;
}

bool MacOSXTextToSpeechManager::resume() {
	[synthesizer continueSpeaking];
	return true;
}

bool MacOSXTextToSpeechManager::isSpeaking() {
	return [synthesizer isSpeaking];
}

bool MacOSXTextToSpeechManager::isPaused() {
	NSDictionary *statusDict = (NSDictionary*) [synthesizer objectForProperty:NSSpeechStatusProperty error:nil];
	return [[statusDict objectForKey:NSSpeechStatusOutputBusy] boolValue] && [[statusDict objectForKey:NSSpeechStatusOutputPaused] boolValue];
}

bool MacOSXTextToSpeechManager::isReady() {
	NSDictionary *statusDict = (NSDictionary*) [synthesizer objectForProperty:NSSpeechStatusProperty error:nil];
	return [[statusDict objectForKey:NSSpeechStatusOutputBusy] boolValue] == NO;
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
