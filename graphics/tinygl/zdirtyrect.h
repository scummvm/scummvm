/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRAPHICS_TINYGL_ZRECT_H
#define GRAPHICS_TINYGL_ZRECT_H

#include "common/types.h"
#include "common/rect.h"
#include "common/array.h"

#include "graphics/tinygl/zblit.h"

namespace TinyGL {

namespace Internal {
void *allocateFrame(int size);
}

struct GLContext;
struct GLVertex;
struct GLTexture;

class DrawCall {
public:

	enum DrawCallType {
		DrawCall_Rasterization,
		DrawCall_Blitting,
		DrawCall_Clear
	};

	DrawCall(DrawCallType type) : _type(type) { }
	virtual ~DrawCall() { }
	bool operator==(const DrawCall &other) const;
	bool operator!=(const DrawCall &other) const {
		return !(*this == other);
	}
	virtual void execute(bool restoreState) const = 0;
	virtual void execute(const Common::Rect &clippingRectangle, bool restoreState) const = 0;
	DrawCallType getType() const { return _type; }
	virtual const Common::Rect getDirtyRegion() const { return _dirtyRegion; }
protected:
	Common::Rect _dirtyRegion;
private:
	DrawCallType _type;
};

class ClearBufferDrawCall : public DrawCall {
public:
	ClearBufferDrawCall(bool clearZBuffer, int zValue, bool clearColorBuffer, int rValue, int gValue, int bValue, bool clearStencilBuffer, int stencilValue);
	virtual ~ClearBufferDrawCall() { }
	bool operator==(const ClearBufferDrawCall &other) const;
	virtual void execute(bool restoreState) const;
	virtual void execute(const Common::Rect &clippingRectangle, bool restoreState) const;

	void *operator new(size_t size) {
		return Internal::allocateFrame(size);
	}

	void operator delete(void *p) { }
private:
	bool _clearZBuffer, _clearColorBuffer, _clearStencilBuffer;
	int _rValue, _gValue, _bValue, _zValue, _stencilValue;
};

// Encapsulate a rasterization call: it might execute either a triangle or line rasterization.
class RasterizationDrawCall : public DrawCall {
public:
	RasterizationDrawCall();
	virtual ~RasterizationDrawCall() { }
	bool operator==(const RasterizationDrawCall &other) const;
	virtual void execute(bool restoreState) const;
	virtual void execute(const Common::Rect &clippingRectangle, bool restoreState) const;

	void *operator new(size_t size) {
		return Internal::allocateFrame(size);
	}

	void operator delete(void *p) { }
private:
	void computeDirtyRegion();
	typedef void (*gl_draw_triangle_func_ptr)(GLContext *c, TinyGL::GLVertex *p0, TinyGL::GLVertex *p1, TinyGL::GLVertex *p2);
	int _vertexCount;
	GLVertex *_vertex;
	gl_draw_triangle_func_ptr _drawTriangleFront, _drawTriangleBack;

	struct RasterizationState {
		int beginType;
		int currentFrontFace;
		int cullFaceEnabled;
		bool colorMaskRed;
		bool colorMaskGreen;
		bool colorMaskBlue;
		bool colorMaskAlpha;
		bool depthTestEnabled;
		int depthFunction;
		int depthWriteMask;
		bool texture2DEnabled;
		int currentShadeModel;
		int polygonModeBack;
		int polygonModeFront;
		int lightingEnabled;
		bool enableBlending;
		int sfactor;
		int dfactor;
		int textureVersion;
		int offsetStates;
		float offsetFactor;
		float offsetUnits;
		float viewportTranslation[3];
		float viewportScaling[3];
		bool alphaTestEnabled;
		int alphaFunc;
		int alphaRefValue;
		bool stencilTestEnabled;
		int stencilTestFunc;
		int stencilValue;
		uint stencilMask;
		uint stencilWriteMask;
		int stencilSfail;
		int stencilDpfail;
		int stencilDppass;
		GLTexture *texture;
		uint wrapS, wrapT;
		bool fogEnabled;
		float fogColorR;
		float fogColorG;
		float fogColorB;

		bool operator==(const RasterizationState &other) const;
	};

	RasterizationState _state;

	RasterizationState captureState() const;
	void applyState(const RasterizationState &state) const;
};

// Encapsulate a blit call: it might execute either a color buffer or z buffer blit.
class BlittingDrawCall : public DrawCall {
public:
	enum BlittingMode {
		BlitMode_Regular,
		BlitMode_Fast,
		BlitMode_ZBuffer
	};

	BlittingDrawCall(BlitImage *image, const BlitTransform &transform, BlittingMode blittingMode);
	virtual ~BlittingDrawCall();
	bool operator==(const BlittingDrawCall &other) const;
	virtual void execute(bool restoreState) const;
	virtual void execute(const Common::Rect &clippingRectangle, bool restoreState) const;

	BlittingMode getBlittingMode() const { return _mode; }

	void *operator new(size_t size) {
		return Internal::allocateFrame(size);
	}

	void operator delete(void *p) { }
private:
	void computeDirtyRegion();
	BlitImage *_image;
	BlitTransform _transform;
	BlittingMode _mode;
	int _imageVersion;

	struct BlittingState {
		bool enableBlending;
		int sfactor, dfactor;
		bool alphaTest;
		int alphaFunc, alphaRefValue;
		int depthTestEnabled;

		bool operator==(const BlittingState &other) const {
			return
				enableBlending == other.enableBlending &&
				sfactor == other.sfactor &&
				dfactor == other.dfactor &&
				alphaTest == other.alphaTest &&
				alphaFunc == other.alphaFunc &&
				alphaRefValue == other.alphaRefValue &&
				depthTestEnabled == other.depthTestEnabled;
		}
	};

	BlittingState captureState() const;
	void applyState(const BlittingState &state) const;

	BlittingState _blitState;
};

} // end of namespace TinyGL

#endif
