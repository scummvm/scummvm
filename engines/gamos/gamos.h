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
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "gamos/pool.h"

#include "gamos/keycodes.h"

#include "gamos/proc.h"
#include "gamos/music.h"
#include "gamos/movie.h"
#include "gamos/vm.h"

#include "gamos/detection.h"
#include "gamos/file.h"

#include "gamos/array2d.h"

namespace Gamos {

struct GamosGameDescription;

enum CONFTYPE {
	CONFTP_P1 = 1,
	CONFTP_P2 = 2,
	CONFTP_P3 = 3,
	CONFTP_IDFLG = 0x80,
	CONFTP_RESMASK = 0x7f,

};


enum RESTYPE {
	RESTP_F = 0xf,
	RESTP_10 = 0x10,
	RESTP_11 = 0x11,
	RESTP_12 = 0x12,
	RESTP_13 = 0x13,
	RESTP_18 = 0x18,
	RESTP_19 = 0x19,
	RESTP_20 = 0x20,
	RESTP_21 = 0x21,
	RESTP_22 = 0x22,
	RESTP_23 = 0x23,
	RESTP_2A = 0x2a,
	RESTP_2B = 0x2b,
	RESTP_2C = 0x2c,
	RESTP_38 = 0x38,
	RESTP_39 = 0x39,
	RESTP_3A = 0x3a,
	RESTP_40 = 0x40,
	RESTP_41 = 0x41,
	RESTP_42 = 0x42,
	RESTP_43 = 0x43,
	RESTP_50 = 0x50,
	RESTP_51 = 0x51,
	RESTP_52 = 0x52,
	RESTP_60 = 0x60,
	RESTP_61 = 0x61,
	RESTP_XORSEQ0 = 0x7c,
	RESTP_XORSEQ1 = 0x7d,
	RESTP_XORSEQ2 = 0x7e,
};

struct BkgImage {
	bool loaded = false;
	uint32 offset = 0;
	Graphics::Surface surface;
	byte *palette = nullptr;

	uint32 field2_0x8 = 0;
	uint32 field3_0xc = 0;

	RawData rawData;
};

struct Image {
	bool loaded = false;
	int32 offset = -1;
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
	uint32 index = 0;
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

struct Unknown1 {
	Common::Array<byte> field_0;
	Common::Array<byte> field_1;
	Common::Array<byte> field_2;
	uint32 field_3;
};

struct ScriptS {
	Common::Array<byte> data;
	int32 codes1 = -1;
	int32 codes2 = -1;
};

struct SomeAction {
	uint32 unk1;
	int32 script1 = -1;
	Common::Array< ScriptS > scriptS;
	int32 script2 = -1;
};

struct Object {
	/* additional data */
	int16 index = 0;
	Sprite *spr = nullptr;

	/* 80 - drawable
	   40 -
	   20 -
	   10 -
	   8 - has storage
	   4 -
	   2 -
	   1 - used */
	uint8 flags = 0;
	uint8 actID = 0;
	uint8 fld_2 = 0;
	uint8 fld_3 = 0;
	uint8 fld_4 = 0;
	uint8 fld_5 = 0;
	uint8 pos = 0xff;
	uint8 blk = 0xff;
	int16 x = 0;
	int16 y = 0;
	ImagePos *pImg = nullptr;
	Common::Array<byte> storage;
};

struct Dat61 {
	int16 x = 0;
	int16 y = 0;
	uint16 v = 0;
};


class GamosEngine : public Engine {
friend class MoviePlayer;

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	bool _errSet = false;
	Common::String _errMessage;

	Archive _arch;

	byte _cmdByte;

	bool _runReadDataMod;
	int _currentModuleID;

	byte _saveLoadID;

	uint32 _magic;
	uint32 _pages1kbCount;
	uint32 _readBufSize;
	uint32 _width;
	uint32 _height;
	int32 _gridCellW;
	int32 _gridCellH;
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

	Common::Point _bkgUpdateSizes;

	Common::Array<BkgImage> _bkgImages;

	Common::Array<Sprite> _sprites;

	Common::Array< Common::Array<byte> >  _midiTracks;

	Common::Array< Common::Array<byte> > _soundSamples;

	Common::Array< Common::Array<byte> > _dat60;
	Common::Array< Common::Array<Dat61> > _dat61;

	uint32 _delayTime = 0;
	uint32 _lastTimeStamp = 0;

	Common::Array<XorArg> _xorSeq[3];

	static const byte _xorKeys[32];

	uint32 _seed = 1;

	Object _cursorObject;
	uint32 _mouseCursorImgId = 0;



	uint8 BYTE_004177f7 = 0;
	uint8 DAT_004177f8 = 0;


	uint8 DAT_004177fd = 0;
	uint8 DAT_004177fe = 0;

	bool _midiStarted = false;

	/* Data2 */

	uint8 _d2_fld14 = 0;
	uint8 _d2_fld16 = 0;
	uint8 _d2_fld17 = 0;
	uint8 _d2_fld18 = 0;

	bool _enableMidi = false;
	int32 _midiTrack = 0;

	uint32 _readingBkgOffset = 0;
	int32 _readingBkgMainId = -1;
	int32 _loadedDataSize = -1;

	uint32 _addrBlk12 = 0;
	uint32 _addrFPS = 1;
	uint32 _addrKeyDown = 2;
	uint32 _addrKeyCode = 3;
	uint32 _addrCurrentFrame = 4;

