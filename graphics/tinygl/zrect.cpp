#include "graphics/tinygl/zrect.h"
#include "graphics/tinygl/zgl.h"

void tglPresentBuffer() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	for(int i = 0; i < c->drawCallsQueue.size(); ++i) {
		c->drawCallsQueue[i]->execute();
		delete c->drawCallsQueue[i];
	}
	c->drawCallsQueue.clear();

	Graphics::Internal::tglCleanupImages();
}

namespace Graphics {

DrawCall::DrawCall(DrawCallType type) : _type(type) {
}

RasterizationDrawCall::RasterizationDrawCall() : DrawCall(DrawCall_Rasterization) {

}

void RasterizationDrawCall::execute() const {

}

void RasterizationDrawCall::execute( const Common::Rect &clippingRectangle ) const {

}

BlittingDrawCall::BlittingDrawCall(Graphics::BlitImage *image, const BlitTransform &transform, BlittingMode blittingMode) : DrawCall(DrawCall_Blitting), _transform(transform), _mode(blittingMode), _image(image) {
	_blitState = loadState();
}

void BlittingDrawCall::execute() const {
	BlittingState backupState = loadState();
	applyState(_blitState);

	switch (_mode)
	{
	case Graphics::BlittingDrawCall::BlitMode_Regular:
		Graphics::Internal::tglBlit(_image, _transform);
		break;
	case Graphics::BlittingDrawCall::BlitMode_NoBlend:
		Graphics::Internal::tglBlitNoBlend(_image, _transform);
		break;
	case Graphics::BlittingDrawCall::BlitMode_Fast:
		Graphics::Internal::tglBlitFast(_image, _transform._destinationRectangle.left, _transform._destinationRectangle.top);
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

} // end of namespace Graphics
