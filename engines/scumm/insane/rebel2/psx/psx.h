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
 */

#if !defined(SCUMM_INSANE_REBEL2_PSX_H) && defined(ENABLE_SCUMM_7_8) && defined(ENABLE_REBEL2_PSX)
#define SCUMM_INSANE_REBEL2_PSX_H

#include "common/array.h"
#include "common/error.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/str.h"
#include "common/types.h"

#ifdef USE_TINYGL
#include "graphics/surface.h"
#include "graphics/tinygl/tinygl.h"
#endif

namespace Common {
class SeekableReadStream;
}

namespace Audio {
class RewindableAudioStream;
}

namespace Scumm {

class ScummEngine_v7;
class RA2PSXLevel1UI;
class RA2PSXLevel2Scene;
class RA2PSXMainMenuUI;
class RA2PSXMovieText;
class RA2PSXOptionsUI;
class RA2PSXChapterSelectUI;
class RA2PSXArchive;
class RA2PSXModel;

bool loadRA2PSXDebrisModels(const RA2PSXArchive &archive,
		const Common::Array<byte> &textureData, Common::Array<RA2PSXModel> &models);

enum RA2PSXMovieTextSequence {
	kRA2PSXMovieTextNone,
	kRA2PSXMovieTextOpening,
	kRA2PSXMovieTextChapter1
};

class RA2PSXArchive {
public:
	bool load(Common::SeekableReadStream &stream);
	bool getMember(const Common::String &path, Common::Array<byte> &data) const;

private:
	struct Entry {
		Common::String name;
		uint32 unpackedSize;
		uint32 offset;
		uint32 endOffset;
	};

	bool findTopLevelEntry(const Common::String &name, Entry &entry) const;
	bool unpack(const Entry &entry, Common::Array<byte> &data) const;
	bool findNestedMember(const Common::Array<byte> &container, const Common::String &path,
			Common::Array<byte> &data) const;
	static bool decompress(const byte *source, uint32 sourceSize, uint32 expectedSize,
			Common::Array<byte> &data);

	Common::Array<byte> _data;
};

class RA2PSXSoundBank {
public:
	bool load(const Common::Array<byte> &sampleData, const Common::Array<byte> &projectData);
	Audio::RewindableAudioStream *makeStream(uint16 id, uint32 macroRate,
			uint16 adsrId = 0xffff) const;
	bool getSFX(uint16 id, uint16 &macro, byte &priority, byte &maxVoices) const;
	bool getMacroCommand(uint16 macro, uint16 step, byte *command) const;

private:
	struct Sample {
		uint32 offset;
		uint16 id;
		uint16 blocks;
		uint16 rate;
	};
	struct SFX {
		uint16 id;
		uint16 macro;
		byte priority;
		byte maxVoices;
	};
	struct Macro {
		uint32 id;
		uint32 offset;
	};
	struct ADSR {
		uint16 id;
		uint16 attack;
		uint16 decay;
		uint16 sustain;
		uint16 release;
	};

	const Sample *findSample(uint16 id) const;
	const ADSR *findADSR(uint16 id) const;

	Common::Array<byte> _data;
	Common::Array<byte> _projectData;
	Common::Array<Sample> _samples;
	Common::Array<SFX> _sfx;
	Common::Array<Macro> _macros;
	Common::Array<ADSR> _adsrs;
};

class RA2PSXSoundPlayer {
public:
	typedef uint32 SoundId;
	enum { kInvalidSoundId = 0 };

	RA2PSXSoundPlayer(ScummEngine_v7 *vm, const RA2PSXSoundBank &bank);
	~RA2PSXSoundPlayer();

	// Pitch is a 14 bit bend; kNeutralBend leaves the sound at its own rate.
	enum { kNeutralBend = 0x2000 };

	SoundId play(uint16 sfx, int volume, int pan, int pitch = kNeutralBend);
	void update();
	void setPan(SoundId sound, int pan);
	void setVolume(SoundId sound, int volume);
	void setPitch(SoundId sound, int pitch);
	void stop(SoundId sound);
	void stopAll();

private:
	struct Impl;
	Impl *_impl;
};

// Options menu settings; sfx steps by 7 up to 0x70, the CD volumes by 0x200 up to 0x1000.
struct RA2PSXSettings {
	enum {
		kSFXMaximum = 0x70,
		kSFXStep = 7,
		kCDMaximum = 0x1000,
		kCDStep = 0x200
	};

