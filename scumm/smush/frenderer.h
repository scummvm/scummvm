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

#ifndef FRENDERER_H
#define FRENDERER_H

#include "config.h"

#ifdef DEBUG
# ifndef NO_DEBUG_FONT_RENDERER
#  define DEBUG_FONT_RENDERER
# endif
#else
# ifdef DEBUG_FONT_RENDERER
#  error DEBUG_FONT_RENDERER defined without DEBUG
# endif
#endif

#include "brenderer.h"
#include "common/util.h"

/*!	@brief ::renderer implementation specifically designed for font files.

	This class is a valid ::renderer implementation. The frames are kept in memory, as bitmap representing characters, so that 
	they can be rendered again in another frame as strings. 
	
	This class also contains some functions useful for printing strings. This is used to show subtitles and more generally texts
	in animations.
	
	@todo update the mehod to use the ::blitter class, instead of direct pointers.
*/
class FontRenderer : public BaseRenderer {
private:
	int32 _nbChars;	//!< The number of frames in the font
	int32 _color;		//!< A color parameter used for font printing.
	bool _original;	//!< flag for color selection
	struct {
		int32 width;
		int32 height;
		char *chr;
	} _chars[256]; //!< array that contains the size of the different frames (i.e. characters) of the font.
public:
	/*!	@brief font_renderer constructor
		
		@param use_original_colors	flag to indicate if the font use it's own color, or if the base color are set at runtime.
	*/
	FontRenderer(bool use_original_colors = false);
	virtual ~FontRenderer();
	virtual bool wait(int32 ms) { return true; };
protected:
	virtual void save(int32 frame = -1);
	/*!	@brief get the width of a character.
		
		@param c	the character we want the width from.

		@return the width of the character
	*/
	int32 charWidth(int32 c) const;
	/*!	@brief get the width of a string.
		
		@param str	the string we want the width from.

		@return the complete width of the string
	*/
	int32 stringWidth(const char *str) const;
	/*!	@brief get the height of a character.
		
		@param c	the character we want the height from.

		@return the height of the character
	*/
	int32 charHeight(int32 c) const;
	/*!	@brief get the height of a string.
		
		@param str	the string we want the height from.

		@return the complete height of the string
	*/
	int32 stringHeight(const char *str) const;
	/*!	@brief draw a character in the given frame buffer.
		
		@param buffer	the frame buffer to draw into.
		@param size		the size of the frame buffer.
		@param x		the horizontal position of the topleft corner of the character.
		@param y		the vertical position of the topleft corner of the character.
		@param c		the character to draw.
		
		@bug	This method does not clip. This is not really a bug, as it should always be correctly called, but some asserts would be welcome.

		@return the width of the character
	*/
	int32 drawChar(char *buffer, const Point &size, int32 x, int32 y, int32 c) const;
	/*!	@brief draw a string in the given frame buffer.
		
		@param str		the string to draw.
		@param buffer	the frame buffer to draw into.
		@param size		the size of the frame buffer.
		@param x		the horizontal position of the topleft corner of the string.
		@param y		the vertical position of the topleft corner of the string.

		@bug	This method does not clip. This is not really a bug, as it should always be correctly called, but some asserts would be welcome.
	*/
	void drawSubstring(const byte *str, char *buffer, const Point &size, int32 x, int32 y) const;
public:
	/*!	@brief change the programmable color of the font.
		
		@param c		the new color to use.

		@return \c true if everything went fine, \c false otherwise
	*/
	bool setColor(int32 c) { _color = c; return true; }
	/*!	@brief draw a centered and possibly using multiple lines string.
	
		This method performs calculation of the string size before choosing where to draw it.
		As I still not have figured out exactly what is the meaning of the fields in the TRES Chunk, 
		the real meaning of the parameters can be quite difficult to understand.
	
		@remark	The current implementation is incorrect in the sense that it does not conform to the original game.
		@todo	rewrite and rethink this to better match the original implementation.
		
		@param str		the string to draw.
		@param buffer	the frame buffer to draw into.
		@param size		the size of the frame buffer.
		@param y		the vertical position of the topleft corner of the string. This position may be changed if it is too low to be correctly drawn.
		@param xmin		the minimum horizontal position of the topleft corner of the string.
		@param width		the maximum width of the string. If the string is too long, it will wrap.
		@param offset	offset to give to the horizontal position.

		@return \c true if everything went fine, \c false otherwise
	*/
	bool drawStringCentered(const char *str, char *buffer, const Point &size, int32 y, int32 xmin, int32 width, int32 offset) const;
	bool drawStringWrap(const char *str, char *buffer, const Point &size, int32 x, int32 y, int32 width) const;
	bool drawStringWrapCentered(const char *str, char *buffer, const Point &size, int32 x, int32 y, int32 width) const;
	/*!	@brief draw a string at an absolute position.
	
		@param str		the string to draw.
		@param buffer	the frame buffer to draw into.
		@param size		the size of the frame buffer.
		@param x		the horizontal position of the topleft corner of the string.
		@param y		the vertical position of the topleft corner of the string. This position may be changed if it is too low to be correctly drawn.

		@return \c true if everything went fine, \c false otherwise
	*/
	bool drawStringAbsolute(const char *str, char *buffer, const Point &size, int32 x, int32 y) const;
};

#endif
