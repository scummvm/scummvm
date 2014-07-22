#include "graphics/tinygl/zrect.h"

namespace Graphics {

	DrawCall::DrawCall(DrawCallType type) : _type(type) {
	}

	BlittingDrawCall::BlittingDrawCall(TinyGL::GLContext *c) : DrawCall(DrawCall_Blitting) {

	}

	RasterizationDrawCall::RasterizationDrawCall(TinyGL::GLContext *c) : DrawCall(DrawCall_Rasterization) {

	}

	void RasterizationDrawCall::execute() const {

	}

	void RasterizationDrawCall::execute( const Common::Rect &clippingRectangle ) const {

	}

	void BlittingDrawCall::execute() const {

	}

	void BlittingDrawCall::execute(const Common::Rect &clippingRectangle) const {

	}
}
