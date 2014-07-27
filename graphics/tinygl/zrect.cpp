#include "graphics/tinygl/zrect.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/gl.h"

namespace TinyGL {

void glIssueDrawCall(Graphics::DrawCall *drawCall) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->_drawCallsQueue.push_back(drawCall);
}

} // end of namespace TinyGL


void tglDrawRectangle(Common::Rect rect) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	if (rect.left < 0)
		rect.left = 0;
	if (rect.right >= c->fb->xsize) 
		rect.right = c->fb->xsize - 1;
	if (rect.top < 0)
		rect.top = 0;
	if (rect.bottom >= c->fb->ysize) 
		rect.bottom = c->fb->ysize - 1;

	for(int x = rect.left; x < rect.right; x++) {
		c->fb->writePixel(rect.top * c->fb->xsize + x, 255, 0, 255, 0);
		c->fb->writePixel(rect.bottom * c->fb->xsize + x, 255, 0, 255, 0);
	}
	for(int y = rect.top; y < rect.bottom; y++) {
		c->fb->writePixel(y * c->fb->xsize + rect.left, 255, 0, 255, 0);
		c->fb->writePixel(y * c->fb->xsize + rect.right, 255, 0, 255, 0);
	}
}

void tglPresentBuffer() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	Common::List<Common::Rect> rectangles;

	Common::List<Graphics::DrawCall *>::const_iterator it = c->_drawCallsQueue.begin();
	Common::List<Graphics::DrawCall *>::const_iterator end = c->_drawCallsQueue.end();
	while (it != end) {
		(*it)->execute(false);
			rectangles.push_back((*it)->getDirtyRegion());
		delete (*it);
		it++;
	}

	c->_drawCallsQueue.clear();

	Common::List<Common::Rect>::const_iterator itRectangles = rectangles.begin();

	bool blendingEnabled = c->fb->isBlendingEnabled();
	bool alphaTestEnabled = c->fb->isAplhaTestEnabled();
	c->fb->enableBlending(false);
	c->fb->enableAlphaTest(false);

	while (itRectangles != rectangles.end()) {
		tglDrawRectangle(*itRectangles);
		itRectangles++;
	}

	c->fb->enableBlending(blendingEnabled);
	c->fb->enableAlphaTest(alphaTestEnabled);

	bool allDisposed;
	do {	
		allDisposed = true;
		TinyGL::GLTexture *t = c->shared_state.texture_hash_table[0];
		while (t) {
			if (t->disposed) {
				TinyGL::free_texture(c, t->handle);
				allDisposed = false;
				break;
			}
			t = t->next;
		}
		
	} while (allDisposed == false);

	Graphics::Internal::tglCleanupImages();
}

namespace Graphics {

DrawCall::DrawCall(DrawCallType type) : _type(type) {
}

RasterizationDrawCall::RasterizationDrawCall() : DrawCall(DrawCall_Rasterization) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	_vertexCount = c->vertex_cnt;
	_vertex = new TinyGL::GLVertex[_vertexCount];
	_drawTriangleFront = c->draw_triangle_front;
	_drawTriangleBack = c->draw_triangle_back;
	memcpy(_vertex, c->vertex, sizeof(TinyGL::GLVertex) * _vertexCount);
	_state = loadState();
}

