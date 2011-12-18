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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DREAMWEB_H
#define DREAMWEB_H

#include "common/error.h"
#include "common/file.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "dreamweb/dreamgen.h"
#include "dreamweb/console.h"

#include "dreamweb/structs.h"

#define SCUMMVM_HEADER MKTAG('S', 'C', 'V', 'M')
#define SCUMMVM_BLOCK_MAGIC_SIZE 0x1234
#define SAVEGAME_VERSION 1

namespace DreamGen {

// These are for ReelRoutine::reelPointer, which is a callback field.
const uint16 addr_backobject = 0xc170;
const uint16 addr_mainman = 0xc138;

// Keyboard buffer. data.word(kBufferin) and data.word(kBufferout) are indexes
// into this, making it a ring buffer
extern uint8 g_keyBuffer[16];

}

namespace DreamWeb {

// Engine Debug Flags
enum {
	kDebugAnimation = (1 << 0),
	kDebugSaveLoad = (1 << 1)
};

struct DreamWebGameDescription {
	ADGameDescription desc;
};

class DreamWebEngine : public Engine {
private:
	DreamWebConsole			*_console;
	bool					_vSyncInterrupt;

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

public:
	DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc);
	virtual ~DreamWebEngine();

	void setVSyncInterrupt(bool flag);
	void waitForVSync();

	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();

	uint8 randomNumber() { return _rnd.getRandomNumber(255); }

	void openFile(const Common::String &name);
	uint32 readFromFile(uint8 *data, unsigned size);
	uint32 skipBytes(uint32 bytes);
	void closeFile();

	void mouseCall(uint16 *x, uint16 *y, uint16 *state); //fill mouse pos and button state
	void processEvents();
	void blit(const uint8 *src, int pitch, int x, int y, int w, int h);
	void cls();

	void getPalette(uint8 *data, uint start, uint count);
	void setPalette(const uint8 *data, uint start, uint count);

	Common::String getSavegameFilename(int slot) const;

	void setShakePos(int pos) { _system->setShakePos(pos); }
	void printUnderMonitor();

	void quit();

	void loadSounds(uint bank, const Common::String &file);
	bool loadSpeech(const Common::String &filename);

	void enableSavingOrLoading(bool enable = true) { _enableSavingOrLoading = enable; }

	Common::Language getLanguage() const { return _language; }
	uint8 modifyChar(uint8 c) const;

	void stopSound(uint8 channel);

	DreamGen::Frame *icons1() const { return (DreamGen::Frame *)_icons1; }
	DreamGen::Frame *icons2() const { return (DreamGen::Frame *)_icons2; }
	void setIcons1(void *frames) { assert(_icons1 == NULL); _icons1 = frames; }
	void setIcons2(void *frames) { assert(_icons2 == NULL); _icons2 = frames; }
	void freeIcons1() { free(_icons1); _icons1 = NULL; }
	void freeIcons2() { free(_icons2); _icons2 = NULL; }

	DreamGen::Frame *tempCharset() const { return (DreamGen::Frame *)_tempCharset; }
	void setTempCharset(void *frames) { assert(_tempCharset == NULL); _tempCharset = frames; }
	void freeTempCharset() { free(_tempCharset); _tempCharset = NULL; }

	DreamGen::Frame *currentCharset() const { return _currentCharset; }
	void setCurrentCharset(DreamGen::Frame *charset) { _currentCharset = charset; }

private:
	void keyPressed(uint16 ascii);
	void setSpeed(uint speed);
	void soundHandler();
	void playSound(uint8 channel, uint8 id, uint8 loops);

	const DreamWebGameDescription	*_gameDescription;
	Common::RandomSource			_rnd;

	Common::File _file;
	Common::InSaveFile *_inSaveFile;

	uint _speed;
	bool _turbo;
	uint _oldMouseState;
	bool _enableSavingOrLoading;
	Common::Language _language;

	struct Sample {
		uint offset;
		uint size;
		Sample(): offset(), size() {}
	};

	struct SoundData {
		Common::Array<Sample> samples;
		Common::Array<uint8> data;
	};
	SoundData _soundData[2];
	Common::Array<uint8> _speechData;

	Audio::SoundHandle _channelHandle[2];
	uint8 _channel0, _channel1;

	void *_icons1;
	void *_icons2;
	void *_tempCharset;
	DreamGen::Frame *_currentCharset;

	DreamGen::DreamGenContext _context;
	DreamGen::DreamBase &_base;
};

} // End of namespace DreamWeb

#endif
