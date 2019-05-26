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

#include "cryomni3d/omni3d.h"

#include "common/rect.h"

namespace CryOmni3D {

void Omni3DManager::init(double hfov) {
	_alpha = 0.;
	_beta = 0.;
	_xSpeed = 0.;
	_ySpeed = 0.;

	double oppositeSide = tan(hfov / 2.) / (4. / 3.);
	double vf = atan2(oppositeSide, 1.);
	_vfov = (M_PI_2 - vf - (13. / 180.*M_PI)) * 10. / 9.;

	double warpVfov = 155. / 180. * M_PI;
	double hypV = 768. / 2. / sin(warpVfov / 2.);
	double oppHTot = tan(hfov / 2.) * 16. / 320.;
	_helperValue = 2048 * 65536 / (2. * M_PI);

	for (int i = 0; i < 31; i++) {
		double oppH = (i - 15) * oppHTot;
		double angle = atan2(oppH, 1.);

		_anglesH[i] = angle;
		_hypothenusesH[i] = sqrt(oppH * oppH + 1);

		double oppVTot = hypV * _hypothenusesH[i];
		for (int j = 0; j < 21; j++) {
			double oppV = (j - 20) * oppHTot;

			_oppositeV[j] = oppV;

			double coord = sqrt(oppV * oppV + _hypothenusesH[i] * _hypothenusesH[i]);
			coord = oppVTot / coord;
			coord = coord * 65536;

			_squaresCoords[i][j] = coord;
		}
	}

	_surface.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	clearConstraints();
}

Omni3DManager::~Omni3DManager() {
	_surface.free();
}

void Omni3DManager::updateCoords(int xDelta, int yDelta, bool useOldSpeed) {
	double xDelta1 = xDelta * 0.00025;
	double yDelta1 = yDelta * 0.0002;

	if (useOldSpeed) {
		_xSpeed += xDelta1;
		_ySpeed += yDelta1;
	} else {
		_xSpeed = xDelta1;
		_ySpeed = yDelta1;
	}
	_alpha += _xSpeed;
	_beta += _ySpeed;

	//debug("alpha = %lf beta = %lf xSpeed = %lf ySpeed = %lf", _alpha, _beta, _xSpeed, _ySpeed);

	_xSpeed *= 0.4;
	_ySpeed *= 0.6;

	if (useOldSpeed) {
		if (fabs(_xSpeed) < 0.001) {
			_xSpeed = 0.;
		}
		if (fabs(_ySpeed) < 0.001) {
			_ySpeed = 0.;
		}
	}

	if (_alpha < _alphaMin) {
		_alpha = _alphaMin;
		_xSpeed = 0.;
	} else if (_alpha > _alphaMax) {
		_alpha = _alphaMax;
		_xSpeed = 0.;
	}
	if (_beta < _betaMin) {
		_beta = _betaMin;
		_ySpeed = 0.;
	} else if (_beta > _betaMax) {
		_beta = _betaMax;
		_ySpeed = 0.;
	}

	if (_alpha >= 2. * M_PI) {
		_alpha -= 2. * M_PI;
	} else if (_alpha < 0.) {
		_alpha += 2. * M_PI;
	}

	_dirtyCoords = true;

	updateImageCoords();
}

void Omni3DManager::updateImageCoords() {
	if (!_dirtyCoords) {
		return;
	}

	if (_alpha >= 2.*M_PI) {
		_alpha -= 2.*M_PI;
	} else if (_alpha < 0) {
		_alpha += 2.*M_PI;
	}
	if (_beta > 0.9 * _vfov) {
		_beta = 0.9 * _vfov;
	} else if (_beta < -0.9 * _vfov) {
		_beta = -0.9 * _vfov;
	}

	double tmp = (2048 * 65536) - 2048 * 65536 / (2. * M_PI) * _alpha;

	uint k = 0;
	for (uint i = 0; i < 31; i++) {
		double v11 = _anglesH[i] + _beta;
		double v26 = sin(v11);
		double v25 = cos(v11) * _hypothenusesH[i];

		uint offset = 80;
		uint j;
		for (j = 0; j < 20; j++) {
			double v16 = atan2(_oppositeV[j], v25);
			double v17 = v16 * _helperValue;
			double v18 = (384 * 65536) - _squaresCoords[i][j] * v26;

			k += 2;
			_imageCoords[k + 0] = (int)(tmp + v17);
			_imageCoords[k + offset + 0] = (int)(tmp - v17);
			_imageCoords[k + 1] = (int) v18;
			_imageCoords[k + offset + 1] = (int) v18;

			offset -= 4;
		}

		double v19 = atan2(_oppositeV[j], v25);

		k += 2;
		_imageCoords[k + 0] = (int)((2048.*65536.) - (_alpha - v19) * _helperValue);
		_imageCoords[k + 1] = (int)((384.*65536.) - _squaresCoords[i][j] * v26);

		k += 40;
	}

	_dirtyCoords = false;
	_dirty = true;
}

const Graphics::Surface *Omni3DManager::getSurface() {
	if (!_sourceSurface) {
		return nullptr;
	}

	if (_dirtyCoords) {
		updateImageCoords();
	}

	if (_dirty) {
		uint off = 2;
		byte *dst = (byte *)_surface.getBasePtr(0, 0);
		const byte *src = (const byte *)_sourceSurface->getBasePtr(0, 0);

		for (uint i = 0; i < 30; i++) {
			for (uint j = 0; j < 40; j++) {
				int x1  = (_imageCoords[off + 2] - _imageCoords[off + 0]) >> 4;
				int y1  = (_imageCoords[off + 3] - _imageCoords[off + 1]) >> 4;
				int x1_ = (_imageCoords[off + 82 + 2] - _imageCoords[off + 82 + 0]) >> 4;
				int y1_ = (_imageCoords[off + 82 + 3] - _imageCoords[off + 82 + 1]) >> 4;

				int dx1 = (x1_ - x1) >> 10;
				int dy1 = (y1_ - y1) >> 15;

				y1 >>= 5;

				int dx2  = (_imageCoords[off + 82 + 0] - _imageCoords[off + 0]) >> 4;
				int dy2  = (_imageCoords[off + 82 + 1] - _imageCoords[off + 1]) >> 9;
				int x2 = (((_imageCoords[off + 0] >> 0) * 2) + dx2) >> 1;
				int y2 = (((_imageCoords[off + 1] >> 5) * 2) + dy2) >> 1;

				for (uint y = 0; y < 16; y++) {
					uint px = (x2 * 2 + x1) * 16;
					uint py = (y2 * 2 + y1) / 2;
					uint deltaX = x1 * 32;
					uint deltaY = y1;

					for (uint x = 0; x < 16; x++) {
						uint srcOff = (py & 0x1ff800) | (px >> 21);
						dst[x] = src[srcOff];
						px += deltaX;
						py += deltaY;
					}
					dst += 640;

					x1 += dx1;
					y1 += dy1;
					x2 += dx2;
					y2 += dy2;
				}
				dst -= 16 * 640 - 16;
				off += 2;
			}
			dst += 15 * 640;
			off += 2;
		}

		_dirty = false;
	}

	return &_surface;
}

void Omni3DManager::clearConstraints() {
	_alphaMin = -HUGE_VAL;
	_alphaMax = HUGE_VAL;
	_betaMin = -HUGE_VAL;
	_betaMax = HUGE_VAL;
}

Common::Point Omni3DManager::mapMouseCoords(const Common::Point &mouse) {
	Common::Point pt;

	if (_dirtyCoords) {
		updateImageCoords();
	}

	int smallX = mouse.x & 0xf, squareX = mouse.x >> 4;
	int smallY = mouse.y & 0xf, squareY = mouse.y >> 4;

	uint off = 82 * squareY + 2 * squareX;

	pt.x = ((_imageCoords[off + 2] +
	         smallY * ((_imageCoords[off + 84] - _imageCoords[off + 2]) >> 4) +
	         (smallX * smallY) * ((_imageCoords[off + 86] - _imageCoords[off + 84]) >> 8) +
	         (smallX * (16 - smallY)) * ((_imageCoords[off + 4] - _imageCoords[off + 2]) >> 8))
	        & 0x07ff0000) >> 16;
	pt.y = (_imageCoords[off + 3] +
	        smallY * ((_imageCoords[off + 85] - _imageCoords[off + 3]) >> 4) +
	        (smallX * smallY) * ((_imageCoords[off + 87] - _imageCoords[off + 85]) >> 8) +
	        (smallX * (16 - smallY)) * ((_imageCoords[off + 5] - _imageCoords[off + 3]) >> 8)) >> 16;

	return pt;
}

} // End of namespace CryOmni3D
