#include "base_render_opengl3d.h"

#include "../../../../../graphics/opengl/system_headers.h"
#include "base_surface_opengl3d.h"
#include "math/glmath.h"

namespace Wintermute {
BaseRenderer *makeOpenGL3DRenderer(BaseGame* inGame) {
	return new BaseRenderOpenGL3D(inGame);
}

}

Wintermute::BaseRenderOpenGL3D::BaseRenderOpenGL3D(Wintermute::BaseGame* inGame)
	: _spriteBatchMode(false) {

}

Wintermute::BaseRenderOpenGL3D::~BaseRenderOpenGL3D() {

}

Wintermute::BaseImage* Wintermute::BaseRenderOpenGL3D::takeScreenshot() {
	return nullptr;
}

bool Wintermute::BaseRenderOpenGL3D::saveScreenShot(const Common::String& filename, int sizeX, int sizeY) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setViewport(int left, int top, int right, int bottom) {
	_viewportRect.setRect(left, top, right, bottom);
	glViewport(left, top, right - left, bottom - top);
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setViewport(Wintermute::Rect32* rect) {
	return setViewport(rect->left, rect->top, rect->right, rect->bottom);
}

Wintermute::Rect32 Wintermute::BaseRenderOpenGL3D::getViewPort() {
	return Rect32(0, 0, 0, 0);
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

bool Wintermute::BaseRenderOpenGL3D::setProjection() {
	// is the viewport already set here?
	float viewportWidth = _viewportRect.right - _viewportRect.left;
	float viewportHeight = _viewportRect.bottom - _viewportRect.top;

	float horizontal_view_angle = M_PI * 0.5f;
	float aspect_ratio = float(viewportHeight) / float(viewportWidth);
	float near_plane = 1.0f;
	float far_plane = 1000.0f;
	float right = near_plane * tanf(horizontal_view_angle * 0.5f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-right, right, -right*aspect_ratio, right*aspect_ratio, near_plane, far_plane);
	glMatrixMode(GL_MODELVIEW);

	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setProjection2D()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _viewportRect.width(), 0, _viewportRect.height(), -1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::windowedBlt() {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::fill(byte r, byte g, byte b, Common::Rect* rect) {
	glClearColor(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

void Wintermute::BaseRenderOpenGL3D::onWindowChange() {

}

bool Wintermute::BaseRenderOpenGL3D::initRenderer(int width, int height, bool windowed) {
	_windowed = windowed;
	_width = width;
	_height = height;

	setViewport(0, 0, width, height);

	_active = true;
	// setup a proper state
	setup2D(true);
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
	setup2D();
}

bool Wintermute::BaseRenderOpenGL3D::setup2D(bool force) {
	if (_state3D || force)
	{
		_state3D = false;

		// some states are still missing here

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL);
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_FOG);
		glLightModeli(GL_LIGHT_MODEL_AMBIENT, 0);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_CCW);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GEQUAL, 0x00);
		glPolygonMode(GL_FRONT, GL_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);

		setProjection2D();
	}

	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setup3D(bool force) {
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setupLines() {
	return true;
}

Wintermute::BaseSurface* Wintermute::BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(nullptr, this);
}

void Wintermute::BaseRenderOpenGL3D::endSaveLoad() {

}

bool Wintermute::BaseRenderOpenGL3D::drawSprite(const OpenGL::Texture& tex, const Wintermute::Rect32& rect,
												float zoomX, float zoomY, const Wintermute::Vector2& pos,
												uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	Vector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(tex, rect, pos, Vector2(0.0f, 0.0f), scale, 0.0f, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

bool Wintermute::BaseRenderOpenGL3D::drawSpriteEx(const OpenGL::Texture& tex, const Wintermute::Rect32& rect,
												  const Wintermute::Vector2& pos, const Wintermute::Vector2& rot, const Wintermute::Vector2& scale,
												  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	// original wme has a batch mode for sprites, we ignore this for the moment

	// The ShaderSurfaceRenderer sets an array buffer which appearently conflicts with us
	// Reset it!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	float width = (rect.right - rect.left) * scale.x;
	float height = (rect.bottom - rect.top) * scale.y;

	float tex_left = (float) rect.left / (float) tex.getWidth();
	float tex_top = (float) rect.top / (float) tex.getHeight();
	float tex_right = (float) rect.right / (float) tex.getWidth();
	float tex_bottom = (float) rect.bottom / (float) tex.getHeight();

	if (mirrorX)
	{

	}

	if (mirrorY)
	{

	}

	// provide space for 3d position coords, 2d texture coords and a 32 bit colot value
	int vertex_size = 24;
	byte vertices[vertex_size * 4];

	for (int i = 0; i < vertex_size * 4; ++i) {
		vertices[i] = 0;
	}

	// texture coords
	*reinterpret_cast<float*>(vertices) = tex_left;
	*reinterpret_cast<float*>(vertices + 4) = tex_top;
	*reinterpret_cast<float*>(vertices + vertex_size) = tex_left;
	*reinterpret_cast<float*>(vertices + vertex_size + 4) = tex_bottom;
	*reinterpret_cast<float*>(vertices + 2 * vertex_size) = tex_right;
	*reinterpret_cast<float*>(vertices + 2 * vertex_size + 4) = tex_top;
	*reinterpret_cast<float*>(vertices + 3 * vertex_size) = tex_right;
	*reinterpret_cast<float*>(vertices + 3 * vertex_size + 4) = tex_bottom;

	// position coords
	*reinterpret_cast<float*>(vertices + 12) = pos.x - 0.5f;
	*reinterpret_cast<float*>(vertices + 12 + 4) = pos.y  + height - 0.5f;
	*reinterpret_cast<float*>(vertices + 12 + 8) = -1.1f;
	*reinterpret_cast<float*>(vertices + vertex_size + 12) = pos.x -0.5f;
	*reinterpret_cast<float*>(vertices + vertex_size + 12 + 4) = pos.y - 0.5f;
	*reinterpret_cast<float*>(vertices + vertex_size + 12 + 8) = -1.1f;
	*reinterpret_cast<float*>(vertices + 2 * vertex_size + 12) = pos.x + width - 0.5f;
	*reinterpret_cast<float*>(vertices + 2 * vertex_size + 12 + 4) = pos.y + height - 0.5f;
	*reinterpret_cast<float*>(vertices + 2 * vertex_size + 12 + 8) = -1.1f;
	*reinterpret_cast<float*>(vertices + 3 * vertex_size + 12) = pos.x + width - 0.5f;
	*reinterpret_cast<float*>(vertices + 3 * vertex_size + 12 + 4) = pos.y - 0.5;
	*reinterpret_cast<float*>(vertices + 3 * vertex_size + 12 + 8) = -1.1f;

	// not exactly sure about the color format, but this seems to work
	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	vertices[8] = r;
	vertices[8 + 1] = g;
	vertices[8 + 2] = b;
	vertices[8 + 3] = a;
	vertices[vertex_size + 8] = r;
	vertices[vertex_size + 8 + 1] = g;
	vertices[vertex_size + 8 + 2] = b;
	vertices[vertex_size + 8 + 3] = a;
	vertices[2 * vertex_size + 8] = r;
	vertices[2 * vertex_size + 8 + 1] = g;
	vertices[2 * vertex_size + 8 + 2] = b;
	vertices[2 * vertex_size + 8 + 3] = a;
	vertices[3 * vertex_size + 8] = r;
	vertices[3 * vertex_size + 8 + 2] = g;
	vertices[3 * vertex_size + 8 + 2] = b;
	vertices[3 * vertex_size + 8 + 3] = a;

	// transform vertices here if necessary, add offset

	if (alphaDisable)
	{
		glDisable(GL_ALPHA_TEST);
	}

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 24, vertices + 12);
	glColorPointer(4, GL_UNSIGNED_BYTE, 24, vertices + 8);
	glTexCoordPointer(2, GL_FLOAT, 24, vertices);

	glBindTexture(GL_TEXTURE_2D, tex.getTextureName());

	// we could do this in a vertex buffer anyways
	//glInterleavedArrays(GL_T2F_C4UB_V3F, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (alphaDisable)
	{
		glEnable(GL_ALPHA_TEST);
	}

	return true;
}
