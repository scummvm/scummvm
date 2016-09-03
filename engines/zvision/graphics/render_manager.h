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

#ifndef ZVISION_RENDER_MANAGER_H
#define ZVISION_RENDER_MANAGER_H

#include "zvision/graphics/render_table.h"
#include "zvision/text/truetype_font.h"

#include "common/rect.h"
#include "common/hashmap.h"

#include "graphics/surface.h"

#include "graphics_effect.h"

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
	RenderManager(ZVision *engine, uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat, bool doubleFPS);
	~RenderManager();

private:
	struct OneSubtitle {
		Common::Rect r;
		Common::String txt;
		int16  timer;
		bool todelete;
		bool redraw;
	};

	typedef Common::HashMap<uint16, OneSubtitle> SubtitleMap;
	typedef Common::List<GraphicsEffect *> EffectsList;

private:
	ZVision *_engine;
	OSystem *_system;
	const Graphics::PixelFormat _pixelFormat;

	/**
	 * A Rectangle centered inside the actual window. All in-game coordinates
	 * are given in this coordinate space. Also, all images are clipped to the
	 * edges of this Rectangle
	 */
	Common::Rect _workingWindow;

	// Center of the screen in the x direction
	const int _screenCenterX;
	// Center of the screen in the y direction
	const int _screenCenterY;

	/** A buffer for background image that's being used to create the background */
	Graphics::Surface _currentBackgroundImage;
	Common::Rect _backgroundDirtyRect;

	/**
	 * The x1 or y1 offset of the subRectangle of the background that is currently displayed on the screen
	 * It will be x1 if PANORAMA, or y1 if TILT
	 */
	int16 _backgroundOffset;
	/** The width of the current background image */
	uint16 _backgroundWidth;
	/** The height of the current background image */
	uint16 _backgroundHeight;

	// A buffer that holds the portion of the background that is used to render the final image
	// If it's a normal scene, the pixels will be blitted directly to the screen
	// If it's a panorma / tilt scene, the pixels will be first warped to _warpedSceneSurface
	Graphics::Surface _backgroundSurface;
	Common::Rect _backgroundSurfaceDirtyRect;

	// A buffer for subtitles
	Graphics::Surface _subtitleSurface;

	// Rectangle for subtitles area
	Common::Rect _subtitleArea;

	// A buffer for menu drawing
	Graphics::Surface _menuSurface;
	Common::Rect _menuSurfaceDirtyRect;

	// Rectangle for menu area
	Common::Rect _menuArea;

	// A buffer used for apply graphics effects
	Graphics::Surface _effectSurface;

	// A buffer to store the result of the panorama / tilt warps
	Graphics::Surface _warpedSceneSurface;


	/** Used to warp the background image */
	RenderTable _renderTable;

	// Internal subtitles counter
	uint16 _subid;

	// Subtitle list
	SubtitleMap _subsList;

	// Visual effects list
	EffectsList _effects;

	bool _doubleFPS;

