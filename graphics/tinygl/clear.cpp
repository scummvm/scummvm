
#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/zrect.h"

namespace TinyGL {

void glopClearColor(GLContext *c, GLParam *p) {
	c->clear_color = Vector4(p[1].f, p[2].f, p[3].f, p[4].f);
}

void glopClearDepth(GLContext *c, GLParam *p) {
	c->clear_depth = p[1].f;
}

void glopClear(GLContext *c, GLParam *p) {
	int mask = p[1].i;
	int z = 0;
	int r = (int)(c->clear_color.X * 65535);
	int g = (int)(c->clear_color.Y * 65535);
	int b = (int)(c->clear_color.Z * 65535);

	// TODO : correct value of Z

	tglIssueDrawCall(new Graphics::ClearBufferDrawCall(mask & TGL_DEPTH_BUFFER_BIT, z, mask & TGL_COLOR_BUFFER_BIT, r, g, b));
}

} // end of namespace TinyGL
