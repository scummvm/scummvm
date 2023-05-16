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

#include "bladerunner/shape.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/mouse.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace BladeRunner {

bool Shape::load(Common::SeekableReadStream *stream) {
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();
	uint32 size = stream->readUint32LE();

	if (size != (uint32)(_width * _height * 2)) {
		warning("Shape::load size mismatch (w %d, h %d, sz %d)", _width, _height, size);
		return false;
	}

	// Enfoce a reasonable size limit
	if (_width >= 2048 || _height >= 2048) {
		warning("Shape::load shape too big (%d, %d)", _width, _height);
	}

	_data = new byte[size];

	if (stream->read(_data, size) != size) {
		warning("Shape::load error reading shape (w %d, h %d, sz %d)", _width, _height, size);
		return false;
	}

	return true;
}

Shape::~Shape() {
	delete[] _data;
}

void Shape::drawFilledTriangleAux(Graphics::Surface &surface, const int &dst_x, const int &dst_y, int x1, int y1, int x2, int y2, int x3, int y3, uint32 colorRGB) const {
	// Code used is based on the Bresenham-like algorithm as described in:
	// https://mcejp.github.io/2020/11/06/bresenham.html
	// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

	const Vector2 triangleV1 = Vector2(x1, y1);
	const Vector2 triangleV2 = Vector2(x2, y2); // (V2, V3) should be the flat side, so either x2 == x3 or y2 == y3
	const Vector2 triangleV3 = Vector2(x3, y3);

	// vTmp1 will be moving along the "left" side of the triangle, and vTmp2 along the "right" side.
	Vector2 vTmp1 = triangleV1;
	Vector2 vTmp2 = triangleV1;

	bool swappedDx1WithDy1 = false;
	bool swappedDx2WithDy2 = false;

	int dx1 = abs(triangleV2.x - triangleV1.x);
	int dy1 = abs(triangleV2.y - triangleV1.y);

	if (dy1 > dx1) {
		SWAP(dy1, dx1);
		swappedDx1WithDy1 = true;
	}

	int dx2 = abs(triangleV3.x - triangleV1.x);
	int dy2 = abs(triangleV3.y - triangleV1.y);

	if (dy2 > dx2) {
		SWAP(dy2, dx2);
		swappedDx2WithDy2 = true;
	}

	int signx1 = (triangleV2.x - triangleV1.x) > 0 ? 1: -1;
	if (triangleV2.x == triangleV1.x) {
		signx1 = 0;
	}

	int signx2 = (triangleV3.x - triangleV1.x) > 0 ? 1: -1;
	if (triangleV3.x == triangleV1.x) {
		signx2 = 0;
	}

	int signy1 = (triangleV2.y - triangleV1.y) > 0 ? 1: -1;
	if (triangleV2.y == triangleV1.y) {
		signy1 = 0;
	}

	int signy2 = (triangleV3.y - triangleV1.y) > 0 ? 1: -1;
	if (triangleV3.y == triangleV1.y) {
		signy2 = 0;
	}

	int e1 = 2 * dy1 - dx1;
	int e2 = 2 * dy2 - dx2;

	for (int i = 0; i <= dx1; ++i) {
		// dx1 here may be dy1 (if they swapped above). It is whichever was the largest of the two.
		// We loop over each pixel of "horizontal" (triangle filling) line
		// check if paint goes from left endpoint of "horizontal" (triangle filling) line to right or other way round
		int leftEndPoint, rightEndPoint;

		if (triangleV2.y == triangleV3.y) {
			if (vTmp1.x < vTmp2.x) {
				leftEndPoint = ceil(vTmp1.x);
				rightEndPoint = int(vTmp2.x);
			} else {
				leftEndPoint = ceil(vTmp2.x);
				rightEndPoint = int(vTmp1.x);
			}
		} else {
			if (vTmp1.y < vTmp2.y) {
				leftEndPoint = ceil(vTmp1.y);
				rightEndPoint = int(vTmp2.y);
			} else {
				leftEndPoint = ceil(vTmp2.y);
				rightEndPoint = int(vTmp1.y);
			}
		}

		void *dstPtr;
		for (int xPos = leftEndPoint; xPos <= rightEndPoint; ++xPos) {
			if (triangleV2.y == triangleV3.y) {
				dstPtr = surface.getBasePtr(CLIP(dst_x + xPos, 0, surface.w - 1), CLIP(dst_y + (int)ceil(vTmp1.y), 0, surface.h - 1));
			} else {
				dstPtr = surface.getBasePtr(CLIP(dst_x + (int)ceil(vTmp1.x), 0, surface.w - 1), CLIP(dst_y + xPos, 0, surface.h - 1));
			}
			drawPixel(surface, dstPtr, colorRGB);
		}

		while (e1 >= 0) {
			if (swappedDx1WithDy1) {
				vTmp1.x += signx1;
			} else {
				vTmp1.y += signy1;
			}
			e1 = e1 - 2 * dx1;
		}

		if (swappedDx1WithDy1) {
			vTmp1.y += signy1;
		} else {
			vTmp1.x += signx1;
		}
		e1 = e1 + 2 * dy1;

		// Here we've rendered the next point on the "left" edge triangle line.
		// We now do the same for the "right" edge triangle line, until we are on
		// the same y-value as on the "left" edge triangle line.
		if (triangleV2.y == triangleV3.y) {
			while (vTmp2.y != vTmp1.y) {
				while (e2 >= 0) {
					if (swappedDx2WithDy2) {
						vTmp2.x += signx2;
					} else {
						vTmp2.y += signy2;
					}
					e2 = e2 - 2 * dx2;
				}

				if (swappedDx2WithDy2) {
					vTmp2.y += signy2;
				} else {
					vTmp2.x += signx2;
				}
				e2 = e2 + 2 * dy2;
			}
		} else {
			while (vTmp2.x != vTmp1.x) {
				while (e2 >= 0) {
					if (swappedDx2WithDy2) {
						vTmp2.x += signx2;
					} else {
						vTmp2.y += signy2;
					}
					e2 = e2 - 2 * dx2;
				}

				if (swappedDx2WithDy2) {
					vTmp2.y += signy2;
				} else {
					vTmp2.x += signx2;
				}
				e2 = e2 + 2 * dy2;
			}
		}
	}
}

