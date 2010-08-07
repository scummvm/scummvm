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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
 * BS_GraphicEngine
 * ----------------
 * This the graphics engine interface.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_GRAPHICENGINE_H
#define SWORD25_GRAPHICENGINE_H

// Includes
#include "common/array.h"
#include "common/str.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"
#include "sword25/kernel/resservice.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/rect.h"
#include "sword25/gfx/framecounter.h"
#include "sword25/gfx/renderobjectptr.h"

namespace Sword25 {

class BS_Kernel;
class BS_Image;
class BS_Panel;
class BS_Screenshot;

// Typen
typedef unsigned int BS_COLOR;

// Makros
#define BS_RGB(R,G,B)       (0xFF000000 | ((R) << 16) | ((G) << 8) | (B))
#define BS_ARGB(A,R,G,B)    (((A) << 24) | ((R) << 16) | ((G) << 8) | (B))

/**
    @brief Dies ist das Graphik-Engine Interface, dass alle Methoden und Klassen enthält, die eine Graphik-Engine implementieren muss.

    Hier sind nur wenige Rumpffunktionen realisiert, wie z.B. das Abfragen der Parameter des Ausgabepuffers.
    Die Hauptfunktionen muss eine Implementation dieses Inferfaces stellen.<br>
    Die bisher einzige Implementation ist BS_DDrawGfx.
*/

class BS_GraphicEngine : public BS_ResourceService, public BS_Persistable {
public:
	// Enums
	// -----

	// Colour formats
	//
	/**
	 * The colour format used by the engine
	 */
	enum COLOR_FORMATS {
		/// Undefined/unknown colour format
		CF_UNKNOWN = 0,
		/// 16-bit colour format (R5G5B5)
		CF_RGB15,
		/// 16-bit colour format (R5G6R5)
		CF_RGB16,
		/**
		 * Special alpha colour format of the engine, which supports very quick display using MMX instructions.
		 * The pixels are 16-bits wide and have the same format as #CF_RGB15. In addition, each pixel has an 8-bit
		 * alpha value.
		 * It summarises groupings of pixels pixels and four alpha values in a 12-byte data block.
		 * The data is stored in the following order:
		 * Alpha0 Alpha1 Alpha2 Alpha3 Pixel0 Pixel1 Pixel2 Pixel3
		 * If the number of pixels in a line is not divisible by 4, then unused pixels and alpha values can have
		 * arbitrary values.
		 */
		CF_RGB15_INTERLEAVED,
		/**
		 * Special alpha colour format of the engine, which supports very quick display using MMX instructions.
		 * The pixels are 16-bits wide and have the same format as #CF_RGB16. In addition, each pixel has an 8-bit
		 * alpha value.
		 * It summarises groupings of pixels pixels and four alpha values in a 12-byte data block.
		 * The data is stored in the following order:
		 * Alpha0 Alpha1 Alpha2 Alpha3 Pixel0 Pixel1 Pixel2 Pixel3
		 * If the number of pixels in a line is not divisible by 4, then unused pixels and alpha values can have
		 * arbitrary values.
		 */
		CF_RGB16_INTERLEAVED,
		/**
		 * 24-bit colour format (R8G8B8)
		 */
		CF_RGB24,
		/**
		 * 32-bit colour format (A8R8G8B8) (little endian)
		*/
		CF_ARGB32,
		/**
		    32-bit colour format (A8B8G8R8) (little endian)
		*/
		CF_ABGR32
	};

	// Interface
	// ---------

	/**
	 * Initialises the graphics engine and sets the screen mode. Returns true if initialisation failed.
	 * Notes: This method should be called immediately after the initialisation of all services.
	 *
	 * @param Height            The height of the output buffer in pixels. The default value is 600
	 * @param BitDepth          The bit depth of the desired output buffer in bits. The default value is 16
	 * @param BackbufferCount   The number of back buffers to be created. The default value is 2
	 * @param Windowed          Indicates whether the engine is to run in windowed mode.
	 */
	virtual bool        Init(int Width = 800, int Height = 600, int BitDepth = 16, int BackbufferCount = 2, bool Windowed = false) = 0;

