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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_SLICE_ANIMATIONS_H
#define BLADERUNNER_SLICE_ANIMATIONS_H

#include "common/array.h"
#include "common/file.h"
#include "common/str.h"
#include "common/types.h"

#include "bladerunner/color.h"
#include "bladerunner/vector.h"


namespace BladeRunner {

class BladeRunnerEngine;


class SliceAnimations {
	friend class SliceRenderer;

	struct Animation {
		uint32 frameCount;
		uint32 frameSize;
		float  fps;
		Vector3 positionChange;
		float facingChange;
		uint32 offset;
	};

	struct Palette {
		uint32 value[256];
		Color256 color[256];

	//	uint16 &operator[](size_t i) { return color555[i]; }
	};

	struct Page {
		void   *_data;
		uint32 _lastAccess;

		Page() : _data(nullptr), _lastAccess(0) {}
	};

	struct PageFile {
		int                  _fileNumber;
		SliceAnimations     *_sliceAnimations;
		Common::File         _files[5];
		Common::Array<int32> _pageOffsets;
		Common::Array<int8>  _pageOffsetsFileIdx;

		PageFile(SliceAnimations *sliceAnimations) : _sliceAnimations(sliceAnimations), _fileNumber(-1) {}

		bool  open(const Common::String &name, int8 fileIdx);
		void  close(int8 fileIdx);
		void *loadPage(uint32 page);
	};

	BladeRunnerEngine *_vm;

	uint32 _timestamp;
	uint32 _pageSize;
	uint32 _pageCount;
	uint32 _paletteCount;

	Common::Array<Palette>      _palettes;
	Common::Array<Animation>    _animations;
	Common::Array<Page>         _pages;

	PageFile _coreAnimPageFile;
	PageFile _framesPageFile;

public:
	SliceAnimations(BladeRunnerEngine *vm)
		: _vm(vm)
		, _coreAnimPageFile(this)
		, _framesPageFile(this)
		, _timestamp(0)
		, _pageSize(0)
		, _pageCount(0)
		, _paletteCount(0) {}
	~SliceAnimations();

	bool open(const Common::String &name);

	bool openCoreAnim();
	bool openFrames(int fileNumber);

	Palette &getPalette(int i) { return _palettes[i]; };
	void    *getFramePtr(uint32 animation, uint32 frame);

	int   getFrameCount(int animation) const { return _animations[animation].frameCount; }
	float getFPS(int animation) const { return _animations[animation].fps; }

	Vector3 getPositionChange(int animation) const;
	float   getFacingChange(int animation) const;
};

} // End of namespace BladeRunner

#endif
