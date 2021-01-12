#ifndef PRIVATE_H
#define PRIVATE_H

#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "gui/debugger.h"

#include "common/installer_archive.h"

#include "audio/mixer.h"
#include "video/smk_decoder.h"
#include "graphics/palette.h"
#include "graphics/managed_surface.h"

#include "private/grammar.h"

namespace Image {
class ImageDecoder;
}

namespace Graphics {
struct ManagedSurface;
}

namespace Private {

class Console;

// our engine debug channels
enum {
	kPrivateDebugExample = 1 << 0,
	kPrivateDebugExample2 = 1 << 1
	// next new channel must be 1 << 2 (4)
	// the current limitation is 32 debug channels (1 << 31 is the last one)
};

// exits

typedef struct ExitInfo {
    Common::String *nextSetting;
    Common::Rect   *rect;
    Common::String *cursor;    
} ExitInfo;

typedef struct MaskInfo {
    Graphics::ManagedSurface *surf;
    Common::String *nextSetting;
    Common::Point *point;
	Symbol *flag;
    Common::String *cursor;    
} MaskInfo;

typedef Common::List<ExitInfo> ExitList;  
typedef Common::List<MaskInfo> MaskList;
typedef Common::List<Common::String> SoundList;   

class PrivateEngine : public Engine {
private:
	// We need random numbers
	Common::RandomSource *_rnd;

    Graphics::PixelFormat _pixelFormat;
	Image::ImageDecoder *_image;
	Graphics::ManagedSurface *_compositeSurface;

	int _screenW, _screenH;

public:
	PrivateEngine(OSystem *syst);
	~PrivateEngine();

	Audio::SoundHandle _soundHandle;
	Video::SmackerDecoder *_videoDecoder;
	Common::InstallerArchive _installerArchive;

	Common::Error run() override;
	void selectMask(Common::Point);
    void selectExit(Common::Point);

	bool cursorExit(Common::Point);
	bool cursorMask(Common::Point);
	
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveAutosaveCurrently() override  { return false; }
	bool canSaveGameStateCurrently() override { return true; }
	
	void selectLoadGame(Common::Point);
	bool cursorLoadGame(Common::Point);

	void selectSaveGame(Common::Point);
	bool cursorSaveGame(Common::Point);

	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);

    Common::String convertPath(Common::String); 
	void playSound(const Common::String &name);
	void playVideo(const Common::String &name);
    void skipVideo();
	void stopSound();

	void loadImage(const Common::String &file, int x, int y, bool drawn = true);
    void drawScreenFrame();
        
	Graphics::ManagedSurface *loadMask(const Common::String &, int, int, bool);
 
    uint32 _transparentColor;	
	void drawScreen();

    // global state
	Common::Point *_origin;
    Common::String *_nextSetting;
	Common::String *_currentSetting;
    Common::String *_nextVS;
    Common::String *_frame;

	// Save/Load games
	MaskInfo *_saveGameMask;
	MaskInfo *_loadGameMask;

	int _mode;
    bool _modified;
    Common::String *_nextMovie;
    ExitList _exits;
    MaskList _masks;

	// Radios
	SoundList _radio;
	SoundList _police;
	SoundList _phone;

	bool getRandomBool(uint);

};

extern PrivateEngine *g_private;

// Example console class
class Console : public GUI::Debugger {
public:
	Console(PrivateEngine *vm) {
	}
	virtual ~Console(void) {
	}
};

} // End of namespace Private

#endif
