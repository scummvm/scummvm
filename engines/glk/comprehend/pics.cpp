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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/pics.h"
#include "glk/comprehend/draw_surface.h"
#include "common/memstream.h"

namespace Glk {
namespace Comprehend {

#define IMAGES_PER_FILE	16

/*-------------------------------------------------------*/

Pics::ImageFile::ImageFile(const Common::String &filename) {
	Common::File f;
	uint16 version;
	int i;

	_filename = filename;
	if (!f.open(filename))
		error("Could not open file - %s", filename.c_str());

	/*
	 * In earlier versions of Comprehend the first word is 0x1000 and
	 * the image offsets start four bytes in. In newer versions the
	 * image offsets start at the beginning of the image file.
	 */
	version = f.readUint16LE();
	if (version == 0x1000)
		f.seek(4);
	else
		f.seek(0);

	// Get the image offsets in the file
	_imageOffsets.resize(IMAGES_PER_FILE);
	for (i = 0; i < IMAGES_PER_FILE; i++) {
		_imageOffsets[i] = f.readUint16LE();
		if (version == 0x1000)
			_imageOffsets[i] += 4;
	}
}

void Pics::ImageFile::draw(uint index, ImageContext *ctx) const {
	if (!ctx->_file.open(_filename))
		error("Opening image file");

	ctx->_file.seek(_imageOffsets[index]);

	for (bool done = false; !done;) {
		done = doImageOp(ctx);
	}
}

bool Pics::ImageFile::doImageOp(Pics::ImageContext *ctx) const {
	uint8 opcode;
	uint16 a, b;

	opcode = ctx->_file.readByte();
	debugCN(kDebugGraphics, "  %.4x [%.2x]: ", ctx->_file.pos() - 1, opcode);

	switch (opcode) {
	case IMAGE_OP_SCENE_END:
	case IMAGE_OP_EOF:
		debugC(kDebugGraphics, "end");
		return true;

	case IMAGE_OP_PEN_COLOR_A:
	case IMAGE_OP_PEN_COLOR_B:
	case IMAGE_OP_PEN_COLOR_C:
	case IMAGE_OP_PEN_COLOR_D:
	case IMAGE_OP_PEN_COLOR_E:
	case IMAGE_OP_PEN_COLOR_F:
	case IMAGE_OP_PEN_COLOR_G:
	case IMAGE_OP_PEN_COLOR_H:
		debugC(kDebugGraphics, "set_pen_color(%.2x)", opcode);
		ctx->_penColor = ctx->_drawSurface->getPenColor(opcode);
		break;

	case IMAGE_OP_DRAW_LINE:
	case IMAGE_OP_DRAW_LINE_FAR:
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);

		if (opcode & 0x1)
			a += 255;

		debugC(kDebugGraphics, "draw_line (%d, %d) - (%d, %d)",
			ctx->_x, ctx->_y, a, b);
		ctx->_drawSurface->drawLine(ctx->_x, ctx->_y, a, b, ctx->_penColor);

		ctx->_x = a;
		ctx->_y = b;
		break;

	case IMAGE_OP_DRAW_BOX:
	case IMAGE_OP_DRAW_BOX_FAR:
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);

		if (opcode & 0x1)
			a += 255;

		debugC(kDebugGraphics, "draw_box (%d, %d) - (%d, %d)",
			ctx->_x, ctx->_y, a, b);

		ctx->_drawSurface->drawBox(ctx->_x, ctx->_y, a, b, ctx->_penColor);
		break;

	case IMAGE_OP_MOVE_TO:
	case IMAGE_OP_MOVE_TO_FAR:
		/* Move to */
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);

		if (opcode & 0x1)
			a += 255;

		debugC(kDebugGraphics, "move_to(%d, %d)", a, b);
		ctx->_x = a;
		ctx->_y = b;
		break;

	case IMAGE_OP_SHAPE_PIXEL:
	case IMAGE_OP_SHAPE_BOX:
	case IMAGE_OP_SHAPE_CIRCLE_TINY:
	case IMAGE_OP_SHAPE_CIRCLE_SMALL:
	case IMAGE_OP_SHAPE_CIRCLE_MED:
	case IMAGE_OP_SHAPE_CIRCLE_LARGE:
	case IMAGE_OP_SHAPE_A:
	case IMAGE_OP_SHAPE_SPRAY:
		debugC(kDebugGraphics,
		             "set_shape_type(%.2x)", opcode - 0x40);
		ctx->_shape = opcode;
		break;

	case 0x48:
		/*
		 * FIXME - This appears to be a _shape type. Only used by
		 *         OO-Topos.
		 */
		debugC(kDebugGraphics, "shape_unknown()");
		ctx->_shape = IMAGE_OP_SHAPE_PIXEL;
		break;

