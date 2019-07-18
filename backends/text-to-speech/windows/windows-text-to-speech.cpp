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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if defined(USE_WINDOWS_TTS)
#include <basetyps.h>
#include <windows.h>
#include <Servprov.h>
#include <sapi.h>
#include "backends/text-to-speech/windows/sphelper-scummvm.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

#include "backends/text-to-speech/windows/windows-text-to-speech.h"


#include "common/translation.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/ustr.h"
#include "common/config-manager.h"

ISpVoice *_voice;

// We need this pointer to be able to stop speech immediately.
ISpAudio *_audio;

WindowsTextToSpeechManager::WindowsTextToSpeechManager()
	: _speechState(BROKEN){
	init();
}

void WindowsTextToSpeechManager::init() {
	// init COM
	if (FAILED(::CoInitialize(NULL)))
		return;

	// init voice
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&_voice);
	if (!SUCCEEDED(hr)) {
		warning("Could not initialize TTS voice");
		return;
	}
	setLanguage("en");

	// init audio
	CSpStreamFormat format;
	format.AssignFormat(SPSF_11kHz8BitMono);
	ISpObjectToken *pToken;
	hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOOUT, &pToken);
	if (FAILED(hr)) {
		warning("Could not initialize TTS audio");
		return;
	}
	pToken->CreateInstance(NULL, CLSCTX_ALL, IID_ISpAudio, (void **)&_audio);
	_audio->SetFormat(format.FormatId(), format.WaveFormatExPtr());
	_voice->SetOutput(_audio, FALSE);

	if(_ttsState->_availaibleVoices.size() > 0)
		_speechState = READY;
	else
		_speechState = NO_VOICE;
}

WindowsTextToSpeechManager::~WindowsTextToSpeechManager() {
	freeVoices();
	if (_voice)
		_voice->Release();
	::CoUninitialize();
}

bool WindowsTextToSpeechManager::say(Common::String str, Common::String charset) {
	if(_speechState == BROKEN || _speechState == NO_VOICE) {
		warning("The tts cannot speak in this state");
		return true;
	}

	if (charset == "") {
#ifdef USE_TRANSLATION
		charset = TransMan.getCurrentCharset();
#else
		charset = "ASCII";
#endif
	}
	if (isPaused()) {
		resume();
	}
	_audio->SetState(SPAS_STOP, 0);
	_audio->SetState(SPAS_RUN, 0);
	// We have to set the pitch by prepending xml code at the start of the said string;
	Common::String pitch= Common::String::format("<pitch absmiddle=\"%d\">", _ttsState->_pitch / 10);
	str.replace((uint32)0, 0, pitch);

	WCHAR *strW = Win32::ansiToUnicode(str.c_str());
	bool result = _voice->Speak(strW, SPF_ASYNC | SPF_PURGEBEFORESPEAK, NULL) != S_OK;
	free(strW);
	_speechState = SPEAKING;
	return result;
}

bool WindowsTextToSpeechManager::stop() {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return true;
	if (isPaused())
		resume();
	_audio->SetState(SPAS_STOP, 0);
	_audio->SetState(SPAS_RUN, 0);
	_voice->Speak(NULL, SPF_PURGEBEFORESPEAK | SPF_ASYNC | SPF_IS_NOT_XML, 0);
	_speechState = READY;
	return false;
}

bool WindowsTextToSpeechManager::pause() {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return true;
	if (isPaused())
		return false;
	_voice->Pause();
	_speechState = PAUSED;
	return false;
}

bool WindowsTextToSpeechManager::resume() {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return true;
	if (!isPaused())
		return false;
	_voice->Resume();
	if (isSpeaking())
		_speechState = SPEAKING;
	else
		_speechState = READY;
	return false;
}

bool WindowsTextToSpeechManager::isSpeaking() {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return false;
	SPVOICESTATUS eventStatus;
	_voice->GetStatus(&eventStatus, NULL);
	return eventStatus.dwRunningState == SPRS_IS_SPEAKING;
}

bool WindowsTextToSpeechManager::isPaused() {
	return _speechState == PAUSED;
}

bool WindowsTextToSpeechManager::isReady() {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return false;
	if (_speechState != PAUSED && !isSpeaking())
		return true;
	else
		return false;
}

void WindowsTextToSpeechManager::setVoice(unsigned index) {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	_voice->SetVoice((ISpObjectToken *) _ttsState->_availaibleVoices[index].getData());
}

void WindowsTextToSpeechManager::setRate(int rate) {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	assert(rate >= -100 && rate <= 100);
	_voice->SetRate(rate / 10);
	_ttsState->_rate = rate;
}

void WindowsTextToSpeechManager::setPitch(int pitch) {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	assert(pitch >= -100 && pitch <= 100);
	_ttsState->_pitch = pitch;
}

