#include "base_surface_opengl3d.h"

Wintermute::BaseSurfaceOpenGL3D::BaseSurfaceOpenGL3D(Wintermute::BaseGame* game)
	: BaseSurface(game), tex(0, 0)
{

}

bool Wintermute::BaseSurfaceOpenGL3D::invalidate() {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::prepareToDraw() {
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
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTransOffset(int x, int y, Wintermute::Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::display(int x, int y, Wintermute::Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTransform(int x, int y, Wintermute::Rect32 rect, Wintermute::Rect32 newRect, const Graphics::TransformStruct& transform) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayZoom(int x, int y, Wintermute::Rect32 rect, float zoomX, float zoomY, uint32 alpha, bool transparent, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::displayTiled(int x, int y, Wintermute::Rect32 rect, int numTimesX, int numTimesY) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::restore() {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::create(const Common::String& filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::create(int width, int height) {
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
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::endPixelOp() {
	return true;
}

bool Wintermute::BaseSurfaceOpenGL3D::isTransparentAtLite(int x, int y) {
	return true;
}
