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
#if defined(USE_TTS) && defined(WIN32)
#include <basetyps.h>
#include <windows.h>
#include <servprov.h>

#include <sapi.h>
#include "backends/platform/sdl/win32/win32_wrapper.h"

#include "backends/text-to-speech/windows/windows-text-to-speech.h"


#include "common/translation.h"
#include "common/system.h"
#include "common/ustr.h"
#include "common/config-manager.h"

ISpVoice *_voice;

// We need this pointer to be able to stop speech immediately.
ISpAudio *_audio;

WindowsTextToSpeechManager::WindowsTextToSpeechManager()
	: _speechState(BROKEN){
	init();
	_threadParams.queue = &_speechQueue;
	_threadParams.state = &_speechState;
	_threadParams.mutex = &_speechMutex;
	_thread = NULL;
	_speechMutex = CreateMutex(NULL, FALSE, NULL);
	if (_speechMutex == NULL) {
		_speechState = BROKEN;
		warning("Could not create TTS mutex");
	}
}

void WindowsTextToSpeechManager::init() {
	// init COM
	if (FAILED(::CoInitialize(NULL)))
		return;

	// init audio
	ISpObjectTokenCategory *pTokenCategory;
	HRESULT hr = CoCreateInstance(CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, IID_ISpObjectTokenCategory, (void **)&pTokenCategory);
	if (SUCCEEDED(hr)) {
		hr = pTokenCategory->SetId(SPCAT_AUDIOOUT, TRUE);
		if (SUCCEEDED(hr)) {
			WCHAR *tokenId;
			hr = pTokenCategory->GetDefaultTokenId(&tokenId);
			if (SUCCEEDED(hr)) {
				ISpObjectToken *pToken;
				hr = CoCreateInstance(CLSID_SpObjectToken, NULL, CLSCTX_ALL, IID_ISpObjectToken, (void **)&pToken);
				if (SUCCEEDED(hr)) {
					hr = pToken->SetId(NULL, tokenId, FALSE);
					if (SUCCEEDED(hr)) {
						hr = pToken->CreateInstance(NULL, CLSCTX_ALL, IID_ISpAudio, (void **)&_audio);
					}
				}
				CoTaskMemFree(tokenId);
			}
		}
	}
	if (FAILED(hr)) {
		warning("Could not initialize TTS audio");
		return;
	}

	// init voice
	hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&_voice);
	if (FAILED(hr)) {
		warning("Could not initialize TTS voice");
		return;
	}

	_speechState = NO_VOICE;

#ifdef USE_TRANSLATION
	setLanguage(TransMan.getCurrentLanguage());
#else
	setLanguage("en");
#endif

	_voice->SetOutput(_audio, FALSE);

	if (!_ttsState->_availableVoices.empty())
		_speechState = READY;
	else
		_speechState = NO_VOICE;
	_lastSaid = "";
	while (!_speechQueue.empty()) {
		free(_speechQueue.front());
		_speechQueue.pop_front();
	}
}

WindowsTextToSpeechManager::~WindowsTextToSpeechManager() {
	stop();

	clearState();

	if (_thread != NULL) {
		WaitForSingleObject(_thread, INFINITE);
		CloseHandle(_thread);
	}
	if (_speechMutex != NULL) {
		CloseHandle(_speechMutex);
	}
	if (_voice)
		_voice->Release();
	::CoUninitialize();
}

DWORD WINAPI startSpeech(LPVOID parameters) {
	WindowsTextToSpeechManager::SpeechParameters *params =
		(WindowsTextToSpeechManager::SpeechParameters *) parameters;
	// wait for the previous speech, if the previous thread exited too early
	_voice->WaitUntilDone(INFINITE);

	while (!params->queue->empty()) {
		WaitForSingleObject(*params->mutex, INFINITE);
		// check again, when we have exclusive access to the queue
		if (params->queue->empty() || *(params->state) == WindowsTextToSpeechManager::PAUSED) {
			ReleaseMutex(*params->mutex);
			break;
		}
		WCHAR *currentSpeech = params->queue->front();
		_voice->Speak(currentSpeech, SPF_PURGEBEFORESPEAK | SPF_ASYNC, 0);
		ReleaseMutex(*params->mutex);

		while (*(params->state) != WindowsTextToSpeechManager::PAUSED)
			if (_voice->WaitUntilDone(10) == S_OK)
				break;

		WaitForSingleObject(*params->mutex, INFINITE);
		if (!params->queue->empty() && params->queue->front() == currentSpeech) {
			if (currentSpeech != NULL)
				free(currentSpeech);
			params->queue->pop_front();
		}
		ReleaseMutex(*params->mutex);
	}

	WaitForSingleObject(*params->mutex, INFINITE);
	if (*(params->state) != WindowsTextToSpeechManager::PAUSED)
		*(params->state) = WindowsTextToSpeechManager::READY;
	ReleaseMutex(*params->mutex);
	return 0;
}

