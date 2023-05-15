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

#ifndef TETRAEDGE_TE_TE_PNG_H
#define TETRAEDGE_TE_TE_PNG_H

#include "common/str.h"
#include "tetraedge/te/te_scummvm_codec.h"

namespace Graphics {
struct Surface;
}

namespace Tetraedge {

class TePng : public TeScummvmCodec {
public:
	TePng(const Common::String &extn);
	virtual ~TePng();

	virtual bool load(const TetraedgeFSNode &node) override;
	virtual bool load(Common::SeekableReadStream &stream) override;

	TeImage::Format imageFormat() override;

	// We support "animated" PNGs which contain 8
	// frames stacked vertically.
	virtual int nbFrames() override { return _nbFrames; }
	virtual uint height() override { return _height; }
	virtual bool isAtEnd() override;
	virtual bool update(uint i, TeImage &imgout) override;

	static bool matchExtension(const Common::String &extn);

private:
	int _nbFrames;
	int16 _height;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_PNG_H
