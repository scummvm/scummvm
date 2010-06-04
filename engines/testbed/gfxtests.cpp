#include "testbed/gfxtests.h"
#include "testbed/testsuite.h"

#include "graphics/pixelformat.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"

namespace Testbed {

bool testFullScreenMode() {

	printf("Testing fullscreen mode\n");
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);

	printf("Testing Feature Presence.. \n");

	if (isFeaturePresent) {
		//Toggle
		printf("Supported\n");

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, isFeatureEnabled);
		g_system->endGFXTransaction();
	}

	return true;
}

bool testAspectRatio() {
	
	int x_lim;
	int y_lim;

	x_lim = g_system->getWidth();
	y_lim = g_system->getHeight();

	Graphics::PixelFormat f = g_system->getScreenFormat();

	printf("Screen is %d x %d using %d bytes per pixel\n", x_lim, y_lim, f.bytesPerPixel);

	char blackbuf[16 * 20];
	memset(blackbuf, 1, 16 * 20); // Prepare a buffer 16px wide and 240px high, to fit on a lateral strip

	uint8 pal[3 * 4];
	g_system->grabPalette(pal, 0, 3);
	pal[4] = 255;
	pal[5] = 255;
	pal[6] = 255;

	pal[8] = 0;
	pal[9] = 255;
	pal[10] = 0;

	g_system->setPalette(pal, 0, 3);

    //g_system->copyRectToScreen((const byte *)blackbuf, 16, 20, 28, 16, 20);
	//g_system->updateScreen();

	// Font usage
	
	Graphics::Surface *screen = g_system->lockScreen();

	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));
	
	uint16 h = font.getFontHeight();
	uint16 y = g_system->getHeight() / 2 - h / 2;
	

	Common::Rect r(0,y,screen->w,y+h);
	screen->fillRect(r,0);

	Common::String text("Hi thr!");

	font.drawString(screen, text, 0, y, screen->w, 1, Graphics::kTextAlignCenter);

	g_system->unlockScreen();
	g_system->updateScreen();
	
	return true;

}

}
