#include "base_surface_opengl3d.h"

#include "../base_image.h"
#include "base_render_opengl3d.h"
#include "graphics/transparent_surface.h"

Wintermute::BaseSurfaceOpenGL3D::BaseSurfaceOpenGL3D(Wintermute::BaseGame* game, BaseRenderOpenGL3D* renderer)
	: BaseSurface(game), tex(nullptr), renderer(renderer), pixelOpReady(false)
{

}

bool Wintermute::BaseSurfaceOpenGL3D::invalidate() {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayHalfTrans(int x, int y, Wintermute::Rect32 rect) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::isTransparentAt(int x, int y) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTransZoom(int x, int y, Wintermute::Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTrans(int x, int y, Wintermute::Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	renderer->drawSprite(*tex, rect, 100, 100, Vector2(x, y), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTransOffset(int x, int y, Wintermute::Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::display(int x, int y, Wintermute::Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	renderer->drawSprite(*tex, rect, 100, 100, Vector2(x, y), 0xFFFFFFFF, true, blendMode, mirrorX, mirrorY);
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTransform(int x, int y, Wintermute::Rect32 rect, Wintermute::Rect32 newRect, const Graphics::TransformStruct& transform) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayZoom(int x, int y, Wintermute::Rect32 rect, float zoomX, float zoomY, uint32 alpha, bool transparent, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTiled(int x, int y, Wintermute::Rect32 rect, int numTimesX, int numTimesY) {
	Vector2 scale(numTimesX, numTimesY);
	renderer->drawSpriteEx(*tex, rect, Vector2(x, y), Vector2(0, 0), scale, 0, 0xFFFFFFFF, true, Graphics::BLEND_NORMAL, false, false);
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::restore() {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::create(const Common::String& filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	BaseImage img	= BaseImage();
	if (!img.loadFile(filename)) {
		return false;
	}

	if (img.getSurface()->format.bytesPerPixel == 1 && img.getPalette() == nullptr) {
		return false;
	}

	_filename = filename;

	if (defaultCK) {
		ckRed   = 255;
		ckGreen = 0;
		ckBlue  = 255;
	}

	_ckDefault = defaultCK;
	_ckRed = ckRed;
	_ckGreen = ckGreen;
	_ckBlue = ckBlue;

	bool needsColorKey = false;
	bool replaceAlpha = true;

	Graphics::Surface* surf = img.getSurface()->convertTo(OpenGL::Texture::getRGBAPixelFormat(), img.getPalette());

	if (_filename.hasSuffix(".bmp") && img.getSurface()->format.bytesPerPixel == 4) {
		// 32 bpp BMPs have nothing useful in their alpha-channel -> color-key
		needsColorKey = true;
		replaceAlpha = false;
	} else if (img.getSurface()->format.aBits() == 0) {
		needsColorKey = true;
	}

	if (needsColorKey) {
		Graphics::TransparentSurface trans(*surf);
		trans.applyColorKey(_ckRed, _ckGreen, _ckBlue, replaceAlpha);
	}

	tex = new OpenGL::Texture(*surf);
	delete surf;

	if (_lifeTime == 0 || lifeTime == -1 || lifeTime > _lifeTime) {
		_lifeTime = lifeTime;
	}

	_keepLoaded = keepLoaded;
	if (_keepLoaded) {
		_lifeTime = -1;
	}

	_valid = true;

	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::create(int width, int height) {
	tex = new OpenGL::Texture(width, height);
	_valid = true;
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::putPixel(int x, int y, byte r, byte g, byte b, int a) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::getPixel(int x, int y, byte* r, byte* g, byte* b, byte* a) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::comparePixel(int x, int y, byte r, byte g, byte b, int a) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::startPixelOp() {
	glBindTexture(GL_TEXTURE_2D, tex->getTextureName());
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::endPixelOp() {
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::isTransparentAtLite(int x, int y) {
	if (x < 0 || y < 0 || x >= tex->getWidth() || y >= tex->getHeight()) {
		return false;
	}

	if (!pixelOpReady) {
		return false;
	}

	uint8* image_data = nullptr;

	// assume 32 bit rgba for now
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

	uint32 pixel = *reinterpret_cast<uint32*>(image_data + y * tex->getWidth() * 4 + x * 4);
	pixel &= 0x000000FF;
	return pixel == 0;
}
