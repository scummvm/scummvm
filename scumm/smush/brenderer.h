/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef BRENDERER_H
#define BRENDERER_H

#include "config.h"

#include "renderer.h"
#include "palette.h"

/*! 	@brief base class for easily creating ::renderer instances

	This class implements some function available in the ::renderer abstract class, so that 
	creation of subclasses of ::renderer is easier.
*/
class BaseRenderer : public Renderer {
protected:
	Palette _pal;		//!< The current palette
	byte *_data;		//!< The current frame buffer
	int32 _frame;			//!< The current frame number
	int32 _nbframes;		//!< The number of frames in the animation
	int32 _width;			//!< The current frame's width
	int32 _height;		//!< The current frame's height
	const char *_fname;	//!< The filename of the animation being played
protected:
	virtual void save(int32 frame = -1) = 0;

protected:
	const char *getFilename() const { return _fname; };	//!< accessor for animation filename
	int32 getNbframes() const { return _nbframes; };	//!< accessor for number of frames
	void clean();	//!< memory cleanup (deletes frame buffer)
	void setFrame(int32 f) { _frame = f; };	//!< allows to change the frame number
public:
	int32 getFrame() const { return _frame; };	//!< accessor for current frame number
	BaseRenderer();
	virtual ~BaseRenderer();

	virtual bool initFrame(const Point &size);
	virtual byte *lockFrame(int32 frame);
	virtual bool unlockFrame();
	virtual bool flipFrame();
	virtual bool setPalette(const Palette &pal);
	virtual bool startDecode(const char *fname, int32 version, int32 nbframes) { _fname = fname; _nbframes = nbframes; return true; }
	virtual Mixer *getMixer() { return 0; };
	virtual bool prematureClose() { return false; };
};

/*! 	@brief A null ::renderer

	This class completely implements ::renderer, without actually doing anything.
	This class is useful for performance measurements.
*/
class NullRenderer : public BaseRenderer {
protected:
	void save(int32 frame = -1) {};
public:
	NullRenderer() {};
	virtual ~NullRenderer() {};
	bool wait(int32 ms) { return true; };
};

#endif
