#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {
	struct TinyGLBlitTexture {
	public:
		TinyGLBlitTexture() : width(0), height(0) { }

		void loadData(int width, int height, Graphics::PixelBuffer &buffer, int colorKey) {
			this->width = width;
			this->height = height;
			this->dataBuffer = Graphics::PixelBuffer(Graphics::PixelFormat(4, 8, 8, 8 , 8, 8, 16, 24, 0), width * height, DisposeAfterUse::YES);
			for (int x = 0;  x < width; x++) {
				for (int y = 0; y < height; y++) {
					int pixel = buffer.getValueAt(y * width + x);
					if (pixel == colorKey) {
						dataBuffer.setPixelAt(y * width + x, 0, 255, 255, 255); // Color keyed pixels become transparent white.
					} else {
						dataBuffer.setPixelAt(y * width + x, pixel);
					}
				}
			}
			// Create opaque lines data.
		}

		Graphics::PixelBuffer dataBuffer;
		int width, height;
	};

	int tglGenBlitTexture() {
		TinyGL::GLContext *c = TinyGL::gl_get_context();
		int handle = -1;
		for(int i = 0; i < BLIT_TEXTURE_MAX_COUNT; i++) {
			if (c->blitTextures == NULL) {
				handle = i;
				c->blitTextures[i] = new TinyGLBlitTexture();
				break;
			}
		}
		return handle;
	}

	void tglUploadBlitTexture(int textureHandle, int width, int height, Graphics::PixelBuffer &buffer, int colorKey) {
		TinyGL::GLContext *c = TinyGL::gl_get_context();
		TinyGLBlitTexture *texture = (TinyGLBlitTexture *)c->blitTextures[textureHandle];
		texture->loadData(width, height, buffer, colorKey);
	}

	void tglDeleteBlitTexture(int textureHandle) {
		TinyGL::GLContext *c = TinyGL::gl_get_context();
		TinyGLBlitTexture *texture = (TinyGLBlitTexture *)c->blitTextures[textureHandle];
		c->blitTextures[textureHandle] = NULL;
		delete texture;
	}

	template <bool disableBlending, bool disableColoring, bool disableTransform>
	void tglBlitGeneric(int blitTextureHandle, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float rotation,
						float originX, float originY, float rTint, float gTint, float bTint, float aTint) {
		TinyGL::GLContext *c = TinyGL::gl_get_context();

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

		TinyGLBlitTexture *texture = (TinyGLBlitTexture *)c->blitTextures[blitTextureHandle];

		Graphics::PixelBuffer srcBuf(texture->dataBuffer);
		srcBuf.shiftBy(srcX + (srcY * texture->width));

		for (int l = 0; l < clampHeight; l++) {
			for (int r = 0; r < clampWidth; ++r) {
				byte aDst, rDst, gDst, bDst;
				srcBuf.getARGBAt(r, aDst, rDst, gDst, bDst);
				c->fb->writePixel((dstX + r) + (dstY + l) * c->fb->xsize, aDst * aTint, rDst * rTint, gDst * gTint, bDst * bTint);
			}
			srcBuf.shiftBy(srcWidth);
		}
	}

	void tglBlit(int blitTextureHandle, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float rotation,
				 float originX, float originY, float rTint, float gTint, float bTint, float aTint) {
		TinyGL::GLContext *c =TinyGL::gl_get_context();
		bool disableColor = aTint == 1.0f && bTint == 1.0f && gTint == 1.0f && rTint == 1.0f;
		bool disableTransform = srcWidth == width && srcHeight == height && rotation == 0;
		bool disableBlend = c->enableBlend == false;
		if (disableColor && disableTransform && disableBlend) {
			tglBlitGeneric<true, true, true>(blitTextureHandle, dstX, dstY, width, height, srcX, srcY, srcWidth, srcHeight, rotation, originX, originY, rTint, gTint, bTint, aTint);
		} else if (disableColor && disableTransform) {
			tglBlitGeneric<false, true, true>(blitTextureHandle, dstX, dstY, width, height, srcX, srcY, srcWidth, srcHeight, rotation, originX, originY, rTint, gTint, bTint, aTint);
		} else if (disableTransform) {
			tglBlitGeneric<false, false, true>(blitTextureHandle, dstX, dstY, width, height, srcX, srcY, srcWidth, srcHeight, rotation, originX, originY, rTint, gTint, bTint, aTint);
		} else {
			tglBlitGeneric<false, false, false>(blitTextureHandle, dstX, dstY, width, height, srcX, srcY, srcWidth, srcHeight, rotation, originX, originY, rTint, gTint, bTint, aTint);
		}
	}

	void tglBlitNoBlend(int blitTextureHandle, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float rotation,
						float originX, float originY, float rTint, float gTint, float bTint, float aTint) {
		tglBlitGeneric<true, false, false>(blitTextureHandle, dstX, dstY, width, height, srcX, srcY, srcWidth, srcHeight, rotation, originX, originY, rTint, gTint, bTint, aTint);
	}

	void tglBlitFast(int blitTextureHandle, int x, int y, int width, int height) {
		tglBlitGeneric<true, true, true>(blitTextureHandle, x, y, width, height, 0, 0, width, height, 0, 0, 0, 255, 255, 255 ,255);
	}

}