void WindowsTextToSpeechManager::setVolume(unsigned volume) {
	if(_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	assert(volume <= 100);
	_voice->SetVolume(volume);
	_ttsState->_volume = volume;
}

int WindowsTextToSpeechManager::getVolume() {
	return _ttsState->_volume;
}

void WindowsTextToSpeechManager::freeVoices() {
	for(Common::TTSVoice *i = _ttsState->_availaibleVoices.begin(); i < _ttsState->_availaibleVoices.end(); i++) {
		ISpObjectToken *voiceData = (ISpObjectToken *)i->getData();
		voiceData->Release();
	}
	_ttsState->_availaibleVoices.clear();
}

void WindowsTextToSpeechManager::setLanguage(Common::String language) {
	if (language == "C")
		language = "en";
	_ttsState->_language = language;
	updateVoices();
	setVoice(0);
}

void WindowsTextToSpeechManager::createVoice(void *cpVoiceToken) {
	ISpObjectToken *voiceToken = (ISpObjectToken *) cpVoiceToken;

	// description
	WCHAR *descW;
	SpGetDescription(voiceToken, &descW);
	char *buffer = Win32::unicodeToAnsi(descW);
	Common::String desc = buffer;
	free(buffer);

	// voice attributes
	HRESULT hr = S_OK;
	ISpDataKey *key = nullptr;
	hr = voiceToken->OpenKey(L"Attributes", &key);

	if (FAILED(hr)) {
		voiceToken->Release();
		warning("Could not open attribute key for voice: %s", desc.c_str());
		return;
	}
	LPWSTR data;

	// language
	hr = key->GetStringValue(L"Language", &data);
	if (FAILED(hr)) {
		voiceToken->Release();
		warning("Could not get the language attribute for voice: %s", desc.c_str());
		return;
	}
	buffer = Win32::unicodeToAnsi(data);
	Common::String language = lcidToLocale(buffer);
	free(buffer);
	CoTaskMemFree(data);

	// only get the voices for the current language
	if (language != _ttsState->_language) {
		voiceToken->Release();
		return;
	}

	// gender
	hr = key->GetStringValue(L"Gender", &data);
	if (FAILED(hr)) {
		voiceToken->Release();
		warning("Could not get the gender attribute for voice: %s", desc.c_str());
		return;
	}
	buffer = Win32::unicodeToAnsi(data);
	Common::TTSVoice::Gender gender = !strcmp(buffer, "Male") ? Common::TTSVoice::MALE : Common::TTSVoice::FEMALE;
	free(buffer);
	CoTaskMemFree(data);

	// age
	hr = key->GetStringValue(L"Age", &data);
	if (FAILED(hr)) {
		voiceToken->Release();
		warning("Could not get the age attribute for voice: %s", desc.c_str());
		return;
	}
	buffer = Win32::unicodeToAnsi(data);
	Common::TTSVoice::Age age = !strcmp(buffer, "Adult") ? Common::TTSVoice::ADULT : Common::TTSVoice::UNKNOWN_AGE;
	free(buffer);
	CoTaskMemFree(data);

	_ttsState->_availaibleVoices.push_back(Common::TTSVoice(gender, Common::TTSVoice::ADULT, (void *) voiceToken, desc));
}

int strToInt(Common::String str) {
	str.toUppercase();
	int result = 0;
	for(unsigned i = 0; i < str.size(); i++) {
		if (str[i] < '0' || (str[i] > '9' && str[i] < 'A') || str[i] > 'F')
			break;
		int num = (str[i] <= '9') ? str[i] - '0' : str[i] - 55;
		result = result * 16 + num;
	}
	return result;
}

Common::String WindowsTextToSpeechManager::lcidToLocale(Common::String lcid) {
	LCID locale = strToInt(lcid);
	int nchars = GetLocaleInfoW(locale, LOCALE_SISO639LANGNAME, NULL, 0);
	wchar_t *languageCode = new wchar_t[nchars];
	GetLocaleInfoW(locale, LOCALE_SISO639LANGNAME, languageCode, nchars);
	char *resultTmp = Win32::unicodeToAnsi(languageCode);
	Common::String result = resultTmp;
	delete[] languageCode;
	free(resultTmp);
	return result;
}

void WindowsTextToSpeechManager::updateVoices() {
	freeVoices();
	HRESULT hr = S_OK;
	ISpObjectToken *cpVoiceToken = nullptr;
	IEnumSpObjectTokens *cpEnum = nullptr;
	unsigned long ulCount = 0;

	hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
	if (SUCCEEDED(hr)) {
		hr = cpEnum->GetCount(&ulCount);
	}
	_voice->SetVolume(0);
	while (SUCCEEDED(hr) && ulCount--) {
		hr = cpEnum->Next(1, &cpVoiceToken, NULL);
		_voice->SetVoice(cpVoiceToken);
		if(SUCCEEDED(_voice->Speak(L"hi, this is test", SPF_PURGEBEFORESPEAK | SPF_ASYNC | SPF_IS_NOT_XML, 0)))
			createVoice(cpVoiceToken);
		else
			cpVoiceToken->Release();
	}
	_voice->SetVolume(_ttsState->_volume);
	cpEnum->Release();

	if(_ttsState->_availaibleVoices.size() == 0) {
		_speechState = NO_VOICE;
		warning("No voice is availaible");
	} else if (_speechState == NO_VOICE)
		_speechState = READY;
}

bool WindowsTextToSpeechManager::popState() {
	if (_ttsState->_next == nullptr)
		return true;

	for (Common::TTSVoice *i = _ttsState->_availaibleVoices.begin(); i < _ttsState->_availaibleVoices.end(); i++) {
		ISpObjectToken *voiceToken = (ISpObjectToken *) i->getData();
		voiceToken->Release();
	}

	Common::TTSState *oldState = _ttsState;
	_ttsState = _ttsState->_next;

	delete oldState;

	setLanguage(_ttsState->_language);
	setPitch(_ttsState->_pitch);
	setVolume(_ttsState->_volume);
	setRate(_ttsState->_rate);
	setVoice(_ttsState->_activeVoice);
	return false;
}
void WindowsTextToSpeechManager::freeVoiceData(void *data) {
	ISpObjectToken *voiceToken = (ISpObjectToken *) data;
	voiceToken->Release();
}

#endif
