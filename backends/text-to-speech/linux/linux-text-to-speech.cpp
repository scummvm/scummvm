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

#include "backends/text-to-speech/linux/linux-text-to-speech.h"

#if defined(USE_TTS) && defined(USE_SPEECH_DISPATCHER) && defined(POSIX)
#include <speech-dispatcher/libspeechd.h>

#include "common/translation.h"
#include "common/system.h"
#include "common/ustr.h"
#include "common/config-manager.h"
#include "common/encoding.h"
#include <pthread.h>

SPDConnection *_connection;

void speech_begin_callback(size_t msg_id, size_t client_id, SPDNotificationType state){
	SpeechDispatcherManager *manager =
		static_cast<SpeechDispatcherManager *> (g_system->getTextToSpeechManager());
	manager->updateState(SpeechDispatcherManager::SPEECH_BEGUN);
}

void speech_end_callback(size_t msg_id, size_t client_id, SPDNotificationType state){
	SpeechDispatcherManager *manager =
		static_cast<SpeechDispatcherManager *> (g_system->getTextToSpeechManager());
	manager->updateState(SpeechDispatcherManager::SPEECH_ENDED);
}

void speech_cancel_callback(size_t msg_id, size_t client_id, SPDNotificationType state){
	SpeechDispatcherManager *manager =
		static_cast<SpeechDispatcherManager *> (g_system->getTextToSpeechManager());
	manager->updateState(SpeechDispatcherManager::SPEECH_CANCELED);
}

void speech_resume_callback(size_t msg_id, size_t client_id, SPDNotificationType state){
	SpeechDispatcherManager *manager =
		static_cast<SpeechDispatcherManager *> (g_system->getTextToSpeechManager());
	manager->updateState(SpeechDispatcherManager::SPEECH_RESUMED);
}

void speech_pause_callback(size_t msg_id, size_t client_id, SPDNotificationType state){
	SpeechDispatcherManager *manager =
		static_cast<SpeechDispatcherManager *> (g_system->getTextToSpeechManager());
	manager->updateState(SpeechDispatcherManager::SPEECH_PAUSED);
}


void *SpeechDispatcherManager::startSpeech(void *p) {
	StartSpeechParams *params = (StartSpeechParams *) p;
	pthread_mutex_lock(params->mutex);
	if (!_connection || g_system->getTextToSpeechManager()->isPaused() ||
			params->speechQueue->front().empty()) {
		pthread_mutex_unlock(params->mutex);
		return NULL;
	}
	if (spd_say(_connection, SPD_MESSAGE, params->speechQueue->front().c_str()) == -1) {
		// close the connection
		if (_connection != 0) {
			spd_close(_connection);
			_connection = 0;
		}
	}
	pthread_mutex_unlock(params->mutex);
	return NULL;
}

SpeechDispatcherManager::SpeechDispatcherManager()
	: _speechState(READY) {
	pthread_mutex_init(&_speechMutex, NULL);
	_params.mutex = &_speechMutex;
	_params.speechQueue = &_speechQueue;
	_threadCreated = false;
	init();
}

void SpeechDispatcherManager::init() {
	_connection = spd_open("ScummVM", "main", NULL, SPD_MODE_THREADED);
	if (_connection == 0) {
		_speechState = BROKEN;
		warning("Couldn't initialize text to speech through speech-dispatcher");
		return;
	}

	_connection->callback_begin = speech_begin_callback;
	spd_set_notification_on(_connection, SPD_BEGIN);
	_connection->callback_end = speech_end_callback;
	spd_set_notification_on(_connection, SPD_END);
	_connection->callback_cancel = speech_cancel_callback;
	spd_set_notification_on(_connection, SPD_CANCEL);
	_connection->callback_resume = speech_resume_callback;
	spd_set_notification_on(_connection, SPD_RESUME);
	_connection->callback_pause = speech_pause_callback;
	spd_set_notification_on(_connection, SPD_PAUSE);

	updateVoices();
	_ttsState->_activeVoice = 0;
#ifdef USE_TRANSLATION
	setLanguage(TransMan.getCurrentLanguage());
#else
	setLanguage("en");
#endif
	_speechQueue.clear();
}

SpeechDispatcherManager::~SpeechDispatcherManager() {
	stop();
	if (_connection != 0)
		spd_close(_connection);
	if (_threadCreated)
		pthread_join(_thread, NULL);
	pthread_mutex_destroy(&_speechMutex);
}

