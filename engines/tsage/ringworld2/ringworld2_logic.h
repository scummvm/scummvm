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

#ifndef TSAGE_RINGWORLD2_LOGIC_H
#define TSAGE_RINGWORLD2_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

#define R2_INVENTORY (*((::TsAGE::Ringworld2::Ringworld2InvObjectList *)g_globals->_inventory))

class SceneFactory {
public:
	static Scene *createScene(int sceneNumber);
};

class SceneArea: public SceneItem {
public:
	bool _enabled;
	bool _insideArea;
	CursorType _cursorNum;
	CursorType _savedCursorNum;
	int _cursorState;
public:
	SceneArea();
	void setDetails(const Rect &bounds, CursorType cursor);

	Common::String getClassName() override { return "SceneArea"; }
	void synchronize(Serializer &s) override;
	void remove() override;
	void process(Event &event) override;
	bool startAction(CursorType action, Event &event) override { return false; }
	void doAction(int action) override {}
};

class SceneExit: public SceneArea {
public:
	bool _moving;
	int _sceneNumber;
	Common::Point _destPos;
public:
	SceneExit();
	virtual void setDetails(const Rect &bounds, CursorType cursor, int sceneNumber);
	virtual void setDest(const Common::Point &p) { _destPos = p; }
	virtual void changeScene();

	void synchronize(Serializer &s) override;
	void process(Event &event) override;
};

class SceneExt: public Scene {
private:
	static void startStrip();
	static void endStrip();
public:
	byte _shadowPaletteMap[256];
	bool _savedPlayerEnabled;
	bool _savedUiEnabled;
	bool _savedCanWalk;
	bool _preventSaving;

	Visage _cursorVisage;
	SynchronizedList<EventHandler *> _sceneAreas;
public:
	SceneExt();

	Common::String getClassName() override { return "SceneExt"; }
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void process(Event &event) override;
	void dispatch() override;
	void refreshBackground(int xAmount, int yAmount) override;
	virtual void saveCharacter(int characterIndex);
	virtual void restore() {}

	bool display(CursorType action, Event &event);
	void fadeOut();
	void clearScreen();
	void scalePalette(int RFactor, int GFactor, int BFactor);
	void loadBlankScene();
};

class SceneHandlerExt: public SceneHandler {
public:
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void process(Event &event) override;
	void postLoad(int priorSceneBeforeLoad, int currentSceneBeforeLoad) override;
	void dispatch() override;

	void setupPaletteMaps();
};