	MidiMusic _musicPlayer;
	SystemProc _messageProc;
	MoviePlayer _moviePlayer;

	uint32 _statesWidth = 0;
	uint32 _statesHeight = 0;
	uint32 _statesShift = 0;
	Array2D<uint16> _states;


	uint8 _preprocDataId = 0;

	Common::Array<Unknown1> _thing2;
	Common::Array<SomeAction> _someActsArr;

	Pool<Object> _drawElements;

	uint8 BYTE_00412200 = 0;

	Object *DAT_00412204 = nullptr;

	SomeAction *PTR_00417214 = nullptr;
	Object *PTR_00417218 = nullptr;
	Object *PTR_004121b4 = nullptr;

	uint8 BYTE_004177f6 = 0;
	uint8 BYTE_004177fc = 0;
	bool DAT_004177ff = false;


	byte *PTR_004173e8 = nullptr;

	int32 DAT_00417220 = 0;
	int32 DAT_00417224 = 0;
	int32 DAT_00417228 = 0;
	int32 DAT_0041722c = 0;

	byte *PTR_00417388 = nullptr;

	int32 _curObjIndex = 0;

	bool DAT_00417802 = false;


	int32 DAT_004173f0 = 0;
	int32 DAT_004173f4 = 0;
	int32 DAT_004173f8 = 0;
	int32 DAT_004173fc = 0;
	uint8 DAT_00417803 = 0;

	uint8 DAT_00417805 = 0;

	uint8 RawKeyCode = 0;

	Common::Array<Common::Rect> _dirtyRects;

	bool _needReload = false;

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

	bool loadRes18(int32 id, const byte *data, size_t dataSize);


	bool playMidi(Common::Array<byte> *buffer);

	bool playSound(uint id);

	void stopMidi();
	void stopMCI();
	void stopSounds();

	bool playIntro();

	bool scriptFunc18(uint32 id);
	uint32 scriptFunc19(uint32 id);
	uint32 scriptFunc16(uint32 id);

	void setErrMessage(const Common::String &msg);

	void updateScreen(bool checkers, Common::Rect rect);

	void readData2(const RawData &data);

	uint8 update(Common::Point screenSize, Common::Point mouseMove, Common::Point actPos, uint8 act2, uint8 act1, uint16 keyCode, bool mouseInWindow);

	int32 ProcessScript(bool p1, const byte *data, size_t dataSize, int32 code1 = -1, int32 code2 = -1);

	void FUN_00404fcc(int32 id);

	uint32 getU32(const void *ptr);

	void preprocessData(int id, byte *data);
	void preprocessDataB1(int id, byte *data);
	int processData(int id, byte *data);

	void executeScript(uint8 p1, uint32 id, uint32 pos, byte *storage, int32 index, Object *pobj, SomeAction *act, int32 scriptAddr);

	void FUN_00402a68(const byte *d);

	void FUN_0040283c(int id, int pos, const byte *data);

	void FUN_00402654(int mode, int id, int pos);

	Object *getFreeObject();
	void removeObject(Object *obj);
	void removeObjectMarkDirty(Object *obj);
	void removeObjectByIDMarkDirty(int32 id);

	void FUN_004023d8(Object *obj);
	void FUN_0040255c(Object *obj);

	bool FUN_00402fb4();

	bool FUN_004033a8(Common::Point mouseMove);
	bool FUN_004038b8();
	bool FUN_00402bc4();
	bool FUN_00402f34(bool p1, bool p2, Object *obj);

	void FUN_0040921c(Object *obj);
	void addDirtRectOnObject(Object *obj);
	void addDirtyRect(const Common::Rect &rect);

	void doDraw();

	bool loadImage(Image *img);

	uint32 doScript(uint32 scriptAddress);

	bool FUN_0040738c(uint32 id, int32 x, int32 y, bool p);

	void FUN_00409378(Sprite *spr, Object *obj, bool p);

	void FUN_004095a0(Object *obj);

	bool playMovie(int id);

	void setCursor(int id, bool dirtRect);

	void FUN_00402c2c(Common::Point move, Common::Point actPos, uint8 act2, uint8 act1);
	bool FUN_00409600(Object *obj, Common::Point pos);

	uint32 FUN_004070f8(const byte *data, size_t dataSize);

	void setNeedReload() {
		_needReload = true;
		VM::_interrupt = true;
	};

	Object *FUN_00407588(int32 seq, int32 spr, int32 *pX, int32 y);

	void FUN_00407a68(VM *vm, byte memtype, int32 offset, int32 val, int32 x, int32 y);

	void vmCallDispatcher(VM *vm, uint32 funcID);


	void dumpActions();

	static void callbackVMCallDispatcher(void *engine, VM *vm, uint32 funcID);

public:

	inline void rndSeed(uint32 seed) {
		_seed = seed * 0x41c64e6d + 0x3039;
	}

	inline uint32 rnd() {
		uint32 val = _seed;
		_seed = _seed * 0x41c64e6d + 0x3039;
		return val;
	}

	inline uint16 rndRange16(uint32 range) {
		uint16 percent = _seed >> 16;
		_seed = _seed * 0x41c64e6d + 0x3039;
		return (percent * range) >> 16;
	}

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
