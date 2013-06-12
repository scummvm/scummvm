#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/decoders/tga.h"

#include "video/video_decoder.h"
#include "video/avi_decoder.h"

#include "engines/util.h"
 
#include "zvision/zvision.h"
#include "zvision/zfsArchive.h"
#include "zvision/zork_avi_decoder.h"
#include "zvision/zork_raw.h"

namespace ZVision {
 
ZVision::ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data1");
	SearchMan.addSubDirectoryMatching(gameDataDir, "data2");
	SearchMan.addSubDirectoryMatching(gameDataDir, "data3");
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemmx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemscr");
 
	// Here is the right place to set up the engine specific debug channels
	//DebugMan.addDebugChannel(kZVisionDebugExample, "example", "this is just an example for a engine specific debug channel");
	//DebugMan.addDebugChannel(kZVisionDebugExample2, "example2", "also an example");
 
	// Don't forget to register your random source
	_rnd = new Common::RandomSource("zvision");
 
	debug("ZVision::ZVision");
}
 
ZVision::~ZVision() {
	debug("ZVision::~ZVision");
 
	// Dispose your resources here
	delete _rnd;
 
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

// Taken from SCI
void scale2x(const byte *src, byte *dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);
	const int newWidth = srcWidth * 2;
	const int pitch = newWidth * bytesPerPixel;
	const byte *srcPtr = src;

	if (bytesPerPixel == 1) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				dst[0] = color;
				dst[1] = color;
				dst[newWidth] = color;
				dst[newWidth + 1] = color;
				dst += 2;
			}
			dst += newWidth;
		}
	} else if (bytesPerPixel == 2) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				const byte color2 = *srcPtr++;
				dst[0] = color;
				dst[1] = color2;
				dst[2] = color;
				dst[3] = color2;
				dst[pitch] = color;
				dst[pitch + 1] = color2;
				dst[pitch + 2] = color;
				dst[pitch + 3] = color2;
				dst += 4;
			}
			dst += pitch;
		}
	}
}

void playVideo(Video::VideoDecoder *videoDecoder /*, VideoState videoState*/) {
	if (!videoDecoder)
		return;

	videoDecoder->start();

	byte *scaleBuffer = 0;
	byte bytesPerPixel = videoDecoder->getPixelFormat().bytesPerPixel;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	uint16 pitch = videoDecoder->getWidth() * bytesPerPixel;
	uint16 screenWidth = 640;//g_sci->_gfxScreen->getDisplayWidth();
	uint16 screenHeight = 480;//g_sci->_gfxScreen->getDisplayHeight();

	bool zoom2x = true;

	if (zoom2x) {
		width *= 2;
		height *= 2;
		pitch *= 2;
		scaleBuffer = new byte[width * height * bytesPerPixel];
	}

	uint16 x, y;

	x = (screenWidth - width) / 2;
	y = (screenHeight - height) / 2;

	bool skipVideo = false;

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				if (scaleBuffer) {
					scale2x((byte *)frame->pixels, scaleBuffer, videoDecoder->getWidth(), videoDecoder->getHeight(), bytesPerPixel);
					g_system->copyRectToScreen(scaleBuffer, pitch, x, y, width, height);
				} else {
					g_system->copyRectToScreen(frame->pixels, frame->pitch, x, y, width, height);
				}

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	delete[] scaleBuffer;
	delete videoDecoder;
}

Common::Error ZVision::run() {
	// Register the files within the zfs archive files with the SearchMan
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.zfs");

	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::String name = (*iter)->getName();
		ZfsArchive *archive = new ZfsArchive(name, (*iter)->createReadStream());

		SearchMan.add(name, archive);
	}

	//Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);	// ARGB8888
	Graphics::PixelFormat format = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);	// RGB555
	initGraphics(640, 480, true, &format);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
 
	// Additional setup.
	debug("ZVision::init");
 
	// Your main even loop should be (invoked from) here.
	debug("ZVision::go: Hello, World!");
 
	// This test will show up if -d1 and --debugflags=example are specified on the commandline
	//debugC(1, kZVisionDebugExample, "Example debug call");
 
	// This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
	//debugC(3, kZVisionDebugExample | kZVisionDebugExample2, "Example debug call two");

	Common::File file;

	if (file.open("ADLIB.MDI")) {
		file.close();
	}



#if 1
	// Video test
	Video::VideoDecoder *videoDecoder = new ZorkAVIDecoder();
	if (videoDecoder && videoDecoder->loadFile("ZASSETS/TEMPLE/T000A11C.AVI")) {
		Common::List<Graphics::PixelFormat> formats;
		formats.push_back(videoDecoder->getPixelFormat());
		initGraphics(640, 480, true, formats);
		
		playVideo(videoDecoder);
	}
#endif

	Common::File f;

#if 1
	// Image test
	
	initGraphics(640, 480, true, &format);
	
	if (f.open("zassets/castle/CB8EB11C.TGA")) {
		Graphics::TGADecoder tga;
		if (!tga.loadStream(f))
			error("Error while reading TGA image");
		f.close();

		const Graphics::Surface *tgaSurface = tga.getSurface();

		Graphics::Surface *screen = g_system->lockScreen();
		for (uint16 y = 0; y < tgaSurface->h; y++)
			memcpy(screen->getBasePtr(0, y), tgaSurface->getBasePtr(0, y), tgaSurface->pitch);
		g_system->unlockScreen();

		tga.destroy();
	}

	
#endif

#if 1
	// Sound test
	if (f.open("zassets/castle/C000H9TC.RAW")) {
		Audio::SeekableAudioStream *audioStream = makeRawZorkStream(&f, 22050, DisposeAfterUse::YES);
		Audio::SoundHandle handle;
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &handle, audioStream);
	}
	
#endif

	// Main loop
	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event event;

	while (!shouldQuit()) {
		eventMan->pollEvent(event);	// eat events
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}
 
} // End of namespace ZVision
