#include "graphics/tinygl/zrect.h"

void tglPresentBuffer() {

}

namespace Graphics {

	DrawCall::DrawCall(DrawCallType type) : _type(type) {
	}

	RasterizationDrawCall::RasterizationDrawCall(TinyGL::GLContext *c) : DrawCall(DrawCall_Rasterization) {

	}

	void RasterizationDrawCall::execute() const {

	}

	void RasterizationDrawCall::execute( const Common::Rect &clippingRectangle ) const {

	}

	BlittingDrawCall::BlittingDrawCall(TinyGL::GLContext *c) : DrawCall(DrawCall_Blitting) {

	}

	void BlittingDrawCall::execute() const {

	}

	void BlittingDrawCall::execute(const Common::Rect &clippingRectangle) const {

	}
}
