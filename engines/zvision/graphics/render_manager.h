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

#ifndef ZVISION_RENDER_MANAGER_H
#define ZVISION_RENDER_MANAGER_H

#include "zvision/graphics/render_table.h"
#include "zvision/fonts/truetype_font.h"

#include "common/rect.h"
#include "common/hashmap.h"

#include "graphics/surface.h"

#include "effect.h"


class OSystem;

namespace Common {
class String;
class SeekableReadStream;
}

namespace Video {
class VideoDecoder;
}

namespace ZVision {

class RenderManager {
public:
	RenderManager(ZVision *engine, uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat);
	~RenderManager();

private:
	struct oneSub {
		Common::Rect _r;
		Common::String _txt;
		int16  timer;
		bool todelete;
		bool redraw;
	};
//	struct AlphaDataEntry {
//		Graphics::Surface *data;
//		uint16 alphaColor;
//		uint16 destX;
//		uint16 destY;
//		uint16 width;
//		uint16 height;
//	};
//
	typedef Common::HashMap<uint16, oneSub> subMap;
	typedef Common::List<Effect *> effectsList;

private:
	ZVision *_engine;
	OSystem *_system;
	const Graphics::PixelFormat _pixelFormat;

	// A buffer for blitting background image to working window
	Graphics::Surface _wrkWnd;

	Common::Rect _wrkWndDirtyRect;

	Graphics::Surface _outWnd;

	Common::Rect _outWndDirtyRect;

	Common::Rect _bkgDirtyRect;

	Graphics::Surface _subWnd;

	Common::Rect _subWndDirtyRect;

	Graphics::Surface _menuWnd;

	Common::Rect _menuWndDirtyRect;

	Graphics::Surface _effectWnd;


	// A buffer the exact same size as the workingWindow
	// This buffer stores everything un-warped, then does a warp at the end of the frame
	//Graphics::Surface _workingWindowBuffer;
	// A buffer representing the entire screen. Any graphical updates are first done with this buffer
	// before actually being blitted to the screen
	//Graphics::Surface _backBuffer;
	// A list of Alpha Entries that need to be blitted to the backbuffer
	//AlphaEntryMap _alphaDataEntries;

	// A rectangle representing the portion of the working window where the pixels have been changed since last frame
	//Common::Rect _workingWindowDirtyRect;
	// A rectangle representing the portion of the backbuffer where the pixels have been changed since last frame
	//Common::Rect _backBufferDirtyRect;

	/** Width of the working window. Saved to prevent extraneous calls to _workingWindow.width() */
	const int _wrkWidth;
	/** Height of the working window. Saved to prevent extraneous calls to _workingWindow.height() */
	const int _wrkHeight;
	/** Center of the screen in the x direction */
	const int _screenCenterX;
	/** Center of the screen in the y direction */
	const int _screenCenterY;

	/**
	 * A Rectangle centered inside the actual window. All in-game coordinates
	 * are given in this coordinate space. Also, all images are clipped to the
	 * edges of this Rectangle
	 */
	const Common::Rect _workingWindow;

	Common::Rect _subWndRect;

	Common::Rect _menuWndRect;

	/** Used to warp the background image */
	RenderTable _renderTable;

	Graphics::Surface _curBkg;
	/** The (x1,y1) coordinates of the subRectangle of the background that is currently displayed on the screen */
	int16 _bkgOff;
	/** The width of the current background image */
	uint16 _bkgWidth;
	/** The height of the current background image */
	uint16 _bkgHeight;

	uint16 _subid;

	subMap _subsList;

	/**
	 * The "velocity" at which the background image is panning. We actually store the inverse of velocity (ms/pixel instead of pixels/ms)
	 * because it allows you to accumulate whole pixels 'steps' instead of rounding pixels every frame
	 */
	//int _backgroundInverseVelocity;
	/** Holds any 'leftover' milliseconds between frames */
	//uint _accumulatedVelocityMilliseconds;

	effectsList _effects;

public:
	void initialize();

	/**
	 * Renders the current state of the backbuffer to the screen
	 */
	void renderBackbufferToScreen();

	/**
	 * Blits the image or a portion of the image to the background.
	 *
	 * @param fileName        Name of the image file
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 */
	void renderImageToBackground(const Common::String &fileName, int16 destinationX, int16 destinationY);

	/**
	 * Blits the image or a portion of the image to the background.
	 *
	 * @param fileName        Name of the image file
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param colorkey   Transparent color
	 */
	void renderImageToBackground(const Common::String &fileName, int16 destX, int16 destY, uint32 colorkey);

