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
#include "graphics/opengles2/shader.h"
#include "common/stack.h"

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
	 * @param fullscreen	true if fullscreen is desired, false otherwise.
	 */
	virtual byte *setupScreen(int screenW, int screenH, bool fullscreen);

	/**
	 * Query whether the current context is hardware-accelerated
	 *
	 * @return true if hw-accelerated, false otherwise
	 */
	virtual bool isHardwareAccelerated() { return true; } ;
	virtual void setupCamera(float fov, float nclip, float fclip, float roll);
	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll);

	Math::Matrix4 getModelView();
	Math::Matrix4 getProjection();

	virtual void clearScreen();

	/**
	 *	Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer();

	virtual void getBoundingBoxPos(const Mesh *mesh, int *x1, int *y1, int *x2, int *y2);
	void getBoundingBoxPos(const EMIModel *model, int *x1, int *y1, int *x2, int *y2);
	virtual void startActorDraw(const Actor *actor);

	virtual void finishActorDraw();
	virtual void setShadow(Shadow *shadow);
	virtual void drawShadowPlanes();
	virtual void setShadowMode();
	virtual void clearShadowMode();
	bool isShadowModeActive();
	virtual void setShadowColor(byte r, byte g, byte b);
	virtual void getShadowColor(byte *r, byte *g, byte *b);

	virtual void set3DMode();

	virtual void translateViewpointStart();
	virtual void translateViewpoint(const Math::Vector3d &vec);
	virtual void rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis);
	virtual void translateViewpointFinish();

	virtual void drawEMIModelFace(const EMIModel* model, const EMIMeshFace* face);
	virtual void drawModelFace(const Mesh *mesh, const MeshFace *face);
	virtual void drawSprite(const Sprite *sprite);
	virtual void drawMesh(const Mesh *mesh);

	virtual void enableLights();
	virtual void disableLights();
	virtual void setupLight(Light *light, int lightId);
	virtual void turnOffLight(int lightId);

	virtual void createMaterial(Texture *material, const char *data, const CMap *cmap);
	virtual void selectMaterial(const Texture *material);
	virtual void destroyMaterial(Texture *material);

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
	virtual void createBitmap(BitmapData *bitmap);

	/**
	 * Draws a bitmap
	 * before this is safe to use, createBitmap MUST have been called
	 *
	 * @param bitmap	the bitmap to be drawn
	 * @see createBitmap
	 * @see destroyBitmap
	 */
	virtual void drawBitmap(const Bitmap *bitmap, int x, int y, uint32 layer = 0);

	/**
	 * Deletes any internal references and representations of a bitmap
	 * after this is called, it is safe to dispose of or change the external
	 * bitmapdata.
	 *
	 * @param bitmap	the bitmap to be destroyed
	 * @see createBitmap
	 * @see drawBitmap
	 */
	virtual void destroyBitmap(BitmapData *bitmap);

	virtual void createFont(Font *font);
	virtual void destroyFont(Font *font);

	virtual void createTextObject(TextObject *text);
	virtual void drawTextObject(const TextObject *text);
	virtual void destroyTextObject(TextObject *text);

	virtual Bitmap *getScreenshot(int w, int h);
	virtual void storeDisplay();
	virtual void copyStoredToDisplay();

	/**
	 * Dims the entire screen
	 * Sets the entire screen to 10% of its current brightness,
	 * and converts it to grayscale.
	 */
	virtual void dimScreen();
	virtual void dimRegion(int x, int y, int w, int h, float level);

	/**
	 * Draw a completely opaque Iris around the specified rectangle.
	 * the arguments specify the distance from the screen-edge to the first
	 * non-iris pixel.
	 *
	 * @param x		the width of the Iris
	 * @param y		the height of the Iris
	 */
	virtual void irisAroundRegion(int x1, int y1, int x2, int y2);

	virtual void drawEmergString(int x, int y, const char *text, const Color &fgColor);
	virtual void loadEmergFont();

	virtual void drawRectangle(const PrimitiveObject *primitive);
	virtual void drawLine(const PrimitiveObject *primitive);
	virtual void drawPolygon(const PrimitiveObject *primitive);

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
	virtual void prepareMovieFrame(Graphics::Surface* frame);
	virtual void drawMovieFrame(int offsetX, int offsetY);

	/**
	 * Release the currently prepared movie-frame, if one exists.
	 *
	 * @see drawMovieFrame
	 * @see prepareMovieFrame
	 */
	virtual void releaseMovieFrame();

	virtual const char *getVideoDeviceName();

	void renderBitmaps(bool render);
	void renderZBitmaps(bool render);

	virtual void createSpecialtyTextures();

	virtual void createModel(Mesh *mesh);
	virtual void createEMIModel(EMIModel *model);
	virtual void updateEMIModel(const EMIModel* model);

protected:
	void setupShaders();
	GLuint compileShader(const char *vertex, const char *fragment);
	GLuint compileShader(const char *shader) { return compileShader(shader, shader); }

private:
	const Actor *_currentActor;
	float _alpha;
	int _maxLights;
	GLuint _emergTexture;
	Graphics::Shader* _emergProgram;

	Graphics::Shader* _backgroundProgram;
	Graphics::Shader* _actorProgram;
	Graphics::Shader* _spriteProgram;
	Graphics::Shader* _smushProgram;
	GLuint _smushVBO, _quadEBO;
	Graphics::Shader* _textProgram;
	Graphics::Shader* _primitiveProgram;
	Graphics::Shader* _irisProgram;
	Graphics::Shader* _shadowPlaneProgram;

	int _smushWidth;
	int _smushHeight;
	GLuint _smushTexId;
	bool _smushSwizzle;
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

	void setupTexturedCenteredQuad();

	GLuint _spriteVBO;

	Common::Stack<Math::Matrix4> _matrixStack;
	Texture *_selectedTexture;

	GLuint _zBufTex;
	Math::Vector2d _zBufTexCrop;

	struct Light {
		Math::Vector4d _position;
		Math::Vector4d _direction;
		Math::Vector4d _color;
	};

	Light *_lights;
	bool _lightsEnabled;

	void setupPrimitives();
	GLuint nextPrimitive();
	GLuint _primitiveVBOs[32];
	uint32 _currentPrimitive;
	void drawGenericPrimitive(const float *vertices, uint32 numVertices, const PrimitiveObject *primitive);
	GLuint _irisVBO;
	GLuint _blastVBO;
};
}
#endif