void RasterizationDrawCall::execute(bool restoreState) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	TinyGL::GLVertex *prevVertex = c->vertex;
	int prevVertexCount = c->vertex_cnt;

	RasterizationDrawCall::RasterizationState backupState;
	if (restoreState) {
		backupState = loadState();
	}
	applyState(_state);

	c->vertex = _vertex;
	c->vertex_cnt = _vertexCount;
	c->draw_triangle_front = (TinyGL::gl_draw_triangle_func)_drawTriangleFront;
	c->draw_triangle_back = (TinyGL::gl_draw_triangle_func)_drawTriangleBack;

	int n, cnt;

	n = c->vertex_n;
	cnt = c->vertex_cnt;

	switch (c->begin_type) {
	case TGL_POINTS:
		for(int i = 0; i < cnt; i++) {
			gl_draw_point(c, &c->vertex[i]);
		}
		break;
	case TGL_LINES:
		for(int i = 0; i < cnt / 2; i++) {
			gl_draw_line(c, &c->vertex[i * 2], &c->vertex[i * 2 + 1]);
		}
		break;
	case TGL_LINE_STRIP:
	case TGL_LINE_LOOP:
		for(int i = 0; i < cnt; i++) {
			gl_draw_line(c, &c->vertex[i], &c->vertex[i + 1]);
		}
		gl_draw_line(c, &c->vertex[0], &c->vertex[cnt - 1]);
		break;
	case TGL_TRIANGLES:
		for(int i = 0; i < cnt / 3; i++) {
			gl_draw_triangle(c, &c->vertex[i * 3], &c->vertex[i * 3 + 1], &c->vertex[i * 3 + 2]);
		}
		break;
	case TGL_TRIANGLE_STRIP:
		for(int i = 0; i < cnt; i += 2) {
			gl_draw_triangle(c, &c->vertex[i], &c->vertex[i + 1], &c->vertex[i + 2]);
			gl_draw_triangle(c, &c->vertex[i + 2], &c->vertex[i + 1], &c->vertex[i + 3]);
		}
		break;
	case TGL_TRIANGLE_FAN:
		for(int i = 1; i < cnt; i += 2) {
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[i], &c->vertex[i + 1]);
		}
		break;
	case TGL_QUADS:
		for(int i = 0; i < cnt / 4; i++) {
			c->vertex[i + 2].edge_flag = 0;
			gl_draw_triangle(c, &c->vertex[i], &c->vertex[i + 1], &c->vertex[i + 2]);
			c->vertex[i + 2].edge_flag = 1;
			c->vertex[i + 0].edge_flag = 0;
			gl_draw_triangle(c, &c->vertex[i], &c->vertex[i + 2], &c->vertex[i + 3]);
		}
		break;
	case TGL_QUAD_STRIP:
		while (n >= 4) {
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			gl_draw_triangle(c, &c->vertex[1], &c->vertex[3], &c->vertex[2]);
			for (int i = 0; i < 2; i++)
				c->vertex[i] = c->vertex[i + 2];
			n -= 2;
		}
		break;
	case TGL_POLYGON: {
		int i = c->vertex_cnt;
		while (i >= 3) {
			i--;
			gl_draw_triangle(c, &c->vertex[i], &c->vertex[0], &c->vertex[i - 1]);
		}
		break;
	}
	default:
		error("glBegin: type %x not handled", c->begin_type);
	}

	c->vertex = prevVertex;
	c->vertex_cnt = prevVertexCount;
	if (restoreState) {
		applyState(backupState);
	}
}

RasterizationDrawCall::RasterizationState RasterizationDrawCall::loadState() const {
	RasterizationState state;
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	state.alphaTest = c->fb->isAplhaTestEnabled();
	c->fb->getBlendingFactors(state.sfactor, state.dfactor);
	state.enableBlending = c->fb->isBlendingEnabled();
	state.alphaFunc = c->fb->getAlphaTestFunc();
	state.alphaRefValue = c->fb->getAlphaTestRefVal();

	state.cullFaceEnabled = c->cull_face_enabled;
	state.beginType = c->begin_type;
	state.colorMask = c->color_mask;
	state.currentFrontFace = c->current_front_face;
	state.currentShadeModel = c->current_shade_model;
	state.depthTest = c->depth_test;
	state.polygonModeBack = c->polygon_mode_back;
	state.polygonModeFront = c->polygon_mode_front;
	state.shadowMode = c->shadow_mode;
	state.texture2DEnabled = c->texture_2d_enabled;
	state.texture = c->current_texture;
	state.shadowMaskBuf = c->fb->shadow_mask_buf;
	state.depthFunction = c->fb->getDepthFunc();
	state.depthWrite = c->fb->getDepthWrite();
	state.lightingEnabled = c->lighting_enabled;

	memcpy(state.viewportScaling, c->viewport.scale._v, sizeof(c->viewport.scale._v));
	memcpy(state.viewportTranslation, c->viewport.trans._v, sizeof(c->viewport.trans._v));
	memcpy(state.currentColor, c->longcurrent_color, sizeof(c->longcurrent_color));

	return state;
}

void RasterizationDrawCall::applyState(const RasterizationDrawCall::RasterizationState &state) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->setBlendingFactors(state.sfactor, state.dfactor);
	c->fb->enableBlending(state.enableBlending);
	c->fb->enableAlphaTest(state.alphaTest);
	c->fb->setAlphaTestFunc(state.alphaFunc, state.alphaRefValue);
	c->fb->setDepthFunc(state.depthFunction);
	c->fb->enableDepthWrite(state.depthWrite);

	c->lighting_enabled = state.lightingEnabled;
	c->cull_face_enabled = state.cullFaceEnabled;
	c->begin_type = state.beginType;
	c->color_mask = state.colorMask;
	c->current_front_face = state.currentFrontFace;
	c->current_shade_model = state.currentShadeModel;
	c->depth_test = state.depthTest;
	c->polygon_mode_back = state.polygonModeBack;
	c->polygon_mode_front = state.polygonModeFront;
	c->shadow_mode = state.shadowMode;
	c->texture_2d_enabled = state.texture2DEnabled;
	c->current_texture = state.texture; 
	c->fb->shadow_mask_buf = state.shadowMaskBuf;

	memcpy(c->viewport.scale._v, state.viewportScaling, sizeof(c->viewport.scale._v));
	memcpy(c->viewport.trans._v, state.viewportTranslation, sizeof(c->viewport.trans._v));
	memcpy(c->longcurrent_color, state.currentColor, sizeof(c->longcurrent_color));
}

