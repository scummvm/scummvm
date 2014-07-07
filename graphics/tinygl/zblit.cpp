#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/pixelbuffer.h"
#include "common/array.h"

struct BlitImage {
public:
	BlitImage() { }

	void loadData(const Graphics::Surface &surface, uint32 colorKey, bool applyColorKey) {
		Graphics::PixelFormat textureFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		_surface.create(surface.w, surface.h, textureFormat);
		Graphics::PixelBuffer buffer(surface.format, (byte *)surface.getPixels());
		Graphics::PixelBuffer dataBuffer(textureFormat, (byte *)_surface.getPixels());
		for (int x = 0;  x < surface.w; x++) {
			for (int y = 0; y < surface.h; y++) {
				uint32 pixel = buffer.getValueAt(y * surface.w + x);
				if (applyColorKey && pixel == colorKey) {
					dataBuffer.setPixelAt(y * surface.w + x, 0, 255, 255, 255); // Color keyed pixels become transparent white.
				} else {
					dataBuffer.setPixelAt(y * surface.w + x, pixel);
				}
			}
		}
		// Create opaque lines data.
	}

	~BlitImage() {
		_surface.free();
	}

	Graphics::Surface _surface;
};

static Common::Array<BlitImage*> blitImages;

BlitImage *tglGenBlitImage() {
	BlitImage *image = new BlitImage();
	blitImages.push_back(image);
	return image;
}

void tglUploadBlitImage(BlitImage *blitImage, const Graphics::Surface& surface, uint32 colorKey, bool applyColorKey) {
	if (blitImage != nullptr) {
		blitImage->loadData(surface, colorKey, applyColorKey);
	}
}

void tglDeleteBlitImage(BlitImage *blitImage) {
	if (blitImage != nullptr) {
		for (uint32 i = 0; i < blitImages.size(); i++) {
			if (blitImages[i] == blitImage) {
				blitImages.remove_at(i);
				break;
			}
		}
		delete blitImage;
	}
}

template <bool disableBlending, bool disableColoring, bool disableTransform, bool flipVertical, bool flipHorizontal>
void tglBlitGeneric(BlitImage *blitImage, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float rotation,
					float originX, float originY, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	if (srcWidth == 0 || srcHeight == 0) {
		srcWidth = blitImage->_surface.w;
		srcHeight = blitImage->_surface.h;
	}

	if (width == 0 && height == 0) {
		width = srcWidth;
		height = srcHeight;
	}

	if (dstX >= c->fb->xsize|| dstY >= c->fb->ysize)
		return;

	int clampWidth, clampHeight;

	if (dstX + width > c->fb->xsize)
		clampWidth = c->fb->xsize - dstX;
	else
		clampWidth = width;

	if (dstY + height > c->fb->ysize)
		clampHeight = c->fb->ysize - dstY;
	else
		clampHeight = height;

	if (dstX < 0) 
		dstX = 0;
	if (dstY < 0)
		dstY = 0;


	Graphics::PixelBuffer srcBuf(blitImage->_surface.format, (byte *)blitImage->_surface.getPixels());

	if (flipVertical) {
		srcBuf.shiftBy(srcX + ((srcY + srcHeight - 1) * blitImage->_surface.w));
	} else {
		srcBuf.shiftBy(srcX + (srcY * blitImage->_surface.w));
	}

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());

	for (int l = 0; l < clampHeight; l++) {
		for (int r = 0; r < clampWidth; ++r) {
			byte aDst, rDst, gDst, bDst;
			if (flipHorizontal) {
				srcBuf.getARGBAt(clampWidth - r, aDst, rDst, gDst, bDst);
			} else {
				srcBuf.getARGBAt(r, aDst, rDst, gDst, bDst);
			}
			if (disableColoring) {
				if (disableBlending && aDst != 0) {
					dstBuf.setPixelAt((dstX + r) + (dstY + l) * c->fb->xsize, aDst, rDst, gDst, bDst);
				} else {
					c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst, rDst, gDst, bDst);
				}
			} else {
				if (disableBlending && aDst != 0) {
					dstBuf.setPixelAt((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				} else {
					c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				}
			}
		}
		if (flipVertical) {
			srcBuf.shiftBy(-blitImage->_surface.w);
		} else {
			srcBuf.shiftBy(blitImage->_surface.w);
		}
	}
}

//Utility function.
template <bool disableBlending, bool disableColoring, bool disableTransform, bool flipVertical, bool flipHorizontal>
FORCEINLINE void tglBlitGeneric(BlitImage *blitImage, const BlitTransform &transform) {
	tglBlitGeneric<disableBlending, disableColoring, disableTransform, flipVertical, flipHorizontal>(blitImage, transform._destinationRectangle.left, transform._destinationRectangle.top,
		transform._destinationRectangle.width(), transform._destinationRectangle.height(), transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(), transform._rotation, 
		transform._originX, transform._originY, transform._aTint, transform._rTint, transform._gTint, transform._bTint);
}

void tglBlit(BlitImage *blitImage, const BlitTransform &transform) {
	TinyGL::GLContext *c =TinyGL::gl_get_context();
	bool disableColor = transform._aTint == 1.0f && transform._bTint == 1.0f && transform._gTint == 1.0f && transform._rTint == 1.0f;
	bool disableTransform = transform._destinationRectangle.width() == 0 && transform._destinationRectangle.height() == 0 && transform._rotation == 0;
	bool disableBlend = c->enableBlend == false;
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true, false, false>(blitImage, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, false, false>(blitImage, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, false, false>(blitImage, transform);
		} else {
			tglBlitGeneric<false, false, false, false, false>(blitImage, transform);
		}
	} else if (transform._flipHorizontally == false) {
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true, true, false>(blitImage, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, true, false>(blitImage, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, true, false>(blitImage, transform);
		} else {
			tglBlitGeneric<false, false, false, true, false>(blitImage, transform);
		}
	} else {
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true, true, true>(blitImage, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, true, true>(blitImage, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, true, true>(blitImage, transform);
		} else {
			tglBlitGeneric<false, false, false, true, true>(blitImage, transform);
		}
	}
}

void tglBlitNoBlend(BlitImage *blitImage, const BlitTransform &transform) {
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		tglBlitGeneric<true, false, false, false, false>(blitImage, transform);
	} else if(transform._flipHorizontally == false) {
		tglBlitGeneric<true, false, false, true, false>(blitImage, transform);
	} else {
		tglBlitGeneric<true, false, false, true, true>(blitImage, transform);
	}
}

void tglBlitFast(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	tglBlitGeneric<true, true, true, false, false>(blitImage, transform);
}