bool WindowsTextToSpeechManager::say(const Common::U32String &str, Action action) {
	if (_speechState == BROKEN || _speechState == NO_VOICE) {
		warning("The text to speech cannot speak in this state");
		return true;
	}

	if (isSpeaking() && action == DROP)
		return true;

	// We have to set the pitch by prepending xml code at the start of the said string;
	Common::U32String pitch = Common::U32String::format("<pitch absmiddle=\"%d\">%S", _ttsState->_pitch / 10, str.c_str());
	WCHAR *strW = (WCHAR *) pitch.encodeUTF16Native();
	if (strW == nullptr) {
		warning("Cannot convert from UTF-32 encoding for text to speech");
		return true;
	}

	WaitForSingleObject(_speechMutex, INFINITE);
	if (isSpeaking() && !_speechQueue.empty() && action == INTERRUPT_NO_REPEAT &&
			_speechQueue.front() != NULL && !wcscmp(_speechQueue.front(), strW)) {
		while (_speechQueue.size() != 1) {
			free(_speechQueue.back());
			_speechQueue.pop_back();
		}
		free(strW);
		ReleaseMutex(_speechMutex);
		return true;
	}

	if (isSpeaking() && !_speechQueue.empty() && action == QUEUE_NO_REPEAT &&
			_speechQueue.front() != NULL &&!wcscmp(_speechQueue.back(), strW)) {
		ReleaseMutex(_speechMutex);
		return true;
	}

	ReleaseMutex(_speechMutex);
	if ((isPaused() || isSpeaking()) && (action == INTERRUPT || action == INTERRUPT_NO_REPEAT)) {
		stop();
	}

	WaitForSingleObject(_speechMutex, INFINITE);
	_speechQueue.push_back(strW);
	ReleaseMutex(_speechMutex);

	if (!isSpeaking() && !isPaused()) {
		DWORD threadId;
		if (_thread != NULL) {
			WaitForSingleObject(_thread, INFINITE);
			CloseHandle(_thread);
		}
		_speechState = SPEAKING;
		_thread = CreateThread(NULL, 0, startSpeech, &_threadParams, 0, &threadId);
		if (_thread == NULL) {
			warning("Could not create speech thread");
			_speechState = READY;
			return true;
		}
	}
	return false;
}

bool WindowsTextToSpeechManager::stop() {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return true;
	if (isPaused())
		resume();
	_audio->SetState(SPAS_STOP, 0);
	WaitForSingleObject(_speechMutex, INFINITE);
	// Delete the speech queue
	while (!_speechQueue.empty()) {
		if (_speechQueue.front() != NULL)
			free(_speechQueue.front());
		_speechQueue.pop_front();
	}
	// Stop the current speech
	_voice->Speak(NULL, SPF_PURGEBEFORESPEAK | SPF_ASYNC, 0);
	_speechState = READY;
	ReleaseMutex(_speechMutex);
	_audio->SetState(SPAS_RUN, 0);
	return false;
}

bool WindowsTextToSpeechManager::pause() {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return true;
	if (isPaused())
		return false;
	WaitForSingleObject(_speechMutex, INFINITE);
	_voice->Pause();
	_speechState = PAUSED;
	ReleaseMutex(_speechMutex);
	return false;
}

bool WindowsTextToSpeechManager::resume() {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return true;
	if (!isPaused())
		return false;
	_voice->Resume();
	DWORD threadId;
	if (_thread != NULL) {
		WaitForSingleObject(_thread, INFINITE);
		CloseHandle(_thread);
	}
	_speechState = SPEAKING;
	_thread = CreateThread(NULL, 0, startSpeech, &_threadParams, 0, &threadId);
	if (_thread == NULL) {
		warning("Could not create speech thread");
		_speechState = READY;
		return true;
	}
	return false;
}

bool WindowsTextToSpeechManager::isSpeaking() {
	return _speechState == SPEAKING;
}

bool WindowsTextToSpeechManager::isPaused() {
	return _speechState == PAUSED;
}

bool WindowsTextToSpeechManager::isReady() {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return false;
	if (_speechState != PAUSED && !isSpeaking())
		return true;
	else
		return false;
}

void WindowsTextToSpeechManager::setVoice(unsigned index) {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	_voice->SetVoice((ISpObjectToken *) _ttsState->_availableVoices[index].getData());
	_ttsState->_activeVoice = index;
}

void WindowsTextToSpeechManager::setRate(int rate) {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	assert(rate >= -100 && rate <= 100);
	_voice->SetRate(rate / 10);
	_ttsState->_rate = rate;
}

void WindowsTextToSpeechManager::setPitch(int pitch) {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	assert(pitch >= -100 && pitch <= 100);
	_ttsState->_pitch = pitch;
}