void SpeechDispatcherManager::updateState(SpeechDispatcherManager::SpeechEvent event) {
	if (_speechState == BROKEN)
		return;
	switch(event) {
	case SPEECH_ENDED:
		pthread_mutex_lock(&_speechMutex);
		_speechQueue.pop_front();
		if (_speechQueue.empty())
			_speechState = READY;
		else {
			// reinitialize if needed
			if (!_connection)
				init();
			if (_speechState != BROKEN) {
				if (_threadCreated)
					pthread_join(_thread, NULL);
				_threadCreated = true;
				if (pthread_create(&_thread, NULL, startSpeech, &_params)) {
					_threadCreated = false;
					warning("TTS: Cannot start new speech");
				}
			}
		}
		pthread_mutex_unlock(&_speechMutex);
		break;
	case SPEECH_PAUSED:
		_speechState = PAUSED;
		break;
	case SPEECH_CANCELED:
		if (_speechState != PAUSED) {
			_speechState = READY;
		}
		break;
	case SPEECH_RESUMED:
		break;
	case SPEECH_BEGUN:
		_speechState = SPEAKING;
		break;
	}
}

bool SpeechDispatcherManager::say(Common::String str, Action action, Common::String charset) {

	pthread_mutex_lock(&_speechMutex);
	// reinitialize if needed
	if (!_connection)
		init();

	if (_speechState == BROKEN) {
		pthread_mutex_unlock(&_speechMutex);
		return true;
	}

	if (action == DROP && isSpeaking()) {
		pthread_mutex_unlock(&_speechMutex);
		return true;
	}

	if (charset.empty()) {
#ifdef USE_TRANSLATION
		charset = TransMan.getCurrentCharset();
#else
		charset = "ASCII";
#endif
	}

	char *tmpStr = Common::Encoding::convert("UTF-8", charset, str.c_str(), str.size());
	if (tmpStr == nullptr) {
		warning("Cannot convert from %s encoding for text to speech", charset.c_str());
		pthread_mutex_unlock(&_speechMutex);
		return true;
	}
	Common::String strUtf8 = tmpStr;
	free(tmpStr);

	if (!_speechQueue.empty() && action == INTERRUPT_NO_REPEAT &&
			_speechQueue.front() == strUtf8 && isSpeaking()) {
		_speechQueue.clear();
		_speechQueue.push_back(strUtf8);
		pthread_mutex_unlock(&_speechMutex);
		return true;
	}

	if (!_speechQueue.empty() && action == QUEUE_NO_REPEAT &&
			_speechQueue.back() == strUtf8 && isSpeaking()) {
		pthread_mutex_unlock(&_speechMutex);
		return true;
	}

	pthread_mutex_unlock(&_speechMutex);
	if (isSpeaking() && (action == INTERRUPT || action == INTERRUPT_NO_REPEAT))
		stop();
	if (!strUtf8.empty()) {
		pthread_mutex_lock(&_speechMutex);
		_speechQueue.push_back(strUtf8);
		pthread_mutex_unlock(&_speechMutex);
		if (isReady()) {
			_speechState = SPEAKING;
			startSpeech((void *)(&_params));
		}
	}

	return false;
}

bool SpeechDispatcherManager::stop() {
	if (_speechState == READY || _speechState == BROKEN)
		return true;
	_speechState = READY;
	pthread_mutex_lock(&_speechMutex);
	_speechQueue.clear();
	bool result = spd_cancel(_connection) == -1;
	pthread_mutex_unlock(&_speechMutex);
	return result;
}

bool SpeechDispatcherManager::pause() {
	if (_speechState == READY || _speechState == PAUSED || _speechState == BROKEN)
		return true;
	pthread_mutex_lock(&_speechMutex);
	_speechState = PAUSED;
	bool result = spd_cancel_all(_connection) == -1;
	pthread_mutex_unlock(&_speechMutex);
	if (result)
		return true;
	return false;
}

bool SpeechDispatcherManager::resume() {
	if (_speechState == READY || _speechState == SPEAKING || _speechState == BROKEN)
		return true;
	// If there is a thread from before pause() waiting, let it finish (it shouln't
	// do anything). There shouldn't be any other threads getting created,
	// because the speech is paused, so we don't need to synchronize
	if (_threadCreated) {
		pthread_join(_thread, NULL);
		_threadCreated = false;
	}
	_speechState = PAUSED;
	if (!_speechQueue.empty()) {
		_speechState = SPEAKING;
		startSpeech((void *) &_params);
	}
	else
		_speechState = READY;
	return false;
}

