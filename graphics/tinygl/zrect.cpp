#include "graphics/tinygl/zrect.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/gl.h"

namespace TinyGL {

void glIssueDrawCall(Graphics::DrawCall *drawCall) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->_drawCallsQueue.push_back(drawCall);
}

} // end of namespace TinyGL


void tglDrawRectangle(Common::Rect rect, int r, int g, int b) {
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
		c->fb->writePixel(rect.top * c->fb->xsize + x, 255, r, g, b);
		c->fb->writePixel(rect.bottom * c->fb->xsize + x, 255, r, g, b);
	}
	for(int y = rect.top; y < rect.bottom; y++) {
		c->fb->writePixel(y * c->fb->xsize + rect.left, 255, r, g, b);
		c->fb->writePixel(y * c->fb->xsize + rect.right, 255, r, g, b);
	}
}

struct DirtyRectangle {
	Common::Rect rectangle;
	int r, g, b;

	DirtyRectangle() { }
	DirtyRectangle(Common::Rect rect, int r, int g, int b) {
		this->rectangle = rect;
		this->r = r;
		this->g = g;
		this->b = b;
	}
};

void tglPresentBuffer() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	Common::List<DirtyRectangle> rectangles;

	Common::List<Graphics::DrawCall *>::const_iterator itFrame = c->_drawCallsQueue.begin();
	Common::List<Graphics::DrawCall *>::const_iterator endFrame = c->_drawCallsQueue.end();

	Common::List<Graphics::DrawCall *>::const_iterator itPrevFrame = c->_previousFrameDrawCallsQueue.begin();
	Common::List<Graphics::DrawCall *>::const_iterator endPrevFrame = c->_previousFrameDrawCallsQueue.end();
	
	// Compare draw calls.
	while (itPrevFrame != endPrevFrame) {
		const Graphics::DrawCall &currentCall = **itFrame;
		const Graphics::DrawCall &previousCall = **itPrevFrame;

		if (!(previousCall == currentCall)) {
			while(itPrevFrame != endPrevFrame) {
				const Graphics::DrawCall &previousCall = **itPrevFrame;
				rectangles.push_back(DirtyRectangle(previousCall.getDirtyRegion(), 255, 255, 255));
				++itPrevFrame;
			}
			break;
		}

		++itFrame;
		++itPrevFrame;
	}

	while (itFrame != endFrame) {
		const Graphics::DrawCall &currentCall = **itFrame;
		rectangles.push_back(DirtyRectangle(currentCall.getDirtyRegion(), 255, 0, 0));
		++itFrame;
	}

	// Merge coalesce dirty rects.
	bool restartMerge;
	Common::List<DirtyRectangle>::iterator rectanglesEnd = rectangles.end();
	do {
		Common::List<DirtyRectangle>::iterator it1 = rectangles.begin();
		restartMerge = false;
		while (it1 != rectangles.end() && restartMerge == false) {
			Common::List<DirtyRectangle>::iterator it2 = it1;
			++it2;
			while (it2 != rectanglesEnd) {
				if ((*it1).rectangle.intersects((*it2).rectangle)) {
					(*it1).rectangle.extend((*it2).rectangle);
					it2 = rectangles.erase(it2);
				} else {
					++it2;
				}
			}
			++it1;
		}
	} while(restartMerge);

	rectanglesEnd = rectangles.end();

	Common::List<DirtyRectangle>::iterator it1 = rectangles.begin();
	while (it1 != rectangles.end()) {
		Common::List<DirtyRectangle>::iterator it2 = it1;
		++it2;
		while (it2 != rectanglesEnd) {
			if ((*it1).rectangle.contains((*it2).rectangle)) {
				it2 = rectangles.erase(it2);
			} else {
				++it2;
			}
		}
		++it1;
	}

	Common::List<Graphics::DrawCall *>::const_iterator it = c->_drawCallsQueue.begin();
	Common::List<Graphics::DrawCall *>::const_iterator end = c->_drawCallsQueue.end();
	
	Common::List<DirtyRectangle>::iterator itRectangles = rectangles.begin();

	while (itRectangles != rectangles.end()) {
		if ((*itRectangles).rectangle.left < 0) {
			(*itRectangles).rectangle.left = 0;
		} 
		if ((*itRectangles).rectangle.top < 0) {
			(*itRectangles).rectangle.top = 0;
		}
		if ((*itRectangles).rectangle.right > c->fb->xsize) {
			(*itRectangles).rectangle.right = c->fb->xsize;
		}
		if ((*itRectangles).rectangle.bottom > c->fb->ysize) {
			(*itRectangles).rectangle.bottom = c->fb->ysize;
		}
		itRectangles++;
	}

	// Execute draw calls.
	while (it != end) {
		Common::Rect drawCallRegion = (*it)->getDirtyRegion();
		itRectangles = rectangles.begin();
		while (itRectangles != rectangles.end()) {
			Common::Rect dirtyRegion = (*itRectangles).rectangle;
			if (dirtyRegion.intersects(drawCallRegion) || drawCallRegion.contains(dirtyRegion)) {
				(*it)->execute(dirtyRegion, false);	
			}
			++itRectangles;
		}
		++it;
	}

	int drawCallNumber = c->_drawCallsQueue.size();

	// Dispose not necessary draw calls.
	it = c->_previousFrameDrawCallsQueue.begin();
	end = c->_previousFrameDrawCallsQueue.end();

	while (it != end) {
		delete *it;
		it++;
	}

	c->_previousFrameDrawCallsQueue = c->_drawCallsQueue;
	c->_drawCallsQueue.clear();

	// Draw debug rectangles.
	// Note: white rectangles are rectangle that contained other rectangles
	// blue rectangles are rectangle merged from other rectangles
	// red rectangles are original dirty rects
	itRectangles = rectangles.begin();

	bool blendingEnabled = c->fb->isBlendingEnabled();
	bool alphaTestEnabled = c->fb->isAplhaTestEnabled();
	c->fb->enableBlending(false);
	c->fb->enableAlphaTest(false);

	while (itRectangles != rectangles.end()) {
		tglDrawRectangle((*itRectangles).rectangle, (*itRectangles).r, (*itRectangles).g, (*itRectangles).b);
		itRectangles++;
	}

	c->fb->enableBlending(blendingEnabled);
	c->fb->enableAlphaTest(alphaTestEnabled);

	// Dispose textures and resources.
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

