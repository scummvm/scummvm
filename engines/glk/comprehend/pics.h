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

#ifndef GLK_COMPREHEND_PICS_H
#define GLK_COMPREHEND_PICS_H

#include "glk/comprehend/draw_surface.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/str-array.h"

namespace Glk {
namespace Comprehend {

#define IMAGE_OP_SCENE_END 0x00

#define IMAGE_OP_SET_TEXT_POS 0x10

#define IMAGE_OP_PEN_COLOR_A 0x20
#define IMAGE_OP_PEN_COLOR_B 0x21
#define IMAGE_OP_PEN_COLOR_C 0x22
#define IMAGE_OP_PEN_COLOR_D 0x23
#define IMAGE_OP_PEN_COLOR_E 0x24
#define IMAGE_OP_PEN_COLOR_F 0x25
#define IMAGE_OP_PEN_COLOR_G 0x26
#define IMAGE_OP_PEN_COLOR_H 0x27

#define IMAGE_OP_DRAW_CHAR 0x30

#define IMAGE_OP_SHAPE_PIXEL 0x40
#define IMAGE_OP_SHAPE_BOX 0x41
#define IMAGE_OP_SHAPE_CIRCLE_TINY 0x42
#define IMAGE_OP_SHAPE_CIRCLE_SMALL 0x43
#define IMAGE_OP_SHAPE_CIRCLE_MED 0x44
#define IMAGE_OP_SHAPE_CIRCLE_LARGE 0x45
#define IMAGE_OP_SHAPE_A 0x46
#define IMAGE_OP_SHAPE_SPRAY 0x47

#define IMAGE_OP_EOF 0x55

#define IMAGE_OP_FILL_COLOR 0x60

#define IMAGE_OP_MOVE_TO 0x80
#define IMAGE_OP_MOVE_TO_FAR 0x81

#define IMAGE_OP_DRAW_BOX 0x90
#define IMAGE_OP_DRAW_BOX_FAR 0x91

#define IMAGE_OP_DRAW_LINE 0xa0
#define IMAGE_OP_DRAW_LINE_FAR 0xa1

#define IMAGE_OP_DRAW_SHAPE 0xc0
#define IMAGE_OP_DRAW_SHAPE_FAR 0xc1

#define IMAGE_OP_PAINT 0xe0
#define IMAGE_OP_PAINT_FAR 0xe1

#define IMAGEF_NO_FLOODFILL (1 << 1)

enum {
	LOCATIONS_OFFSET = 0,
	LOCATIONS_NO_BG_OFFSET = 100,
	ITEMS_OFFSET = 200,
	DARK_ROOM = 1000,
	BRIGHT_ROOM = 1001,
	TITLE_IMAGE = 9999
};

class Pics : public Common::Archive {
	struct ImageContext {
		Common::File _file;
		DrawSurface *_drawSurface;
		Graphics::Font *_font;
		uint _drawFlags;

		uint16 _x;
		uint16 _y;
		uint32 _penColor;
		uint32 _fillColor;
		uint32 _shape;

		uint16 _textX;
		uint16 _textY;

		ImageContext(DrawSurface *drawSurface, Graphics::Font *font, uint flags) :
			_drawSurface(drawSurface), _font(font), _drawFlags(0),
			_x(0), _y(0), _penColor(G_COLOR_BLACK), _fillColor(G_COLOR_BLACK),
			_shape(IMAGE_OP_SHAPE_CIRCLE_LARGE), _textX(0), _textY(0) {
		}

	};

	struct ImageFile {
	private:
		Common::Array<uint16> _imageOffsets;
		Common::String _filename;

	private:
		bool doImageOp(ImageContext *ctx) const;
		uint16 imageGetOperand(ImageContext *ctx) const;

	public:
		ImageFile() {}
		ImageFile(const Common::String &filename);

		void draw(uint index, ImageContext *ctx) const;
	};

private:
	Common::Array<ImageFile> _rooms;
	Common::Array<ImageFile> _items;
	ImageFile _title;
	Graphics::Font *_font;

private:
	/**
	 * Returns the image number if the passed filename is a picture
	 */
	int getPictureNumber(const Common::String &filename) const;

	/**
	 * Draw the specified picture
	 */
	void drawPicture(int pictureNum) const;

public:
	Pics();
	~Pics();

	void clear();

	void load(const Common::StringArray &roomFiles,
	          const Common::StringArray &itemFiles,
	          const Common::String &titleFile);

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::String &name) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

} // namespace Comprehend
} // namespace Glk

#endif
