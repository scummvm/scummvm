#ifndef FREESCAPE_H
#define FREESCAPE_H

#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/pixelbuffer.h"
#include "gui/debugger.h"

#include "freescape/area.h"
#include "freescape/gfx.h"

namespace Freescape {

class Renderer;

// from shooter
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

typedef Common::HashMap<uint16, Area *> AreaMap;

typedef struct Binary {
	uint8 bits;
	uint16 startArea; 
	AreaMap *areasByAreaID;
	Common::Array<uint8> *border;
	Common::Array<uint8> *palette;
	uint16 ncolors;
} Binary;

class Console;

// our engine debug channels
enum {
	kFreescapeDebug = 1 << 0,
	kFreescapeDebug2 = 1 << 1
};

class FreescapeEngine : public Engine {
private:
	// We need random numbers
	Common::RandomSource *_rnd;
	int _screenW, _screenH;

	Graphics::Surface *_border;
	
	uint32 _timeOfLastTick;
	bool _hasReceivedTime;

public:
	FreescapeEngine(OSystem *syst);
	~FreescapeEngine();

	Renderer *_gfx;
	Common::Error run() override;
	void convertBorder();
	void drawBorder();


	// Parsing
	void loadAssets();
	void load16bitBinary(Common::SeekableReadStream *file);
	void load8bitBinary(Common::SeekableReadStream *file, int offset, int ncolors);

	uint8 _binaryBits;

	// Areas
	uint16 _startArea;
	AreaMap *_areasByAreaID;


	// Movement
	void move(CameraMovement direction, float deltaTime);
	void rotate(Common::Point lastMousePos, Common::Point mousePos);
	// Eular Angles
	float _yaw;
	float _pitch;
	// Camera options
	float _mouseSensitivity;
	float _movementSpeed;
	Vector3d _cameraFront, _cameraUp, _cameraRight;
	// Spacial attributes
	Vector3d _position, _rotation, _velocity;


	// Rendering
	uint8 _colorNumber;

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return true; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(FreescapeEngine *vm) {
	}
	virtual ~Console(void) {
	}
};

extern FreescapeEngine *g_freescape;

} // namespace Freescape

#endif
