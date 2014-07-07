#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/pixelbuffer.h"

struct TinyGLBlitTexture {
public:
	TinyGLBlitTexture() { }

	void loadData(const Graphics::Surface& surface, int colorKey, bool applyColorKey) {
		Graphics::PixelFormat textureFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		_surface.create(surface.w, surface.h, textureFormat);
		Graphics::PixelBuffer buffer(surface.format, (byte *)surface.getPixels());
		Graphics::PixelBuffer dataBuffer(textureFormat, (byte *)_surface.getPixels());
		for (int x = 0;  x < surface.w; x++) {
			for (int y = 0; y < surface.h; y++) {
				uint32 pixel = buffer.getValueAt(y * surface.w + x);
				if (pixel == colorKey && applyColorKey) {
					dataBuffer.setPixelAt(y * surface.w + x, 0, 255, 255, 255); // Color keyed pixels become transparent white.
				} else {
					dataBuffer.setPixelAt(y * surface.w + x, pixel);
				}
			}
		}
		// Create opaque lines data.
	}

	~TinyGLBlitTexture() {
		_surface.free();
	}

	Graphics::Surface _surface;
};

int tglGenBlitTexture() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	int handle = -1;
	for(int i = 0; i < BLIT_TEXTURE_MAX_COUNT; i++) {
		if (c->blitTextures[i] == NULL) {
			handle = i;
			c->blitTextures[i] = new TinyGLBlitTexture();
			break;
		}
	}
	return handle;
}

void tglUploadBlitTexture(int textureHandle, const Graphics::Surface& surface, int colorKey, bool applyColorKey) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGLBlitTexture *texture = (TinyGLBlitTexture *)c->blitTextures[textureHandle];
	texture->loadData(surface, colorKey, applyColorKey);
}

void tglDeleteBlitTexture(int textureHandle) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGLBlitTexture *texture = (TinyGLBlitTexture *)c->blitTextures[textureHandle];
	c->blitTextures[textureHandle] = NULL;
	delete texture;
}

template <bool disableBlending, bool disableColoring, bool disableTransform, bool flipVertical, bool flipHorizontal>
void tglBlitGeneric(int blitTextureHandle, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float rotation,
					float originX, float originY, float aTint, float rTint, float gTint, float bTint) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGLBlitTexture *texture = (TinyGLBlitTexture *)c->blitTextures[blitTextureHandle];

	if (srcWidth == 0 || srcHeight == 0) {
		srcWidth = texture->_surface.w;
		srcHeight = texture->_surface.h;
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


	Graphics::PixelBuffer srcBuf(texture->_surface.format, (byte *)texture->_surface.getPixels());

	if (flipVertical) {
		srcBuf.shiftBy(srcX + ((srcY + srcHeight - 1) * texture->_surface.w));
	} else {
		srcBuf.shiftBy(srcX + (srcY * texture->_surface.w));
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
			srcBuf.shiftBy(-texture->_surface.w);
		} else {
			srcBuf.shiftBy(texture->_surface.w);
		}
	}
}

//Utility function.
template <bool disableBlending, bool disableColoring, bool disableTransform, bool flipVertical, bool flipHorizontal>
FORCEINLINE void tglBlitGeneric(int blitTextureHandle, const BlitTransform &transform) {
	tglBlitGeneric<disableBlending, disableColoring, disableTransform, flipVertical, flipHorizontal>(blitTextureHandle, transform._dstX, transform._dstY,
		transform._width, transform._height, transform._srcX, transform._srcY, transform._srcWidth, transform._srcHeight, transform._rotation, 
		transform._originX, transform._originY, transform._aTint, transform._rTint, transform._gTint, transform._bTint);
}

void tglBlit(int blitTextureHandle, const BlitTransform &transform) {
	TinyGL::GLContext *c =TinyGL::gl_get_context();
	bool disableColor = transform._aTint == 1.0f && transform._bTint == 1.0f && transform._gTint == 1.0f && transform._rTint == 1.0f;
	bool disableTransform = transform._width == 0 && transform._height == 0 && transform._rotation == 0;
	bool disableBlend = c->enableBlend == false;
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true, false, false>(blitTextureHandle, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, false, false>(blitTextureHandle, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, false, false>(blitTextureHandle, transform);
		} else {
			tglBlitGeneric<false, false, false, false, false>(blitTextureHandle, transform);
		}
	} else if (transform._flipHorizontally == false) {
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true, true, false>(blitTextureHandle, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, true, false>(blitTextureHandle, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, true, false>(blitTextureHandle, transform);
		} else {
			tglBlitGeneric<false, false, false, true, false>(blitTextureHandle, transform);
		}
	} else {
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true, true, true>(blitTextureHandle, transform);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true, true, true>(blitTextureHandle, transform);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true, true, true>(blitTextureHandle, transform);
		} else {
			tglBlitGeneric<false, false, false, true, true>(blitTextureHandle, transform);
		}
	}

}

void tglBlitNoBlend(int blitTextureHandle, const BlitTransform &transform) {
	if (transform._flipHorizontally == false && transform._flipVertically == false) {
		tglBlitGeneric<true, false, false, false, false>(blitTextureHandle, transform);
	} else if(transform._flipHorizontally == false) {
		tglBlitGeneric<true, false, false, true, false>(blitTextureHandle, transform);
	} else {
		tglBlitGeneric<true, false, false, true, true>(blitTextureHandle, transform);
	}
}

void tglBlitFast(int blitTextureHandle, int x, int y) {
	BlitTransform transform(x, y);
	tglBlitGeneric<true, true, true, false, false>(blitTextureHandle, transform);
}
