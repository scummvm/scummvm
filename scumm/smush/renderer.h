/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "config.h"

#include "rect.h"

class Palette;
class Mixer;
	
/*! 	@brief interface for general output (rendering)

	This is the interface for frame output. 
	Several implementations of these interface exist, each having a particular
	application.
*/
class Renderer {
public:
	virtual ~Renderer() {};
	/*!	@brief start of animation output
		
		This is called by the animation player when output is going to start.
		
		@param fname name of the animation being played.
		@param version version number of the animation
		@param nbframes total number of frames of the animation.
		
		@return true if initialisation was ok, false otherwise
	*/
	virtual bool startDecode(const char * fname, int version, int nbframes) = 0;
	/*!	@brief start of animation output
		
		This is called by the animation player when the frame size is changing.
		
		@param size new size of the frames.
		
		@return true if everything went fine, false otherwise
	*/
	virtual bool initFrame(const Point & size) = 0;
	/*!	@brief set a new palette
		
		This is called by the animation player when the palette is changing.
		
		@param pal new palette.
		
		@return true if everything went fine, false otherwise
	*/
	virtual bool setPalette(const Palette & pal) = 0;
	/*!	@brief lock a frame buffer
		
		This is called by the animation player when a frame is going to be decoded.
		
		@param frame the frame number.
		
		@return a pointer to the frame buffer to output data to.
	*/
	virtual char * lockFrame(int frame) = 0;
	/*!	@brief unlock a frame buffer
		
		This is called by the animation player when a frame has been decoded.
		
		@return true if everything went fine, false otherwise
	*/
	virtual bool unlockFrame() = 0;
	/*!	@brief flip a frame buffer
		
		This is called by the animation player when the current frame should be shown.
		
		@return true if everything went fine, false otherwise
	*/
	virtual bool flipFrame() = 0;
	/*!	@brief wait for some time
		
		This is called by the animation player when the animation should stay idle.
		
		@param ms number of millisecond to wait.
		
		@return true if everything went fine, false otherwise
	*/
	virtual bool wait(int ms) = 0;
	/*!	@brief does the renderer want a premature end of the animation ?
		
		This is called by the animation player after each frame.
		
		@return true if playing should be stopped, false otherwise.
	*/
	virtual bool prematureClose() = 0;
	/*!	@brief request for a mixer
		
		This is called by the animation player when sound output is required by the animation.
		
		@return a valid pointer to an uninitialized mixer instance, or null if none is available.
	*/
	virtual Mixer * getMixer() = 0;
	/*!	@brief debugging function : do not use
		
		@return true if everything went fine, false otherwise
	*/
	virtual bool saveCurrent() { return false; };
};

#endif
