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

#ifndef BRENDERER_H
#define BRENDERER_H

#include "config.h"

#include "renderer.h"
#include "palette.h"
#include "rect.h"

/*! 	@brief base class for easily creating ::renderer instances

	This class implements some function available in the ::renderer abstract class, so that 
	creation of subclasses of ::renderer is easier.
*/
class BaseRenderer : public Renderer {
private:
	Palette _pal;		//!< The current palette
	char * _data;		//!< The current frame buffer
	int _frame;			//!< The current frame number
	int _nbframes;		//!< The number of frames in the animation
	int _width;			//!< The current frame's width
	int _height;		//!< The current frame's height
	const char * _fname;	//!< The filename of the animation being played
protected:
	virtual void save(int frame = -1) = 0;

protected:
	const char * getFilename() const { return _fname; };	//!< accessor for animation filename
	int getNbframes() const { return _nbframes; };	//!< accessor for number of frames
	int getWidth() const { return _width; };	//!< accessor for current width
	int getHeight() const { return _height; };	//!< accessor for current height
	const Palette & pal() const { return _pal; };	//!< accessor for current palette
	const char * data() const { return _data; };	//!< accessor for current frame buffer
	void clean();	//!< memory cleanup (deletes frame buffer)
	void setFrame(int f) { _frame = f; };	//!< allows to change the frame number
public:
	int getFrame() const { return _frame; };	//!< accessor for current frame number
	BaseRenderer();
	virtual ~BaseRenderer();

	virtual bool initFrame(const Point & size);
	virtual char * lockFrame(int frame);
	virtual bool unlockFrame();
	virtual bool flipFrame();
	virtual bool setPalette(const Palette & pal);
	virtual bool startDecode(const char * fname, int version, int nbframes) { _fname = fname; _nbframes = nbframes; return true; }
	virtual Mixer * getMixer() { return 0; };
	virtual bool prematureClose() { return false; };
};

/*! 	@brief A null ::renderer

	This class completely implements ::renderer, without actually doing anything.
	This class is useful for performance measurements.
*/
class NullRenderer : public BaseRenderer {
protected:
	void save(int frame = -1) {};
public:
	NullRenderer() {};
	virtual ~NullRenderer() {};
	bool wait(int ms) { return true; };
};

#endif
