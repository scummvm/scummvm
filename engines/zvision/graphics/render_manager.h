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
#include "zvision/fonts/truetype_font.h"

#include "common/rect.h"
#include "common/hashmap.h"

#include "graphics/surface.h"


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
	RenderManager(OSystem *system, uint32 windowWidth, uint32 windowHeight, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat);
	~RenderManager();

private:
	struct AlphaDataEntry {
		Graphics::Surface *data;
		uint16 alphaColor;
		uint16 destX;
		uint16 destY;
		uint16 width;
		uint16 height;
	};

	typedef Common::HashMap<uint32, AlphaDataEntry> AlphaEntryMap;

private:
	OSystem *_system;
	const Graphics::PixelFormat _pixelFormat;

	// A buffer the exact same size as the workingWindow
	// This buffer stores everything un-warped, then does a warp at the end of the frame
	Graphics::Surface _workingWindowBuffer;
	// A buffer representing the entire screen. Any graphical updates are first done with this buffer
	// before actually being blitted to the screen
	Graphics::Surface _backBuffer;
	// A list of Alpha Entries that need to be blitted to the backbuffer
	AlphaEntryMap _alphaDataEntries;

	// A rectangle representing the portion of the working window where the pixels have been changed since last frame
	Common::Rect _workingWindowDirtyRect;
	// A rectangle representing the portion of the backbuffer where the pixels have been changed since last frame
	Common::Rect _backBufferDirtyRect;

	/** Width of the working window. Saved to prevent extraneous calls to _workingWindow.width() */
	const int _workingWidth;
	/** Height of the working window. Saved to prevent extraneous calls to _workingWindow.height() */
	const int _workingHeight;
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
	/** Used to warp the background image */
	RenderTable _renderTable;

	Graphics::Surface _currentBackground;
	/** The (x1,y1) coordinates of the subRectangle of the background that is currently displayed on the screen */
	Common::Point _backgroundOffset;
	/** The width of the current background image */
	uint16 _backgroundWidth;
	/** The height of the current background image */
	uint16 _backgroundHeight;

	/**
	 * The "velocity" at which the background image is panning. We actually store the inverse of velocity (ms/pixel instead of pixels/ms)
	 * because it allows you to accumulate whole pixels 'steps' instead of rounding pixels every frame
	 */
	int _backgroundInverseVelocity;
	/** Holds any 'leftover' milliseconds between frames */
	uint _accumulatedVelocityMilliseconds;

