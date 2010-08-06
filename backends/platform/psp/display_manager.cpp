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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 47541 2010-01-25 01:39:44Z lordhoto $
 *
 */

#include <pspgu.h>
#include <pspdisplay.h>
#include <pspthreadman.h>

#include "common/scummsys.h"
#include "backends/base-backend.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/default_display_client.h"
#include "backends/platform/psp/cursor.h"
#include "backends/platform/psp/pspkeyboard.h"

#define USE_DISPLAY_CALLBACK	// to use callback for finishing the render
#include "backends/platform/psp/display_manager.h"

#define PSP_BUFFER_WIDTH (512)
#define	PSP_SCREEN_WIDTH	480
#define	PSP_SCREEN_HEIGHT	272
#define PSP_FRAME_SIZE (PSP_BUFFER_WIDTH * PSP_SCREEN_HEIGHT)

//#define ENABLE_RENDER_MEASURE /* how long it takes to render a frame */

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

uint32 __attribute__((aligned(16))) MasterGuRenderer::_displayList[2048];

const OSystem::GraphicsMode DisplayManager::_supportedModes[] = {
	{ "320x200 (centered)", "320x200 16-bit centered", CENTERED_320X200 },
	{ "435x272 (best-fit, centered)", "435x272 16-bit centered", CENTERED_435X272 },
	{ "480x272 (full screen)", "480x272 16-bit stretched", STRETCHED_480X272 },
	{ "362x272 (4:3, centered)", "362x272 16-bit centered", CENTERED_362X272 },
	{0, 0, 0}
};

// Class MasterGuRenderer ----------------------------------------------

void MasterGuRenderer::setupCallbackThread() {
	DEBUG_ENTER_FUNC();
	
	// start the thread that updates the display
	threadCreateAndStart("DisplayCbThread", PRIORITY_DISPLAY_THREAD, STACK_DISPLAY_THREAD);		
}

// this function gets called by PspThread when starting the new thread
void MasterGuRenderer::threadFunction() {
	DEBUG_ENTER_FUNC();
	
	// Create the callback. It should always get the pointer to MasterGuRenderer
	_callbackId = sceKernelCreateCallback("Display Callback", guCallback, this);
	if (_callbackId < 0) {
		PSP_ERROR("failed to create display callback\n");		
	}
	
	PSP_DEBUG_PRINT("created callback. Going to sleep\n");

	sceKernelSleepThreadCB();	// sleep until we get a callback	
}

// This callback is called when the render is finished. It swaps the buffers
int MasterGuRenderer::guCallback(int, int, void *__this) {

	MasterGuRenderer *_this = (MasterGuRenderer *)__this;

	sceGuSync(0, 0);				// make sure we wait for GU to finish		
	sceDisplayWaitVblankStartCB();	// wait for v-blank without eating main thread cycles
	sceGuSwapBuffers();				// swap the back and front buffers

	_this->_renderFinished = true;	// Only this thread can set the variable to true
	
	return 0;
}

void MasterGuRenderer::guInit() {
	DEBUG_ENTER_FUNC();

	sceGuInit();
	sceGuStart(0, _displayList);

	guProgramDisplayBufferSizes();

	sceGuOffset(2048 - (PSP_SCREEN_WIDTH / 2), 2048 - (PSP_SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xC350, 0x2710);
	sceGuDisable(GU_DEPTH_TEST);	// We'll use depth buffer area
	sceGuDepthMask(GU_TRUE);		// Prevent writes to depth buffer
	sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);

	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
}

