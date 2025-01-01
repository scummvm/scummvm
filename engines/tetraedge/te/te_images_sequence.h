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

#ifndef TETRAEDGE_TE_TE_IMAGES_SEQUENCE_H
#define TETRAEDGE_TE_TE_IMAGES_SEQUENCE_H

#include "common/str.h"
#include "tetraedge/te/te_i_codec.h"

namespace Graphics {
struct Surface;
class ManagedSurface;
}

namespace Tetraedge {

class TeImagesSequence : public TeICodec {
public:
	TeImagesSequence();
	virtual ~TeImagesSequence();

	virtual bool load(const TetraedgeFSNode &node) override;
	virtual uint width() override { return _width; }
	virtual uint height() override { return _height; }
	virtual int nbFrames() override { return _files.size(); }
	virtual void setLeftBorderSize(uint val) override { }
	virtual uint leftBorderSize() override { return 0; }
	virtual void setRightBorderSize(uint val) override  { }
	virtual uint rightBorderSize() override { return 0; }
	virtual void setBottomBorderSize(uint val) override  { }
	virtual uint bottomBorderSize() override { return 0; }
	virtual void setTopBorderSize(uint val) override  { }
	virtual uint topBorderSize() override { return 0; }
	virtual TeImage::Format imageFormat() override;
	virtual float frameRate() override { return _frameRate; }
	virtual bool update(uint i, TeImage &imgout) override;
	virtual bool isAtEnd() override;
	virtual void setColorKeyActivated(bool val) override { }
	virtual void setColorKey(const TeColor &col) override { }
	virtual void setColorKeyTolerence(float val) override { }

	static bool matchExtension(const Common::String &extn);

private:
	float _frameRate;
	uint _width;
	uint _height;
	Common::Array<TetraedgeFSNode> _files;
	Common::Array<Graphics::ManagedSurface *> _cachedSurfaces;
	uint _curFrame;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_IMAGES_SEQUENCE_H
