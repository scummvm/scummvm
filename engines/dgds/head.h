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

#ifndef DGDS_HEAD_H
#define DGDS_HEAD_H

#include "common/str.h"
#include "common/types.h"
#include "common/ptr.h"
#include "common/array.h"

#include "graphics/managed_surface.h"

#include "dgds/dgds_rect.h"
#include "dgds/ttm.h"

// Classes related to talking heads and conversations.

namespace Dgds {

class Image;
class SoundRaw;


class TalkDataHeadFrame {
public:
	TalkDataHeadFrame() : _xoff(0), _yoff(0), _frameNo(0), _flipFlags(0) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _frameNo;
	int16 _xoff;
	int16 _yoff;
	uint16 _flipFlags;
};

enum HeadFlags {
	kHeadFlagNone = 0,
	kHeadFlag1 = 1,
	kHeadFlag2 = 2,
	kHeadFlag4 = 4,
	kHeadFlag8 = 8,
	kHeadFlag10 = 0x10,
	kHeadFlagVisible = 0x20,
	kHeadFlag40 = 0x40,
	kHeadFlag80 = 0x80,
};

class TalkData;

class TalkDataHead {
public:
	TalkDataHead() : _num(0), _drawType(0), _drawCol(0), _flags(kHeadFlagNone) {}
	Common::String dump(const Common::String &indent) const;

	void updateHead();

	void drawHead(Graphics::ManagedSurface *dst, const TalkData &data) const;
	void drawHeadType1(Graphics::ManagedSurface *dst, const Image &img) const;
	void drawHeadType2(Graphics::ManagedSurface *dst, const Image &img) const;
	void drawHeadType3(Graphics::ManagedSurface *dst, const Image &img) const;
	void drawHeadType3Beamish(Graphics::ManagedSurface *dst, const TalkData &data) const;

	uint16 _num;
	uint16 _drawType;
	uint16 _drawCol;
	DgdsRect _rect;
	Common::Array<TalkDataHeadFrame> _headFrames;
	Common::String _bmpFile;
	HeadFlags _flags;
	Common::SharedPtr<Image> _shape;
};

/** TDS talking head data from HOC+ */
class TalkData {
public:
	TalkData() : _num(0), _val(0) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _num;
	Common::SharedPtr<Image> _shape;
	Common::Array<TalkDataHead> _heads;
	uint16 _val;
	Common::String _bmpFile;

	void updateVisibleHeads();
	void drawVisibleHeads(Graphics::ManagedSurface *dst) const;
	bool hasVisibleHead() const;
};

/** CDS data from Willy Beamish talkie */
class Conversation {
public:
	Conversation() : _nextExec(0) {}

	void unload();
	void runScript();
	void loadData(uint16 num, uint16 num2, int16 sub);

	Common::SharedPtr<SoundRaw> _sound;
	Common::SharedPtr<Image> _img;
	Common::SharedPtr<TTMInterpreter> _ttmScript;
	Common::Array<Common::SharedPtr<TTMSeq>> _ttmSeqs;
	TTMEnviro _ttmEnv;
	uint32 _nextExec;
	DgdsRect _drawRect;
};


} // end namespace Dgds

#endif // DGDS_HEAD_H
