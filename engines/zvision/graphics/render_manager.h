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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZVISION_RENDER_MANAGER_H
#define ZVISION_RENDER_MANAGER_H

#include "common/hashmap.h"
#include "common/rect.h"
#include "graphics/framelimiter.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "zvision/graphics/graphics_effect.h"
#include "zvision/graphics/render_table.h"
#include "zvision/text/truetype_font.h"

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
	RenderManager(ZVision *engine, const ScreenLayout layout, const Graphics::PixelFormat pixelFormat, bool doubleFPS, bool widescreen = false);
	~RenderManager();


	typedef Common::List<GraphicsEffect *> EffectsList;

private:
	ZVision *_engine;
	OSystem *_system;
	const Graphics::PixelFormat _pixelFormat;
	const ScreenLayout _layout;
	bool _hiRes = false;
	Graphics::FrameLimiter _frameLimiter;

	/**
	 * A Rectangle representing the screen/window resolution.
	 */
	Common::Rect _screenArea;

	Common::Rect _HDscreenArea = Common::Rect(800, 600);
	Common::Rect _HDscreenAreaWide = Common::Rect(720, 377);

	Common::Point _textOffset;  //Position vector of text area origin relative to working window origin

	/**
	 * A Rectangle placed inside _screenArea All in-game coordinates
	 * are given in this coordinate space. Also, all images are clipped to the
	 * edges of this Rectangle
	 */
	Common::Rect _workingArea;

	Common::Point _workingAreaCenter; //Center of the working area in working area coordinates

	/**
	Managed surface representing physical screen; dirty rectangles will be handled automatically by this from now on
	*/
	Graphics::Screen _screen;

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
	Graphics::ManagedSurface _workingManagedSurface;
	Common::Rect _backgroundSurfaceDirtyRect;

//TODO: Migrate this functionality to SubtitleManager to improve encapsulation
//*
	// Buffer for drawing subtitles & other messages
	Graphics::Surface _textSurface;
	Graphics::ManagedSurface _textManagedSurface;
	Common::Rect _textSurfaceDirtyRect;
//*/

	// Rectangle for subtitles & other messages
	Common::Rect _textArea; //NB Screen coordinates
	Common::Rect _textLetterbox;  //Section of text area outside working window, to be filled with black when blanked
	Common::Rect _textOverlay;  //Section of text area to be filled with colorkey when blanked (may potentially intersect text letterbox area if screen/window is wider than working area!)

	// Buffer for drawing menu
	Graphics::Surface _menuSurface;
	Graphics::ManagedSurface _menuManagedSurface;
	Common::Rect _menuSurfaceDirtyRect;  //subrectangle of menu area outside working area

	// Rectangle for menu area
	Common::Rect _menuArea; //Screen coordinates
	Common::Rect _menuLetterbox; //Section of menu area to be filled with black when blanked
	Common::Rect _menuOverlay;  //Section of menu area to be filled with colorkey when blanked (may potentially intersect menu letterbox area if screen/window is wider than working area!)

	//Buffer for streamed video playback
//*
	Graphics::ManagedSurface _vidManagedSurface;
	/*/
	  Graphics::Surface _vidSurface;
	//*/

	//Area of streamed video playback
	Common::Rect _vidArea;

	// A buffer used for apply graphics effects
	Graphics::Surface _effectSurface;

	// A buffer to store the result of the panorama / tilt warps
	Graphics::Surface _warpedSceneSurface;


	/** Used to warp the background image */
	RenderTable _renderTable;



	// Visual effects list
	EffectsList _effects;

	//Pointer to currently active backbuffer output surface
	Graphics::Surface *_outputSurface;
	bool _doubleFPS;
	bool _widescreen;