bool DrawCall::operator==(const DrawCall &other) const {
	if (_type == other._type) {
		switch (_type) {
		case Graphics::DrawCall_Rasterization:
			return *(RasterizationDrawCall *)this == (const RasterizationDrawCall &)other;
			break;
		case Graphics::DrawCall_Blitting:
			return *(BlittingDrawCall *)this == (const BlittingDrawCall &)other;
			break;
		case Graphics::DrawCall_Clear:
			return *(ClearBufferDrawCall *)this == (const ClearBufferDrawCall &)other;
			break;
		default:
			break;
		}
	} else {
		return false;
	}
}

RasterizationDrawCall::RasterizationDrawCall() : DrawCall(DrawCall_Rasterization) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	_vertexCount = c->vertex_cnt;
	_vertex = new TinyGL::GLVertex[_vertexCount];
	_drawTriangleFront = c->draw_triangle_front;
	_drawTriangleBack = c->draw_triangle_back;
	memcpy(_vertex, c->vertex, sizeof(TinyGL::GLVertex) * _vertexCount);
	_state = loadState();
	_dirtyRegion = computeDirtyRegion();
}

Common::Rect RasterizationDrawCall::computeDirtyRegion() {
	Common::Rect region;
	region.left = 9999;
	region.top = 9999;

	for (int i = 0; i < _vertexCount; i++) {
		if (_vertex[i].zp.x < region.left) {
			region.left = _vertex[i].zp.x;
		}
		if (_vertex[i].zp.y < region.top) {
			region.top = _vertex[i].zp.y;
		}
		if (_vertex[i].zp.x > region.right) {
			region.right = _vertex[i].zp.x;
		}
		if (_vertex[i].zp.y > region.bottom) {
			region.bottom = _vertex[i].zp.y;
		}
	}

	return region;
}

void RasterizationDrawCall::execute(bool restoreState) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	RasterizationDrawCall::RasterizationState backupState;
	if (restoreState) {
		backupState = loadState();
	}
	applyState(_state);

	TinyGL::GLVertex *prevVertex = c->vertex;
	int prevVertexCount = c->vertex_cnt;

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
	if (c->current_texture != nullptr) 
		state.textureVersion = c->current_texture->versionNumber;

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
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->setScissorRectangle(clippingRectangle.left, clippingRectangle.right, clippingRectangle.top, clippingRectangle.bottom);
	execute(restoreState);
	c->fb->setScissorRectangle(0, c->fb->xsize, 0, c->fb->ysize);
}

const Common::Rect RasterizationDrawCall::getDirtyRegion() const {
	return _dirtyRegion;
}

bool RasterizationDrawCall::operator==(const RasterizationDrawCall &other) const {
	if (_vertexCount == other._vertexCount && 
		_drawTriangleFront == other._drawTriangleFront && 
		_drawTriangleBack == other._drawTriangleBack && 
		_state == other._state) {
		for (int i = 0; i < _vertexCount; i++) {
			if ((_vertex[i] == other._vertex[i]) == false) {
				return false;
			}
		}
		return true;
	}
	return false;
}

