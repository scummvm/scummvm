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

#ifndef TETRAEDGE_TE_TE_SCUMMVM_CODEC_H
#define TETRAEDGE_TE_TE_SCUMMVM_CODEC_H

#include "tetraedge/te/te_i_codec.h"

namespace Tetraedge {

/** Not part of the original engine - but PNG, JPEG, and TGA are all handled by ScummVM so we just wrap them simply. */
class TeScummvmCodec : public TeICodec {
public:
	TeScummvmCodec();
	virtual ~TeScummvmCodec();

	virtual bool load(const TetraedgeFSNode &node) override;
	virtual bool load(Common::SeekableReadStream &stream) = 0;
	virtual uint width() override;
	virtual uint height() override;
	virtual int nbFrames() override { return 1; }
	virtual TeImage::Format imageFormat() override;
	virtual void setLeftBorderSize(uint val) override;
	virtual uint leftBorderSize() override { return 0; }
	virtual void setRightBorderSize(uint val) override;
	virtual uint rightBorderSize() override { return 0; }
	virtual void setBottomBorderSize(uint val) override;
	virtual uint bottomBorderSize() override { return 0; }
	virtual void setTopBorderSize(uint val) override;
	virtual uint topBorderSize() override { return 0; }
	virtual float frameRate() override { return _frameRate; }
	virtual bool update(uint i, TeImage &imgout) override;
	virtual bool isAtEnd() override { return true; }
	virtual void setColorKeyActivated(bool val) override { }
	virtual void setColorKey(const TeColor &col) override { }
	virtual void setColorKeyTolerence(float val) override { }

protected:
	Graphics::Surface *_loadedSurface;
	Common::Path _loadedPath;
	float _frameRate;
	uint _bottomBorder;
	uint _topBorder;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SCUMMVM_CODEC_H