void MasterGuRenderer::guProgramDisplayBufferSizes() {
	DEBUG_ENTER_FUNC();

	PSP_DEBUG_PRINT("Outputbits[%u]\n", GuRenderer::_displayManager->getOutputBitsPerPixel());

	switch (GuRenderer::_displayManager->getOutputBitsPerPixel()) {
	case 16:
		sceGuDrawBuffer(GU_PSM_4444, (void *)0, PSP_BUFFER_WIDTH);
		sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)(PSP_FRAME_SIZE * sizeof(uint16)), PSP_BUFFER_WIDTH);
		sceGuDepthBuffer((void*)(PSP_FRAME_SIZE * sizeof(uint16) * 2), PSP_BUFFER_WIDTH);
		VramAllocator::instance().allocate(PSP_FRAME_SIZE * sizeof(uint16) * 2);
		break;
	case 32:
		sceGuDrawBuffer(GU_PSM_8888, (void *)0, PSP_BUFFER_WIDTH);
		sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)(PSP_FRAME_SIZE * sizeof(uint32)), PSP_BUFFER_WIDTH);
		sceGuDepthBuffer((void*)(PSP_FRAME_SIZE * sizeof(uint32) * 2), PSP_BUFFER_WIDTH);
		VramAllocator::instance().allocate(PSP_FRAME_SIZE * sizeof(uint32) * 2);
		break;
	}
}

// These are GU commands that should always stay the same
inline void MasterGuRenderer::guPreRender() {
	DEBUG_ENTER_FUNC();

	_renderFinished = false;	// set to synchronize with callback thread
	
#ifdef ENABLE_RENDER_MEASURE
	_lastRenderTime = g_system->getMillis();
#endif /* ENABLE_RENDER_MEASURE */

	sceGuStart(0, _displayList);

	sceGuClearColor(0xFF000000);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuAmbientColor(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);
	sceGuTexOffset(0, 0);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA); // Also good enough for all purposes
	sceGuAlphaFunc(GU_GREATER, 0, 0xFF);	   // Also good enough for all purposes
}

inline void MasterGuRenderer::guPostRender() {
	DEBUG_ENTER_FUNC();

	sceGuFinish();
#ifdef USE_DISPLAY_CALLBACK
	if (_callbackId < 0)
		PSP_ERROR("bad callbackId[%d]\n", _callbackId);
	else
		sceKernelNotifyCallback(_callbackId, 0);	// notify the callback. Nothing extra to pass
#else
	sceGuSync(0, 0);	

#ifdef ENABLE_RENDER_MEASURE
	uint32 now = g_system->getMillis();
	PSP_INFO_PRINT("Render took %d milliseconds\n", now - _lastRenderTime);
#endif /* ENABLE_RENDER_MEASURE */

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	_renderFinished = true;
#endif /* !USE_DISPLAY_CALLBACK */	
}

void MasterGuRenderer::guShutDown() {
	sceGuTerm();
}


// Class DisplayManager -----------------------------------------------------

DisplayManager::~DisplayManager() {
	_masterGuRenderer.guShutDown();
}

void DisplayManager::init() {
	DEBUG_ENTER_FUNC();

	_displayParams.outputBitsPerPixel = 32;	// can be changed to produce 16-bit output

	GuRenderer::setDisplayManager(this);
	_screen->init();
	_overlay->init();
	_cursor->init();

	_masterGuRenderer.guInit();				// start up the renderer
#ifdef USE_DISPLAY_CALLBACK
	_masterGuRenderer.setupCallbackThread();
#endif
	
}

void DisplayManager::setSizeAndPixelFormat(uint width, uint height, const Graphics::PixelFormat *format) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("w[%u], h[%u], pformat[%p]\n", width, height, format);

	_overlay->deallocate();
	_screen->deallocate();

	_screen->setScummvmPixelFormat(format);
	_screen->setSize(width, height);
	_screen->allocate();

	_cursor->setScreenPaletteScummvmPixelFormat(format);

	_overlay->setBytesPerPixel(sizeof(OverlayColor));
	_overlay->setSize(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	_overlay->allocate();

	_displayParams.screenSource.width = width;
	_displayParams.screenSource.height = height;
	calculateScaleParams();
}

bool DisplayManager::setGraphicsMode(const char *name) {
	DEBUG_ENTER_FUNC();

	int i = 0;

	while (_supportedModes[i].name) {
		if (!strcmpi(_supportedModes[i].name, name)) {
			setGraphicsMode(_supportedModes[i].id);
			return true;
		}
		i++;
	}

	return false;
}

