#include "sound.h"
#include "resource.h"

// Instrument mapping for MT32 tracks emulated under GM.
static const byte mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

namespace Kyra {
	MusicPlayer::MusicPlayer(MidiDriver* driver, KyraEngine* engine) {
		_engine = engine;
		_driver = driver;
		_passThrough = false;
		_isPlaying = _nativeMT32 = false;
		
		memset(_channel, 0, sizeof(MidiChannel*) * 16);
		memset(_channelVolume, 255, sizeof(uint8) * 16);
		_volume = 0;
		
		int ret = open();
		if (ret != MERR_ALREADY_OPEN && ret != 0) {
			error("couldn't open midi driver");
		}
	}
	
	MusicPlayer::~MusicPlayer() {
		_driver->setTimerCallback(NULL, NULL);
		close();
	}
	
	void MusicPlayer::setVolume(int volume) {
		if (volume < 0)
			volume = 0;
		else if (volume > 255)
			volume = 255;

		if (_volume == volume)
			return;

		_volume = volume;

		for (int i = 0; i < 16; ++i) {
			if (_channel[i]) {
				_channel[i]->volume(_channelVolume[i] * _volume / 255);
			}
		}
	}
	
	int MusicPlayer::open() {
		// Don't ever call open without first setting the output driver!
		if (!_driver)
			return 255;

		int ret = _driver->open();
		if (ret)
			return ret;

		_driver->setTimerCallback(this, &onTimer);
		return 0;
	}

	void MusicPlayer::close() {
		if (_driver)
			_driver->close();
		_driver = 0;
	}
	
	void MusicPlayer::send(uint32 b) {
		if (_passThrough) {
			_driver->send(b);
			return;
		}

		uint8 channel = (byte)(b & 0x0F);
		if ((b & 0xFFF0) == 0x07B0) {
			// Adjust volume changes by master volume
			uint8 volume = (uint8)((b >> 16) & 0x7F);
			_channelVolume[channel] = volume;
			volume = volume * _volume / 255;
			b = (b & 0xFF00FFFF) | (volume << 16);
		} else if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
			b = (b & 0xFFFF00FF) | mt32_to_gm[(b >> 8) & 0xFF] << 8;
		} else if ((b & 0xFFF0) == 0x007BB0) {
			//Only respond to All Notes Off if this channel
			//has currently been allocated
			if (!_channel[channel])
				return;
		}

		if (!_channel[channel])
			_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

		if (_channel[channel])
			_channel[channel]->send(b);
	}

	void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
		switch (type) {
			case 0x2F:	// End of Track
				_parser->jumpToTick(0);
			break;
			default:
				warning("Unhandled meta event: 0x%02x", type);
			break;
		}
	}
	
	void MusicPlayer::playMusic(const char* file) {
		uint32 size;
		uint8* data = 0;
		
		data = (_engine->resManager())->fileData(file, &size);
		
		if (!data) {
			warning("couldn't load '%s'", file);
			return;
		}
	
		playMusic(data, size);
	}
	
	void MusicPlayer::playMusic(uint8* data, uint32 size) {
		if (_isPlaying)
			stopMusic();
			
		_parser = MidiParser::createParser_XMIDI();
		assert(_parser);
		
		if (!_parser->loadMusic(data, size)) {
			warning("Error reading track!");
			delete _parser;
			_parser = 0;
			return;
		}

		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(getBaseTempo());

		_isPlaying = true;
	}

	void MusicPlayer::stopMusic() {
		_isPlaying = false;
		if (_parser) {
			_parser->unloadMusic();
			delete _parser;
			_parser = NULL;
		}
	}
	
	void MusicPlayer::onTimer(void *refCon) {
		MusicPlayer *music = (MusicPlayer *)refCon;
		if (music->_isPlaying)
			music->_parser->onTimer();
	}
	
	void MusicPlayer::playTrack(uint8 track) {
		if (_parser) {
			_isPlaying = true;
			_parser->setTrack(track);
			_parser->jumpToTick(0);
		}
	}
} // end of namespace Kyra
