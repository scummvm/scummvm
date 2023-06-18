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

#ifndef TETRAEDGE_TE_TE_THEORA_H
#define TETRAEDGE_TE_TE_THEORA_H

#include "common/str.h"
#include "tetraedge/te/te_i_codec.h"

namespace Video {
class TheoraDecoder;
}

namespace Tetraedge {

class TeTheora : public TeICodec {
public:
	TeTheora();
	virtual ~TeTheora();

	virtual bool load(const TetraedgeFSNode &node) override;
	virtual uint width() override;
	virtual uint height() override;
	virtual int nbFrames() override;
	virtual TeImage::Format imageFormat() override;
	virtual void setLeftBorderSize(uint val) override;
	virtual uint leftBorderSize() override;
	virtual void setRightBorderSize(uint val) override;
	virtual uint rightBorderSize() override;
	virtual void setBottomBorderSize(uint val) override;
	virtual uint bottomBorderSize() override;
	virtual void setTopBorderSize(uint val) override;
	virtual uint topBorderSize() override;
	virtual float frameRate() override;
	virtual bool update(uint i, TeImage &imgout) override;
	virtual bool isAtEnd() override;
	virtual void setColorKeyActivated(bool val) override;
	virtual void setColorKey(const TeColor &col) override;
	virtual void setColorKeyTolerence(float val) override;

	static bool matchExtension(const Common::String &extn);

private:
	Video::TheoraDecoder *_decoder;

	TetraedgeFSNode _loadedNode;
	bool _hitEnd;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_THEORA_H