	RA2PSXSettings() { reset(); }

	void reset();
	void load();
	void save() const;
	void apply(ScummEngine_v7 *vm) const;
	byte videoVolume() const;
	// The original keeps a per-difficulty count of the chapters reached.
	int unlockedChapters() const;

	int difficulty;
	int sfx;
	int music;
	int movies;
	bool mono;
	int unlocked[3];
	bool unlockAll;
};

struct RA2PSXVertex {
	int16 x;
	int16 y;
	int16 z;
};

// PlayStation matrix helpers: 4096 angle units per turn, 1/4096 fixed point scales,
// and pre-rotating multiplies from the left.
struct RA2PSXMatrix {
	RA2PSXMatrix() { setIdentity(); }

	void setIdentity();
	void setScale(int x, int y, int z);
	void setRotationX(int angle);
	void setRotationY(int angle);
	void setRotationZ(int angle);
	void preRotateX(int angle);
	void preRotateY(int angle);
	void preRotateZ(int angle);
	void setTranslation(int x, int y, int z);

	float rotation[3][3];
	float translation[3];
};

struct RA2PSXTexture {
	Common::String name;
	uint16 width;
	uint16 height;
	Common::Array<uint32> pixels;
	// The resolved CLUT, kept so sprites streamed into this texture's VRAM slot can
	// borrow its colours the way the original's fixed page assignment does.
	Common::Array<uint32> palette;
};

uint32 decodeRA2PSXColor(uint16 value);
bool loadRA2PSXTextures(const Common::Array<byte> &data,
		Common::Array<RA2PSXTexture> &textures);

// One frame of a playN script: where it lands on screen, its 8 bit pixels, and the flags
// that say whether it can be shot, whether it hurts the player, and when it makes noise.
struct RA2PSXPlayFrame {
	RA2PSXPlayFrame() : x(0), y(0), width(0), height(0), boxLeft(0), boxTop(0),
			boxRight(0), boxBottom(0), flags(0) {}

	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 boxLeft;
	int16 boxTop;
	int16 boxRight;
	int16 boxBottom;
	uint16 flags;
	Common::Array<byte> pixels;
};

enum {
	kRA2PSXPlayLastFrame = 0x0001,
	kRA2PSXPlayCompressed = 0x0002,
	kRA2PSXPlayTargetable = 0x0004,
	kRA2PSXPlayHitsPlayer = 0x0008,
	kRA2PSXPlaySound = 0x0010
};

typedef Common::Array<RA2PSXPlayFrame> RA2PSXPlayAnimation;

// playN: a table of animation offsets, each opening a chain of frame records.
bool loadRA2PSXPlayScript(const Common::Array<byte> &data,
		Common::Array<RA2PSXPlayAnimation> &animations);

// The fireball frames are 68x56, the size the original's VRAM upload uses.
enum { kRA2PSXExplosionHeight = 56 };

// bigEx: a 4 byte format word, a 256 colour palette and equal sized 8 bit frames.
bool loadRA2PSXSpriteAnimation(const Common::Array<byte> &data, uint16 frameHeight,
		Common::Array<RA2PSXTexture> &frames, const Common::Array<uint32> *clut = nullptr);

// Level 2's three parts, each a sheet of backdrop halves, parallax walls and actors.
enum {
	kRA2PSXLevel2PartCount = 3,
	kRA2PSXLevel2LayerCount = 5,
	kRA2PSXLevel2SceneTop = 28,
	// Frames 0 to 4 are the rookie leaning out of cover; 5 to 29 are a five by five
	// grid of aim poses picked from where the crosshair sits.
	kRA2PSXLevel2CoverFrames = 5,
	// Part three's rookie owns three more poses, and shifts its aim grid past them.
	kRA2PSXLevel2FrameCount = 33,
	kRA2PSXLevel2RookieDelay = 3,
	kRA2PSXLevel2AimColumns = 5,
	kRA2PSXLevel2AimRows = 5,
	// The slide runs at a constant speed of distance/20 for 14 ticks, then decays by
	// 0xe000/0x10000 a tick until it lands - ra2 eases out rather than lerping.
	kRA2PSXLevel2ScrollSteps = 20,
	kRA2PSXLevel2ScrollHold = 14,
	kRA2PSXLevel2ScrollDamping = 0xe000
};

enum {
	kRA2PSXLevel2TrooperCount = 3,
	// The gameplay clock is the 60Hz vblank, and a play script steps every fourth tick.
	kRA2PSXLevel2TickRate = 60,
	kRA2PSXLevel2FrameTicks = 4,
	// A part ends a second after its last trooper falls.
	kRA2PSXLevel2ClearTicks = 60,
	// A trooper hides behind its own sprite until its slot has counted down.
	kRA2PSXLevel2SlotNever = 0x7fffffff,
	// How often a held fire button repeats, in ticks.
	kRA2PSXLevel2FireRepeat = 12,
	// A player bolt runs a 3D line from the gun to the crosshair: the parameter steps
	// by 300 of 4096 a tick, the bolt stops drawing at 4000 and is dropped past 0x112f.
	// Two slots, the same as the DOS build's cover handler.
	kRA2PSXLevel2ShotCount = 2,
	kRA2PSXLevel2ShotStep = 300,
	kRA2PSXLevel2ShotDraw = 4000,
	kRA2PSXLevel2ShotEnd = 0x112f,
	// The bolt leaves the muzzle at z 1000 and reaches the crosshair at z 18000.
	kRA2PSXLevel2ShotNearZ = 1000,
	kRA2PSXLevel2ShotFarZ = 18000
};

// The states a play script actor walks, named after the values the original stores.
enum {
	kRA2PSXLevel2StateIdle = 0,
	kRA2PSXLevel2StateAppear = 1,
	kRA2PSXLevel2StateAimed = 2,
	kRA2PSXLevel2StateWild = 3,
	kRA2PSXLevel2StateDie = 4,
	kRA2PSXLevel2StateShot = 5,
	kRA2PSXLevel2StateCover = 0x80
};

// One of the player's own bolts, travelling from the gun toward the crosshair.
struct RA2PSXLevel2Shot {
	RA2PSXLevel2Shot() : step(0), muzzleX(0), muzzleY(0), targetX(0), targetY(0) {}