bool DisplayManager::setGraphicsMode(int mode) {
	DEBUG_ENTER_FUNC();

	_graphicsMode = mode;

	switch (_graphicsMode) {
	case CENTERED_320X200:
		_displayParams.screenOutput.width = 320;
		_displayParams.screenOutput.height = 200;
		break;
	case CENTERED_435X272:
		_displayParams.screenOutput.width = 435;
		_displayParams.screenOutput.height = 272;
		break;
	case STRETCHED_480X272:
		_displayParams.screenOutput.width = 480;
		_displayParams.screenOutput.height = 272;
		break;
	case CENTERED_362X272:
		_displayParams.screenOutput.width = 362;
		_displayParams.screenOutput.height = 272;
		break;
	default:
		PSP_ERROR("Unsupported graphics mode[%d].\n", _graphicsMode);
	}

	calculateScaleParams();

	return true;
}

void DisplayManager::calculateScaleParams() {
	if (_displayParams.screenOutput.width && _displayParams.screenSource.width &&
	        _displayParams.screenOutput.height && _displayParams.screenSource.height) {
		_displayParams.scaleX = ((float)_displayParams.screenOutput.width) / _displayParams.screenSource.width;
		_displayParams.scaleY = ((float)_displayParams.screenOutput.height) / _displayParams.screenSource.height;
	}
}

// return true if we really rendered or no dirty. False otherwise
bool DisplayManager::renderAll() {
	DEBUG_ENTER_FUNC();

#ifdef USE_DISPLAY_CALLBACK
	if (!_masterGuRenderer.isRenderFinished()) {
		PSP_DEBUG_PRINT("Callback render not finished.\n");
		return false;	// didn't render
	}	
#endif /* USE_DISPLAY_CALLBACK */
	
	// This is cheaper than checking time, so we do it first
	if (!_screen->isDirty() &&
	        (!_overlay->isDirty()) &&
	        (!_cursor->isDirty()) &&
	        (!_keyboard->isDirty())) {
		PSP_DEBUG_PRINT("Nothing dirty\n");
		return true;	// nothing to render
	}

	if (!isTimeToUpdate()) 
		return false;	// didn't render

	PSP_DEBUG_PRINT("screen[%s], overlay[%s], cursor[%s], keyboard[%s]\n",
	                _screen->isDirty() ? "true" : "false",
	                _overlay->isDirty() ? "true" : "false",
	                _cursor->isDirty() ? "true" : "false",
	                _keyboard->isDirty() ? "true" : "false"
	               );

	_masterGuRenderer.guPreRender();	// Set up rendering

	_screen->render();

	_screen->setClean();				// clean out dirty bit

	if (_overlay->isVisible())
		_overlay->render();

	_overlay->setClean();

	if (_cursor->isVisible())
		_cursor->render();

	_cursor->setClean();

	if (_keyboard->isVisible())
		_keyboard->render();

	_keyboard->setClean();

	_masterGuRenderer.guPostRender();
	
	return true;	// rendered successfully
}

inline bool DisplayManager::isTimeToUpdate() {

#define MAX_FPS 30

	uint32 now = g_system->getMillis();
	if (now - _lastUpdateTime < (1000 / MAX_FPS))
		return false;

	_lastUpdateTime = now;
	return true;
}

Common::List<Graphics::PixelFormat> DisplayManager::getSupportedPixelFormats() const {
	Common::List<Graphics::PixelFormat> list;

	// In order of preference
	list.push_back(PSPPixelFormat::convertToScummvmPixelFormat(PSPPixelFormat::Type_5650));
	list.push_back(PSPPixelFormat::convertToScummvmPixelFormat(PSPPixelFormat::Type_5551));
	list.push_back(PSPPixelFormat::convertToScummvmPixelFormat(PSPPixelFormat::Type_4444));
	list.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return list;
}

