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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_GFX_TINYGL_H
#define GRIM_GFX_TINYGL_H

#include "engines/grim/gfx_base.h"

#include "graphics/tinygl/zgl.h"

namespace TinyGL {
	struct Buffer;
}

namespace Grim {

class ModelNode;
class Mesh;
class MeshFace;
class BlitImage;

class GfxTinyGL : public GfxBase {
public:
	GfxTinyGL();
	virtual ~GfxTinyGL();

	byte *setupScreen(int screenW, int screenH, bool fullscreen) override;

	const char *getVideoDeviceName() override;

	void setupCamera(float fov, float nclip, float fclip, float roll) override;
	void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) override;

	Math::Matrix4 getModelView() override;
	Math::Matrix4 getProjection() override;

	void clearScreen() override;
	void clearDepthBuffer() override;
	void flipBuffer() override;

	bool isHardwareAccelerated() override;
	bool supportsShaders() override;

	void getBoundingBoxPos(const Mesh *model, int *x1, int *y1, int *x2, int *y2) override;
	void getBoundingBoxPos(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) override;

	void startActorDraw(const Actor *actor) override;
	void finishActorDraw() override;
	void setShadow(Shadow *shadow) override;
	void drawShadowPlanes() override;
	void setShadowMode() override;
	void clearShadowMode() override;
	void setShadowColor(byte r, byte g, byte b) override;
	void getShadowColor(byte *r, byte *g, byte *b) override;

	void set3DMode() override;

	void translateViewpointStart() override;
	void translateViewpoint(const Math::Vector3d &vec) override;
	void rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis) override;
	void rotateViewpoint(const Math::Matrix4 &matrix) override;
	void translateViewpointFinish() override;

	void drawEMIModelFace(const EMIModel *model, const EMIMeshFace *face) override;
	void drawModelFace(const Mesh *mesh, const MeshFace *face) override;
	void drawSprite(const Sprite *sprite) override;

	void enableLights() override;
	void disableLights() override;
	void setupLight(Light *light, int lightId) override;
	void turnOffLight(int lightId) override;

	void createTexture(Texture *texture, const char *data, const CMap *cmap, bool clamp) override;
	void selectTexture(const Texture *texture) override;
	void destroyTexture(Texture *texture) override;

	void createBitmap(BitmapData *bitmap) override;
	void drawBitmap(const Bitmap *bitmap, int x, int y, uint32 layer) override;
	void destroyBitmap(BitmapData *bitmap) override;

	void createFont(Font *font) override;
	void destroyFont(Font *font) override;

	void drawTextObject(const TextObject *text) override;
	void createTextObject(TextObject *text) override;
	void destroyTextObject(TextObject *text) override;

	void dimScreen() override;
	void dimRegion(int x, int y, int w, int h, float level) override;
	void irisAroundRegion(int x1, int y1, int x2, int y2) override;

	Bitmap *getScreenshot(int w, int h) override;
	void storeDisplay() override;
	void copyStoredToDisplay() override;

	void drawEmergString(int x, int y, const char *text, const Color &fgColor) override;
	void loadEmergFont() override;

	void drawRectangle(const PrimitiveObject *primitive) override;
	void drawLine(const PrimitiveObject *primitive) override;
	void drawPolygon(const PrimitiveObject *primitive) override;

	void prepareMovieFrame(Graphics::Surface *frame) override;
	void drawMovieFrame(int offsetX, int offsetY) override;
	void releaseMovieFrame() override;

	void createSpecialtyTextures() override;

	int genBuffer() override;
	void delBuffer(int buffer) override;
	void selectBuffer(int buffer) override;
	void clearBuffer(int buffer) override;
	void drawBuffers() override;
	void refreshBuffers() override;

protected:

private:
	TinyGL::FrameBuffer *_zb;
	Graphics::BlitImage *_smushImage;
	Graphics::PixelBuffer _storedDisplay;
	float _alpha;
	Common::HashMap<int, TinyGL::Buffer *> _buffers;
	uint _bufferId;
	const Actor *_currentActor;
	TGLenum _depthFunc;

	void readPixels(int x, int y, int width, int height, uint8 *buffer);
	void blit(const Graphics::PixelFormat &format, BlitImage *blit, byte *dst, byte *src, int x, int y, int width, int height, bool trans);
	void blit(const Graphics::PixelFormat &format, BlitImage *blit, byte *dst, byte *src, int dstX, int dstY, int srcX, int srcY, int width, int height, int srcWidth, int srcHeight, bool trans);
};

} // end of namespace Grim

#endif