public:
	void initialize(bool hiRes = false);

	/**
	 * Renders the scene to the screen
	 * Returns true if screen was updated
	 * If streamMode is set true, all background processing is skipped and the previous framebuffer is used
	 */
	bool renderSceneToScreen(bool immediate = false, bool overlayOnly = false, bool preStream = false);

	Graphics::ManagedSurface &getVidSurface(Common::Rect dstRect);  //dstRect is defined relative to working area origin

	const Common::Rect &getMenuArea() const {
		return _menuArea;
	}
	const Common::Rect &getWorkingArea() const {
		return _workingArea;
	}

	/**
	 * Blits the image or a portion of the image to the background.
	 *
	 * @param fileName        Name of the image file
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 */
	void renderImageToBackground(const Common::Path &fileName, int16 destinationX, int16 destinationY);

	/**
	 * Blits the image or a portion of the image to the background.
	 *
	 * @param fileName   Name of the image file
	 * @param destX      X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destY      Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param colorkey   Transparent color
	 */
	void renderImageToBackground(const Common::Path &fileName, int16 destX, int16 destY, uint32 colorkey);

	/**
	 * Blits the image or a portion of the image to the background.
	 *
	 * @param fileName   Name of the image file
	 * @param destX      X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destY      Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param keyX       X position of transparent color
	 * @param keyY       Y position of transparent color
	 */
	void renderImageToBackground(const Common::Path &fileName, int16 destX, int16 destY, int16 keyX, int16 keyY);

	/**
	 * Sets the current background image to be used by the RenderManager and immediately
	 * blits it to the screen. (It won't show up until the end of the frame)
	 *
	 * @param fileName    The name of the image file
	 */
	void setBackgroundImage(const Common::Path &fileName);

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

	/**
	 * Blit from one surface to another surface
	 *
	 * @param src       Source surface
	 * @param srcRect  Rectangle defining area of source surface to blit; if this rectangle is empty or not supplied, entire source surface is blitted
	 * @param dst       Destination surface
	 * @param x         Destination surface x coordinate
	 * @param y         Destination surface y coordinate
	 */

	void blitSurfaceToSurface(const Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, int _x, int _y);
	void blitSurfaceToSurface(const Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, int _x, int _y, uint32 colorkey);
	void blitSurfaceToSurface(const Graphics::Surface &src, Graphics::Surface &dst, int _x, int _y) {blitSurfaceToSurface(src, Common::Rect(src.w, src.h), dst, _x, _y);}
	void blitSurfaceToSurface(const Graphics::Surface &src, Graphics::Surface &dst, int _x, int _y, uint32 colorkey) {blitSurfaceToSurface(src, Common::Rect(src.w, src.h), dst, _x, _y, colorkey);}

	// Blitting surface-to-background methods
	void blitSurfaceToBkg(const Graphics::Surface &src, int x, int y, int32 colorkey = -1);

	// Blitting surface-to-background methods with scale
	void blitSurfaceToBkgScaled(const Graphics::Surface &src, const Common::Rect &dstRect, int32 colorkey = -1);

	/**
	 * Blit from source surface to menu area
	 *
	 * @param src       Source surface
	 * @param x         x coordinate relative to menu area origin
	 * @param y         y coordinate relative to menu area origin
	 */
	void blitSurfaceToMenu(const Graphics::Surface &src, int16 x, int16 y, int32 colorkey = 0);

	/**
	 * Blit from source surface to text area
	 *
	 * @param src       Source surface
	 * @param x         x coordinate relative to text area origin
	 * @param y         y coordinate relative to text area origin
	 */
	void blitSurfaceToText(const Graphics::Surface &src, int16 x, int16 y, int32 colorkey = 0);

	// Return background size
	Common::Point getBkgSize();

	// Return portion of background as new surface
	Graphics::Surface *getBkgRect(Common::Rect &rect);

	// Load image into new surface
	Graphics::Surface *loadImage(const Common::Path &file);
	Graphics::Surface *loadImage(const Common::Path &file, bool transposed);

	// Clear whole/area of menu backbuffer
	void clearMenuSurface(bool force = false, int32 colorkey = -1);

	// Clear whole/area of subtitle backbuffer
	void clearTextSurface(bool force = false, int32 colorkey = -1);

	// Copy needed portion of background surface to workingArea surface
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
	void readImageToSurface(const Common::Path &fileName, Graphics::Surface &destination);

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
	void readImageToSurface(const Common::Path &fileName, Graphics::Surface &destination, bool transposed);

	// Add visual effect to effects list
	void addEffect(GraphicsEffect *_effect);

	// Delete effect(s) by ID (ID equal to slot of action:region that create this effect)
	void deleteEffect(uint32 ID);

	// Create "mask" for effects - (color +/- depth) will be selected as not transparent. Like color selection
	// xy - base color
	// depth - +/- of base color
	// rect - rectangle where select pixels
	// minD - if not NULL will receive real bottom border of depth
	// maxD - if not NULL will receive real top border of depth
	EffectMap *makeEffectMap(const Common::Point &xy, int16 depth, const Common::Rect &rect, int8 *minD, int8 *maxD);

	// Create "mask" for effects by simple transparent color
	EffectMap *makeEffectMap(const Graphics::Surface &surf, uint16 transp);

	// Return background rectangle in screen coordinates
	Common::Rect transformBackgroundSpaceRectToScreenSpace(const Common::Rect &src);

	// Mark whole background surface as dirty
	void markDirty();

/*
	// Fill background surface by color
	void bkgFill(uint8 r, uint8 g, uint8 b);
*/

	void checkBorders();
	void rotateTo(int16 to, int16 time);
	void updateRotation();
	void upscaleRect(Common::Rect &rect);
};

} // End of namespace ZVision

#endif
