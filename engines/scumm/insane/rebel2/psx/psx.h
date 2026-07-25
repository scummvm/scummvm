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
};

bool loadRA2PSXTextures(const Common::Array<byte> &data,
		Common::Array<RA2PSXTexture> &textures);

// The fireball frames are 68x56, the size the original's VRAM upload uses.
enum { kRA2PSXExplosionHeight = 56 };

// bigEx: a 4 byte format word, a 256 colour palette and equal sized 8 bit frames.
bool loadRA2PSXSpriteAnimation(const Common::Array<byte> &data, uint16 frameHeight,
		Common::Array<RA2PSXTexture> &frames);

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
