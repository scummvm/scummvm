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
#if defined(USE_TTS)

namespace Common {

TTSVoice::TTSVoice()
	: _gender(UNKNOWN_GENDER)
	, _age(UNKNOWN_AGE)
	, _data(nullptr)
	, _description("") {
	_refCount = new int;
	*_refCount = 1;
}

TTSVoice::TTSVoice(Gender gender, Age age, void *data, String description)
	: _gender(gender)
	, _age(age)
	, _data(data)
	, _description(description) {
	_refCount = new int;
	*_refCount = 1;
}

TTSVoice::TTSVoice(const TTSVoice& voice)
	: _gender(voice._gender)
	, _age(voice._age)
	, _data(voice._data)
	, _refCount(voice._refCount)
	, _description(voice._description) {
	if (_data)
		(*_refCount)++;
}

TTSVoice::~TTSVoice() {
	// _data is a platform specific field and so it the
	// way it is freed differs from platform to platform
	if (--(*_refCount) == 0) {
		if (_data)
			g_system->getTextToSpeechManager()->freeVoiceData(_data);
		delete _refCount;
	}
}

TTSVoice& TTSVoice::operator=(const TTSVoice& voice) {
	if (&voice != this) {
		_gender = voice._gender;
		_data = voice._data;
		_age = voice._age;
		_refCount = voice._refCount;
		if (_data)
			(*_refCount)++;
		_description = voice._description;
	}
	return *this;
}

TextToSpeechManager::TextToSpeechManager() {
	_ttsState = new TTSState;
	_ttsState->_pitch = 0;
	_ttsState->_volume = 0;
	_ttsState->_rate = 0;
	_ttsState->_activeVoice = 0;
	_ttsState->_language = "en";
	_ttsState->_next = nullptr;
}

TextToSpeechManager::~TextToSpeechManager() {
	TTSState *tmp = _ttsState;
	while (tmp != nullptr) {
		tmp = _ttsState->_next;
		delete _ttsState;
		_ttsState = tmp;
	}
}

void TextToSpeechManager::pushState() {
	stop();
	TTSState *newState = new TTSState;
	newState->_pitch = _ttsState->_pitch;
	newState->_volume = _ttsState->_volume;
	newState->_rate = _ttsState->_rate;
	newState->_activeVoice = _ttsState->_activeVoice;
	newState->_language = _ttsState->_language;
	newState->_next = _ttsState;
	_ttsState = newState;
	updateVoices();
}

bool TextToSpeechManager::popState() {
	stop();
	if (_ttsState->_next == nullptr)
		return true;

	Common::TTSState *oldState = _ttsState;
	_ttsState = _ttsState->_next;

	delete oldState;

	// The voice has to be saved, because some backends change it when changing language
	int voice = _ttsState->_activeVoice;
	setLanguage(_ttsState->_language);
	setPitch(_ttsState->_pitch);
	setVolume(_ttsState->_volume);
	setRate(_ttsState->_rate);
	setVoice(voice);
	return false;
}

TTSVoice TextToSpeechManager::getVoice() {
	if (!_ttsState->_availableVoices.empty())
		return _ttsState->_availableVoices[_ttsState->_activeVoice];
	return TTSVoice();
}

Array<int> TextToSpeechManager::getVoiceIndicesByGender(TTSVoice::Gender gender) {
	Array<int> results;
	for (unsigned i = 0; i < _ttsState->_availableVoices.size(); i++) {
		if (_ttsState->_availableVoices[i].getGender() == gender)
			results.push_back(i);
	}
	return results;
}

}
#endif
