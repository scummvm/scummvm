#include "cryo/sound.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Cryo {

	CSoundChannel::CSoundChannel(Audio::Mixer *mixer, unsigned int sampleRate, bool stereo) : _mixer(mixer), _sampleRate(sampleRate), _stereo(stereo) {
	_audioStream = nullptr;
	_volumeLeft = _volumeRight = Audio::Mixer::kMaxChannelVolume;
}

CSoundChannel::~CSoundChannel() {
	stop();
	if (_audioStream)
		delete _audioStream;
}

void CSoundChannel::queueBuffer(byte *buffer, unsigned int size, bool playNow) {
	if (playNow)
		stop();
	if (!_audioStream)
		_audioStream = Audio::makeQueuingAudioStream(_sampleRate, _stereo);
	_audioStream->queueBuffer(buffer, size, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
	if (!_mixer->isSoundHandleActive(_soundHandle)) {
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		applyVolumeChange();
	}
}

void CSoundChannel::stop() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);

	if (_audioStream) {
		_audioStream->finish();
		delete _audioStream;
		_audioStream = nullptr;
	}
}

unsigned int CSoundChannel::numQueued() {
	return _audioStream ? _audioStream->numQueuedStreams() : 0;
}

unsigned int CSoundChannel::getVolume() {
	return (_volumeRight + _volumeLeft) / 2;
}

void CSoundChannel::setVolume(unsigned int volumeLeft, unsigned int volumeRight) {
	_volumeLeft = volumeLeft;
	_volumeRight = volumeRight;
	applyVolumeChange();
}

void CSoundChannel::setVolumeLeft(unsigned int volume) {
	setVolume(volume, _volumeRight);
}

void CSoundChannel::setVolumeRight(unsigned int volume) {
	setVolume(_volumeLeft, volume);
}

void CSoundChannel::applyVolumeChange() {
	unsigned int volume = (_volumeRight + _volumeLeft) / 2;
	int balance = (signed int)(_volumeRight - _volumeLeft) / 2;
	_mixer->setChannelVolume(_soundHandle, volume);
	_mixer->setChannelBalance(_soundHandle, balance);
}

/****************************************************************/

SoundGroup::SoundGroup(CryoEngine *vm, int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode) : _vm(vm) {
	if (numSounds < kCryoMaxClSounds)
		_numSounds = numSounds;
	else
		error("SoundGroup - numSounds >= kCryoMaxClSounds");

	for (int i = 0; i < _numSounds; i++) {
		_sounds[i] = new sound_t(length, rate, sampleSize, mode);
		_sounds[i]->_maxLength = length;
	}
	_soundIndex = 0;
	_playIndex = 0;
	_forceWait = true;
}

// Original name: CLSoundGroup_Free
SoundGroup::~SoundGroup() {
	for (int16 i = 0; i < _numSounds; i++)
		delete(_sounds[i]);
}

// Original name: CLSoundGroup_Reverse16All
void SoundGroup::reverse16All() {
	for (int16 i = 0; i < _numSounds; i++)
		_sounds[i]->_reversed = true;
}

// Original name: CLSoundGroup_GetNextBuffer
void *SoundGroup::getNextBuffer() {
	sound_t *sound = _sounds[_soundIndex];
	if (_forceWait)
		while (sound->_locked) ;
	return sound->_sndHandle + sound->_headerLen;
}

// Original name: CLSoundGroup_AssignDatas
bool SoundGroup::assignDatas(void *buffer, int length, bool isSigned) {
	sound_t *sound = _sounds[_soundIndex];
	if (_forceWait)
		while (sound->_locked)
			;
	else if (sound->_locked)
		return false;

	sound->_buffer = (char *)buffer;
	sound->setLength(length);
	sound->_length = length;
	//	if(sound->reversed && sound->sampleSize == 16)
	//		ReverseBlock16(buffer, length);
	//	if(isSigned)
	//		CLSound_Signed2NonSigned(buffer, length);
	if (_soundIndex == _numSounds - 1)
		_soundIndex = 0;
	else
		_soundIndex++;

	return true;
}

// Original name: CLSoundGroup_SetDatas
bool SoundGroup::setDatas(void *data, int length, bool isSigned) {
	sound_t *sound = _sounds[_soundIndex];
	if (length >= sound->_maxLength)
		error("CLSoundGroup_SetDatas - Unexpected length");

	if (_forceWait)
		while (sound->_locked) ;
	else if (sound->_locked)
		return false;

	void *buffer = sound->_sndHandle + sound->_headerLen;
	sound->_buffer = (char *)buffer;
	memcpy(buffer, data, length);
	sound->setLength(length);
	sound->_length = length;
	//	if(sound->reversed && sound->sampleSize == 16)
	//		ReverseBlock16(buffer, length);
	//	if(isSigned)
	//		CLSound_Signed2NonSigned(buffer, length);
	if (_soundIndex == _numSounds - 1)
		_soundIndex = 0;
	else
		_soundIndex++;

	return true;
}

// Original name: CLSoundGroup_PlayNextSample
void SoundGroup::playNextSample(SoundChannel *ch) {
	ch->play(_sounds[_playIndex]);
	if (_playIndex == _numSounds - 1)
		_playIndex = 0;
	else
		_playIndex++;
}


}
