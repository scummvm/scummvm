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
#if defined(USE_TTS)

namespace Common {
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
		for (TTSVoice *i = _ttsState->_availaibleVoices.begin(); i < _ttsState->_availaibleVoices.end(); i++) {
			free(i->_data);
		}
		delete _ttsState;
		_ttsState = tmp;
	}
}

void TextToSpeechManager::pushState() {
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
	if (_ttsState->_next == nullptr)
		return true;

	for (TTSVoice *i = _ttsState->_availaibleVoices.begin(); i < _ttsState->_availaibleVoices.end(); i++) {
		free(i->_data);
	}

	TTSState *oldState = _ttsState;
	_ttsState = _ttsState->_next;

	delete oldState;

	setLanguage(_ttsState->_language);
	setPitch(_ttsState->_pitch);
	setVolume(_ttsState->_volume);
	setRate(_ttsState->_rate);
	return false;
}

}
#endif
