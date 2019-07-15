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

#include "backends/text-to-speech/windows/windows-text-to-speech.h"


#include "common/translation.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/ustr.h"
#include "common/config-manager.h"

ISpVoice *_voice;

WindowsTextToSpeechManager::WindowsTextToSpeechManager()
	: _speechState(BROKEN){
	init();
}

void WindowsTextToSpeechManager::init() {
	if (FAILED(::CoInitialize(NULL)))
		return;

	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&_voice);
	if (!SUCCEEDED(hr)) {
		warning("Could not initialize TTS voice");
		return;
	}
	updateVoices();
	_speechState = READY;
}

WindowsTextToSpeechManager::~WindowsTextToSpeechManager() {
	if (_voice)
		_voice->Release();
	::CoUninitialize();
}

bool WindowsTextToSpeechManager::say(Common::String str) {
	return true;
}

bool WindowsTextToSpeechManager::stop() {
	return true;
}

bool WindowsTextToSpeechManager::pause() {
	return true;
}

bool WindowsTextToSpeechManager::resume() {
	return true;
}

bool WindowsTextToSpeechManager::isSpeaking() {
	return true;
}

bool WindowsTextToSpeechManager::isPaused() {
	return true;
}

bool WindowsTextToSpeechManager::isReady() {
	return true;
}

void WindowsTextToSpeechManager::setVoice(unsigned index) {
}

void WindowsTextToSpeechManager::setRate(int rate) {
}

void WindowsTextToSpeechManager::setPitch(int pitch) {
}

void WindowsTextToSpeechManager::setVolume(unsigned volume) {
}

int WindowsTextToSpeechManager::getVolume() {
	return 0;
}

void WindowsTextToSpeechManager::setLanguage(Common::String language) {
}

void WindowsTextToSpeechManager::createVoice(int typeNumber, Common::TTSVoice::Gender gender, char *description) {
}

void WindowsTextToSpeechManager::updateVoices() {

}

#endif
