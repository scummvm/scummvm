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

#include "glk/comprehend/pics.h"
#include "common/memstream.h"
#include "glk/comprehend/charset.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_data.h"

namespace Glk {
namespace Comprehend {

#define IMAGES_PER_FILE 16

enum Opcode {
	OPCODE_END = 0,
	OPCODE_SET_TEXT_POS = 1,
	OPCODE_SET_PEN_COLOR = 2,
	OPCODE_TEXT_CHAR = 3,
	OPCODE_SET_SHAPE = 4,
	OPCODE_TEXT_OUTLINE = 5,
	OPCODE_SET_FILL_COLOR = 6,
	OPCODE_END2 = 7,
	OPCODE_MOVE_TO = 8,
	OPCODE_DRAW_BOX = 9,
	OPCODE_DRAW_LINE = 10,
	OPCODE_DRAW_CIRCLE = 11,
	OPCODE_DRAW_SHAPE = 12,
	OPCODE_DELAY = 13,
	OPCODE_PAINT = 14,
	OPCODE_RESET = 15
};

enum SpecialOpcode {
	RESETOP_0 = 0,
	RESETOP_RESET = 1,
	RESETOP_OO_TOPOS_UNKNOWN = 3
};

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
	if (version == 0x6300 /* Single image file */
			|| version == 0x81f3 /* OO-Topos title - t0 */) {
		_imageOffsets.resize(1);
		_imageOffsets[0] = 4;
		return;
	}

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

	byte param = opcode & 0xf;
	opcode >>= 4;

	switch (opcode) {
	case OPCODE_END:
	case OPCODE_END2:
		// End of the rendering
		debugC(kDebugGraphics, "End of image");
		return true;

	case OPCODE_SET_TEXT_POS:
		a = imageGetOperand(ctx) + (param & 1 ? 256 : 0);
		b = imageGetOperand(ctx);
		debugC(kDebugGraphics, "set_text_pos(%d, %d)", a, b);

		ctx->_textX = a;
		ctx->_textY = b;
		break;

	case OPCODE_SET_PEN_COLOR:
		debugC(kDebugGraphics, "set_pen_color(%.2x)", opcode);
		if (!(ctx->_drawFlags & IMAGEF_NO_FILL))
			ctx->_penColor = ctx->_drawSurface->getPenColor(param);
		break;

	case OPCODE_TEXT_CHAR:
	case OPCODE_TEXT_OUTLINE:
		// Text outline mode draws a bunch of pixels that sort of looks like the char
		// TODO: See if the outline mode is ever used
		if (opcode == OPCODE_TEXT_OUTLINE)
			warning("TODO: Implement drawing text outlines");

		a = imageGetOperand(ctx);
		if (a < 0x20 || a >= 0x7f) {
			warning("Invalid character - %c", a);
			a = '?';
		}

		debugC(kDebugGraphics, "draw_char(%c)", a);
		ctx->_font->drawChar(ctx->_drawSurface, a, ctx->_textX, ctx->_textY, ctx->_fillColor);
		ctx->_textX += ctx->_font->getCharWidth(a);
		break;

	case OPCODE_SET_SHAPE:
		debugC(kDebugGraphics, "set_shape_type(%.2x)", param);

		if (param == 8) {
			// FIXME: This appears to be a _shape type. Only used by OO-Topos
			warning("TODO: Shape type 8");
			ctx->_shape = SHAPE_PIXEL;
		} else {
			ctx->_shape = (Shape)param;
		}
		break;

	case OPCODE_SET_FILL_COLOR:
		a = imageGetOperand(ctx);
		debugC(kDebugGraphics, "set_fill_color(%.2x)", a);
		ctx->_fillColor = ctx->_drawSurface->getFillColor(a);
		break;

	case OPCODE_MOVE_TO:
		a = imageGetOperand(ctx) + (param & 1 ? 256 : 0);
		b = imageGetOperand(ctx);

		debugC(kDebugGraphics, "move_to(%d, %d)", a, b);
		ctx->_x = a;
		ctx->_y = b;
		break;

	case OPCODE_DRAW_BOX:
		a = imageGetOperand(ctx) + (param & 1 ? 256 : 0);
		b = imageGetOperand(ctx);

		debugC(kDebugGraphics, "draw_box (%d, %d) - (%d, %d)",
		       ctx->_x, ctx->_y, a, b);

		ctx->_drawSurface->drawBox(ctx->_x, ctx->_y, a, b, ctx->_penColor);
		break;

	case OPCODE_DRAW_LINE:
		a = imageGetOperand(ctx) + (param & 1 ? 256 : 0);
		b = imageGetOperand(ctx);

		debugC(kDebugGraphics, "draw_line (%d, %d) - (%d, %d)",
		       ctx->_x, ctx->_y, a, b);
		ctx->_drawSurface->drawLine(ctx->_x, ctx->_y, a, b, ctx->_penColor);

		ctx->_x = a;
		ctx->_y = b;
		break;

	case OPCODE_DRAW_CIRCLE:
		a = imageGetOperand(ctx);
		debugC(kDebugGraphics, "draw_circle (%d, %d) diameter=%d",
		       ctx->_x, ctx->_y, a);

		ctx->_drawSurface->drawCircle(ctx->_x, ctx->_y, a, ctx->_penColor);
		break;

	case OPCODE_DRAW_SHAPE:
		a = imageGetOperand(ctx) + (param & 1 ? 256 : 0);
		b = imageGetOperand(ctx);
		debugC(kDebugGraphics, "draw_shape(%d, %d), style=%.2x, fill=%.2x",
		       a, b, ctx->_shape, ctx->_fillColor);

		if (!(ctx->_drawFlags & IMAGEF_NO_FILL))
			ctx->_drawSurface->drawShape(a, b, ctx->_shape, ctx->_fillColor);
		break;

	case OPCODE_DELAY:
		// The original allowed for rendering to be paused briefly. We don't do
		// that in ScummVM, and just show the finished rendered image
		(void)imageGetOperand(ctx);
		break;

	case OPCODE_PAINT:
		a = imageGetOperand(ctx) + (param & 1 ? 256 : 0);
		b = imageGetOperand(ctx);

		if (opcode & 0x1)
			a += 255;

		debugC(kDebugGraphics, "paint(%d, %d)", a, b);
		if (!(ctx->_drawFlags & IMAGEF_NO_FILL))
			ctx->_drawSurface->floodFill(a, b, ctx->_fillColor);
		break;

	case OPCODE_SPECIAL:
		a = imageGetOperand(ctx);
		doResetOp(ctx, a);
		break;
	}

