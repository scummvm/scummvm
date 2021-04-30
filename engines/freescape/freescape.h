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

typedef Common::HashMap<uint16, Area *> AreaMap;

typedef struct Binary {
	uint16 startArea; 
	AreaMap *areasByAreaID;
	Common::Array<uint8> *border;
	Common::Array<uint8> *palette;
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
	Graphics::PixelFormat _currentPixelFormat;
	Graphics::PixelFormat _originalPixelFormat;
	Graphics::PixelFormat _palettePixelFormat;
	int _screenW, _screenH;

	Graphics::PixelBuffer *_border;
	Graphics::PixelBuffer *_palette;

	Graphics::Surface *_borderSurf;

	
	uint32 _timeOfLastTick;
	bool _hasReceivedTime;

	uint16 _startArea;
	AreaMap *_areasByAreaID;
	float _rotation[3], _velocity[3], _position[3];

public:
	FreescapeEngine(OSystem *syst);
	~FreescapeEngine();

	Renderer *_gfx;
	Common::Error run() override;
	void convertBorder();
	void drawBorder();

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
