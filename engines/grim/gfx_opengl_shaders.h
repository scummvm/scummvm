/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_GFX_OPENGL_SHADERS_H
#define GRIM_GFX_OPENGL_SHADERS_H

#include "engines/grim/actor.h"
#include "engines/grim/gfx_base.h"
#include "graphics/opengl/shader.h"
#include "common/stack.h"
#include "common/rect.h"

namespace Grim {

class GfxOpenGLS : public GfxBase {
public:
	GfxOpenGLS();
	virtual ~GfxOpenGLS();

	/**
	 * Creates a render-context.
	 *
	 * @param screenW		the width of the context
	 * @param screenH		the height of the context
	 */
	virtual void setupScreen(int screenW, int screenH) override;

	/**
	 * Query whether the current context is hardware-accelerated
	 *
	 * @return true if hw-accelerated, false otherwise
	 */
	virtual bool isHardwareAccelerated() override { return true; };
	virtual bool supportsShaders() override { return true; }
	virtual void setupCameraFrustum(float fov, float nclip, float fclip) override;
	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) override;
	virtual void positionCamera(const Math::Vector3d &pos, const Math::Matrix4 &rot) override;

	virtual Math::Matrix4 getModelView() override;
	virtual Math::Matrix4 getProjection() override;

	virtual void clearScreen() override;
	virtual void clearDepthBuffer() override;

	/**
	 *	Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() override;

	virtual void getScreenBoundingBox(const Mesh *mesh, int *x1, int *y1, int *x2, int *y2) override;
	virtual void getScreenBoundingBox(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) override;
	void getActorScreenBBox(const Actor *actor, Common::Point &p1, Common::Point &p2) override;
	virtual void startActorDraw(const Actor *actor) override;

	virtual void finishActorDraw() override;
	virtual void setShadow(Shadow *shadow) override;
	virtual void drawShadowPlanes() override;
	virtual void setShadowMode() override;
	virtual void clearShadowMode() override;
	virtual bool isShadowModeActive() override;
	virtual void setShadowColor(byte r, byte g, byte b) override;
	virtual void getShadowColor(byte *r, byte *g, byte *b) override;
	virtual void destroyShadow(Shadow *shadow) override;

	virtual void set3DMode() override;

	virtual void translateViewpointStart() override;
	virtual void translateViewpoint(const Math::Vector3d &vec) override;
	virtual void rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis) override;
	virtual void rotateViewpoint(const Math::Matrix4 &rot) override;
	virtual void translateViewpointFinish() override;

	virtual void drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face) override;
	virtual void drawModelFace(const Mesh *mesh, const MeshFace *face) override;
	virtual void drawSprite(const Sprite *sprite) override;
	virtual void drawMesh(const Mesh *mesh) override;
	virtual void drawDimPlane() override;

	virtual void enableLights() override;
	virtual void disableLights() override;
	virtual void setupLight(Light *light, int lightId) override;
	virtual void turnOffLight(int lightId) override;

	virtual void createTexture(Texture *texture, const uint8 *data, const CMap *cmap, bool clamp) override;
	virtual void selectTexture(const Texture *texture) override;
	virtual void destroyTexture(Texture *texture) override;

	/**
	 * Prepares a bitmap for drawing
	 * performs any format conversions needed for the renderer,
	 * and might create an internal representation of the bitmap
	 * external changes to the bitmap may not be visible after this
	 * is called. Must be called before drawBitmap can be used.
	 *
	 * the external bitmap might have its data changed by this function,
	 *
	 * @param bitmap	the bitmap to be prepared
	 * @see destroyBitmap
	 * @see drawBitmap
	 */
	virtual void createBitmap(BitmapData *bitmap) override;

	/**
	 * Draws a bitmap
	 * before this is safe to use, createBitmap MUST have been called
	 *
	 * @param bitmap	the bitmap to be drawn
	 * @see createBitmap
	 * @see destroyBitmap
	 */
	virtual void drawBitmap(const Bitmap *bitmap, int x, int y, uint32 layer = 0) override;

	/**
	 * Deletes any internal references and representations of a bitmap
	 * after this is called, it is safe to dispose of or change the external
	 * bitmapdata.
	 *
	 * @param bitmap	the bitmap to be destroyed
	 * @see createBitmap
	 * @see drawBitmap
	 */
	virtual void destroyBitmap(BitmapData *bitmap) override;

	virtual void createFont(Font *font) override;
	virtual void destroyFont(Font *font) override;

	virtual void createTextObject(TextObject *text) override;
	virtual void drawTextObject(const TextObject *text) override;
	virtual void destroyTextObject(TextObject *text) override;

	virtual Bitmap *getScreenshot(int w, int h, bool useStored) override;
	virtual void storeDisplay() override;
	virtual void copyStoredToDisplay() override;