bool SpeechDispatcherManager::isSpeaking() {
	return _speechState == SPEAKING;
}

bool SpeechDispatcherManager::isPaused() {
	return _speechState == PAUSED;
}

bool SpeechDispatcherManager::isReady() {
	return _speechState == READY;
}

void SpeechDispatcherManager::setVoice(unsigned index) {
	if (_speechState == BROKEN)
		return;
	assert(index < _ttsState->_availableVoices.size());
	Common::TTSVoice voice = _ttsState->_availableVoices[index];
	spd_set_voice_type(_connection, *(SPDVoiceType *)(voice.getData()));
	_ttsState->_activeVoice = index;
}

void SpeechDispatcherManager::setRate(int rate) {
	if (_speechState == BROKEN)
		return;
	assert(rate >= -100 && rate <= 100);
	spd_set_voice_rate(_connection, rate);
	_ttsState->_rate = rate;
}

void SpeechDispatcherManager::setPitch(int pitch) {
	if (_speechState == BROKEN)
		return;
	assert(pitch >= -100 && pitch <= 100);
	spd_set_voice_pitch(_connection, pitch);
	_ttsState->_pitch = pitch;
}

void SpeechDispatcherManager::setVolume(unsigned volume) {
	if (_speechState == BROKEN)
		return;
	assert(volume <= 100);
	spd_set_volume(_connection, (volume - 50) * 2);
	_ttsState->_volume = volume;
}

void SpeechDispatcherManager::setLanguage(Common::String language) {
	if (_speechState == BROKEN)
		return;
	spd_set_language(_connection, language.c_str());
	_ttsState->_language = language;
	setVoice(_ttsState->_activeVoice);
}

void SpeechDispatcherManager::createVoice(int typeNumber, Common::TTSVoice::Gender gender, Common::TTSVoice::Age age, char *description) {
	// This pointer will point to data needed for voice switching. It is stored
	// in the Common::TTSVoice and it is freed by freeVoiceData() once it
	// is not needed.
	SPDVoiceType *type = (SPDVoiceType *) malloc(sizeof(SPDVoiceType));
	*type = static_cast<SPDVoiceType>(typeNumber);
	Common::TTSVoice voice(gender, age, (void *) type, description);
	_ttsState->_availableVoices.push_back(voice);
}

void SpeechDispatcherManager::updateVoices() {
	if (_speechState == BROKEN)
		return;
	/* just use these voices:
	   SPD_MALE1, SPD_MALE2, SPD_MALE3,
	   SPD_FEMALE1, SPD_FEMALE2, SPD_FEMALE3,
	   SPD_CHILD_MALE, SPD_CHILD_FEMALE

	   it depends on the user to map them to the right voices in speech-dispatcher
	   configuration
	*/
	_ttsState->_availableVoices.clear();

	char **voiceInfo = spd_list_voices(_connection);

	createVoice(SPD_MALE1, Common::TTSVoice::MALE, Common::TTSVoice::ADULT, voiceInfo[0]);
	createVoice(SPD_MALE2, Common::TTSVoice::MALE, Common::TTSVoice::ADULT, voiceInfo[1]);
	createVoice(SPD_MALE3, Common::TTSVoice::MALE, Common::TTSVoice::ADULT, voiceInfo[2]);
	createVoice(SPD_FEMALE1, Common::TTSVoice::FEMALE, Common::TTSVoice::ADULT, voiceInfo[3]);
	createVoice(SPD_FEMALE2, Common::TTSVoice::FEMALE, Common::TTSVoice::ADULT, voiceInfo[4]);
	createVoice(SPD_FEMALE3, Common::TTSVoice::FEMALE, Common::TTSVoice::ADULT, voiceInfo[5]);
	createVoice(SPD_CHILD_MALE, Common::TTSVoice::MALE, Common::TTSVoice::CHILD, voiceInfo[6]);
	createVoice(SPD_CHILD_FEMALE, Common::TTSVoice::FEMALE, Common::TTSVoice::CHILD, voiceInfo[7]);

	for (int i = 0; i < 8; i++)
		free(voiceInfo[i]);

	free(voiceInfo);
}

void SpeechDispatcherManager::freeVoiceData(void *data) {
	free(data);
}


#endif