void WindowsTextToSpeechManager::setVolume(unsigned volume) {
	if (_speechState == BROKEN || _speechState == NO_VOICE)
		return;
	assert(volume <= 100);
	_voice->SetVolume(volume);
	_ttsState->_volume = volume;
}

void WindowsTextToSpeechManager::setLanguage(Common::String language) {
	Common::TextToSpeechManager::setLanguage(language);
	updateVoices();
	setVoice(0);
}

void WindowsTextToSpeechManager::createVoice(void *cpVoiceToken) {
	ISpObjectToken *voiceToken = (ISpObjectToken *) cpVoiceToken;

	// description
	WCHAR *descW;
	char *buffer;
	Common::String desc;
	HRESULT hr = voiceToken->GetStringValue(NULL, &descW);
	if (SUCCEEDED(hr)) {
		buffer = Win32::unicodeToAnsi(descW);
		desc = buffer;
		free(buffer);
		CoTaskMemFree(descW);
	}

	if (desc == "Sample TTS Voice") {
		// This is really bad voice, it is basicaly unusable
		return;
	}

	// voice attributes
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
	Common::String language = lcidToLocale(wcstol(data, NULL, 16));
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
	Common::TTSVoice::Gender gender = !wcscmp(data, L"Male") ? Common::TTSVoice::MALE : Common::TTSVoice::FEMALE;
	CoTaskMemFree(data);

	// age
	hr = key->GetStringValue(L"Age", &data);
	if (FAILED(hr)) {
		voiceToken->Release();
		warning("Could not get the age attribute for voice: %s", desc.c_str());
		return;
	}
	Common::TTSVoice::Age age = !wcscmp(data, L"Adult") ? Common::TTSVoice::ADULT : Common::TTSVoice::UNKNOWN_AGE;
	CoTaskMemFree(data);

	_ttsState->_availableVoices.push_back(Common::TTSVoice(gender, age, (void *) voiceToken, desc));
}

Common::String WindowsTextToSpeechManager::lcidToLocale(LCID locale) {
	int nchars = GetLocaleInfoA(locale, LOCALE_SISO639LANGNAME, NULL, 0);
	char *languageCode = new char[nchars];
	GetLocaleInfoA(locale, LOCALE_SISO639LANGNAME, languageCode, nchars);
	Common::String result = languageCode;
	delete[] languageCode;
	return result;
}

void WindowsTextToSpeechManager::updateVoices() {
	if (_speechState == BROKEN)
		return;
	_ttsState->_availableVoices.clear();
	ISpObjectToken *cpVoiceToken = nullptr;
	IEnumSpObjectTokens *cpEnum = nullptr;
	unsigned long ulCount = 0;

	ISpObjectTokenCategory *cpCategory;
	HRESULT hr = CoCreateInstance(CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, IID_ISpObjectTokenCategory, (void**)&cpCategory);
	if (SUCCEEDED(hr)) {
		hr = cpCategory->SetId(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices", FALSE);
		if (!SUCCEEDED(hr)) {
			hr = cpCategory->SetId(SPCAT_VOICES, FALSE);
		}

		if (SUCCEEDED(hr)) {
			hr = cpCategory->EnumTokens(NULL, NULL, &cpEnum);
		}
	}

	if (SUCCEEDED(hr)) {
		hr = cpEnum->GetCount(&ulCount);
	}
	_voice->SetVolume(0);
	while (SUCCEEDED(hr) && ulCount--) {
		hr = cpEnum->Next(1, &cpVoiceToken, NULL);
		_voice->SetVoice(cpVoiceToken);
		if (SUCCEEDED(_voice->Speak(L"hi, this is test", SPF_PURGEBEFORESPEAK | SPF_ASYNC | SPF_IS_NOT_XML, 0)))
			createVoice(cpVoiceToken);
		else
			cpVoiceToken->Release();
	}
	// stop the test speech, we don't use stop(), because we don't wan't it to set state to READY
	// and we could easily be in NO_VOICE or BROKEN state here, in which the stop() wouldn't work
	_audio->SetState(SPAS_STOP, 0);
	_audio->SetState(SPAS_RUN, 0);
	_voice->Speak(NULL, SPF_PURGEBEFORESPEAK | SPF_ASYNC | SPF_IS_NOT_XML, 0);
	_voice->SetVolume(_ttsState->_volume);
	cpEnum->Release();

	if (_ttsState->_availableVoices.empty()) {
		_speechState = NO_VOICE;
		warning("No voice is available for language: %s", _ttsState->_language.c_str());
	} else if (_speechState == NO_VOICE)
		_speechState = READY;
}

void WindowsTextToSpeechManager::freeVoiceData(void *data) {
	ISpObjectToken *voiceToken = (ISpObjectToken *) data;
	voiceToken->Release();
}

#endif