	void renderImageToBackground(const Common::String &fileName, int16 destX, int16 destY, int16 keyX, int16 keyY);

	/**
	 * Sets the current background image to be used by the RenderManager and immediately
	 * blits it to the screen. (It won't show up until the end of the frame)
	 *
	 * @param fileName    The name of the image file
	 */
	void setBackgroundImage(const Common::String &fileName);

	/**
	 * Set the background position (_backgroundOffset). If the current RenderState is PANORAMA, the offset
	 * will be in the horizontal direction. If the current RenderState is TILT, the offset will be in the
	 * vertical direction.
	 *
	 * This method will not render anything on the screen. So if nothing else is called that renders the
	 * background, the change won't be seen until next frame.
	 *
	 * @param offset The amount to offset the background
	 */
	void setBackgroundPosition(int offset);

	/**
	 * Converts a point in screen coordinate space to image coordinate space
	 *
	 * @param point    Point in screen coordinate space
	 * @return         Point in image coordinate space
	 */
	const Common::Point screenSpaceToImageSpace(const Common::Point &point);

	RenderTable *getRenderTable();
	uint32 getCurrentBackgroundOffset();

	/**
	 * Creates a copy of surface and transposes the data.
	 *
	 * Note: The user is responsible for calling free() on the returned surface
	 * and then deleting it
	 *
	 * @param surface    The data to be transposed
	 * @return           A copy of the surface with the data transposed
	 */
	static Graphics::Surface *tranposeSurface(const Graphics::Surface *surface);

	void scaleBuffer(const void *src, void *dst, uint32 srcWidth, uint32 srcHeight, byte bytesPerPixel, uint32 dstWidth, uint32 dstHeight);


	void blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int x, int y);
	void blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int _x, int _y, uint32 colorkey);
	void blitSurfaceToSurface(const Graphics::Surface &src, Graphics::Surface &dst, int x, int y);
	void blitSurfaceToSurface(const Graphics::Surface &src, Graphics::Surface &dst, int x, int y, uint32 colorkey);
	void blitSurfaceToBkg(const Graphics::Surface &src, int x, int y);
	void blitSurfaceToBkg(const Graphics::Surface &src, int x, int y, uint32 colorkey);
	void blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &_dstRect);
	void blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &_dstRect, uint32 colorkey);
	void blitSurfaceToMenu(const Graphics::Surface &src, int x, int y);
	void blitSurfaceToMenu(const Graphics::Surface &src, int x, int y, uint32 colorkey);

	uint16 createSubArea(const Common::Rect &area);
	uint16 createSubArea();
	void deleteSubArea(uint16 id);
	void deleteSubArea(uint16 id, int16 delay);
	void updateSubArea(uint16 id, const Common::String &txt);
	void processSubs(uint16 deltatime);

	Common::Point getBkgSize();

	Graphics::Surface *getBkgRect(Common::Rect &rect);
	Graphics::Surface *loadImage(const char *file);
	Graphics::Surface *loadImage(Common::String &file);
	Graphics::Surface *loadImage(const char *file, bool transposed);
	Graphics::Surface *loadImage(Common::String &file, bool transposed);

	void clearMenuSurface();
	void clearMenuSurface(const Common::Rect &r);
	void renderMenuToScreen();

	void prepareBkg();

	/**
	 * Reads an image file pixel data into a Surface buffer. In the process
	 * it converts the pixel data from RGB 555 to RGB 565. Also, if the image
	 * is transposed, it will un-transpose the pixel data. The function will
	 * call destination::create() if the dimensions of destination do not match
	 * up with the dimensions of the image.
	 *
	 * @param fileName       The name of a .tga file
	 * @param destination    A reference to the Surface to store the pixel data in
	 */
	void readImageToSurface(const Common::String &fileName, Graphics::Surface &destination);
	void readImageToSurface(const Common::String &fileName, Graphics::Surface &destination, bool transposed);

	void addEffect(Effect *_effect);
	void deleteEffect(uint32 ID);
	EffectMap *makeEffectMap(const Common::Point &xy, int16 depth, const Common::Rect &rect, int8 *minD, int8 *maxD);
	EffectMap *makeEffectMap(const Graphics::Surface &surf, uint16 transp);

	Common::Rect bkgRectToScreen(const Common::Rect &src);

	void markDirty();

	void bkgFill(uint8 r, uint8 g, uint8 b);
};

} // End of namespace ZVision

#endif