BlittingDrawCall::BlittingDrawCall(Graphics::BlitImage *image, const BlitTransform &transform, BlittingMode blittingMode) : DrawCall(DrawCall_Blitting), _transform(transform), _mode(blittingMode), _image(image) {
	_blitState = loadState();
	_imageVersion = tglGetBlitImageVersion(image);
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
	Graphics::Internal::tglBlitScissorRect(clippingRectangle.left, clippingRectangle.top, clippingRectangle.right, clippingRectangle.bottom);
	execute(restoreState);
	Graphics::Internal::tglBlitScissorRect(0, 0, 0, 0);
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
	int blitHeight = _transform._destinationRectangle.height();
	if (blitWidth == 0) {
		if (_transform._sourceRectangle.width() != 0) {
			blitWidth = _transform._sourceRectangle.width();
		} else {
			tglGetBlitImageSize(_image, blitWidth, blitHeight);
		}
	} else if (blitHeight == 0) {
		if (_transform._sourceRectangle.height() != 0) {
			blitHeight = _transform._sourceRectangle.height();
		} else {
			tglGetBlitImageSize(_image, blitWidth, blitHeight);
		}
	}
	return Common::Rect(_transform._destinationRectangle.left, _transform._destinationRectangle.top, _transform._destinationRectangle.left + blitWidth, _transform._destinationRectangle.top + blitHeight);
}

bool BlittingDrawCall::operator==(const BlittingDrawCall &other) const {
	return _mode == other._mode && _image == other._image && _transform == other._transform && _blitState == other._blitState && _imageVersion == tglGetBlitImageVersion(other._image);
}

ClearBufferDrawCall::ClearBufferDrawCall(bool clearZBuffer, int zValue, bool clearColorBuffer, int rValue, int gValue, int bValue) : clearZBuffer(clearZBuffer), 
	clearColorBuffer(clearColorBuffer), zValue(zValue), rValue(rValue), gValue(gValue), bValue(bValue), DrawCall(DrawCall_Clear) {
}

void ClearBufferDrawCall::execute(bool restoreState) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->clear(clearZBuffer, zValue, clearColorBuffer, rValue, gValue, bValue);
}

void ClearBufferDrawCall::execute(const Common::Rect &clippingRectangle, bool restoreState) const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->fb->clearRegion(clippingRectangle.left, clippingRectangle.top, clippingRectangle.width(), clippingRectangle.height(), clearZBuffer, zValue, clearColorBuffer, rValue, gValue, bValue);
}

const Common::Rect ClearBufferDrawCall::getDirtyRegion() const {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	return Common::Rect(0, 0, c->fb->xsize, c->fb->ysize);
}

bool ClearBufferDrawCall::operator==(const ClearBufferDrawCall &other) const {
	return clearZBuffer == other.clearZBuffer && clearColorBuffer == other.clearColorBuffer && rValue == other.rValue && gValue == other.gValue && 
		bValue == other.bValue && zValue == other.zValue;
}


bool RasterizationDrawCall::RasterizationState::operator==(const RasterizationState &other) const {
	return beginType == other.beginType && currentFrontFace == other.currentFrontFace && cullFaceEnabled == other.cullFaceEnabled &&
		colorMask == other.colorMask && depthTest == other.depthTest && depthFunction == other.depthFunction && depthWrite == other.depthWrite &&
		shadowMode == other.shadowMode && texture2DEnabled == other.texture2DEnabled && currentShadeModel == other.currentShadeModel && polygonModeBack == other.polygonModeBack &&
		polygonModeFront == other.polygonModeFront && lightingEnabled == other.lightingEnabled && enableBlending == other.enableBlending && sfactor == other.sfactor &&
		dfactor == other.dfactor && alphaTest == other.alphaTest && alphaFunc == other.alphaFunc && alphaRefValue == other.alphaRefValue && texture == other.texture &&
		shadowMaskBuf == other.shadowMaskBuf && currentColor[0] == other.currentColor[0] && currentColor[1] == other.currentColor[1] && currentColor[2] == other.currentColor[2] &&
		currentColor[3] == other.currentColor[3] && viewportTranslation[0] == other.viewportTranslation[0] && viewportTranslation[1] == other.viewportTranslation[1] &&
		viewportTranslation[2] == other.viewportTranslation[2] && viewportScaling[0] == other.viewportScaling[0] && viewportScaling[1] == other.viewportScaling[1] && 
		viewportScaling[2] == other.viewportScaling[2] && textureVersion == texture->versionNumber;
}

} // end of namespace Graphics
