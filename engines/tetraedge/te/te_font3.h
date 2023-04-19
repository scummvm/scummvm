/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_TE_TE_FONT3_H
#define TETRAEDGE_TE_TE_FONT3_H

#include "common/file.h"
#include "common/str.h"
#include "common/path.h"
#include "common/types.h"
#include "common/hashmap.h"
#include "common/rect.h"

#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_3d_texture.h"

struct FT_FaceRec_;
struct FT_LibraryRec_;

namespace Graphics {
class Font;
}

namespace Tetraedge {

class TeFont3 : public TeResource {
public:
	TeFont3();
	~TeFont3();

	enum AlignStyle {
		AlignLeft,
		AlignRight,
		AlignJustify,
		AlignCenter
	};

	struct GlyphData {
		uint32 _charcode;
		Common::Rect _bitmapSize;
		TeIntrusivePtr<TeImage> _img;
	};

	bool load(const Common::String &path);
	bool load(const Common::FSNode &node);
	void unload();

	GlyphData glyph(uint size, uint charcode);

	float ascender(uint pxSize);
	float descender(uint pxSize);
	float height(uint pxSize);
	TeVector3f32 kerning(uint pxSize, uint isocode1, uint isocode2);
	TeIntrusivePtr<Te3DTexture> getFontSizeData(int size) const {
		return _fontSizeData[size];
	}

	Common::Rect getBoundingBox(const Common::String &str, int fontSize);
	int getHeight(int fontSize);

	void draw(TeImage &destImage, const Common::String &str, int fontSize, int yoff, const TeColor &col, AlignStyle alignMode);

	int wordWrapText(const Common::String &str, int fontSize, int maxWidth, Common::Array<Common::String> &lines);

private:
	void init();
	Graphics::Font *getAtSize(uint size);
	Common::CodePage codePage() const;

	Common::CodePage _codePage;
	Common::File _fontFile;
	Common::HashMap<uint, Graphics::Font *> _fonts;
	Common::String _loadedPath;
	Common::HashMap<uint, TeIntrusivePtr<Te3DTexture>> _fontSizeData;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_FONT3_H