	// Zero when the slot is free; otherwise how far along the line the bolt is.
	int step;
	// Both ends in screen space: the gun this pose fires from, and the crosshair.
	int muzzleX;
	int muzzleY;
	int targetX;
	int targetY;
};

// One play script actor: a trooper, or the bolt it fires.
struct RA2PSXLevel2Actor {
	RA2PSXLevel2Actor() : state(kRA2PSXLevel2StateIdle), animation(-1), frame(0), hold(0),
			hit(false), slotTick(0), fireNext(0), fireEnd(0) {}

	int state;
	int animation;
	int frame;
	int hold;
	bool hit;
	int slotTick;
	int fireNext;
	int fireEnd;
};

// The frame window a trooper shoots inside, per part, per trooper, per animation.
struct RA2PSXLevel2FireWindow {
	int16 start;
	int16 end;
};

// Where each rookie frame is drawn, straight out of the original's per level table.
struct RA2PSXLevel2Pose {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
};

// One backdrop sprite, as the original's display list holds it: a fixed screen position
// the scroll is subtracted from, and the size the SPRT draws rather than the sheet's.
struct RA2PSXLevel2Layer {
	const char *name;
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	// 0 rides the backdrop scroll, 1 the nearer parallax one.
	byte plane;
};

struct RA2PSXLevel2PartInfo {
	const char *sheet;
	const char *anims;
	// Where each plane sits in cover and out in the open, as 16.16 pixels.
	int16 coverScroll[2][2];
	int16 openScroll[2][2];
	// Which axis eases out; the other one holds its speed all the way.
	byte scrollAxis;
	RA2PSXLevel2Layer layers[kRA2PSXLevel2LayerCount];
	int rookieOffsetX;
	int rookieOffsetY;
	// Crosshair box, then the thresholds that split it into the aim grid.
	int16 aimLeft, aimTop, aimRight, aimBottom;
	int16 aimColumns[kRA2PSXLevel2AimColumns];
	int16 aimRows[kRA2PSXLevel2AimRows];
	// Where the aim grid starts; the cover run and the pose count shift with it.
	int16 aimBase;
	RA2PSXLevel2Pose poses[kRA2PSXLevel2FrameCount];
	// How many trooper slots the part scripts, and the sheet sprites whose palettes the
	// streamed trooper and bolt frames borrow.
	int trooperCount;
	const char *trooperPalettes[kRA2PSXLevel2TrooperCount];
	const char *boltPalettes[kRA2PSXLevel2TrooperCount];
	// Empty windows mean the part fires as it changes state instead of on a frame count.
	RA2PSXLevel2FireWindow fireWindows[kRA2PSXLevel2TrooperCount][3];
};

// Waves per part by difficulty, and the timings and odds that drive a trooper's slot.
extern const int16 kRA2PSXLevel2WaveTable[kRA2PSXLevel2PartCount][3][2];
// Per slot: how long until it reappears, the odds it aims rather than sprays, and how
// long a killed trooper's spot stays empty.
extern const int16 kRA2PSXLevel2SlotTable[kRA2PSXLevel2PartCount][3][5];
// How many animation frames pass between two shots. Part 3 fires on state changes only.
extern const int16 kRA2PSXLevel2FireTable[2][3][2];
// The odds an enemy bolt connects, and what it takes off the shield when it does.
extern const int16 kRA2PSXLevel2BoltTable[kRA2PSXLevel2PartCount][3][2];
extern const int16 kRA2PSXLevel2KillScore[3];

extern const RA2PSXLevel2PartInfo kRA2PSXLevel2Parts[kRA2PSXLevel2PartCount];

// How far the level 1 background is panned and tilted inside its oversized frame, and the
// matching shift the 3D scene rides along with.
struct RA2PSXBackgroundView {
	RA2PSXBackgroundView() : panX(0), panY(0), tiltLeft(0), tiltRight(0),
			sceneX(0), sceneY(0) {}

