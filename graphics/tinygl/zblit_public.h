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

#ifndef GRAPHICS_TINYGL_ZBLIT_PUBLIC_H
#define GRAPHICS_TINYGL_ZBLIT_PUBLIC_H

#include "common/rect.h"

#include "graphics/surface.h"

namespace TinyGL {

struct BlitTransform {
	BlitTransform(int dstX, int dstY) : _rotation(0), _originX(0), _originY(0), _aTint(1.0f),
	                                    _rTint(1.0f), _gTint(1.0f), _bTint(1.0), _flipHorizontally(false),
	                                    _flipVertically(false) {
		_destinationRectangle.translate(dstX, dstY);
	}

	void sourceRectangle(int srcX, int srcY, int srcWidth, int srcHeight) {
		_sourceRectangle.left = srcX;
		_sourceRectangle.top = srcY;
		_sourceRectangle.setWidth(srcWidth);
		_sourceRectangle.setHeight(srcHeight);
	}

	void tint(float aTint, float rTint = 1.0f, float gTint = 1.0f, float bTint = 1.0f) {
		_aTint = aTint;
		_rTint = rTint;
		_gTint = gTint;
		_bTint = bTint;
	}
/*
	void scale(int width, int height) {
		_destinationRectangle.setWidth(width);
		_destinationRectangle.setHeight(height);
	}
*/
	void rotate(int rotation, int originX, int originY) {
		_rotation = rotation;
		_originX = originX;
		_originY = originY;
	}

	void flip(bool verticalFlip, bool horizontalFlip) {
		_flipVertically = verticalFlip;
		_flipHorizontally = horizontalFlip;
	}

	bool operator==(const BlitTransform &other) const {
		return
			_sourceRectangle == other._sourceRectangle && _destinationRectangle == other._destinationRectangle &&
			_rotation == other._rotation && _originX == other._originX && _originY == other._originY &&
			_aTint == other._aTint && _rTint == other._rTint && _gTint == other._gTint && _bTint == other._bTint &&
			_flipHorizontally == other._flipHorizontally && _flipVertically == other._flipVertically;
	}

	Common::Rect _sourceRectangle;
	Common::Rect _destinationRectangle;
	int _rotation;
	int _originX, _originY;
	float _aTint, _rTint, _gTint, _bTint;
	bool _flipHorizontally, _flipVertically;
};

struct BlitImage;

} // end of namespace TinyGL

/**
@brief Generates a new blit image.
@return returns an opaque pointer to the blit image.
*/
TinyGL::BlitImage *tglGenBlitImage();

/**
@brief Copies a surface data into the provided blit image.
@param pointer to the blit image.
@param referece to the surface that's being copied
@param color key value for alpha color keying
@param boolean that enables alpha color keying
*/
void tglUploadBlitImage(TinyGL::BlitImage *blitImage, const Graphics::Surface &surface, uint32 colorKey, bool applyColorKey, bool zBuffer = false);

/**
@brief Destroys an instance of blit image.
@param pointer to the blit image.
*/
void tglDeleteBlitImage(TinyGL::BlitImage *blitImage);

/**
@brief Getter for current blit image width and height
@param pointer to the blit image.
@param reference to the width variable
@param reference to the height variable
*/
void tglGetBlitImageSize(TinyGL::BlitImage *blitImage, int &width, int &height);

/**
@brief Provides a way to check if the image has been updated.
@param pointer to the blit image.
@param boolean that enables alpha color keying
*/
int tglGetBlitImageVersion(TinyGL::BlitImage *blitImage);

/**
@brief Blits an image to the color buffer.
@param pointer to the blit image.
@param blit transform information.
*/
void tglBlit(TinyGL::BlitImage *blitImage, const TinyGL::BlitTransform &transform);

/**
@brief Blits an image to the color buffer.
@param pointer to the blit image.
@param x destination coordinate.
@param y destination coordinate.
*/
void tglBlit(TinyGL::BlitImage *blitImage, int x, int y);

/**
@brief Blits an image to the color buffer without performinc any type of blending, image transformation or tinting.
@param pointer to the blit image.
@param x destination coordinate.
@param y destination coordinate.
*/
void tglBlitFast(TinyGL::BlitImage *blitImage, int x, int y);

/**
@brief Blits an image to the depth buffer.
@param pointer to the blit image.
@param x destination coordinate.
@param y destination coordinate.
*/
void tglBlitZBuffer(TinyGL::BlitImage *blitImage, int x, int y);

void tglIncBlitImageRef(TinyGL::BlitImage *blitImage);

#endif // GRAPHICS_TINYGL_ZBLIT_PUBLIC_H
