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

class RaycastDeferredLoader;
class RaycastLevelBuilder;

// Action record implementing nancy3's maze minigame
class RaycastPuzzle : public RenderActionRecord {
	friend class RaycastDeferredLoader;
	friend class RaycastLevelBuilder;
public:
	RaycastPuzzle() : RenderActionRecord(7), _map(7) {}
	~RaycastPuzzle() override;

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void onPause(bool pause) override;
	void handleInput(NancyInput &input) override;
	void updateGraphics() override;

protected:
	Common::String getRecordTypeName() const override { return "RaycastPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void validateMap();

	void createTextureLightSourcing(Common::Array<Graphics::ManagedSurface> *array, const Common::Path &textureName);

	void drawMap();
	void updateMap();
	void drawMaze();
	void clearZBuffer();

	void checkSwitch();
	void checkExit();

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
	Common::Array<uint16> _wallLightMapBackup, _floorCeilingLightMapBackup;

	uint16 _mapFullWidth = 0;
	uint16 _mapFullHeight = 0;

	RenderObject _map;
	Graphics::ManagedSurface _mapBaseSurface;

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

										// Improvement: we store position as float for smoother movement
	float _playerX = -1;				// Player position with precision 1/128th of cell width/height
	float _playerY = -1;
	int32 _playerRotation = 0; 			// Rotation of player (0 - 4096)
	uint32 _playerAltitude = 88;		// Z position of "camera"; only modified in god mode

	float _rotationSingleStep = 4096.0 / (_pi * 2);
	float _maxWorldDistance = 0.0;

	uint32 _lastMovementTime = 0;
	int _lastMouseX = -1;

	uint32 _nextSlowdownMovementTime = 0;
	byte _slowdownFramesLeft = 0;
	int32 _slowdownDeltaX = -1;
	int32 _slowdownDeltaY = -1;

	Common::Array<byte> _lightSwitchIDs;
	Common::Array<Common::Point> _lightSwitchPositions;
	Common::Array<bool> _lightSwitchStates;
	int _lightSwitchPlayerIsOn = -1;

	const RCPR *_puzzleData = nullptr;
	Common::SharedPtr<RaycastDeferredLoader> _loaderPtr;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_RAYCASTPUZZLE_H
