#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/pixelbuffer.h"
#include "common/array.h"
#include <math.h>

namespace Graphics {

Common::Point transformPoint(float x, float y, int rotation);
Common::Rect rotateRectangle(int x, int y, int width, int height, int rotation, int originX, int originY);

struct BlitImage {
public:
	BlitImage() { }

	void loadData(const Graphics::Surface &surface, uint32 colorKey, bool applyColorKey) {
		Graphics::PixelFormat textureFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		_surface.create(surface.w, surface.h, textureFormat);
		Graphics::PixelBuffer buffer(surface.format, (byte *)surface.getPixels());
		Graphics::PixelBuffer dataBuffer(textureFormat, (byte *)_surface.getPixels());
		dataBuffer.copyBuffer(0, 0, surface.w * surface.h, buffer);
		if (applyColorKey) {
			for (int x = 0;  x < surface.w; x++) {
				for (int y = 0; y < surface.h; y++) {
					uint32 pixel = dataBuffer.getValueAt(y * surface.w + x);
					if (pixel == colorKey) {
						dataBuffer.setPixelAt(y * surface.w + x, 0, 255, 255, 255); // Color keyed pixels become transparent white.
					}
				}
			}
		}

		// Create opaque lines data.
		// A line of pixels can not wrap more that one line of the image, since it would break
		// blitting of bitmaps with a non-zero x position.
		Graphics::PixelBuffer srcBuf = dataBuffer;
		_lines.clear();
		for (int y = 0; y < surface.h; y++) {
			int start = -1;
			for (int x = 0; x < surface.w; ++x) {
				// We found a transparent pixel, so save a line from 'start' to the pixel before this.
				uint8 r, g, b, a;
				srcBuf.getARGBAt(x, a, r, g, b);
				if (a == 0 && start >= 0) {
					_lines.push_back(Line(start, y, x - start, srcBuf.getRawBuffer(start)));
					start = -1;
				} else if (a != 0 && start == -1) {
					start = x;
				}
			}
			// end of the bitmap line. if start is an actual pixel save the line.
			if (start >= 0) {
				_lines.push_back(Line(start, y, surface.w- start, srcBuf.getRawBuffer(start)));
			}
			srcBuf.shiftBy(surface.w);
		}
	}

	~BlitImage() {
		_surface.free();
	}

	struct Line {
		int _x;
		int _y;
		int _length;
		byte *_pixels;
		Graphics::PixelBuffer _buf; // This is needed for the conversion.

		Line() { }
		Line(int x, int y, int length, byte *pixels) : _buf(TinyGL::gl_get_context()->fb->cmode, length * TinyGL::gl_get_context()->fb->cmode.bytesPerPixel, DisposeAfterUse::NO) {
			_x = x;
			_y = y;
			_length = length;
			// Performing texture to screen conversion.
			Graphics::PixelFormat textureFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
			Graphics::PixelBuffer srcBuf(textureFormat, pixels);
			_buf.copyBuffer(0, 0, length, srcBuf);
			_pixels = _buf.getRawBuffer();
		}

		Line(const Line& other) : _buf(TinyGL::gl_get_context()->fb->cmode, other._length * TinyGL::gl_get_context()->fb->cmode.bytesPerPixel, DisposeAfterUse::NO) {
			_x = other._x;
			_y = other._y;
			_length = other._length;
			_buf.copyBuffer(0, 0, _length, other._buf);
			_pixels = _buf.getRawBuffer();
		}

		~Line() {
			_buf.free();
		}
	};

	FORCEINLINE bool clipBlitImage(TinyGL::GLContext *c, int &srcWidth, int &srcHeight, int &width, int &height, int &dstX, int &dstY, int &clampWidth, int &clampHeight) {
		if (srcWidth == 0 || srcHeight == 0) {
			srcWidth = _surface.w;
			srcHeight = _surface.h;
		}

		if (width == 0 && height == 0) {
			width = srcWidth;
			height = srcHeight;
		}

		if (dstX >= c->fb->xsize|| dstY >= c->fb->ysize)
			return false;

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

		return true;
	}

	template <bool disableColoring>
	void tglBlitRLE(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint);

	template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
	void tglBlitSimple(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint);

	template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
	void tglBlitScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint);

