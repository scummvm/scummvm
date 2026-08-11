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

#ifndef TETRAEDGE_TE_TE_FONT2_H
#define TETRAEDGE_TE_TE_FONT2_H

#include "common/str.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "graphics/font.h"

#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_i_font.h"

namespace Tetraedge {
	struct KernChars {
		uint32 _c1;
		uint32 _c2;
	};
	bool operator==(const KernChars &l, const KernChars &r);
}

namespace Common {
	template<> struct Hash<Tetraedge::KernChars> : public UnaryFunction<Tetraedge::KernChars, uint> {
		uint operator()(Tetraedge::KernChars val) const { return val._c1 * 7333 + val._c2; }
	};
}

namespace Tetraedge {

/**
 * A pre-rendered font format with positioning data used in Amerzone
 * ('tef' format)
 */
class TeFont2 : public TeIFont, public Graphics::Font {
public:

	struct GlyphData2 {
		float _xSz;
		float _ySz;
		float _xOff; // from nominal location
		float _yOff; // top location, from baseline
		float _xAdvance;
		float _floats[3];
		TeVector3f32 _vec; // location in texture - offset from bottom left
	};

	TeFont2();
	virtual ~TeFont2();

	bool load(const Common::Path &path);
	bool load(const TetraedgeFSNode &node);
	void unload();

	Graphics::Font *getAtSize(uint size) override;

	virtual int getFontHeight() const override;
	virtual int getMaxCharWidth() const override;
	virtual int getCharWidth(uint32 chr) const override;
	virtual void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	virtual int getKerningOffset(uint32 left, uint32 right) const override;
	virtual Common::Rect getBBox(const Common::String &str, int fontSize) override;
	virtual Common::Rect getBoundingBox(uint32 chr) const override;
	virtual TeVector3f32 kerning(uint pxSize, uint isocode1, uint isocode2) override;

	virtual float height(uint pxSize) override;

private:
	Common::Path _loadedPath;

	uint32 _numChars;
	TeVector2s32 _somePt;
	TeVector3f32 _someVec;
	bool _hasKernData;
	float _maxHeight;
	// Records a map of character pairs to kerning offsets
	Common::HashMap<KernChars, TeVector3f32> _kernData;
	Common::Array<uint32> _uintArray;
	Common::Array<GlyphData2> _glyphs;
	TeImage _texture;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_FONT2_H
