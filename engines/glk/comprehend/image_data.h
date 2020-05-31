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

#ifndef GLK_COMPREHEND_IMAGE_DATA_H
#define GLK_COMPREHEND_IMAGE_DATA_H

#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/graphics.h"
#include "common/scummsys.h"

namespace Glk {
namespace Comprehend {

class ComprehendGame;
struct FileBuffer;
struct ImageContext;

struct ImageFileData {
private:
	Common::Array<uint16> _imageOffsets;
	FileBuffer _fb;

private:
	bool doImageOp(DrawSurface *ds, ImageContext *ctx);
	uint16 imageGetOperand();

	public:
	void load(const char *filename);

	void draw(uint index, ImageContext *ctx);
};

struct ImageData {
private:
	Common::Array<ImageFileData> _files;

public:
	ImageData();
	void clear();

	uint size() const { return _files.size(); }
	ImageFileData &operator[](uint index) { return _files[index]; }

	void load(const Common::Array<const char *> &filenames);
};

#define IMAGEF_OP_WAIT_KEYPRESS		(1 << 0)
#define IMAGEF_NO_FLOODFILL		(1 << 1)

#define IMAGE_OP_SCENE_END		0x00

#define IMAGE_OP_SET_TEXT_POS		0x10

#define IMAGE_OP_PEN_COLOR_A		0x20
#define IMAGE_OP_PEN_COLOR_B		0x21
#define IMAGE_OP_PEN_COLOR_C		0x22
#define IMAGE_OP_PEN_COLOR_D		0x23
#define IMAGE_OP_PEN_COLOR_E		0x24
#define IMAGE_OP_PEN_COLOR_F		0x25
#define IMAGE_OP_PEN_COLOR_G		0x26
#define IMAGE_OP_PEN_COLOR_H		0x27

#define IMAGE_OP_DRAW_CHAR		0x30

#define IMAGE_OP_SHAPE_PIXEL		0x40
#define IMAGE_OP_SHAPE_BOX		0x41
#define IMAGE_OP_SHAPE_CIRCLE_TINY	0x42
#define IMAGE_OP_SHAPE_CIRCLE_SMALL	0x43
#define IMAGE_OP_SHAPE_CIRCLE_MED	0x44
#define IMAGE_OP_SHAPE_CIRCLE_LARGE	0x45
#define IMAGE_OP_SHAPE_A		0x46
#define IMAGE_OP_SHAPE_SPRAY		0x47

#define IMAGE_OP_EOF			0x55

#define IMAGE_OP_FILL_COLOR		0x60

#define IMAGE_OP_MOVE_TO		0x80
#define IMAGE_OP_MOVE_TO_FAR		0x81

#define IMAGE_OP_DRAW_BOX		0x90
#define IMAGE_OP_DRAW_BOX_FAR		0x91

#define IMAGE_OP_DRAW_LINE		0xa0
#define IMAGE_OP_DRAW_LINE_FAR		0xa1

#define IMAGE_OP_DRAW_SHAPE		0xc0
#define IMAGE_OP_DRAW_SHAPE_FAR		0xc1

#define IMAGE_OP_PAINT			0xe0
#define IMAGE_OP_PAINT_FAR		0xe1

void image_set_draw_flags(unsigned flags);

void draw_dark_room(void);
void draw_bright_room(void);
void draw_image(ImageData *info, unsigned index);
void draw_location_image(ImageData *info, unsigned index);

} // namespace Comprehend
} // namespace Glk

#endif