public:
	void initialize();

	/**
	 * Renders the scene to the screen
	 */
	void renderSceneToScreen();

	void copyToScreen(const Graphics::Surface &surface, Common::Rect &rect, int16 srcLeft, int16 srcTop);

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
	 * @param fileName   Name of the image file
	 * @param destX      X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destY      Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param colorkey   Transparent color
	 */
	void renderImageToBackground(const Common::String &fileName, int16 destX, int16 destY, uint32 colorkey);

	/**
	 * Blits the image or a portion of the image to the background.
	 *
	 * @param fileName   Name of the image file
	 * @param destX      X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destY      Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param keyX       X position of transparent color
	 * @param keyY       Y position of transparent color
	 */
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

	// Return pointer of RenderTable object
	RenderTable *getRenderTable();

	// Return current background offset
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

	// Scale buffer (nearest)
	void scaleBuffer(const void *src, void *dst, uint32 srcWidth, uint32 srcHeight, byte bytesPerPixel, uint32 dstWidth, uint32 dstHeight);

	// Blitting surface-to-surface methods
	void blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int x, int y);
	void blitSurfaceToSurface(const Graphics::Surface &src, const Common::Rect &_srcRect , Graphics::Surface &dst, int _x, int _y, uint32 colorkey);

	// Blitting surface-to-background methods
	void blitSurfaceToBkg(const Graphics::Surface &src, int x, int y, int32 colorkey = -1);

	// Blitting surface-to-background methods with scale
	void blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &_dstRect, int32 colorkey = -1);

	// Blitting surface-to-menu methods
	void blitSurfaceToMenu(const Graphics::Surface &src, int x, int y, int32 colorkey = -1);

	// Subtitles methods

	void initSubArea(uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow);

	// Create subtitle area and return ID
	uint16 createSubArea(const Common::Rect &area);
	uint16 createSubArea();

	// Delete subtitle by ID
	void deleteSubArea(uint16 id);
	void deleteSubArea(uint16 id, int16 delay);

	// Update subtitle area
	void updateSubArea(uint16 id, const Common::String &txt);

	// Processing subtitles
	void processSubs(uint16 deltatime);

	// Return background size
	Common::Point getBkgSize();

	// Return portion of background as new surface
	Graphics::Surface *getBkgRect(Common::Rect &rect);

	// Load image into new surface
	Graphics::Surface *loadImage(Common::String file);
	Graphics::Surface *loadImage(Common::String file, bool transposed);

	// Clear whole/area of menu surface
	void clearMenuSurface();
	void clearMenuSurface(const Common::Rect &r);

	// Copy menu buffer to screen
	void renderMenuToScreen();

	// Copy needed portion of background surface to workingWindow surface
	void prepareBackground();

	/**
	 * Reads an image file pixel data into a Surface buffer. Also, if the image
	 * is transposed, it will un-transpose the pixel data. The function will
	 * call destination::create() if the dimensions of destination do not match
	 * up with the dimensions of the image.
	 *
	 * @param fileName       The name of a .tga file
	 * @param destination    A reference to the Surface to store the pixel data in
	 */
	void readImageToSurface(const Common::String &fileName, Graphics::Surface &destination);

	/**
	 * Reads an image file pixel data into a Surface buffer. Also, if the image
	 * is transposed, it will un-transpose the pixel data. The function will
	 * call destination::create() if the dimensions of destination do not match
	 * up with the dimensions of the image.
	 *
	 * @param fileName       The name of a .tga file
	 * @param destination    A reference to the Surface to store the pixel data in
	 * @param transposed     Transpose flag
	 */
	void readImageToSurface(const Common::String &fileName, Graphics::Surface &destination, bool transposed);

	// Add visual effect to effects list
	void addEffect(GraphicsEffect *_effect);

	// Delete effect(s) by ID (ID equal to slot of action:region that create this effect)
	void deleteEffect(uint32 ID);

	// Create "mask" for effects - (color +/- depth) will be selected as not transparent. Like color selection
	// xy - base color
	// depth - +/- of base color
	// rect - rectangle where select pixels
	// minD - if not NULL will recieve real bottom border of depth
	// maxD - if not NULL will recieve real top border of depth
	EffectMap *makeEffectMap(const Common::Point &xy, int16 depth, const Common::Rect &rect, int8 *minD, int8 *maxD);

	// Create "mask" for effects by simple transparent color
	EffectMap *makeEffectMap(const Graphics::Surface &surf, uint16 transp);

	// Return background rectangle in screen coordinates
	Common::Rect transformBackgroundSpaceRectToScreenSpace(const Common::Rect &src);

	// Mark whole background surface as dirty
	void markDirty();

#if 0
	// Fill background surface by color
	void bkgFill(uint8 r, uint8 g, uint8 b);
#endif

	bool askQuestion(const Common::String &str);
	void delayedMessage(const Common::String &str, uint16 milsecs);
	void timedMessage(const Common::String &str, uint16 milsecs);
	void showDebugMsg(const Common::String &msg, int16 delay = 3000);

	void checkBorders();
	void rotateTo(int16 to, int16 time);
	void updateRotation();

	void upscaleRect(Common::Rect &rect);
};

} // End of namespace ZVision

#endif
