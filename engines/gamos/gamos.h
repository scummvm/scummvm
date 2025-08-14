/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GAMOS_H
#define GAMOS_H

#include "common/events.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/array.h"
#include "common/memstream.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "gamos/proc.h"
#include "gamos/music.h"
#include "gamos/movie.h"


#include "gamos/detection.h"
#include "gamos/file.h"

namespace Gamos {

struct GamosGameDescription;

enum CONFTYPE {
	CONFTP_P1 = 1,
	CONFTP_P2 = 2,
	CONFTP_P3 = 3,
	CONFTP_IDFLG = 0x80,
	CONFTP_RESMASK = 0x7f,

};

enum ACT2 {
	ACT_NONE = 0xe,
	ACT2_81 = 0x81,
	ACT2_82 = 0x82,
	ACT2_83 = 0x83,
	ACT2_84 = 0x84,
	ACT2_8f = 0x8f,
};

enum RESTYPE {
	RESTP_F = 0xf,
	RESTP_10 = 0x10,
	RESTP_11 = 0x11,
	RESTP_12 = 0x12,
	RESTP_18 = 0x18,
	RESTP_19 = 0x19,
	RESTP_20 = 0x20,
	RESTP_40 = 0x40,
	RESTP_41 = 0x41,
	RESTP_42 = 0x42,
	RESTP_43 = 0x43,
	RESTP_50 = 0x50,
	RESTP_51 = 0x51,
	RESTP_52 = 0x52,
	RESTP_XORSEQ0 = 0x7c,
	RESTP_XORSEQ1 = 0x7d,
	RESTP_XORSEQ2 = 0x7e,
};

struct BkgImage {
	bool loaded = false;
	uint32 offset = 0;
	Graphics::Surface surface;

	RawData rawData;
};

struct Image {
	bool loaded = false;
	int32 offset = 0;
	int32 size = 0;
	int32 cSize = 0;

	Graphics::Surface surface;

	RawData rawData;
};

struct ImagePos {
	int16 xoffset = 0;
	int16 yoffset = 0;

	Image *image = nullptr;
};

typedef Common::Array<ImagePos> ImageSeq;

struct Sprite {
	byte field_0;
	byte field_1;
	byte field_2;
	byte field_3;

	Common::Array<ImageSeq> sequences;
};

/* Used to xor savedata */
struct XorArg {
	uint32 pos;
	uint32 len;
};

class GamosEngine : public Engine {
friend class MoviePlayer;

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	bool _errSet = false;;
	Common::String _errMessage;

	Archive _arch;
	
	byte _cmdByte;

	bool _runReadDataMod;
	bool _currentModuleID;

	byte _saveLoadID;

	uint32 _magic;
	uint32 _pages1kbCount;
	uint32 _readBufSize;
	uint32 _width;
	uint32 _height;
	uint32 _unk2;
	uint32 _unk3;
	uint32 _movieCount;
	byte _unk5;
	byte _unk6;
	byte _unk7;
	byte _fps;
	byte _unk8;
	byte _unk9;
	byte _fadeEffectID;
	byte _unk11;

	int _isMoviePlay = 0;

	bool _onlyScanImage = false;
	int32 _resReadOffset = 0;

	Common::String _string1;
	Common::String _winCaption;

	Common::Array<uint32> _movieOffsets;

	Common::Array<Image *> _images;

	Common::Array<BkgImage> _bkgImages;

	Common::Array<Sprite> _sprites;

	Common::Array< Common::Array<byte> >  _midiTracks;

	uint32 _delayTime = 0;

	Common::Array<XorArg> _xorSeq[3];

	static const byte _xorKeys[32];



	uint8 BYTE_004177f7 = 0;

	bool _midiStarted = false;

	/* Data2 */



	MidiMusic _musicPlayer;
	SystemProc _messageProc;
	MoviePlayer _moviePlayer;

private:
	static const uint16 _winkeyMap[256];
	
protected:
	// Engine APIs
	Common::Error run() override;

	void readCMDByte() {
		_cmdByte = _arch.readByte();
	}

	bool loadModule(uint id);
	bool loader2();

	bool loadResHandler(uint tp, uint pid, uint p1, uint p2, uint p3, const byte *data, size_t dataSize);
	bool loadResHandler(uint tp, uint pid, uint p1, uint p2, uint p3, const RawData &data);

	bool reuseLastResource(uint tp, uint pid, uint p1, uint p2, uint p3);

	bool initOrLoadSave(int32);

	bool initMainDatas();

	bool init(const Common::String &moduleName);
	bool loadInitModule();

	void readElementsConfig(const RawData &data);

	void setFPS(uint fps);

	void loadXorSeq(const byte *data, size_t dataSize, int id);

	bool loadRes40(int32 id, const byte *data, size_t dataSize);
	bool loadRes41(int32 id, const byte *data, size_t dataSize);
	bool loadRes42(int32 id, int32 p1, const byte *data, size_t dataSize);
	bool loadRes43(int32 id, int32 p1, int32 p2, const byte *data, size_t dataSize);

	bool loadRes52(int32 id, const byte *data, size_t dataSize);


	void playMidi(Common::Array<byte> *buffer);

	void stopMidi();
	void stopMCI();
	void stopSounds();

	bool playIntro();

	bool scriptFunc18(int id);

	void setErrMessage(const Common::String &msg);

	void updateScreen(bool checkers, Common::Rect rect);

	void readData2(const RawData &data);


public:
	Graphics::Screen *_screen = nullptr;
public:
	GamosEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~GamosEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}
};

extern GamosEngine *g_engine;
#define SHOULD_QUIT ::Gamos::g_engine->shouldQuit()

} // End of namespace Gamos

#endif // GAMOS_H