void Shape::draw(Graphics::Surface &surface, int x, int y, uint16 drawModeBitFlags) const {
	int src_x = CLIP(-x, 0, _width);
	int src_y = CLIP(-y, 0, _height);

	int dst_x = CLIP<int>(x, 0, surface.w);
	int dst_y = CLIP<int>(y, 0, surface.h);

	int rect_w = MIN(CLIP(_width + x, 0, _width), surface.w - x);
	int rect_h = MIN(CLIP(_height + y, 0, _height), surface.h - y);

	if (rect_w <= 0 || rect_h <= 0) {
		// Checking here for negative values also,
		// prevents segmentation fault (in the for loop below)
		return;
	}

	if (drawModeBitFlags & Mouse::MouseDrawFlags::CUSTOM) {
		// for both scene exit cursors and static ESPER edge cursors
		// we choose 25x25px -- odd dimensions chosen so that pixel 12 is the absolute middle (with edges 0-24)
		rect_w = MIN(CLIP(25 + x, 0, 25), surface.w - x);
		rect_h = MIN(CLIP(25 + y, 0, 25), surface.h - y);

		const uint32 exitDemoShapeColors[3] = { surface.format.RGBToColor(255, 255, 143),
		                                        surface.format.RGBToColor(228, 108,  10),
		                                        surface.format.RGBToColor(140,   0,  37)  };

		const uint32 esperExitDemoShapeColor = surface.format.RGBToColor(230, 230, 230); //  For ESPER arrows
		Common::Rect tailRect;

		// We mask out the irrelevant bitflags and exploit the fact
		// that these exit cursor bitflags do not combine,
		// so only one of them can be set at any time.
		switch (drawModeBitFlags & ~0x007F) {
		case Mouse::MouseDrawFlags::EXIT_RIGHT:
			// 6 px h, 6 px w
			drawFilledTriangleAux(surface, dst_x, dst_y, 24, 12, 18,  6, 18, 18, exitDemoShapeColors[ (drawModeBitFlags & 0x7)/2 % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y, 16, 12, 10,  6, 10, 18, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 1) % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y,  8, 12,  2,  6,  2, 18, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 2) % 3]);
			break;

		case Mouse::MouseDrawFlags::EXIT_LEFT:
			drawFilledTriangleAux(surface, dst_x, dst_y,  0, 12,  6,  6,  6, 18, exitDemoShapeColors[ (drawModeBitFlags & 0x7)/2 % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y,  8, 12, 14,  6, 14, 18, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 1) % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y, 16, 12, 22,  6, 22, 18, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 2) % 3]);
			break;

		case Mouse::MouseDrawFlags::EXIT_UP:
			drawFilledTriangleAux(surface, dst_x, dst_y, 12,  0,  6,  6, 18,  6, exitDemoShapeColors[ (drawModeBitFlags & 0x7)/2 % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y, 12,  8,  6, 14, 18, 14, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 1) % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y, 12, 16,  6, 22, 18, 22, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 2) % 3]);
			break;

		case Mouse::MouseDrawFlags::EXIT_DOWN:
			drawFilledTriangleAux(surface, dst_x, dst_y, 12, 24,  6, 18, 18, 18, exitDemoShapeColors[ (drawModeBitFlags & 0x7)/2 % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y, 12, 16,  6, 10, 18, 10, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 1) % 3]);
			drawFilledTriangleAux(surface, dst_x, dst_y, 12,  8,  6,  2, 18,  2, exitDemoShapeColors[((drawModeBitFlags & 0x7)/2 + 2) % 3]);
			break;

		case Mouse::MouseDrawFlags::ESPER_RIGHT:
			drawFilledTriangleAux(surface, dst_x, dst_y, 24, 12, 12,  0, 12, 24, esperExitDemoShapeColor);
			tailRect = Common::Rect(dst_x +  4, dst_y + 6, dst_x + 12, dst_y + 18);
			surface.fillRect(tailRect, esperExitDemoShapeColor);
			break;

		case Mouse::MouseDrawFlags::ESPER_LEFT:
			drawFilledTriangleAux(surface, dst_x, dst_y,  0, 12, 12,  0, 12, 24, esperExitDemoShapeColor);
			tailRect = Common::Rect(dst_x + 12, dst_y + 6, dst_x + 20, dst_y + 18);
			surface.fillRect(tailRect, esperExitDemoShapeColor);
			break;

		case Mouse::MouseDrawFlags::ESPER_UP:
			drawFilledTriangleAux(surface, dst_x, dst_y, 12,  0,  0, 12, 24, 12, esperExitDemoShapeColor);
			tailRect = Common::Rect(dst_x +  6, dst_y + 12, dst_x + 18, dst_y + 20);
			surface.fillRect(tailRect, esperExitDemoShapeColor);
			break;

		case Mouse::MouseDrawFlags::ESPER_DOWN:
			drawFilledTriangleAux(surface, dst_x, dst_y, 12, 24,  0, 12, 24, 12, esperExitDemoShapeColor);
			tailRect = Common::Rect(dst_x +  6,  dst_y + 4, dst_x + 18, dst_y + 12);
			surface.fillRect(tailRect, esperExitDemoShapeColor);
			break;

		default:
			debug("Unsupported custom shape %d", drawModeBitFlags &~ 0xEF);
			break;
		}
	} else {
		const uint8 *src_p = _data + 2 * (src_y * _width + src_x);

		uint16 shpColor = 0;
		uint32 surfaceColorRGBPrev = 0;
		uint32 newSurfaceColorRGB = 0;
		uint8 a, r, g, b;
		uint8 rPrev, gPrev, bPrev;
		uint16 rgb16bitPrev = 0;
		uint16 rgb16bitAdd = 0;
		for (int yi = 0; yi != rect_h; ++yi) {
			for (int xi = 0; xi != rect_w; ++xi) {
				shpColor = READ_LE_UINT16(src_p);
				src_p += 2;

				getGameDataColor(shpColor, a, r, g, b);

				if (!a) {
					// Ignore the alpha in the output as it is inversed in the input
					void *dstPtr = surface.getBasePtr(CLIP(dst_x + xi, 0, surface.w - 1), CLIP(dst_y + yi, 0, surface.h - 1));
					if (drawModeBitFlags & Mouse::MouseDrawFlags::SPECIAL) {
						// It seems that the additive mode was supposed to be used only for cursor shapes
						// From testing, the only cursor shape that seems to work with it is the green rotating cursor
						// We add extra code here to cover the cases of the beta crosshairs cursor
						// being drawn a different color based on bullet power
						// The code for creating the specific color is custom.
						if (drawModeBitFlags & Mouse::MouseDrawFlags::REDCROSSHAIRS) {
							newSurfaceColorRGB = surface.format.RGBToColor((b & 0x8B) | (g >> 1), 0, 0);
						} else if (drawModeBitFlags & Mouse::MouseDrawFlags::YELLOWCROSSHAIRS) {
							newSurfaceColorRGB = surface.format.RGBToColor(b & 0xDF, (b & 0xA5) | (g >> 1), 0);
						} else if (drawModeBitFlags & Mouse::MouseDrawFlags::BLUECROSSHAIRS) {
							newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
						} else {
							// Additive modes
							getPixel(surface, dstPtr, surfaceColorRGBPrev);
							if (drawModeBitFlags & Mouse::MouseDrawFlags::ADDITIVE_MODE0) {
								// This code makes the cursor semi-transparent
								// but it may not be what the disassembly of the original was going for.
								newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
								newSurfaceColorRGB = (((uint16)surfaceColorRGBPrev >> 1) & 0xFBEF)
								                    + (((uint16)newSurfaceColorRGB >> 1) & 0xFBEF);
							} else if (drawModeBitFlags & Mouse::MouseDrawFlags::ADDITIVE_MODE1) {
								// This code may be closer to what the disassembly of the original was doing
								// for additive draw mode but it doesn't look well.
								surface.format.colorToRGB(surfaceColorRGBPrev, rPrev, gPrev, bPrev);
								rgb16bitPrev = (  ((uint16)(rPrev >> 3) << 10)
								                | ((uint16)(gPrev >> 3) <<  5)
								                | ((uint16)(bPrev >> 3)));
								rgb16bitAdd = (((uint16)rgb16bitPrev >> 1) & 0xFBEF)
								             + ((shpColor >> 1) & 0xFBEF);
								getGameDataColor(rgb16bitAdd, a, r, g, b);
								newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
							}
						}
					} else {
						newSurfaceColorRGB = surface.format.RGBToColor(r, g, b);
					}
					drawPixel(surface, dstPtr, newSurfaceColorRGB);
				}
			}
			src_p += 2 * (_width - rect_w);
		}
	}
}

Shapes::Shapes(BladeRunnerEngine *vm) {
	_vm = vm;
}

Shapes::~Shapes() {
	unload();
}

bool Shapes::load(const Common::String &container) {
	unload();

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->getResourceStream(container));
	if (!stream) {
		warning("Shape::open failed to open '%s'", container.c_str());
		return false;
	}

	uint32 count = stream->readUint32LE();

	_shapes.resize(count);

	for (uint32 i = 0; i < count; ++i) {
		if (!_shapes[i].load(stream.get())) {
			return false;
		}
	}

	return true;
}

void Shapes::unload() {
	_shapes.clear();
}


} // End of namespace BladeRunner
