#include <common/textconsole.h>
#include <common/debug.h>
#include <audio/decoders/xa.h>
#include <audio/audiostream.h>
#include <audio/mixer.h>
#include "common/memstream.h"
#include "VabSound.h"
#include "dragons.h"

namespace Dragons {
	VabSound::VabSound(Common::SeekableReadStream *msfData, const DragonsEngine *_vm) {
		loadHeader(msfData);

		auto dataSize = msfData->size() - msfData->pos();
		byte *newData = new byte[dataSize];
		msfData->read(newData, dataSize);

		_vbData = new Common::MemoryReadStream(newData, dataSize, DisposeAfterUse::YES);

		Audio::AudioStream *str = Audio::makeXAStream(_vbData, 11025);
		Audio::SoundHandle _speechHandle;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speechHandle, str);

		delete msfData;
	}

	VabSound::VabSound(Common::SeekableReadStream *vhData, Common::SeekableReadStream *vbData): _vbData(vbData) {
		loadHeader(vhData);

		assert(vhData->pos() == vhData->size());

		delete vhData;
	}

	void VabSound::loadHeader(Common::SeekableReadStream *vhData) {
		vhData->seek(0);
		vhData->read(&_header, sizeof(_header));
		if (strncmp(_header.magic, "pBAV", 4) != 0) {
			error("Invalid VAB file");
		}
		// TODO: is sizeof(array) the right thing to do here?
		vhData->read(&_programAttrs, sizeof(_programAttrs));

		const int numTones = 16 * _header.numPrograms;
		_toneAttrs = new VabToneAttr[numTones];
		vhData->read(_toneAttrs, sizeof(VabToneAttr) * numTones);

		uint16 tempOffsets[0x100];
		vhData->read(tempOffsets, sizeof(tempOffsets));
		_vagOffsets[0] = tempOffsets[0] << 3u;
		for (int j = 1; j < 0x100; ++j) {
			const int vagSize = tempOffsets[j] << 3u;
			_vagSizes[j - 1] = vagSize;
			_vagOffsets[j] = vagSize + _vagOffsets[j - 1];
		}
	}

	VabSound::~VabSound() {
		delete _toneAttrs;
		delete _vbData;
	}

	void VabSound::playSound(uint16 program, uint16 key) {
		// TODO
		debug("Playing program %d, key %d", program, key);
	}
} // End of namespace Dragons