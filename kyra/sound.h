#ifndef SOUND_H
#define SOUND_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "kyra.h"

namespace Kyra {
	class MusicPlayer : public MidiDriver {
	
	public:
	
		MusicPlayer(MidiDriver* driver, KyraEngine* engine);
		~MusicPlayer();
		
		void setVolume(int volume);
		int getVolume() { return _volume; }
		
		void hasNativeMT32(bool nativeMT32) { _nativeMT32 = nativeMT32; }

		void playMusic(const char* file);
		void playMusic(uint8* data, uint32 size);
		void stopMusic();
		
		void playTrack(uint8 track);

		//MidiDriver interface implementation
		int open();
		void close();
		void send(uint32 b);
		void metaEvent(byte type, byte *data, uint16 length);

		void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
		uint32 getBaseTempo(void) { return _driver ? _driver->getBaseTempo() : 0; }

		//Channel allocation functions
		MidiChannel *allocateChannel()		{ return 0; }
		MidiChannel *getPercussionChannel()	{ return 0; }
	
	protected:
	
		static void onTimer(void *data);
	
		MidiChannel* _channel[16];
		uint8 _channelVolume[16];
		MidiDriver* _driver;
		bool _nativeMT32;
		uint8 _volume;
		bool _isPlaying;
		MidiParser* _parser;
		KyraEngine* _engine;

	};
} // end of namespace Kyra

#endif
