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

#ifndef TETRAEDGE_TE_TE_JPEG_H
#define TETRAEDGE_TE_TE_JPEG_H

#include "common/str.h"
#include "tetraedge/te/te_scummvm_codec.h"

namespace Graphics {
struct Surface;
}

namespace Tetraedge {

class TeJpeg : public TeScummvmCodec {
public:
	TeJpeg();
	virtual ~TeJpeg();

	virtual bool load(Common::SeekableReadStream &stream) override;
	virtual int nbFrames() override { return 1; }
	virtual float frameRate() override { return 0.0f; }
	virtual bool isAtEnd() override  { return true; }
	virtual void setColorKeyActivated(bool val) override { }
	virtual void setColorKey(const TeColor &col) override { }
	virtual void setColorKeyTolerence(float val) override { }

	static bool matchExtension(const Common::String &extn);

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_JPEG_H