	/**
	 * Begins rendering a new frame.
	 * Notes: This method must be called at the beginning of the main loop, before any rendering methods are used.
	 * Notes: Implementations of this method must call _UpdateLastFrameDuration()
	 * @param UpdateAll         Specifies whether the renderer should redraw everything on the next frame.
	 * This feature can be useful if the renderer with Dirty Rectangles works, but sometimes the client may
	*/
	virtual bool        StartFrame(bool UpdateAll = false) = 0;

	/**
	 * Ends the rendering of a frame and draws it on the screen.
	 *
	 * This method must be at the end of the main loop. After this call, no further Render method may be called.
	 * This should only be called once for a given previous call to #StartFrame.
	*/
	virtual bool        EndFrame() = 0;

	// Debug methods

	/**
	 * Draws a line in the frame buffer
	 *
	 * This method must be called between calls to StartFrame() and EndFrame(), and is intended only for debugging
	 * purposes. The line will only appear for a single frame. If the line is to be shown permanently, it must be
	 * called for every frame.
	* @param Start      The starting point of the line
	* @param End        The ending point of the line
	* @param Color      The colour of the line. The default is BS_RGB (255,255,255) (White)
	*/
	virtual void        DrawDebugLine(const BS_Vertex &Start, const BS_Vertex &End, unsigned int Color = BS_RGB(255, 255, 255)) = 0;

	/**
	 * Creates a screenshot of the current frame buffer and writes it to a graphic file in PNG format.
	 * Returns true if the screenshot was saved successfully.
	 * Notes: This method should only be called after a call to EndFrame(), and before the next call to StartFrame().
	 * @param Filename  The filename for the screenshot
	 */
	bool SaveScreenshot(const Common::String &Filename);

	/**
	 * Creates a thumbnail with the dimensions of 200x125. This will not include the top and bottom of the screen..
	 * the interface boards the the image as a 16th of it's original size.
	 * Notes: This method should only be called after a call to EndFrame(), and before the next call to StartFrame().
	 * The frame buffer must have a resolution of 800x600.
	 * @param Filename  The filename for the screenshot
	 */
	bool SaveThumbnailScreenshot(const Common::String &Filename);

	/**
	 * Reads the current contents of the frame buffer
	 * Notes: This method is for creating screenshots. It is not very optimised. It should only be called
	 * after a call to EndFrame(), and before the next call to StartFrame().
	 * @param Width     Returns the width of the frame buffer
	 * @param Height    Returns the height of the frame buffer
	 * @param Data      Returns the raw data of the frame buffer as an array of 32-bit colour values.
	*/
	virtual bool GetScreenshot(unsigned int &Width, unsigned int &Height, byte **Data) = 0;


	virtual BS_RenderObjectPtr<BS_Panel> GetMainPanel() = 0;

	/**
	 * Specifies the time (in microseconds) since the last frame has passed
	 */
	int GetLastFrameDurationMicro() {
		if (m_TimerActive) return m_LastFrameDuration;
		else return 0;
	}

	/**
	 * Specifies the time (in microseconds) the previous frame took
	*/
	float GetLastFrameDuration() {
		if (m_TimerActive) return static_cast<float>(m_LastFrameDuration) / 1000000.0f;
		else return 0;
	}

	void StopMainTimer() {
		m_TimerActive = false;
	}
	void ResumeMainTimer() {
		m_TimerActive = true;
	}
	float GetSecondaryFrameDuration() {
		return static_cast<float>(m_LastFrameDuration) / 1000000.0f;
	}

	// Accessor methods

	/**
	 * Returns the width of the output buffer in pixels
	 */
	int         GetDisplayWidth() {
		return m_Width;
	}

	/**
	 * Returns the height of the output buffer in pixels
	 */
	int         GetDisplayHeight() {
		return m_Height;
	}

	/**
	 * Returns the bounding box of the output buffer: (0, 0, Width, Height)
	 */
	BS_Rect    &GetDisplayRect() {
		return m_ScreenRect;
	}

	/**
	 * Returns the bit depth of the output buffer
	 */
	int         GetBitDepth() {
		return m_BitDepth;
	}

	/**
	 * Determines whether the frame buffer change is to be synchronised with Vsync. This is turned on by default.
	 * Notes: In windowed mode, this setting has no effect.
	 * @param Vsync     Indicates whether the frame buffer changes are to be synchronised with Vsync.
	 */
	virtual void    SetVsync(bool Vsync) = 0;

