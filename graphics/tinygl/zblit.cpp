#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/pixelbuffer.h"
#include "common/array.h"
#include <math.h>

namespace Graphics {

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

BlitImage *tglGenBlitImage() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	BlitImage *image = new BlitImage();
	c->blitImages.push_back(image);
	return image;
}

void tglUploadBlitImage(BlitImage *blitImage, const Graphics::Surface& surface, uint32 colorKey, bool applyColorKey) {
	if (blitImage != nullptr) {
		blitImage->loadData(surface, colorKey, applyColorKey);
	}
}

void tglDeleteBlitImage(BlitImage *blitImage) {
	if (blitImage != nullptr) {
		TinyGL::GLContext *c = TinyGL::gl_get_context();
		for (uint32 i = 0; i < c->blitImages.size(); i++) {
			if (c->blitImages[i] == blitImage) {
				c->blitImages.remove_at(i);
				break;
			}
		}
		delete blitImage;
	}
}

Common::Point transformPoint(float x, float y, int rotation) {
	float rotateRad = rotation * M_PI / 180.0f;
	Common::Point newPoint;
	newPoint.x = x * cos(rotateRad) - y * sin(rotateRad);
	newPoint.y = x * sin(rotateRad) + y * cos(rotateRad);
	return newPoint;
}

Common::Rect rotateRectangle(int x, int y, int width, int height, int rotation, int originX, int originY) {
	Common::Point nw, ne, sw, se;
	nw = transformPoint(x - originX, y - originY, rotation);
	ne = transformPoint(x + width - originX, y - originY, rotation);
	sw = transformPoint(x + width - originX, y + height- originY, rotation);
	se = transformPoint(x - originX, y + height - originY, rotation);

	float top = MIN(nw.y, MIN(ne.y, MIN(sw.y, se.y)));
	float bottom = MAX(nw.y, MAX(ne.y, MAX(sw.y, se.y)));
	float left = MIN(nw.x, MIN(ne.x, MIN(sw.x, se.x)));
	float right = MAX(nw.x, MAX(ne.x, MAX(sw.x, se.x)));

	Common::Rect res;
	res.top = (int32)(floor(top)) + originY;
	res.bottom = (int32)(ceil(bottom)) + originY;
	res.left = (int32)(floor(left)) + originX;
	res.right = (int32)(ceil(right)) + originX;

	return res;
}

template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
void tglBlitGenericNoTransform(BlitImage *blitImage, int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	if (srcWidth == 0 || srcHeight == 0) {
		srcWidth = blitImage->_surface.w;
		srcHeight = blitImage->_surface.h;
	}

	// Width and height are always 0 when you call this function.
	int width = srcWidth;
	int height = srcHeight;

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

template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
void tglBlitGenericTransform(BlitImage *blitImage, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, int rotation,
							 int originX, int originY, float aTint, float rTint, float gTint, float bTint) {
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
	srcBuf.shiftBy(srcX + (srcY * blitImage->_surface.w));

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());

	if (rotation == 0) {
		for (int l = 0; l < clampHeight; l++) {
			for (int r = 0; r < clampWidth; ++r) {
				byte aDst, rDst, gDst, bDst;
				int xSource, ySource;
				if (flipVertical) {
					ySource = clampHeight - l - 1;
				} else {
					ySource = l;
				}

				if (flipHorizontal) {
					xSource = clampWidth - r - 1;
				} else {
					xSource = r;
				}

				srcBuf.getARGBAt(((ySource * srcHeight) / height) * blitImage->_surface.w + ((xSource * srcWidth) / width), aDst, rDst, gDst, bDst);

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
		}
	} else {
		// Transform destination rectangle accordingly.
		Common::Rect destinationRectangle = rotateRectangle(dstX, dstY, width, height, rotation, originX, originY);

		if (dstX + destinationRectangle.width() > c->fb->xsize)
			clampWidth = c->fb->xsize - dstX;
		else
			clampWidth = destinationRectangle.width();

		if (dstY + destinationRectangle.height() > c->fb->ysize)
			clampHeight = c->fb->ysize - dstY;
		else
			clampHeight = destinationRectangle.height();

		uint32 invAngle = 360 - (rotation % 360);
		float invCos = cos(invAngle * M_PI / 180.0f);
		float invSin = sin(invAngle * M_PI / 180.0f	);

		int icosx = (int)(invCos * (65536.0f * srcWidth / width));
		int isinx = (int)(invSin * (65536.0f * srcWidth / width));
		int icosy = (int)(invCos * (65536.0f * srcHeight / height));
		int isiny = (int)(invSin * (65536.0f * srcHeight / height));

		int xd = (srcX + originX) << 16;
		int yd = (srcY + originY) << 16;
		int cx = originX;
		int cy = originY;

		int ax = -icosx * cx;
		int ay = -isiny * cx;
		int sw = width - 1;
		int sh = height - 1;

		for (int l = 0; l < clampHeight; l++) {
			int t = cy - l;
			int sdx = ax + (isinx * t) + xd;
			int sdy = ay - (icosy * t) + yd;
			for (int r = 0; r < clampWidth; ++r) {
				byte aDst, rDst, gDst, bDst;

				int xSource, ySource;
				int dx = (sdx >> 16);
				int dy = (sdy >> 16);

				if (flipHorizontal) dx = sw - dx;
				if (flipVertical) dy = sh - dy;

				if ((dx >= 0) && (dy >= 0) && (dx < srcWidth) && (dy < srcHeight)) {
					srcBuf.getARGBAt(dy * blitImage->_surface.w + dx, aDst, rDst, gDst, bDst);
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
				sdx += icosx;
				sdy += isiny;
			}
		}
	}
}

//Utility function.
template <bool disableBlending, bool disableColoring, bool disableTransform, bool flipVertical, bool flipHorizontal>
FORCEINLINE void tglBlitGeneric(BlitImage *blitImage, const BlitTransform &transform) {
	if (disableTransform) {
		tglBlitGenericNoTransform<disableBlending, disableColoring, flipVertical, flipHorizontal>(blitImage, transform._destinationRectangle.left, transform._destinationRectangle.top,
			transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(),
			transform._aTint, transform._rTint, transform._gTint, transform._bTint);
	} else {
		tglBlitGenericTransform<disableBlending, disableColoring, flipVertical, flipHorizontal>(blitImage, transform._destinationRectangle.left, transform._destinationRectangle.top,
			transform._destinationRectangle.width(), transform._destinationRectangle.height(), transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(), transform._rotation, 
			transform._originX, transform._originY, transform._aTint, transform._rTint, transform._gTint, transform._bTint);
	}
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
			tglBlitGeneric<true, true, true, false, true>(blitImage, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, false, true>(blitImage, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, false, true>(blitImage, transform);
		} else {
			tglBlitGeneric<false, false, false, false, true>(blitImage, transform);
		}
	}
}

void tglBlitNoBlend(BlitImage *blitImage, const BlitTransform &transform) {
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		tglBlitGeneric<true, false, false, false, false>(blitImage, transform);
	} else if(transform._flipHorizontally == false) {
		tglBlitGeneric<true, false, false, true, false>(blitImage, transform);
	} else {
		tglBlitGeneric<true, false, false, false, true>(blitImage, transform);
	}
}

void tglBlitFast(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	tglBlitGeneric<true, true, true, false, false>(blitImage, transform);
}

}
