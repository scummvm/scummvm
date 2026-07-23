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

namespace Scumm {

class ScummEngine_v7;
class RA2PSXLevel1UI;

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

struct RA2PSXVertex {
	int16 x;
	int16 y;
	int16 z;
};

struct RA2PSXFace {
	uint16 vertex[4];
	byte vertexCount;
	byte r;
	byte g;
	byte b;
};

class RA2PSXModel {
public:
	RA2PSXModel();

	bool load(const Common::Array<byte> &data);
	const Common::Array<RA2PSXVertex> &vertices() const { return _vertices; }
	const Common::Array<RA2PSXFace> &faces() const { return _faces; }
	float radius() const { return _radius; }

private:
	bool parseModel(const Common::Array<byte> &data, uint32 modelOffset, int depth);
	bool parseObject(const Common::Array<byte> &data, uint32 objectOffset);

	Common::Array<RA2PSXVertex> _vertices;
	Common::Array<RA2PSXFace> _faces;
	float _radius;
};

#ifdef USE_TINYGL
class RA2PSXTinyGLRenderer {
public:
	RA2PSXTinyGLRenderer();
	~RA2PSXTinyGLRenderer();

	bool init(int width, int height);
	void beginFrame(const Graphics::Surface &background);
	void renderModel(const RA2PSXModel &model, float x, float y, float size,
			float pitch, float yaw, float roll, bool depthTest = true);
	void renderPerspectiveModel(const RA2PSXModel &model, float x, float y, float z,
			float directionX, float directionY, float directionZ, float roll,
			bool depthTest = true);
	void finishFrame(Graphics::Surface &surface);

private:
	TinyGL::ContextHandle *_context;
	int _width;
	int _height;
};
#endif

class Rebel2PSX {
public:
	explicit Rebel2PSX(ScummEngine_v7 *vm);
	~Rebel2PSX() = default;

	Common::Error runGame();

private:
	enum Level1Result {
		kLevel1Quit,
		kLevel1Complete,
		kLevel1Death,
		kLevel1Error
	};

	Common::SeekableReadStream *openResource(int number);
	Common::SeekableReadStream *openRawFile(const Common::Path &path, int discNumber);
	bool playVideo(const Common::Path &path, int discNumber, bool version2);
	bool loadLevel1Assets(RA2PSXModel &enemy, RA2PSXModel &ship,
			RA2PSXModel &crosshair, RA2PSXModel &laser, RA2PSXLevel1UI &ui);
	Level1Result playLevel1(const RA2PSXModel &enemy, const RA2PSXModel &ship,
			const RA2PSXModel &crosshair, const RA2PSXModel &laser,
			const RA2PSXLevel1UI &ui, int lives, int &score);

	ScummEngine_v7 *_vm;
};

} // End of namespace Scumm

#endif
