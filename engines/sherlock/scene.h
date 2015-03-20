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

#ifndef SHERLOCK_SCENE_H
#define SHERLOCK_SCENE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "sherlock/objects.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define SCENES_COUNT 63

class SherlockEngine;

struct BgFileHeader {
	int _numStructs;
	int _numImages;
	int _numcAnimations;
	int _descSize;
	int _seqSize;
	int _fill;

	void synchronize(Common::SeekableReadStream &s);
};

struct BgfileheaderInfo {
	int _fSize;					// How long images are
	int _maxFrames;				// How many unique frames in object
	Common::String _filename;	// Filename of object

	void synchronize(Common::SeekableReadStream &s);
};

class Scene {
private:
	SherlockEngine *_vm;

	void loadScene();

	void loadScene(const Common::String &filename);
public:
	bool _stats[SCENES_COUNT][9];
	bool _savedStats[SCENES_COUNT][9];
	int _goToRoom;
	Common::Point _bigPos;
	Common::Point _overPos;
	int _oldCharPoint;
	ImageFile *_controls;
	int _numExits;
	bool _windowOpen, _infoFlag;
	int _menuMode, _keyboardInput;
	int _oldKey, _help, _oldHelp;
	int _oldTemp, _temp;
	bool _walkedInScene;
	int _ongoingCans;
	int _version;
	bool _lzwMode;
	int _invGraphicItems;
	Common::String _comments;
	Common::Array<char> _descText;
	Common::Array<Common::Rect> _roomBounds;
	Common::Array<Object> _bgShapes;
	Common::Array<CAnim> _cAnim;
	Common::Array<byte> _sequenceBuffer;
public:
	Scene(SherlockEngine *vm);
	~Scene();

	void selectScene();
};

} // End of namespace Sherlock

#endif