	template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
	void tglBlitRotoScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, int rotation,
		int originX, int originY, float aTint, float rTint, float gTint, float bTint);

	//Utility function.
	template <bool disableBlending, bool disableColoring, bool disableTransform, bool flipVertical, bool flipHorizontal>
	FORCEINLINE void tglBlitGeneric(const BlitTransform &transform) {
		if (disableTransform) {
			if (disableBlending && flipVertical == false && flipHorizontal == false) {
				tglBlitRLE<disableColoring>(transform._destinationRectangle.left, transform._destinationRectangle.top,
					transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(),
					transform._aTint, transform._rTint, transform._gTint, transform._bTint);
			} else {
				tglBlitSimple<disableBlending, disableColoring, flipVertical, flipHorizontal>(transform._destinationRectangle.left, transform._destinationRectangle.top,
					transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(),
					transform._aTint, transform._rTint, transform._gTint, transform._bTint);
			}
		} else {
			if (transform._rotation == 0) {
				tglBlitScale<disableBlending, disableColoring, flipVertical, flipHorizontal>(transform._destinationRectangle.left, transform._destinationRectangle.top,
					transform._destinationRectangle.width(), transform._destinationRectangle.height(), transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(), transform._aTint, transform._rTint, transform._gTint, transform._bTint);
			} else {
				tglBlitRotoScale<disableBlending, disableColoring, flipVertical, flipHorizontal>(transform._destinationRectangle.left, transform._destinationRectangle.top,
					transform._destinationRectangle.width(), transform._destinationRectangle.height(), transform._sourceRectangle.left, transform._sourceRectangle.top, transform._sourceRectangle.width() , transform._sourceRectangle.height(), transform._rotation, 
					transform._originX, transform._originY, transform._aTint, transform._rTint, transform._gTint, transform._bTint);
			}
		}
	}

	Common::Array<Line> _lines;
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

template <bool disableColoring>
void BlitImage::tglBlitRLE(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	int clampWidth, clampHeight;
	int width = srcWidth, height = srcHeight;
	if (clipBlitImage(c, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;

	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());
	srcBuf.shiftBy(srcX + (srcY * _surface.w));

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());
	dstBuf.shiftBy(dstY * c->fb->xsize + dstX);

	const int kBytesPerPixel = 2;

	int lineIndex = 0;
	int maxY = srcY + clampHeight;
	int maxX = srcX + clampWidth;
	while (lineIndex < _lines.size() && _lines[lineIndex]._y < srcY) {
		lineIndex++;
	}
	while (lineIndex < _lines.size() && _lines[lineIndex]._y < maxY) {
		const BlitImage::Line &l = _lines[lineIndex];
		if (l._x < maxX && l._x + l._length > srcX) {
			int length = l._length;
			int skipStart = (l._x < srcX) ? (srcX - l._x) : 0;
			length -= skipStart;
			int skipEnd   = (l._x + l._length > maxX) ? (l._x + l._length - maxX) : 0;
			length -= skipEnd;
			if (disableColoring) {
				memcpy(dstBuf.getRawBuffer((l._y - srcY) * c->fb->xsize + MAX(l._x - srcX, 0)),
					l._pixels + skipStart * kBytesPerPixel, length * kBytesPerPixel);
			} else {
				int xStart = MAX(l._x - srcX, 0);
				for(int x = xStart; x < xStart + length; x++) {
					byte aDst, rDst, gDst, bDst;
					srcBuf.getARGBAt((l._y - srcY) *c->fb->xsize + x, aDst, rDst, gDst, bDst);
					c->fb->writePixel((dstX + x) + (dstY + (l._y - srcY)) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				}
			}
		}
		lineIndex++;
	}
}

template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
void BlitImage::tglBlitSimple(int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	int clampWidth, clampHeight;
	int width = srcWidth, height = srcHeight;
	if (clipBlitImage(c, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;

	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());

	if (flipVertical) {
		srcBuf.shiftBy(srcX + ((srcY + srcHeight - 1) * _surface.w));
	} else {
		srcBuf.shiftBy(srcX + (srcY * _surface.w));
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
				if (disableBlending && aDst * aTint != 0) {
					dstBuf.setPixelAt((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				} else {
					c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				}
			}
		}
		if (flipVertical) {
			srcBuf.shiftBy(-_surface.w);
		} else {
			srcBuf.shiftBy(_surface.w);
		}
	}
}

template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
void BlitImage::tglBlitScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight,
					 float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	int clampWidth, clampHeight;
	if (clipBlitImage(c, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;

	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());
	srcBuf.shiftBy(srcX + (srcY * _surface.w));

	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());

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

			srcBuf.getARGBAt(((ySource * srcHeight) / height) * _surface.w + ((xSource * srcWidth) / width), aDst, rDst, gDst, bDst);

			if (disableColoring) {
				if (disableBlending && aDst != 0) {
					dstBuf.setPixelAt((dstX + r) + (dstY + l) * c->fb->xsize, aDst, rDst, gDst, bDst);
				} else {
					c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst, rDst, gDst, bDst);
				}
			} else {
				if (disableBlending && aDst * aTint != 0) {
					dstBuf.setPixelAt((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				} else {
					c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
				}
			}
		}
	}
}

