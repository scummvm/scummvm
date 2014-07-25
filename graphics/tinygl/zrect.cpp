#include "graphics/tinygl/zrect.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/gl.h"

namespace TinyGL {

void glIssueDrawCall(Graphics::DrawCall *drawCall) {
	drawCall->execute();
	delete drawCall;
}

} // end of namespace TinyGL


void tglPresentBuffer() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	for(int i = 0; i < c->_drawCallsQueue.size(); ++i) {
		c->_drawCallsQueue[i]->execute();
		delete c->_drawCallsQueue[i];
	}
	c->_drawCallsQueue.clear();

	TinyGL::GLTexture *t = c->shared_state.texture_hash_table[0];
	while (t) {
		int handle = t->handle;
		if (t->disposed) {
			TinyGL::free_texture(c, handle);
		}
		t = t->next;
	}

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

void RasterizationDrawCall::execute() const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	TinyGL::GLVertex *prevVertex = c->vertex;
	int prevVertexCount = c->vertex_cnt;

	RasterizationDrawCall::RasterizationState backupState = loadState();
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
		gl_draw_point(c, &c->vertex[0]);
		n = 0;
		break;
	case TGL_LINES:
		if (n == 2) {
			gl_draw_line(c, &c->vertex[0], &c->vertex[1]);
			n = 0;
		}
		break;
	case TGL_LINE_STRIP:
	case TGL_LINE_LOOP:
		if (n == 1) {
			c->vertex[2] = c->vertex[0];
		} else if (n == 2) {
			gl_draw_line(c, &c->vertex[0], &c->vertex[1]);
			c->vertex[0] = c->vertex[1];
			n = 1;
		} else if (c->vertex_cnt >= 3) {
			gl_draw_line(c, &c->vertex[0], &c->vertex[2]);
		}
		break;
	case TGL_TRIANGLES:
		if (n == 3) {
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			n = 0;
		}
		break;
	case TGL_TRIANGLE_STRIP:
		if (cnt >= 3) {
			if (n == 3)
				n = 0;
			// needed to respect triangle orientation
			switch (cnt & 1) {
			case 0:
				gl_draw_triangle(c, &c->vertex[2], &c->vertex[1], &c->vertex[0]);
				break;
			case 1:
				gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
				break;
			}
		}
		break;
	case TGL_TRIANGLE_FAN:
		if (n == 3) {
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			c->vertex[1] = c->vertex[2];
			n = 2;
		}
		break;
	case TGL_QUADS:
		if (n == 4) {
			c->vertex[2].edge_flag = 0;
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			c->vertex[2].edge_flag = 1;
			c->vertex[0].edge_flag = 0;
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[2], &c->vertex[3]);
			n = 0;
		}
		break;
	case TGL_QUAD_STRIP:
		if (n == 4) {
			gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
			gl_draw_triangle(c, &c->vertex[1], &c->vertex[3], &c->vertex[2]);
			for (int i = 0; i < 2; i++)
				c->vertex[i] = c->vertex[i + 2];
			n = 2;
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
	applyState(backupState);
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

void RasterizationDrawCall::execute( const Common::Rect &clippingRectangle ) const {

}

BlittingDrawCall::BlittingDrawCall(Graphics::BlitImage *image, const BlitTransform &transform, BlittingMode blittingMode) : DrawCall(DrawCall_Blitting), _transform(transform), _mode(blittingMode), _image(image) {
	_blitState = loadState();
}

void BlittingDrawCall::execute() const {
	BlittingState backupState = loadState();
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
		//Graphics::Internal::tglBlitZBuffer(_image, _transform._destinationRectangle.left, _transform._destinationRectangle.top);
		break;
	default:
		break;
	}

	applyState(backupState);
}

void BlittingDrawCall::execute(const Common::Rect &clippingRectangle) const {

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

ClearBufferDrawCall::ClearBufferDrawCall(bool clearZBuffer, int zValue, bool clearColorBuffer, int rValue, int gValue, int bValue) : clearZBuffer(clearZBuffer), 
	clearColorBuffer(clearColorBuffer), zValue(zValue), rValue(rValue), gValue(gValue), bValue(bValue), DrawCall(DrawCall_Clear) {

}

void ClearBufferDrawCall::execute() const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->clear(clearZBuffer, zValue, clearColorBuffer, rValue, gValue, bValue);
}

void ClearBufferDrawCall::execute( const Common::Rect &clippingRectangle ) const {

}

} // end of namespace Graphics