	/**
	 * Returns true if V-Sync is on.
	 * Notes: In windowed mode, this setting has no effect.
	 */
	virtual bool    GetVsync() const = 0;

	/**
	 * Returns true if the engine is running in Windowed mode.
	 */
	bool    IsWindowed() {
		return m_Windowed;
	}

	/**
	 * Fills a rectangular area of the frame buffer with a colour.
	 * Notes: It is possible to create transparent rectangles by passing a colour with an Alpha value of 255.
	 * @param FillRectPtr   Pointer to a BS_Rect, which specifies the section of the frame buffer to be filled.
	 * If the rectangle falls partly off-screen, then it is automatically trimmed.
	 * If a NULL value is passed, then the entire image is to be filled.
	 * @param Color         The 32-bit colour with which the area is to be filled. The default is BS_RGB(0, 0, 0) (black)
	    @remark Falls das Rechteck nicht völlig innerhalb des Bildschirms ist, wird es automatisch zurechtgestutzt.
	 */
	virtual bool Fill(const BS_Rect *FillRectPtr = 0, unsigned int Color = BS_RGB(0, 0, 0)) = 0;

	// Debugging Methods

	int GetFPSCount() const {
		return m_FPSCounter.GetFPS();
	}
	int GetRepaintedPixels() const {
		return m_RepaintedPixels;
	}

	// Access methods

	/**
	 * Returns the size of a pixel entry in bytes for a particular colour format
	 * @param ColorFormat   The desired colour format. The parameter must be of type COLOR_FORMATS
	 * @return              Returns the size of a pixel in bytes. If the colour format is unknown, -1 is returned.
	 */
	static int GetPixelSize(BS_GraphicEngine::COLOR_FORMATS ColorFormat) {
		switch (ColorFormat) {
		case BS_GraphicEngine::CF_RGB16:
		case BS_GraphicEngine::CF_RGB15:
			return 2;

		case BS_GraphicEngine::CF_RGB16_INTERLEAVED:
		case BS_GraphicEngine::CF_RGB15_INTERLEAVED:
			return 3;

		case BS_GraphicEngine::CF_ARGB32:
			return 4;
		}

		return -1;
	}

	/**
	 * Calculates the length of an image line in bytes, depending on a given colour format.
	 * @param ColorFormat   The colour format
	 * @param Width         The width of the line in pixels
	 * @return              Reflects the length of the line in bytes. If the colour format is
	 * unknown, -1 is returned
	 */
	static int CalcPitch(BS_GraphicEngine::COLOR_FORMATS ColorFormat, int Width) {
		switch (ColorFormat) {
		case BS_GraphicEngine::CF_RGB16:
		case BS_GraphicEngine::CF_RGB15:
			return Width * 2;

		case BS_GraphicEngine::CF_RGB16_INTERLEAVED:
		case BS_GraphicEngine::CF_RGB15_INTERLEAVED:
			return (Width + 3) / 4 * 12;

		case BS_GraphicEngine::CF_ARGB32:
		case BS_GraphicEngine::CF_ABGR32:
			return Width * 4;

		default:
			BS_ASSERT(false);
		}

		return -1;
	}

	// Persistence Methods
	// -------------------
	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

	static void ARGBColorToLuaColor(lua_State *L, unsigned int Color);
	static unsigned int LuaColorToARGBColor(lua_State *L, int StackIndex);

protected:
	// Constructor
	// -----------
	BS_GraphicEngine(BS_Kernel *pKernel);

	// Display Variables
	// -----------------
	int     m_Width;
	int     m_Height;
	BS_Rect m_ScreenRect;
	int     m_BitDepth;
	bool    m_Windowed;

	// Debugging Variables
	// -------------------
	BS_Framecounter m_FPSCounter;

	unsigned int    m_RepaintedPixels;

	/**
	 * Calculates the time since the last frame beginning has passed.
	 */
	void UpdateLastFrameDuration();

private:
	bool RegisterScriptBindings();

	// LastFrameDuration Variables
	// ---------------------------
	uint64                      m_LastTimeStamp;
	unsigned int                m_LastFrameDuration;
	bool                        m_TimerActive;
	Common::Array<unsigned int> m_FrameTimeSamples;
	unsigned int                m_FrameTimeSampleSlot;
};

} // End of namespace Sword25

#endif