	int panX;
	int panY;
	int tiltLeft;
	int tiltRight;
	int sceneX;
	int sceneY;
};

struct RA2PSXFace {
	uint16 vertex[4];
	int16 normalX[4];
	int16 normalY[4];
	int16 normalZ[4];
	byte u[4];
	byte v[4];
	byte vertexCount;
	byte mode;
	byte r[4];
	byte g[4];
	byte b[4];
	int16 texture;
};

class RA2PSXModel {
public:
	RA2PSXModel();

	bool load(const Common::Array<byte> &data);
	bool loadTextures(const Common::Array<byte> &data);
	const Common::Array<RA2PSXVertex> &vertices() const { return _vertices; }
	const Common::Array<RA2PSXFace> &faces() const { return _faces; }
	const RA2PSXTexture *texture(int index) const;
	float radius() const { return _radius; }

private:
	bool parseModel(const Common::Array<byte> &data, uint32 modelOffset, int depth);
	bool parseObject(const Common::Array<byte> &data, uint32 objectOffset);

	Common::Array<RA2PSXVertex> _vertices;
	Common::Array<RA2PSXFace> _faces;
	Common::Array<RA2PSXTexture> _textures;
	float _radius;
};

#ifdef USE_TINYGL
class RA2PSXTinyGLRenderer {
public:
	RA2PSXTinyGLRenderer();
	~RA2PSXTinyGLRenderer();

