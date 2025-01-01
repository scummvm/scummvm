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

#ifndef TETRAEDGE_TE_TE_TILED_SURFACE_H
#define TETRAEDGE_TE_TE_TILED_SURFACE_H

#include "common/ptr.h"
#include "common/path.h"

#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_frame_anim.h"
#include "tetraedge/te/te_image.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_tiled_texture.h"
#include "tetraedge/te/te_i_codec.h"
#include "tetraedge/te/te_intrusive_ptr.h"

namespace Tetraedge {

class TeTiledSurface : public TeModel {
public:
	TeTiledSurface();
	virtual ~TeTiledSurface();

	virtual int bufferSize() { return 1; } // unused?
	void cont();
	void draw() override;
	virtual void entry() {};
	byte isLoaded();
	bool load(const TetraedgeFSNode &node);
	bool load(const TeImage &image);
	bool load(const TeIntrusivePtr<Te3DTexture> &texture);

	bool onFrameAnimCurrentFrameChanged();
	void pause();
	void play();

	void setBottomCropping(float val) {
		_bottomCrop = val;
		updateSurface();
	}
	void setLeftCropping(float val) {
		_leftCrop = val;
		updateSurface();
	}
	void setRightCropping(float val) {
		_rightCrop = val;
		updateSurface();
	}
	void setTopCropping(float val) {
		_topCrop = val;
		updateSurface();
	}

	virtual void setBufferSize(long bufSz) {}; // unused?

	void setColorKey(const TeColor &col);
	void setColorKeyActivated(bool val);
	void setColorKeyTolerence(float val);
	void setTiledTexture(const TeIntrusivePtr<TeTiledTexture> &texture);

	void stop();
	void unload();
	void update(const TeImage &image);
	void updateSurface();
	void updateVideoProperties();

	const Common::Path &loadedPath() const { return _loadedPath; }
	void setLoadedPath(const Common::Path &p) { _loadedPath = p; }

	TeFrameAnim _frameAnim;

	TeICodec *codec() { return _codec; }

private:
	float _bottomCrop;
	float _leftCrop;
	float _rightCrop;
	float _topCrop;

	TeICodec *_codec;

	TeColor _colorKey;
	bool _colorKeyActive;
	float _colorKeyTolerence;

	bool _shouldDraw;

	TeImage::Format _imgFormat;

	Common::Path _loadedPath;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TILED_SURFACE_H
