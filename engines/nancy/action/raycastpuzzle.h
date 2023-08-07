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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_RAYCASTPUZZLE_H
#define NANCY_ACTION_RAYCASTPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

struct RCPR;

namespace Action {

// Action record implementing nancy3's maze minigame
class RaycastPuzzle : public RenderActionRecord {
public:
	RaycastPuzzle()  : RenderActionRecord(7), _map(7) {}
	~RaycastPuzzle() override {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;
	void updateGraphics() override;

	uint16 _mapWidth = 0;
	uint16 _mapHeight = 0;
	byte _wallHeight = 0;

	Common::String _switchSoundName;
	uint16 _switchSoundChannelID = 0;
	Common::String _unknownSoundName;
	uint16 _unknownSoundChannelID = 0;
	SoundDescription _dummySound;

	SceneChangeWithFlag _solveScene;
	SoundDescription _solveSound;

	Common::Array<uint32> _wallMap, _infoMap;
	Common::Array<int16> _floorMap, _ceilingMap;
	Common::Array<uint16> _wallLightMap, _floorCeilingLightMap, _heightMap;

	uint16 _mapFullWidth = 0;
	uint16 _mapFullHeight = 0;

	RenderObject _map;

	double _pi = 3.141592653589793;
	uint _fov = 192;
	Common::Array<float> _sinTable, _cosTable;

	Common::HashMap<uint16, Common::Array<Graphics::ManagedSurface>> _wallTextures;
	Common::HashMap<uint16, Common::Array<Graphics::ManagedSurface>> _specialWallTextures;
	Common::HashMap<uint16, Common::Array<Graphics::ManagedSurface>> _ceilingTextures;
	Common::HashMap<uint16, Common::Array<Graphics::ManagedSurface>> _floorTextures;
	Common::HashMap<uint16, Common::Array<Graphics::ManagedSurface>> _exitFloorTextures;

	Common::Array<int32> _wallCastColumnAngles;
	Common::Array<byte> _zBuffer;
	byte _lastZDepth = 0;
	Common::Array<float> _depthBuffer;
	int32 _leftmostAngle = -1;
	int32 _rightmostAngle = -1;

	int32 _playerX = -1;				// Player position with precision 1/128th of cell width/height
	int32 _playerY = -1;
	int32 _playerRotation; 				// Rotation of player (0 - 4096)
	uint32 _playerAltitude = 88;		// Z position of "camera"; only modified in god mode

	float _rotationSingleStep = 4096.0 / (_pi * 2);
	float _maxWorldDistance;

	uint32 _lastMovementTime;
	int _lastMouseX = -1;

	uint32 _nextSlowdownMovementTime = 0;
	byte _slowdownFramesLeft = 0;
	int32 _slowdownDeltaX = -1;
	int32 _slowdownDeltaY = -1;

	RCPR *_puzzleData = nullptr;

protected:
	Common::String getRecordTypeName() const override { return "RaycastPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void loadTextures();
	void createTextureLightSourcing(Common::Array<Graphics::ManagedSurface> *array, Common::String &textureName);

	void drawMap();
	void drawMaze();
	void clearZBuffer();
	
	void checkSwitch();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RAYCASTPUZZLE_H