class DisplayHotspot : public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayHotspot(int regionId, ...);

	void doAction(int action) override {
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class DisplayObject : public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayObject(int firstAction, ...);

	void doAction(int action) override {
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class SceneObjectExt : public SceneObject {
public:
	int _state;

	void synchronize(Serializer &s) override {
		SceneObject::synchronize(s);
		s.syncAsSint16LE(_state);
	}
	Common::String getClassName() override { return "SceneObjectExt"; }
};

/*--------------------------------------------------------------------------*/

class Ringworld2InvObjectList : public InvObjectList {
private:
	static bool SelectItem(int objectNumber);
	static void selectDefault(int obectNumber);
public:
	InvObject _none;
	InvObject _optoDisk;
	InvObject _reader;
	InvObject _negatorGun;
	InvObject _steppingDisks;
	InvObject _attractorUnit;
	InvObject _sensorProbe;
	InvObject _sonicStunner;
	InvObject _cableHarness;
	InvObject _comScanner;
	InvObject _spentPowerCapsule;	// 10
	InvObject _chargedPowerCapsule;
	InvObject _aerosol;
	InvObject _remoteControl;
	InvObject _opticalFiber;
	InvObject _clamp;
	InvObject _attractorHarness;
	InvObject _fuelCell;
	InvObject _gyroscope;
	InvObject _airbag;
	InvObject _rebreatherTank;		// 20
	InvObject _reserveTank;
	InvObject _guidanceModule;
	InvObject _thrusterValve;
	InvObject _balloonBackpack;
	InvObject _radarMechanism;
	InvObject _joystick;
	InvObject _ignitor;
	InvObject _diagnosticsDisplay;
	InvObject _glassDome;
	InvObject _wickLamp;			// 30
	InvObject _scrithKey;
	InvObject _tannerMask;
	InvObject _pureGrainAlcohol;
	InvObject _blueSapphire;
	InvObject _ancientScrolls;
	InvObject _flute;
	InvObject _gunpowder;
	InvObject _unused;
	InvObject _comScanner2;
	InvObject _superconductorWire;	// 40
	InvObject _pillow;
	InvObject _foodTray;
	InvObject _laserHacksaw;
	InvObject _photonStunner;
	InvObject _battery;
	InvObject _soakedFaceMask;
	InvObject _lightBulb;
	InvObject _alcoholLamp1;
	InvObject _alcoholLamp2;
	InvObject _alocholLamp3;		// 50
	InvObject _brokenDisplay;
	InvObject _toolbox;

	Ringworld2InvObjectList();
	void reset();
	void setObjectScene(int objectNum, int sceneNumber);

	Common::String getClassName() override { return "Ringworld2InvObjectList"; }
};

#define RING2_INVENTORY (*((::TsAGE::Ringworld2::Ringworld2InvObjectList *)g_globals->_inventory))

class Ringworld2Game: public Game {
public:
	void start() override;
	void restartGame() override;
	void restart() override;
	void endGame(int resNum, int lineNum) override;

	Scene *createScene(int sceneNumber) override;
	void processEvent(Event &event) override;
	void rightClick() override;
	bool canSaveGameStateCurrently() override;
	bool canLoadGameStateCurrently() override;
};

class NamedHotspot : public SceneHotspot {
public:
	NamedHotspot();

	bool startAction(CursorType action, Event &event) override;
	Common::String getClassName() override { return "NamedHotspot"; }
};

class NamedHotspotExt : public NamedHotspot {
public:
	int _flag;
	NamedHotspotExt() { _flag = 0; }

	Common::String getClassName() override { return "NamedHotspot"; }
	void synchronize(Serializer &s) override {
		NamedHotspot::synchronize(s);
		s.syncAsSint16LE(_flag);
	}
};

class SceneActor: public SceneObject {
public:
	Common::String getClassName() override { return "SceneActor"; }
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	bool startAction(CursorType action, Event &event) override;
	GfxSurface getFrame() override;
};

class SceneActorExt: public SceneActor {
public:
	int _state;

	SceneActorExt() { _state = 0; }

	Common::String getClassName() override { return "SceneActorExt"; }
	void synchronize(Serializer &s) override {
		SceneActor::synchronize(s);
		s.syncAsSint16LE(_state);
	}
};

enum MazeDirection { MAZEDIR_NONE = 0, MAZEDIR_NORTH = 1, MAZEDIR_NORTHEAST = 2, MAZEDIR_EAST = 3,
	MAZEDIR_SOUTHEAST = 4, MAZEDIR_SOUTH = 5, MAZEDIR_SOUTHWEST = 6, MAZEDIR_WEST = 7,
	MAZEDIR_NORTHWEST = 8 };

class MazeUI: public SceneObject {
private:
	void clear();
public:
	// The dimensions (in cells) of the entire maze map
	Rect _mapBounds;

	// Encoded cell map specifying the features of the maze
	byte *_mapData;
	// Image surface used to store a line of the map for rendering
	GfxSurface _mapImage;

	Common::Point _cellsVisible;
	Common::Point _mapCells;
	Common::Point _cellSize;
	Common::Point _mapOffset;
	int _resNum;
	int _cellsResNum;
	int _frameCount;
	int _resCount;
	int _mapImagePitch;
public:
	MazeUI();
	~MazeUI() override;

	void setDisplayBounds(const Rect &r);
	bool setMazePosition(const Common::Point &pt);
	void load(int resNum);
	int getCellFromPixelXY(const Common::Point &pt);
	int getCellFromCellXY(const Common::Point &p);
	int pixelToCellXY(Common::Point &pt);

	Common::String getClassName() override { return "MazeUI"; }
	void synchronize(Serializer &s) override;
	void reposition() override;
	void draw() override;
};

class SceneAreaObject: public SceneArea {
	class Object1: public SceneActor {
	public:
	};
public:
	Object1 _object1;
	int _insetCount;

	void remove() override;
	void process(Event &event) override;
	void setDetails(int visage, int strip, int frameNumber, const Common::Point &pt);
	void setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
};

/*--------------------------------------------------------------------------*/

class AnimationSlice {
public:
	int _sliceOffset;
	int _drawMode;
	int _secondaryIndex;
public:
	void load(Common::File &f);
};

class AnimationSlices {
public:
	int _dataSize;
	int _dataSize2;
	AnimationSlice _slices[4];
	byte *_pixelData;
public:
	AnimationSlices();
	~AnimationSlices();

	void load(Common::File &f);
	int loadPixels(Common::File &f, int slicesSize);
};

class AnimationPlayerSubData {
public:
	int _duration;
	int _frameRate;
	int _framesPerSlices;
	int _drawType;
	int _sliceSize;
	int _ySlices;
	int _field16;
	int _palStart;
	int _palSize;
	byte _palData[256 * 3];
	int32 _totalSize;
	AnimationSlices _slices;
public:
	void load(Common::File &f);
};

class AnimationData {
public:
	AnimationSlices _slices;
	int _dataSize;
	int _animSlicesSize;
};

enum AnimationPaletteMode { ANIMPALMODE_REPLACE_PALETTE = 0, ANIMPALMODE_CURR_PALETTE = 1,
		ANIMPALMODE_NONE = 2 };

enum AnimationObjectMode { ANIMOBJMODE_1 = 1, ANIMOBJMODE_2 = 2, ANIMOBJMODE_42 = 42 };

class AnimationPlayer: public EventHandler {
private:
	void rleDecode(const byte *pSrc, byte *pDest, int size);

	void drawFrame(int sliceIndex);
	void nextSlices();
	void getSlices();
public:
	AnimationData *_animData1, *_animData2;
	AnimationData *_sliceCurrent;
	AnimationData *_sliceNext;
	Common::File _resourceFile;
	Rect _rect1, _screenBounds;
	bool _animLoaded;
	bool _canSkip;
	AnimationPaletteMode _paletteMode;
	AnimationObjectMode _objectMode;
	int _sliceHeight;
	byte _palIndexes[256];
	ScenePalette _palette;
	AnimationPlayerSubData _subData;
	Action *_endAction;
	int _dataNeeded;
	int _playbackTick;
	int _playbackTickPrior;
	int _position;
	int _nextSlicesPosition;
	uint _frameDelay;
	uint32 _gameFrame;
public:
	AnimationPlayer();
	~AnimationPlayer() override;

	void synchronize(Serializer &s) override;
	void remove() override;
	void process(Event &event) override;
	void dispatch() override;
	virtual void flipPane() {}
	virtual void changePane() {}
	virtual void closing() {}


	bool load(int animId, Action *endAction = NULL);
	bool isCompleted();
	void close();
};

class AnimationPlayerExt: public AnimationPlayer {
public:
	bool _isActive;
public:
	AnimationPlayerExt();

	void synchronize(Serializer &s) override;
};

class ModalWindow: public SceneArea {
public:
	SceneActor _object1;
	int _insetCount;
public:
	ModalWindow();

	void remove() override;
	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "ModalWindow"; }
	void process(Event &event) override;
	virtual void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY);
	virtual void setup3(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
};

class ScannerDialog: public ModalWindow {

	class Button: public SceneActor {
	private:
		void reset();
	public:
		int _buttonId;
		bool _buttonDown;
	public:
		Button();
		void setup(int buttonId);

		void synchronize(Serializer &s) override;
		Common::String getClassName() override { return "ScannerButton"; }
		void process(Event &event) override;
		bool startAction(CursorType action, Event &event) override;
	};
	class Slider: public SceneActor {
	private:
		void update();
	public:
		int _initial;
		int _xStart;
		int _yp;
		int _width;
		int _xInc;
		bool _sliderDown;
	public:
		Slider();
		void setup(int initial, int xStart, int yp, int width, int xInc);

		void synchronize(Serializer &s) override;
		Common::String getClassName() override { return "ScannerSlider"; }
		void remove() override;
		void process(Event &event) override;
		bool startAction(CursorType action, Event &event) override;
	};
public:
	Button _talkButton;
	Button _scanButton;
	Slider _slider;
	SceneActor _obj4;
	SceneActor _obj5;
	SceneActor _obj6;
	SceneActor _obj7;
public:
	ScannerDialog();

	Common::String getClassName() override { return "ScannerDialog"; }
	void remove() override;
	void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY) override;
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
