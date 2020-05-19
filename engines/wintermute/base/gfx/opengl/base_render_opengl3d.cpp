#include "base_render_opengl3d.h"

#include "../../../../../graphics/opengl/system_headers.h"
#include "base_surface_opengl3d.h"

Wintermute::BaseRenderOpenGL3D::~BaseRenderOpenGL3D() {

}

Wintermute::BaseImage* Wintermute::BaseRenderOpenGL3D::takeScreenshot() {
	return nullptr;
}

bool Wintermute::BaseRenderOpenGL3D::saveScreenShot(const Common::String& filename, int sizeX, int sizeY) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setViewport(int left, int top, int right, int bottom) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setViewport(Wintermute::Rect32* rect) {
	return true;
}

Wintermute::Rect32 Wintermute::BaseRenderOpenGL3D::getViewPort() {
	return Rect32(0, 0, 0, 0);
}

bool Wintermute::BaseRenderOpenGL3D::setScreenViewport() {
	return true;
}

void Wintermute::BaseRenderOpenGL3D::setWindowed(bool windowed) {

}

Graphics::PixelFormat Wintermute::BaseRenderOpenGL3D::getPixelFormat() const
{
	return OpenGL::Texture::getRGBAPixelFormat();
}

void Wintermute::BaseRenderOpenGL3D::fade(uint16 alpha) {
	fadeToColor(0, 0, 0, (byte)(255 - alpha));
}

void Wintermute::BaseRenderOpenGL3D::fadeToColor(byte r, byte g, byte b, byte a) {
	setup2D(true);
}

bool Wintermute::BaseRenderOpenGL3D::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::drawRect(int x1, int y1, int x2, int y2, uint32 color, int width) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::windowedBlt() {
	return true;
}

Wintermute::BaseRenderOpenGL3D::BaseRenderOpenGL3D(Wintermute::BaseGame* inGame)
	: _spriteBatchMode(false) {

}

bool Wintermute::BaseRenderOpenGL3D::fill(byte r, byte g, byte b, Common::Rect* rect) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
	return true;
}

void Wintermute::BaseRenderOpenGL3D::onWindowChange() {

}

bool Wintermute::BaseRenderOpenGL3D::initRenderer(int width, int height, bool windowed) {
	_windowed = windowed;
	_width = width;
	_height = height;
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::flip() {
	g_system->updateScreen();
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::indicatorFlip() {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::forcedFlip() {
	return true;
}

void Wintermute::BaseRenderOpenGL3D::initLoop() {

}

bool Wintermute::BaseRenderOpenGL3D::setup2D(bool force) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setup3D(bool force) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setupLines() {

}

Wintermute::BaseSurface* Wintermute::BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(nullptr);
}

void Wintermute::BaseRenderOpenGL3D::endSaveLoad() {

}

bool Wintermute::BaseRenderOpenGL3D::drawSprite(const OpenGL::Texture& tex, const Wintermute::Rect32& rect,
												float zoomX, float zoomY, const Wintermute::Vector2& pos, float angle,
												uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	Vector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(tex, rect, pos, Vector2(0.0f, 0.0f), scale, angle, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

bool Wintermute::BaseRenderOpenGL3D::drawSpriteEx(const OpenGL::Texture& tex, const Wintermute::Rect32& rect,
												  const Wintermute::Vector2& pos, const Wintermute::Vector2& rot, const Wintermute::Vector2& scale,
												  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return true;
}
