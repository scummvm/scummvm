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

	void mouseCall(); //fill mouse pos and button state
	void processEvents();
	void setPalette();
	void fadeDos();
	void blit(const uint8 *src, int pitch, int x, int y, int w, int h);
	void cls();

	void getPalette(uint8 *data, uint start, uint count);
	void setPalette(const uint8 *data, uint start, uint count);

	void openSaveFileForWriting(const Common::String &name);
	uint writeToSaveFile(const uint8 *data, uint size);

	bool openSaveFileForReading(const Common::String &name);
	uint readFromSaveFile(uint8 *data, uint size);

	void setShakePos(int pos) { _system->setShakePos(pos); }
	void printUnderMonitor();

	void quit();

	void loadSounds(uint bank, const Common::String &file);
	bool loadSpeech(const Common::String &filename);

	void enableSavingOrLoading(bool enable = true) { _enableSavingOrLoading = enable; }

	Common::Language getLanguage() const { return _language; }
	uint8 modifyChar(uint8 c) const;

	void stopSound(uint8 channel);

private:
	void keyPressed(uint16 ascii);
	void setSpeed(uint speed);
	void soundHandler();
	void playSound(uint8 channel, uint8 id, uint8 loops);

	const DreamWebGameDescription	*_gameDescription;
	Common::RandomSource			_rnd;

	Common::File _file;
	Common::OutSaveFile *_outSaveFile;
	Common::InSaveFile *_inSaveFile;

	uint _speed;
	bool _turbo;
	uint _oldMouseState;
	int _loadSavefile;
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

	DreamGen::DreamGenContext _context;
};

} // End of namespace DreamWeb

#endif
