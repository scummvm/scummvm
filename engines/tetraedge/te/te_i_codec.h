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

#ifndef TETRAEDGE_TE_TE_I_CODEC_H
#define TETRAEDGE_TE_TE_I_CODEC_H

#include "common/path.h"
#include "common/stream.h"

#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_signal.h"

namespace Tetraedge {

class TeICodec {
public:
	TeICodec() {};

	virtual ~TeICodec() {};
	virtual bool load(const TetraedgeFSNode &node) = 0;
	virtual uint width() = 0;
	virtual uint height() = 0;
	virtual int nbFrames() = 0;
	virtual TeImage::Format imageFormat() = 0;
	virtual void setLeftBorderSize(uint val) = 0;
	virtual uint leftBorderSize() = 0;
	virtual void setRightBorderSize(uint val) = 0;
	virtual uint rightBorderSize() = 0;
	virtual void setBottomBorderSize(uint val) = 0;
	virtual uint bottomBorderSize() = 0;
	virtual void setTopBorderSize(uint val) = 0;
	virtual uint topBorderSize() = 0;
	virtual float frameRate() = 0;
	virtual bool update(uint i, TeImage &imgout) = 0;
	virtual bool isAtEnd() = 0;
	virtual TeSignal0Param &onVideoFinished() { return _finishedSignal; };
	virtual void setColorKeyActivated(bool val) = 0;
	virtual void setColorKey(const TeColor &col) = 0;
	virtual void setColorKeyTolerence(float val) = 0;

private:
	TeSignal0Param _finishedSignal;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_I_CODEC_H