public:
	void initialize();
	/**
	 * Rotates the background image in accordance to the current _backgroundInverseVelocity
	 *
	 * @param deltaTimeInMillis    The amount of time that has passed since the last frame
	 */
	void update(uint deltaTimeInMillis);

	/**
	 * Renders the current state of the backbuffer to the screen
	 */
	void renderBackbufferToScreen();

	/**
	 * Renders all AlphaEntries to the backbuffer
	 */
	void processAlphaEntries();
	/**
	 * Clears the AlphaEntry list
	 */
	void clearAlphaEntries() { _alphaDataEntries.clear(); }
	/**
	 * Removes a specific AlphaEntry from the list
	 *
	 * @param idNumber    The id number identifing the AlphaEntry
	 */
	void removeAlphaEntry(uint32 idNumber) { _alphaDataEntries.erase(idNumber); }

	/**
	 * Copies a sub-rectangle of a buffer to the working window
	 *
	 * @param buffer        The pixel data to copy to the working window
	 * @param destX         The X destination in the working window where the subRect of data should be put
	 * @param destY         The Y destination in the working window where the subRect of data should be put
	 * @param imageWidth    The width of the source image
	 * @param width         The width of the sub rectangle
	 * @param height        The height of the sub rectangle
	 */
	void copyRectToWorkingWindow(const uint16 *buffer, int32 destX, int32 destY, int32 imageWidth, int32 width, int32 height);
	/**
	 * Copies a sub-rectangle of a buffer to the working window with binary alpha support.
	 *
	 * @param buffer        The pixel data to copy to the working window
	 * @param destX         The X destination in the working window where the subRect of data should be put
	 * @param destY         The Y destination in the working window where the subRect of data should be put
	 * @param imageWidth    The width of the source image
	 * @param width         The width of the sub rectangle
	 * @param height        The height of the sub rectangle
	 * @param alphaColor    The color to interpret as meaning 'transparent'
	 * @param idNumber      A unique identifier for the data being copied over.
	 */
	void copyRectToWorkingWindow(const uint16 *buffer, int32 destX, int32 destY, int32 imageWidth, int32 width, int32 height, int16 alphaColor, uint32 idNumber);

	/**
	 * Renders the supplied text to the working window
	 *
	 * @param idNumber     A unique identifier for the text
	 * @param text         The text to be rendered
	 * @param font         The font to use to render the text
	 * @param destX        The X destination in the working window where the text should be rendered
	 * @param destY        The Y destination in the working window where the text should be rendered
	 * @param textColor    The color to render the text with (in RBG 565)
	 * @param maxWidth     The max width the text should take up.
	 * @param maxHeight    The max height the text should take up.
	 * @param align        The alignment of the text within the bounds of maxWidth
	 * @param wrap         If true, any words extending past maxWidth will wrap to a new line. If false, ellipses will be rendered to show that the text didn't fit
	 * @return             A rectangle representing where the text was drawn in the working window
	 */
	Common::Rect renderTextToWorkingWindow(uint32 idNumber, const Common::String &text, TruetypeFont *font, int destX, int destY, uint16 textColor, int maxWidth, int maxHeight = -1, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool wrap = true);

	/**
	 * Fills the entire workingWindow with the specified color. Internally, the color
	 * will be converted to RGB 565 and then blitted.
	 *
	 * @param color    The color to fill the working window with. (In RGB 555)
	 */
	void clearWorkingWindowTo555Color(uint16 color);

	/**
	 * Blits the image or a portion of the image to the backbuffer. Actual screen updates won't happen until the end of the frame.
	 * The image will be clipped to fit inside the working window. Coords are in working window space, not screen space!
	 *
	 * @param fileName        Name of the image file
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 */
	void renderImageToScreen(const Common::String &fileName, int16 destinationX, int16 destinationY, bool wrap = false);

	/**
	 * Blits the image or a portion of the image to the backbuffer. Actual screen updates won't happen until the end of the frame.
	 * The image will be clipped to fit inside the working window. Coords are in working window space, not screen space!
	 *
	 * @param stream          Surface to read the image data from
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 */
	void renderImageToScreen(Graphics::Surface &surface, int16 destinationX, int16 destinationY, bool wrap = false);

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
	 * Set the background scroll velocity. Negative velocities correspond to left / up scrolling and
	 * positive velocities correspond to right / down scrolling
	 *
	 * @param velocity    Velocity
	 */
	void setBackgroundVelocity(int velocity);

	/**
	 * Converts a point in screen coordinate space to image coordinate space
	 *
	 * @param point    Point in screen coordinate space
	 * @return         Point in image coordinate space
	 */
	const Common::Point screenSpaceToImageSpace(const Common::Point &point);
	/**
	 * Converts a point in image coordinate space to ***PRE-WARP***
	 * working window coordinate space
	 *
	 * @param point    Point in image coordinate space
	 * @return         Point in PRE-WARP working window coordinate space
	 */
	const Common::Point imageSpaceToWorkingWindowSpace(const Common::Point &point);

	/**
	 * Clip a rectangle to the working window. If it returns false, the original rect
	 * is not inside the working window.
	 *
	 * @param rect    The rectangle to clip against the working window
	 * @return        Is rect at least partially inside the working window (true) or completely outside (false)
	 */
	bool clipRectToWorkingWindow(Common::Rect &rect);

	RenderTable *getRenderTable();
	uint32 getCurrentBackgroundOffset();
	const Graphics::Surface *getBackBuffer() { return &_backBuffer; }

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

private:
	/**
	 * Renders a subRectangle of an image to the backbuffer. The destinationRect and SubRect
	 * will be clipped to image bound and to working window bounds
	 *
	 * @param buffer             Pointer to (0, 0) of the image data
	 * @param imageWidth         The width of the original image (not of the subRectangle)
	 * @param imageHeight        The width of the original image (not of the subRectangle)
	 * @param horizontalPitch    The horizontal pitch of the original image
	 * @param destinationX       The x coordinate (in working window space) of where to put the final image
	 * @param destinationY       The y coordinate (in working window space) of where to put the final image
	 * @param subRectangle       A rectangle representing the part of the image that should be rendered
	 * @param wrap               Should the image wrap (tile) if it doesn't completely fill the screen?
	 */
	void renderSubRectToScreen(Graphics::Surface &surface, int16 destinationX, int16 destinationY, bool wrap);

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

	/**
	 * Move the background image by an offset. If we are currently in Panorama mode,
	 * the offset will correspond to a horizontal motion. If we are currently in Tilt mode,
	 * the offset will correspond to a vertical motion. This function should not be called
	 * if we are in Flat mode.
	 *
	 * The RenderManager will take care of wrapping the image.
	 * Ex: If the image has width 1400px, it is legal to offset 1500px.
	 *
	 * @param offset    The amount to move the background
	 */
	void moveBackground(int offset);
};

} // End of namespace ZVision

#endif
