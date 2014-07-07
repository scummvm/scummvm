#ifndef _tgl_zblit_h_
#define _tgl_zblit_h_

#include "graphics/surface.h"

struct BlitTransform {
	BlitTransform(int dstX, int dstY) {
		_dstX = dstX;
		_dstY = dstY;
		_srcX = _srcY = 0;
		_srcWidth = _srcHeight = _width = _height = 0;
		_aTint = _rTint = _gTint = _bTint = 1.0f;
		_rotation = 0.0f;
		_originX = _originY = 0.0f;
		_flipHorizontally = _flipVertically = false;
	}

	void sourceRectangle(int srcX, int srcY, int srcWidth, int srcHeight) {
		_srcX = srcX;
		_srcY = srcY;
		_srcWidth = srcWidth;
		_srcHeight = srcHeight;
	}

	void tint(float aTint, float rTint = 1.0f, float gTint = 1.0f, float bTint = 1.0f) {
		_aTint = aTint;
		_rTint = rTint;
		_gTint = gTint;
		_bTint = bTint;
	}

	void scale(int width, int height) {
		_width = width;
		_height = height;
	}

	void rotate(float rotation, float originX, float originY) {
		_rotation = rotation;
		_originX = originX;
		_originY = originY;
	}

	void flip(bool verticalFlip, bool horizontalFlip) {
		_flipVertically = verticalFlip;
		_flipHorizontally = horizontalFlip;
	}

	int _dstX, _dstY;
	int _srcX, _srcY;
	int _srcWidth, _srcHeight;
	int _width, _height;
	float _rotation;
	float _originX, _originY;
	float _aTint, _rTint, _gTint, _bTint;
	bool _flipHorizontally, _flipVertically;
};

int tglGenBlitTexture();
void tglUploadBlitTexture(int textureHandle, const Graphics::Surface& surface, int colorKey, bool applyColorKey);
void tglDeleteBlitTexture(int textureHandle);

void tglBlit(int blitTextureHandle, const BlitTransform &transform);
	
// Disables blending explicitly.
void tglBlitNoBlend(int blitTextureHandle, const BlitTransform &transform);
	
// Disables blending, transforms and tinting.
void tglBlitFast(int blitTextureHandle, int x, int y);

#endif
