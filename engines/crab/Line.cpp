#include "Line.h"
#include "stdafx.h"

//------------------------------------------------------------------------
// Purpose: Draw a line from start to end
//------------------------------------------------------------------------
void DrawLine(const int &x1, const int &y1, const int &x2, const int &y2,
			  const Uint8 &r, const Uint8 &g, const Uint8 &b, const Uint8 &a) {
	SDL_SetRenderDrawColor(gRenderer, r, g, b, a);
	SDL_RenderDrawLine(gRenderer, x1, y1, x2, y2);
}