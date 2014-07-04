#ifndef _tgl_zblit_h_
#define _tgl_zblit_h_

#include "graphics/pixelbuffer.h"

namespace TinyGL {

	int tglGenBlitTexture();
	void tglUploadBlitTexture(int textureHandle, int width, int height, Graphics::PixelBuffer &buffer, int colorKey);
	void tglDeleteBlitTexture(int textureHandle);

	void tglBlit(int blitTextureHandle, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight,
				 float rotation = 0, float originX = 0, float originY = 0, float rTint = 1.0f, float gTint = 1.0f, float bTint = 1.0f, float aTint = 1.0f);
	
	// Disables blending explicitly.
	void tglBlitNoBlend(int blitTextureHandle, int dstX, int dstY, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float rotation, float rTint, float gTint, float bTint, float aTint);
	
	// Disables blending, transforms and tinting.
	void tglBlitFast(int blitTextureHandle, int x, int y, int width, int height);
}

#endif