	case IMAGE_OP_DRAW_SHAPE:
	case IMAGE_OP_DRAW_SHAPE_FAR:
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);

		if (opcode & 0x1)
			a += 255;

		debugC(kDebugGraphics, "draw_shape(%d, %d), style=%.2x, fill=%.2x",
		             a, b, ctx->_shape, ctx->_fillColor);

		ctx->_drawSurface->drawShape(a, b, ctx->_shape, ctx->_fillColor);
		break;

	case IMAGE_OP_PAINT:
	case IMAGE_OP_PAINT_FAR:
		/* Paint */
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);

		if (opcode & 0x1)
			a += 255;

		debugC(kDebugGraphics, "paint(%d, %d)", a, b);
		if (!(ctx->_drawFlags & IMAGEF_NO_FLOODFILL))
			ctx->_drawSurface->floodFill(a, b, ctx->_fillColor,
			            ctx->_drawSurface->getPixelColor(a, b));
		break;

	case IMAGE_OP_FILL_COLOR:
		a = imageGetOperand(ctx);
		debugC(kDebugGraphics, "set_fill_color(%.2x)", a);
		ctx->_fillColor = ctx->_drawSurface->getFillColor(a);
		break;

	case IMAGE_OP_SET_TEXT_POS:
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);
		debugC(kDebugGraphics, "set_text_pos(%d, %d)", a, b);

		ctx->_textX = a;
		ctx->_textY = b;
		break;

	case IMAGE_OP_DRAW_CHAR:
		a = imageGetOperand(ctx);
		debugC(kDebugGraphics, "draw_char(%c)",
		             a >= 0x20 && a < 0x7f ? a : '?');

		ctx->_drawSurface->drawBox(ctx->_textX, ctx->_textY,
		           ctx->_textX + 6, ctx->_textY + 7, ctx->_fillColor);
		ctx->_textX += 8;
		break;

	case 0xf3:
		/*
		 * FIXME - Oo-Topos uses this at the beginning of some room
		 *         images.
		 */
		debugC(kDebugGraphics, "unknown()");
		break;

	case 0xb5:
	case 0x82:
	case 0x50:
		/* FIXME - unknown, no arguments */
		debugC(kDebugGraphics, "unknown");
		break;

	case 0x73:
	case 0xb0:
	case 0xd0:
		/* FIXME - unknown, one argument */
		a = imageGetOperand(ctx);
		debugC(kDebugGraphics, "unknown %.2x: (%.2x) '%c'",
		             opcode, a,
		             a >= 0x20 && a < 0x7f ? a : '?');
		break;

	default:
		/* FIXME - Unknown, two arguments */
		a = imageGetOperand(ctx);
		b = imageGetOperand(ctx);

		debugC(kDebugGraphics, "unknown(%.2x, %.2x)", a, b);
		ctx->_drawSurface->drawPixel(a, b, 0x00ff00ff);
		break;
	}

	return false;
}

uint16 Pics::ImageFile::imageGetOperand(ImageContext *ctx) const {
	return ctx->_file.readByte();
}

/*-------------------------------------------------------*/

void Pics::clear() {
	_rooms.clear();
	_items.clear();
}

void Pics::load(const Common::StringArray &roomFiles,
                const Common::StringArray &itemFiles) {
	clear();

	for (uint idx = 0; idx < roomFiles.size(); ++idx)
		_rooms.push_back(ImageFile(roomFiles[idx]));
	for (uint idx = 0; idx < itemFiles.size(); ++idx)
		_items.push_back(ImageFile(itemFiles[idx]));
}

int Pics::getPictureNumber(const Common::String &filename) const {
	// Ensure prefix and suffix
	if (!filename.hasPrefixIgnoreCase("pic") ||
	    !filename.hasSuffixIgnoreCase(".raw"))
		return -1;

	// Get the number part
	Common::String num(filename.c_str() + 3, filename.size() - 7);
	if (num.empty() || !Common::isDigit(num[0]))
		return -1;

	return atoi(num.c_str());
}

bool Pics::hasFile(const Common::String &name) const {
	int num = getPictureNumber(name);
	if (num == -1)
		return false;

	if (num == DARK_ROOM || num == BRIGHT_ROOM)
		return true;
	if (num >= ITEMS_OFFSET && num < (int)(ITEMS_OFFSET + _items.size() * IMAGES_PER_FILE))
		return true;
	if (num < ITEMS_OFFSET && (num % 100) < (int)(_rooms.size() * IMAGES_PER_FILE))
		return true;

	return false;
}

int Pics::listMembers(Common::ArchiveMemberList &list) const {
	return list.size();
}

const Common::ArchiveMemberPtr Pics::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *Pics::createReadStreamForMember(const Common::String &name) const {
	// Get the picture number
	int num = getPictureNumber(name);
	if (num == -1 || !hasFile(name))
		return nullptr;

	// Draw the image
	drawPicture(num);

	// Create a stream with the data for the surface
	Common::MemoryReadWriteStream *stream =
	    new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	const DrawSurface &ds = *g_comprehend->_drawSurface;
	stream->writeUint16LE(ds.w);
	stream->writeUint16LE(ds.h);
	stream->writeUint16LE(0);		// Palette size
	stream->write(ds.getPixels(), ds.w * ds.h * 4);

	return stream;
}

void Pics::drawPicture(int pictureNum) const {
	ImageContext ctx(g_comprehend->_drawSurface, g_comprehend->_drawFlags);

	if (pictureNum == DARK_ROOM) {
		ctx._drawSurface->clearScreen(G_COLOR_BLACK);

	} else if (pictureNum == BRIGHT_ROOM) {
		ctx._drawSurface->clearScreen(G_COLOR_WHITE);

	} else if (pictureNum >= ITEMS_OFFSET) {
		_items[pictureNum / IMAGES_PER_FILE].draw(
		    pictureNum % IMAGES_PER_FILE, &ctx);

	} else {
		if (pictureNum < LOCATIONS_NO_BG_OFFSET)
			ctx._drawSurface->clearScreen(G_COLOR_WHITE);

		pictureNum %= 100;
		_rooms[pictureNum / IMAGES_PER_FILE].draw(
			pictureNum % IMAGES_PER_FILE, &ctx);
	}
}

} // namespace Comprehend
} // namespace Glk