	bool init(int width, int height);
	void beginFrame(const Graphics::Surface &background);
	void beginFrame(const Graphics::Surface &background, const RA2PSXBackgroundView &view);
	// Shifts every world space draw, the way the original moves the GTE screen offset.
	void setViewOffset(int x, int y) { _viewOffsetX = x; _viewOffsetY = y; }
	// A camera facing quad, sized in world units like the original's explosion billboard.
	void renderSprite(const RA2PSXTexture &texture, float x, float y, float z,
			float halfWidth, float halfHeight, int rotation, int brightness = 0x80);
	void renderModel(const RA2PSXModel &model, float x, float y, float size,
			float pitch, float yaw, float roll, bool depthTest = true);
	void renderPerspectiveModel(const RA2PSXModel &model, float x, float y, float z,
			float directionX, float directionY, float directionZ, float roll,
			bool depthTest = true, int scale = 0x1000);
	void renderTransformedModel(const RA2PSXModel &model, const RA2PSXMatrix &transform,
			bool depthTest = true);
	void finishFrame(Graphics::Surface &surface);

private:
	struct TextureBinding {
		const RA2PSXTexture *texture;
		TGLuint id;
	};

	void setFaceState(const RA2PSXModel &model, const RA2PSXFace &face);
	void setFaceColor(const RA2PSXFace &face, uint vertexIndex,
			float normalX, float normalY, float normalZ, float depth);
	TGLuint getTextureId(const RA2PSXTexture &texture);

	TinyGL::ContextHandle *_context;
	Common::Array<TextureBinding> _textureBindings;
	const RA2PSXTexture *_activeTexture;
	bool _textureEnabled;
	bool _blendEnabled;
	int _width;
	int _height;
	int _viewOffsetX;
	int _viewOffsetY;
};
#endif

class Rebel2PSX {
public:
	explicit Rebel2PSX(ScummEngine_v7 *vm);
	~Rebel2PSX() = default;

	Common::Error runGame();

private:
	class Level1Handler;
	class Level2Handler;

	enum MenuResult {
		kMenuStart,
		kMenuQuit
	};

	enum Level1Result {
		kLevel1Quit,
		kLevel1Complete,
		kLevel1Death,
		kLevel1Error
	};

	Common::SeekableReadStream *openResource(int number);
	Common::SeekableReadStream *openRawFile(const Common::Path &path, int discNumber);
	bool playVideo(const Common::Path &path, int discNumber, bool version2,
			const RA2PSXMovieText *movieText = nullptr,
			RA2PSXMovieTextSequence textSequence = kRA2PSXMovieTextNone);
	bool playIntroSequence(const RA2PSXMovieText &movieText);
	MenuResult runMainMenu(const RA2PSXMainMenuUI &ui, const RA2PSXOptionsUI &options);
#ifdef USE_TINYGL
	// TinyGL tracks one context, so the options screen borrows the title screen's renderer.
	void runOptionsMenu(const RA2PSXOptionsUI &ui, RA2PSXSoundPlayer &sound,
			RA2PSXTinyGLRenderer &renderer);
#endif
	// Returns the chosen chapter, 1 to 16, or 0 when the player backs out.
	int runChapterSelect(const RA2PSXChapterSelectUI &ui);
	bool loadGlobalAssets(RA2PSXMainMenuUI &menu);
	bool loadMovieTextAssets(RA2PSXMovieText &movieText);
	bool loadLevel1Assets(RA2PSXModel &enemy, RA2PSXModel &ship,
			RA2PSXModel &crosshair, RA2PSXModel &laser, RA2PSXModel &tieLaser,
			Common::Array<RA2PSXModel> &debris, RA2PSXLevel1UI &ui);
	Common::Error runLevel2();
	Level1Result playLevel1(const RA2PSXModel &enemy, const RA2PSXModel &ship,
			const RA2PSXModel &crosshair, const RA2PSXModel &laser,
			const RA2PSXModel &tieLaser, const Common::Array<RA2PSXModel> &debris,
			const RA2PSXLevel1UI &ui, int &lives, int &score);

	ScummEngine_v7 *_vm;
	RA2PSXSoundBank _soundBank;
	RA2PSXSettings _settings;
	// The crest on the title screen, the freighter behind the options list and
	// the double sided crest on the chapter select.
	RA2PSXModel _logoModel;
	RA2PSXModel _cloakModel;
	RA2PSXModel _crestModel;
	// The 42 frame fireball the original streams from the bigEx resource, and the
	// single SMALLEX cell it trails smoke with.
	Common::Array<RA2PSXTexture> _explosionFrames;
	RA2PSXTexture _smokeTexture;
};

} // End of namespace Scumm

#endif