	return false;
}

void Pics::ImageFile::doResetOp(ImageContext *ctx, byte param) const {
	switch (param) {
	case RESETOP_0:
		// In Transylvania this sub-opcode is a do nothing
		break;

	case RESETOP_RESET:
		// TODO: Calls same reset that first gets called when rendering starts.
		// Figure out what the implication of resetting the variables does
		break;

	case RESETOP_OO_TOPOS_UNKNOWN:
		// TODO: This is called for some scenes in OO-Topis. Figure out what it does
		break;

	default:
		break;
	}
}

uint16 Pics::ImageFile::imageGetOperand(ImageContext *ctx) const {
	return ctx->_file.readByte();
}

/*-------------------------------------------------------*/

Pics::Pics() : _font(nullptr) {
	if (Common::File::exists("charset.gda"))
		_font = new CharSet();
}

Pics::~Pics() {
	delete _font;
}

void Pics::clear() {
	_rooms.clear();
	_items.clear();
}

void Pics::load(const Common::StringArray &roomFiles,
                const Common::StringArray &itemFiles,
                const Common::String &titleFile) {
	clear();

	for (uint idx = 0; idx < roomFiles.size(); ++idx)
		_rooms.push_back(ImageFile(roomFiles[idx]));
	for (uint idx = 0; idx < itemFiles.size(); ++idx)
		_items.push_back(ImageFile(itemFiles[idx]));

	if (!titleFile.empty())
		_title = ImageFile(titleFile);
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

	if (num == DARK_ROOM || num == BRIGHT_ROOM || num == TITLE_IMAGE)
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
	stream->writeUint16LE(0); // Palette size
	stream->write(ds.getPixels(), ds.w * ds.h * 4);

	return stream;
}

void Pics::drawPicture(int pictureNum) const {
	ImageContext ctx(g_comprehend->_drawSurface, _font, g_comprehend->_drawFlags);

	if (pictureNum == DARK_ROOM) {
		ctx._drawSurface->clearScreen(G_COLOR_BLACK);

	} else if (pictureNum == BRIGHT_ROOM) {
		ctx._drawSurface->clearScreen(G_COLOR_WHITE);

	} else if (pictureNum == TITLE_IMAGE) {
		ctx._drawSurface->clearScreen(G_COLOR_WHITE);
		_title.draw(0, &ctx);

	} else if (pictureNum >= ITEMS_OFFSET) {
		pictureNum -= ITEMS_OFFSET;
		ctx._drawSurface->clear(0);
		_items[pictureNum / IMAGES_PER_FILE].draw(
		    pictureNum % IMAGES_PER_FILE, &ctx);

	} else {
		if (pictureNum < LOCATIONS_NO_BG_OFFSET) {
			ctx._drawSurface->clearScreen((ctx._drawFlags & IMAGEF_REVERSE) ? G_COLOR_BLACK : G_COLOR_WHITE);
			if (ctx._drawFlags & IMAGEF_REVERSE)
				ctx._penColor = RGB(255, 255, 255);
		} else {
			ctx._drawSurface->clear(0);
		}
		pictureNum %= 100;
		_rooms[pictureNum / IMAGES_PER_FILE].draw(
		    pictureNum % IMAGES_PER_FILE, &ctx);
	}
}

} // namespace Comprehend
} // namespace Glk
