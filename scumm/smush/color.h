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

#ifndef COLOR_H
#define COLOR_H

#include "config.h"

/*! 	@brief simple class for handling a Chunk.

	This small class is an helper for Chunks.
*/
class Color {
public:
	typedef unsigned char value_type;	//!< The type of the Chunk components.
private:
	value_type _r;	//!< The red component.
	value_type _g;	//!< The green component.
	value_type _b;	//!< The blue component.
public:
	Color();
	Color(value_type, value_type, value_type);
	Color(const Color &);
	Color & operator=(const Color &);
	virtual ~Color();
	value_type red() const;
	value_type green() const;
	value_type blue() const;
	/*!	@brief handle delta palette modification

		This method is used specifically by player::handleDeltaPalette().
		It updates the Chunk component using delta values given as short.

		@param ptr pointer to a table of 3 shorts that contain delta values to use.
	*/
	void delta(short * ptr);
};

#endif
