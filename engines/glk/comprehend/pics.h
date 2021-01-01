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

enum ImageFlag {
	IMAGEF_REVERSE = 1 << 0,
	IMAGEF_NO_PAINTING = 1 << 1,
	IMAGEF_NO_FILL = IMAGEF_REVERSE | IMAGEF_NO_PAINTING
};

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
		uint _picIndex;
		DrawSurface *_drawSurface;
		Graphics::Font *_font;
		uint _drawFlags;

		uint16 _x;
		uint16 _y;
		uint32 _penColor;
		uint32 _fillColor;
		Shape _shape;

		uint16 _textX;
		uint16 _textY;

		ImageContext(DrawSurface *drawSurface, Graphics::Font *font, uint flags, uint picIndex) :
			_drawSurface(drawSurface), _font(font), _drawFlags(flags), _picIndex(picIndex),
			_x(0), _y(0), _penColor(G_COLOR_BLACK), _fillColor(G_COLOR_BLACK),
			_shape(SHAPE_CIRCLE_LARGE), _textX(0), _textY(0) {
		}

		uint32 getFillColor() const;
		void lineFixes();
	};

	struct ImageFile {
	private:
		Common::Array<uint16> _imageOffsets;
		Common::String _filename;

	private:
		bool doImageOp(ImageContext *ctx) const;
		uint16 imageGetOperand(ImageContext *ctx) const;
		void doResetOp(ImageContext *ctx, byte param) const;
	public:
		ImageFile() {}
		ImageFile(const Common::String &filename, bool isSingleImage = false);

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