	/**
	 * Dims the entire screen
	 * Sets the entire screen to 10% of its current brightness,
	 * and converts it to grayscale.
	 */
	virtual void dimScreen() override;
	virtual void dimRegion(int x, int y, int w, int h, float level) override;

	/**
	 * Draw a completely opaque Iris around the specified rectangle.
	 * the arguments specify the distance from the screen-edge to the first
	 * non-iris pixel.
	 *
	 * @param x		the width of the Iris
	 * @param y		the height of the Iris
	 */
	virtual void irisAroundRegion(int x1, int y1, int x2, int y2) override;

	virtual void drawEmergString(int x, int y, const char *text, const Color &fgColor) override;
	virtual void loadEmergFont() override;

	virtual void drawRectangle(const PrimitiveObject *primitive) override;
	virtual void drawLine(const PrimitiveObject *primitive) override;
	virtual void drawPolygon(const PrimitiveObject *primitive) override;

	/**
	 * Prepare a movie-frame for drawing
	 * performing any necessary conversion
	 *
	 * @param width			the width of the movie-frame.
	 * @param height		the height of the movie-frame.
	 * @param bitmap		a pointer to the data for the movie-frame.
	 * @see drawMovieFrame
	 * @see releaseMovieFrame
	 */
	virtual void prepareMovieFrame(Graphics::Surface* frame) override;
	virtual void drawMovieFrame(int offsetX, int offsetY) override;

	/**
	 * Release the currently prepared movie-frame, if one exists.
	 *
	 * @see drawMovieFrame
	 * @see prepareMovieFrame
	 */
	virtual void releaseMovieFrame() override;

	virtual const char *getVideoDeviceName() override;

	virtual void renderBitmaps(bool render) override;
	virtual void renderZBitmaps(bool render) override;

	virtual void createMesh(Mesh *mesh) override;
	virtual void destroyMesh(const Mesh *mesh) override;
	virtual void createEMIModel(EMIModel *model) override;
	virtual void updateEMIModel(const EMIModel* model) override;
	virtual void destroyEMIModel(EMIModel *model) override;

	virtual void setBlendMode(bool additive) override;

protected:
	void setupShaders();
	GLuint compileShader(const char *vertex, const char *fragment);
	GLuint compileShader(const char *shader) { return compileShader(shader, shader); }
	void createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) override;

private:
	const Actor *_currentActor;
	float _alpha;
	int _maxLights;
	GLuint _emergTexture;
	OpenGL::ShaderGL* _emergProgram;

	OpenGL::ShaderGL* _backgroundProgram;
	OpenGL::ShaderGL* _actorProgram;
	OpenGL::ShaderGL* _actorLightsProgram;
	OpenGL::ShaderGL* _spriteProgram;
	OpenGL::ShaderGL* _dimProgram;
	OpenGL::ShaderGL* _dimPlaneProgram;
	OpenGL::ShaderGL* _dimRegionProgram;
	OpenGL::ShaderGL* _smushProgram;
	GLuint _smushVBO, _quadEBO;
	OpenGL::ShaderGL* _textProgram;
	OpenGL::ShaderGL* _primitiveProgram;
	OpenGL::ShaderGL* _irisProgram;
	OpenGL::ShaderGL* _shadowPlaneProgram;

	int _smushWidth;
	int _smushHeight;
	GLuint _smushTexId;
	bool _smushSwizzle;
	bool _smushSwap;
	void setupTexturedQuad();
	void setupQuadEBO();

	void setupZBuffer();
	void drawDepthBitmap(int x, int y, int w, int h, char *data);

	float _fov;
	float _nclip;
	float _fclip;
	Math::Matrix4 _projMatrix;
	Math::Matrix4 _viewMatrix;
	Math::Matrix4 _mvpMatrix;
	Math::Matrix4 _overworldProjMatrix;

	void setupTexturedCenteredQuad();

	GLuint _spriteVBO;

	Common::Stack<Math::Matrix4> _matrixStack;
	Texture *_selectedTexture;

	GLuint _zBufTex;
	Math::Vector2d _zBufTexCrop;

	struct GLSLight {
		Math::Vector4d _position;
		Math::Vector4d _direction;
		Math::Vector4d _color;
		Math::Vector4d _params;
	};

	GLSLight *_lights;
	bool _hasAmbientLight;
	bool _lightsEnabled;

	void setupPrimitives();
	GLuint nextPrimitive();
	GLuint _primitiveVBOs[32];
	uint32 _currentPrimitive;
	void drawGenericPrimitive(const float *vertices, uint32 numVertices, const PrimitiveObject *primitive);
	GLuint _irisVBO;
	GLuint _dimVBO;
	GLuint _dimRegionVBO;
	GLuint _blastVBO;
	GLuint _storedDisplay;
};
}
#endif
