#ifndef GRAPHICS_TINYGL_ZBLIT_H_
#define GRAPHICS_TINYGL_ZBLIT_H_

#include "graphics/surface.h"
#include "common/rect.h"

struct BlitTransform {
	BlitTransform(int dstX, int dstY) {
		_destinationRectangle.translate(dstX,dstY);
		_sourceRectangle;
		_aTint = _rTint = _gTint = _bTint = 1.0f;
		_rotation = 0.0f;
		_originX = _originY = 0.0f;
		_flipHorizontally = _flipVertically = false;
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

	void scale(int width, int height) {
		_destinationRectangle.setWidth(width);
		_destinationRectangle.setHeight(height);
	}

	void rotate(int rotation, int originX, int originY) {
		_rotation = rotation;
		_originX = originX;
		_originY = originY;
	}

	void flip(bool verticalFlip, bool horizontalFlip) {
		_flipVertically = verticalFlip;
		_flipHorizontally = horizontalFlip;
	}

	Common::Rect _sourceRectangle;
	Common::Rect _destinationRectangle;
	int _rotation;
	int _originX, _originY;
	float _aTint, _rTint, _gTint, _bTint;
	bool _flipHorizontally, _flipVertically;
};

struct BlitImage;

BlitImage *tglGenBlitImage();
void tglUploadBlitImage(BlitImage *blitImage, const Graphics::Surface &surface, uint32 colorKey, bool applyColorKey);
void tglDeleteBlitImage(BlitImage *blitImage);

void tglBlit(BlitImage *blitImage, const BlitTransform &transform);

// Disables blending explicitly.
void tglBlitNoBlend(BlitImage *blitImage, const BlitTransform &transform);

// Disables blending, transforms and tinting.
void tglBlitFast(BlitImage *blitImage, int x, int y);

#endif // GRAPHICS_TINYGL_ZBLIT_H_