RasterizationDrawCall::~RasterizationDrawCall() {
	delete [] _vertex;
}

void RasterizationDrawCall::execute(const Common::Rect &clippingRectangle, bool restoreState) const {

}

const Common::Rect RasterizationDrawCall::getDirtyRegion() const {
	return Common::Rect(0, 0, 0, 0);
}

BlittingDrawCall::BlittingDrawCall(Graphics::BlitImage *image, const BlitTransform &transform, BlittingMode blittingMode) : DrawCall(DrawCall_Blitting), _transform(transform), _mode(blittingMode), _image(image) {
	_blitState = loadState();
}

void BlittingDrawCall::execute(bool restoreState) const {
	BlittingState backupState;
	if (restoreState) {
		backupState = loadState();
	}
	applyState(_blitState);

	switch (_mode) {
	case Graphics::BlittingDrawCall::BlitMode_Regular:
		Graphics::Internal::tglBlit(_image, _transform);
		break;
	case Graphics::BlittingDrawCall::BlitMode_NoBlend:
		Graphics::Internal::tglBlitNoBlend(_image, _transform);
		break;
	case Graphics::BlittingDrawCall::BlitMode_Fast:
		Graphics::Internal::tglBlitFast(_image, _transform._destinationRectangle.left, _transform._destinationRectangle.top);
		break;
	case Graphics::BlittingDrawCall::BlitMode_ZBuffer:
		Graphics::Internal::tglBlitZBuffer(_image, _transform._destinationRectangle.left, _transform._destinationRectangle.top);
		break;
	default:
		break;
	}
	if (restoreState) {
		applyState(backupState);
	}
}

void BlittingDrawCall::execute(const Common::Rect &clippingRectangle, bool restoreState) const {

}

BlittingDrawCall::BlittingState BlittingDrawCall::loadState() const {
	BlittingState state;
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	state.alphaTest = c->fb->isAplhaTestEnabled();
	c->fb->getBlendingFactors(state.sfactor, state.dfactor);
	state.enableBlending = c->fb->isBlendingEnabled();
	state.alphaFunc = c->fb->getAlphaTestFunc();
	state.alphaRefValue = c->fb->getAlphaTestRefVal();
	return state;
}

void BlittingDrawCall::applyState(const BlittingState &state) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->setBlendingFactors(state.sfactor, state.dfactor);
	c->fb->enableBlending(state.enableBlending);
	c->fb->enableAlphaTest(state.alphaTest);
	c->fb->setAlphaTestFunc(state.alphaFunc, state.alphaRefValue);
}

const Common::Rect BlittingDrawCall::getDirtyRegion() const {
	int blitWidth = _transform._destinationRectangle.width();
	int blitHeight = _transform._destinationRectangle.width();
	if (blitWidth == 0) {
		if (_transform._sourceRectangle.width() != 0) {
			blitWidth = _transform._sourceRectangle.width();
		} else {
			tglGetBlitImageSize(_image, blitWidth, blitHeight);
		}
	} 
	if (blitHeight == 0) {
		if (_transform._sourceRectangle.height() != 0) {
			blitHeight = _transform._sourceRectangle.height();
		} else {
			tglGetBlitImageSize(_image, blitWidth, blitHeight);
		}
	}
	return Common::Rect(_transform._destinationRectangle.left, _transform._destinationRectangle.top, _transform._destinationRectangle.left + blitWidth, _transform._destinationRectangle.top + blitHeight);
}

ClearBufferDrawCall::ClearBufferDrawCall(bool clearZBuffer, int zValue, bool clearColorBuffer, int rValue, int gValue, int bValue) : clearZBuffer(clearZBuffer), 
	clearColorBuffer(clearColorBuffer), zValue(zValue), rValue(rValue), gValue(gValue), bValue(bValue), DrawCall(DrawCall_Clear) {

}

void ClearBufferDrawCall::execute(bool restoreState) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->clear(clearZBuffer, zValue, clearColorBuffer, rValue, gValue, bValue);
}

void ClearBufferDrawCall::execute(const Common::Rect &clippingRectangle, bool restoreState) const {

}

const Common::Rect ClearBufferDrawCall::getDirtyRegion() const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	return Common::Rect(0, 0, c->fb->xsize, c->fb->ysize);
}

} // end of namespace Graphics