template <bool disableBlending, bool disableColoring, bool flipVertical, bool flipHorizontal>
void BlitImage::tglBlitRotoScale(int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, int rotation,
							 int originX, int originY, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	
	int clampWidth, clampHeight;
	if (clipBlitImage(c, srcWidth, srcHeight, width, height, dstX, dstY, clampWidth, clampHeight) == false)
		return;
	
	Graphics::PixelBuffer srcBuf(_surface.format, (byte *)_surface.getPixels());
	srcBuf.shiftBy(srcX + (srcY * _surface.w));
	
	Graphics::PixelBuffer dstBuf(c->fb->cmode, c->fb->getPixelBuffer());
	
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
	float invSin = sin(invAngle * M_PI / 180.0f);
	
	int icosx = (int)(invCos * (65536.0f * srcWidth / width));
	int isinx = (int)(invSin * (65536.0f * srcWidth / width));
	int icosy = (int)(invCos * (65536.0f * srcHeight / height));
	int isiny = (int)(invSin * (65536.0f * srcHeight / height));
	
	int xd = (srcX + originX) << 16;
	int yd = (srcY + originY) << 16;
	int cx = originX * ((float)width / srcWidth);
	int cy = originY * ((float)height / srcHeight);
	
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
			
			int dx = (sdx >> 16);
			int dy = (sdy >> 16);
			
			if (flipHorizontal)
				dx = sw - dx;
			if (flipVertical)
				dy = sh - dy;
			
			if ((dx >= 0) && (dy >= 0) && (dx < srcWidth) && (dy < srcHeight)) {
				srcBuf.getARGBAt(dy * _surface.w + dx, aDst, rDst, gDst, bDst);
				if (disableColoring) {
					if (disableBlending && aDst != 0) {
						dstBuf.setPixelAt((dstX + r) + (dstY + l) * c->fb->xsize, aDst, rDst, gDst, bDst);
					} else {
						c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst, rDst, gDst, bDst);
					}
				} else {
					if (disableBlending && aDst * aTint != 0) {
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

void tglBlit(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	tglBlit(blitImage, transform);
}

void tglBlit(BlitImage *blitImage, const BlitTransform &transform) {
	TinyGL::GLContext *c =TinyGL::gl_get_context();
	bool disableColor = transform._aTint == 1.0f && transform._bTint == 1.0f && transform._gTint == 1.0f && transform._rTint == 1.0f;
	bool disableTransform = transform._destinationRectangle.width() == 0 && transform._destinationRectangle.height() == 0 && transform._rotation == 0;
	bool disableBlend = c->enableBlend == false;
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		if (disableColor && disableTransform && disableBlend) {
			blitImage->tglBlitGeneric<true, true, true, false, false>(transform);
		} else if (disableColor && disableTransform) {
			blitImage->tglBlitGeneric<false, true, true, false, false>(transform);
		} else if (disableTransform) {
			blitImage->tglBlitGeneric<false, false, true, false, false>(transform);
		} else {
			blitImage->tglBlitGeneric<false, false, false, false, false>(transform);
		}
	} else if (transform._flipHorizontally == false) {
		if (disableColor && disableTransform && disableBlend) {
			blitImage->tglBlitGeneric<true, true, true, true, false>(transform);
		} else if (disableColor && disableTransform) {
			blitImage->tglBlitGeneric<false, true, true, true, false>(transform);
		} else if (disableTransform) {
			blitImage->tglBlitGeneric<false, false, true, true, false>(transform);
		} else {
			blitImage->tglBlitGeneric<false, false, false, true, false>(transform);
		}
	} else {
		if (disableColor && disableTransform && disableBlend) {
			blitImage->tglBlitGeneric<true, true, true, false, true>(transform);
		} else if (disableColor && disableTransform) {
			blitImage->tglBlitGeneric<false, true, true, false, true>(transform);
		} else if (disableTransform) {
			blitImage->tglBlitGeneric<false, false, true, false, true>(transform);
		} else {
			blitImage->tglBlitGeneric<false, false, false, false, true>(transform);
		}
	}
}

void tglBlitNoBlend(BlitImage *blitImage, const BlitTransform &transform) {
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		blitImage->tglBlitGeneric<true, false, false, false, false>(transform);
	} else if(transform._flipHorizontally == false) {
		blitImage->tglBlitGeneric<true, false, false, true, false>(transform);
	} else {
		blitImage->tglBlitGeneric<true, false, false, false, true>(transform);
	}
}

void tglBlitFast(BlitImage *blitImage, int x, int y) {
	BlitTransform transform(x, y);
	blitImage->tglBlitGeneric<true, true, true, false, false>(transform);
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
	sw = transformPoint(x + width - originX, y + height -	 originY, rotation);
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

}
