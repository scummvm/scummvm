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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#include "colony/colony.h"
#include "colony/render_internal.h"
#include "common/system.h"
#include <math.h>

namespace Colony {

void ColonyEngine::getWallFace3D(int cellX, int cellY, int direction, float corners[4][3]) {
	float x0 = cellX * 256.0f;
	float y0 = cellY * 256.0f;
	float x1 = (cellX + 1) * 256.0f;
	float y1 = (cellY + 1) * 256.0f;
	const float zBot = -160.0f;
	const float zTop = 160.0f;
	// Pull slightly into the cell to prevent z-fighting
	const float eps = 1.0f;

	switch (direction) {
	case kDirNorth: // Wall at y1 (+Y); viewed from inside (looking North)
		corners[0][0] = x0;  corners[0][1] = y1 - eps;  corners[0][2] = zBot; // BL
		corners[1][0] = x1;  corners[1][1] = y1 - eps;  corners[1][2] = zBot; // BR
		corners[2][0] = x1;  corners[2][1] = y1 - eps;  corners[2][2] = zTop; // TR
		corners[3][0] = x0;  corners[3][1] = y1 - eps;  corners[3][2] = zTop; // TL
		break;
	case kDirSouth: // Wall at y0 (-Y); viewed from inside (looking South)
		corners[0][0] = x1;  corners[0][1] = y0 + eps;  corners[0][2] = zBot; // BL
		corners[1][0] = x0;  corners[1][1] = y0 + eps;  corners[1][2] = zBot; // BR
		corners[2][0] = x0;  corners[2][1] = y0 + eps;  corners[2][2] = zTop; // TR
		corners[3][0] = x1;  corners[3][1] = y0 + eps;  corners[3][2] = zTop; // TL
		break;
	case kDirEast: // Wall at x1 (+X); viewed from inside (looking East)
		corners[0][0] = x1 - eps;  corners[0][1] = y1;  corners[0][2] = zBot; // BL
		corners[1][0] = x1 - eps;  corners[1][1] = y0;  corners[1][2] = zBot; // BR
		corners[2][0] = x1 - eps;  corners[2][1] = y0;  corners[2][2] = zTop; // TR
		corners[3][0] = x1 - eps;  corners[3][1] = y1;  corners[3][2] = zTop; // TL
		break;
	case kDirWest: // Wall at x0 (-X); viewed from inside (looking West)
		corners[0][0] = x0 + eps;  corners[0][1] = y0;  corners[0][2] = zBot; // BL
		corners[1][0] = x0 + eps;  corners[1][1] = y1;  corners[1][2] = zBot; // BR
		corners[2][0] = x0 + eps;  corners[2][1] = y1;  corners[2][2] = zTop; // TR
		corners[3][0] = x0 + eps;  corners[3][1] = y0;  corners[3][2] = zTop; // TL
		break;
	default:
		return;
	}
}

// Interpolate a point on the wall face.
// u: 0=left, 1=right (horizontal fraction)
// v: 0=bottom, 1=top (vertical fraction)
static void wallPoint(const float corners[4][3], float u, float v, float out[3]) {
	float botX = corners[0][0] + (corners[1][0] - corners[0][0]) * u;
	float botY = corners[0][1] + (corners[1][1] - corners[0][1]) * u;
	float botZ = corners[0][2] + (corners[1][2] - corners[0][2]) * u;
	float topX = corners[3][0] + (corners[2][0] - corners[3][0]) * u;
	float topY = corners[3][1] + (corners[2][1] - corners[3][1]) * u;
	float topZ = corners[3][2] + (corners[2][2] - corners[3][2]) * u;
	out[0] = botX + (topX - botX) * v;
	out[1] = botY + (topY - botY) * v;
	out[2] = botZ + (topZ - botZ) * v;
}

// Draw a line on a wall face using normalized (u,v) coordinates
void ColonyEngine::wallLine(const float corners[4][3], float u1, float v1, float u2, float v2, uint32 color) {
	float p1[3], p2[3];
	wallPoint(corners, u1, v1, p1);
	wallPoint(corners, u2, v2, p2);
	// We assume this is only called when lit (handled in drawWallFeatures3D)
	_gfx->draw3DLine(p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], color);
}

// Draw a filled polygon on a wall face using normalized (u,v) coordinates
void ColonyEngine::wallPolygon(const float corners[4][3], const float *u, const float *v, int count, uint32 color) {
	float px[64], py[64], pz[64];
	if (count > 64)
		count = 64;
	for (int i = 0; i < count; i++) {
		float p[3];
		wallPoint(corners, u[i], v[i], p);
		px[i] = p[0]; py[i] = p[1]; pz[i] = p[2];
	}
	_gfx->draw3DPolygon(px, py, pz, count, color);
}

static bool nearlyEqual(float a, float b, float eps = 0.0001f) {
	return fabsf(a - b) <= eps;
}

static void addSortedUniqueFloat(float *values, int &count, float value, float eps = 0.0001f) {
	for (int i = 0; i < count; ++i) {
		if (nearlyEqual(values[i], value, eps))
			return;
		if (value < values[i]) {
			for (int j = count; j > i; --j)
				values[j] = values[j - 1];
			values[i] = value;
			++count;
			return;
		}
	}

	values[count++] = value;
}

static bool segmentIntersection2D(float ax, float ay, float bx, float by,
		float cx, float cy, float dx, float dy, float &ix, float &iy) {
	const float den = (ax - bx) * (cy - dy) - (ay - by) * (cx - dx);
	if (fabsf(den) < 0.0001f)
		return false;

	const float t = ((ax - cx) * (cy - dy) - (ay - cy) * (cx - dx)) / den;
	const float u = ((ax - cx) * (ay - by) - (ay - cy) * (ax - bx)) / den;
	if (t <= 0.0001f || t >= 0.9999f || u <= 0.0001f || u >= 0.9999f)
		return false;

	ix = ax + t * (bx - ax);
	iy = ay + t * (by - ay);
	return true;
}

static float edgeXAtY(float x0, float y0, float x1, float y1, float y) {
	if (nearlyEqual(y0, y1))
		return x0;

	const float t = (y - y0) / (y1 - y0);
	return x0 + (x1 - x0) * t;
}

struct WallCharSpan {
	float xMid;
	float xTop;
	float xBottom;
};

static void sortWallCharSpans(WallCharSpan *spans, int count) {
	for (int i = 1; i < count; ++i) {
		WallCharSpan span = spans[i];
		int j = i - 1;
		while (j >= 0 && spans[j].xMid > span.xMid) {
			spans[j + 1] = spans[j];
			--j;
		}
		spans[j + 1] = span;
	}
}

static const char *const kWallCharData[] = {
	"\00",
	"\02\10\02\00\03\00\03\01\02\01\10\02\02\03\02\03\06\02\06",
	"\02\10\01\04\02\04\02\05\01\05\10\03\04\04\04\04\05\03\05",
	"\04\10\01\00\02\00\02\05\01\05\10\03\00\04\00\04\05\03\05\10\00\01\05\01\05\02\00\02\10\00\03\05\03\05\04\00\04",
	"\02\10\02\00\03\00\03\06\02\06\050\00\02\00\01\01\00\04\00\05\01\05\02\01\04\01\05\04\05\04\04\05\04\05\05\04\06\01\06\00\05\00\04\04\02\04\01\01\01\01\02",
	"\03\10\01\00\06\05\05\06\00\01\032\01\03\02\03\03\04\03\05\02\06\01\06\00\05\00\04\01\03\01\05\02\05\02\04\01\04\032\04\00\05\00\06\01\06\02\05\03\04\03\03\02\03\01\04\00\04\02\05\02\05\01\04\01",
	"\03\10\05\01\05\02\03\04\02\04\014\02\04\01\05\02\06\01\06\00\05\01\04\032\05\04\03\01\01\01\01\03\03\05\02\06\01\06\02\05\00\03\00\01\01\00\03\00\05\03",
	"\01\10\02\04\03\05\03\06\02\06",
	"\01\014\04\00\03\02\03\04\04\06\02\04\02\02",
	"\01\014\02\00\04\02\04\04\02\06\03\04\03\02",
	"\06\06\01\00\03\03\00\02\06\02\00\04\00\03\03\06\05\00\06\02\03\03\06\06\04\05\06\03\03\06\02\06\04\06\03\03\06\00\04\01\06\03\03",
	"\02\10\02\00\03\00\03\05\02\05\10\00\02\05\02\05\03\00\03",
	"\01\10\02\00\03\01\03\02\02\02",
	"\01\10\00\02\05\02\05\03\00\03",
	"\01\10\02\00\03\00\03\01\02\01",
	"\01\10\01\00\06\05\05\06\00\01",
	"\02\032\01\00\05\00\06\01\06\05\05\06\01\06\00\05\00\01\01\00\01\05\05\05\05\01\01\01\10\01\01\02\01\05\05\04\05",
	"\01\026\01\00\04\00\04\01\03\01\03\06\02\06\01\05\01\04\02\04\02\01\01\01",
	"\01\042\06\00\06\01\02\01\05\02\06\03\06\05\05\06\01\06\00\05\00\04\01\04\01\05\05\05\05\04\01\02\00\01\00\00",
	"\01\054\00\02\00\01\01\00\05\00\06\01\06\02\05\03\06\04\06\05\05\06\01\06\00\05\00\04\01\04\01\05\05\05\05\04\04\03\05\02\05\01\01\01\01\02",
	"\01\036\04\00\05\00\05\02\06\02\06\03\05\03\05\06\04\06\04\03\01\03\02\06\01\06\00\03\00\02\04\02",
	"\01\044\00\02\00\01\01\00\05\00\06\01\06\03\05\04\01\04\02\05\06\05\06\06\01\06\00\04\00\03\05\03\05\01\01\01\01\02",
	"\01\046\01\02\05\02\05\01\01\01\01\05\05\05\05\04\06\04\06\05\05\06\01\06\00\05\00\01\01\00\05\00\06\01\06\02\05\03\01\03",
	"\01\020\02\00\03\00\03\03\06\06\00\06\00\05\04\05\02\03",
	"\02\040\03\00\03\01\01\01\01\02\04\03\01\04\01\05\03\05\03\06\01\06\00\05\00\04\01\03\00\02\00\01\01\00\040\03\00\05\00\06\01\06\02\05\03\06\04\06\05\05\06\03\06\03\05\05\05\05\04\02\03\05\02\05\01\03\01",
	"\01\046\00\02\00\01\01\00\05\00\06\01\06\05\05\06\01\06\00\05\00\04\01\03\05\03\05\04\01\04\01\05\05\05\05\01\01\01\01\02",
	"\02\10\02\01\03\01\03\02\02\02\10\02\03\03\03\03\04\02\04",
	"\02\10\02\00\03\01\03\02\02\02\10\02\03\03\03\03\04\02\04",
	"\01\014\06\00\06\01\02\03\06\05\06\06\00\03",
	"\02\10\00\01\05\01\05\02\00\02\10\00\03\05\03\05\04\00\04",
	"\01\014\00\00\06\03\00\06\00\05\04\03\00\01",
	"\02\10\02\00\03\00\03\01\02\01\030\02\02\03\02\05\04\05\05\04\06\01\06\00\05\00\04\01\04\01\05\04\05\04\04",
	"\05\012\04\00\01\01\00\04\00\01\01\00\012\02\00\05\00\06\01\06\04\05\01\012\06\02\06\05\05\06\02\06\05\05\012\04\06\01\06\00\05\00\02\01\05\034\05\01\05\02\03\05\02\05\01\04\01\02\02\01\03\01\05\04\05\05\03\02\02\02\02\04\03\04",
	"\01\034\03\06\04\06\06\00\05\00\04\02\02\02\01\00\00\00\02\06\03\06\03\05\02\03\04\03\03\05",
	"\01\050\00\00\00\06\05\06\06\05\06\04\05\03\06\02\06\01\05\00\01\00\01\01\05\01\05\02\04\03\01\03\01\04\05\04\05\05\01\05\01\00",
	"\01\040\06\02\06\01\05\00\01\00\00\01\00\05\01\06\05\06\06\05\06\04\05\04\05\05\01\05\01\01\05\01\05\02",
	"\01\034\00\00\00\06\04\06\06\04\06\02\04\00\01\00\01\01\04\01\05\02\05\04\04\05\01\05\01\00",
	"\01\030\00\00\00\06\06\06\06\05\01\05\01\04\04\04\04\03\01\03\01\01\06\01\06\00",
	"\01\024\00\00\00\06\06\06\06\05\01\05\01\03\04\03\04\02\01\02\01\00",
	"\01\044\03\03\06\03\06\01\05\00\01\00\00\01\00\05\01\06\05\06\06\05\06\04\05\04\05\05\01\05\01\01\05\01\05\02\03\02",
	"\01\030\00\00\00\06\01\06\01\04\05\04\05\06\06\06\06\00\05\00\05\03\01\03\01\00",
	"\01\030\01\00\01\01\02\01\02\05\01\05\01\06\04\06\04\05\03\05\03\01\04\01\04\00",
	"\01\034\00\02\00\01\01\00\04\00\05\01\05\05\06\05\06\06\03\06\03\05\04\05\04\01\01\01\01\02",
	"\01\026\00\00\00\06\01\06\01\04\04\06\06\06\02\03\06\00\04\00\01\02\01\00",
	"\01\014\00\06\00\00\06\00\06\01\01\01\01\06",
	"\01\030\00\00\00\06\01\06\03\04\05\06\06\06\06\00\05\00\05\04\03\03\01\04\01\00",
	"\01\024\00\00\00\06\02\06\05\01\05\06\06\06\06\00\04\00\01\05\01\00",
	"\01\032\00\01\00\05\01\06\05\06\06\05\06\01\05\00\01\00\00\01\05\01\05\05\01\05\01\01",
	"\01\030\00\00\00\06\05\06\06\05\06\03\05\02\01\02\01\03\05\03\05\05\01\05\01\00",
	"\02\036\04\00\01\00\00\01\00\05\01\06\05\06\06\05\06\02\04\00\04\02\05\02\05\05\01\05\01\01\04\01\014\06\00\06\01\05\02\04\02\04\01\05\00",
	"\01\036\00\00\00\06\05\06\06\05\06\03\05\02\06\00\05\00\04\02\01\02\01\03\05\03\05\05\01\05\01\00",
	"\01\054\00\02\00\01\01\00\05\00\06\01\06\02\05\03\01\04\01\05\05\05\05\04\06\04\06\05\05\06\01\06\00\05\00\04\01\03\05\02\05\01\01\01\01\02",
	"\01\020\02\00\02\05\00\05\00\06\05\06\05\05\03\05\03\00",
	"\01\024\00\06\00\01\01\00\05\00\06\01\06\06\05\06\05\01\01\01\01\06",
	"\01\016\00\06\02\00\04\00\06\06\05\06\03\01\01\06",
	"\01\030\00\06\01\00\02\00\03\02\04\00\05\00\06\06\05\06\04\02\03\04\02\02\01\06",
	"\01\030\00\00\02\03\00\06\01\06\03\04\05\06\06\06\04\03\06\00\05\00\03\02\01\00",
	"\02\014\00\06\02\03\02\00\03\00\03\03\01\06\10\02\03\03\03\05\06\04\06",
	"\01\024\00\05\04\05\00\01\00\00\06\00\06\01\02\01\06\05\06\06\00\06",
	"\01\020\04\00\02\00\02\06\04\06\04\05\03\05\03\01\04\01",
	"\01\10\00\05\05\00\06\01\01\06",
	"\01\020\02\00\02\01\03\01\03\05\02\05\02\06\04\06\04\00",
	"\01\014\00\02\03\06\06\02\05\02\03\05\01\02",
	"\01\10\00\01\06\01\06\02\00\02",
	"\01\10\02\04\03\05\03\06\02\06",
	"\04\06\03\04\03\05\04\06\024\00\02\00\04\01\05\02\05\03\04\04\05\05\05\06\04\05\04\04\02\012\00\02\04\02\04\01\02\00\01\00\014\02\01\04\04\05\02\06\02\05\00\04\00",
	"\01\016\00\03\03\00\03\02\06\02\06\04\03\04\03\06",
	"\01\016\00\02\00\04\03\04\03\06\06\03\03\00\03\02",
	"\01\06\00\00\03\06\06\00",
	"\03\06\01\00\05\00\03\03\06\03\03\00\03\01\06\06\03\03\06\03\05\06",
};

void ColonyEngine::wallChar(const float corners[4][3], uint8 cnum) {
	if (cnum < 0x20 || cnum > 0x65)
		cnum = 0x20;

	const uint8 *data = reinterpret_cast<const uint8 *>(kWallCharData[cnum - 0x20]);
	if (!data || data[0] == 0)
		return;

	const bool macMode = (_renderMode == Common::kRenderMacintosh);
	const bool macColors = (macMode && _hasMacColors);
	const uint32 fillColor = macColors ? packMacColor(_macColors[8 + _level - 1].bg) : 0;
	const uint32 lineColor = macColors ? (uint32)0xFF000000 : 0;

	auto drawFilledCharPolygon = [&](const float *u, const float *v, int count) {
		if (!macMode || count < 3)
			return;

		float cuts[96];
		int cutCount = 0;
		for (int i = 0; i < count; ++i)
			addSortedUniqueFloat(cuts, cutCount, v[i]);

		for (int i = 0; i < count; ++i) {
			const int nextI = (i + 1) % count;
			for (int j = i + 1; j < count; ++j) {
				const int nextJ = (j + 1) % count;
				if (j == i || j == nextI || nextJ == i)
					continue;
				if (i == 0 && nextJ == 0)
					continue;
				if ((nearlyEqual(u[i], u[j]) && nearlyEqual(v[i], v[j])) ||
					(nearlyEqual(u[i], u[nextJ]) && nearlyEqual(v[i], v[nextJ])) ||
					(nearlyEqual(u[nextI], u[j]) && nearlyEqual(v[nextI], v[j])) ||
					(nearlyEqual(u[nextI], u[nextJ]) && nearlyEqual(v[nextI], v[nextJ])))
					continue;

				float ix, iy;
				if (segmentIntersection2D(u[i], v[i], u[nextI], v[nextI], u[j], v[j], u[nextJ], v[nextJ], ix, iy))
					addSortedUniqueFloat(cuts, cutCount, iy);
			}
		}

		for (int band = 0; band < cutCount - 1; ++band) {
			const float y0 = cuts[band];
			const float y1 = cuts[band + 1];
			if (y1 - y0 <= 0.0001f)
				continue;

			const float yMid = (y0 + y1) * 0.5f;
			const float yTopSample = y0 + (y1 - y0) * 0.001f;
			const float yBottomSample = y1 - (y1 - y0) * 0.001f;
			WallCharSpan spans[32];
			int spanCount = 0;

			for (int i = 0; i < count; ++i) {
				const int next = (i + 1) % count;
				const float yA = v[i];
				const float yB = v[next];
				if (nearlyEqual(yA, yB))
					continue;
				if (!((yA <= yMid && yMid < yB) || (yB <= yMid && yMid < yA)))
					continue;

				spans[spanCount].xMid = edgeXAtY(u[i], yA, u[next], yB, yMid);
				spans[spanCount].xTop = edgeXAtY(u[i], yA, u[next], yB, yTopSample);
				spans[spanCount].xBottom = edgeXAtY(u[i], yA, u[next], yB, yBottomSample);
				++spanCount;
			}

			if (spanCount < 2)
				continue;

			sortWallCharSpans(spans, spanCount);
			for (int i = 0; i + 1 < spanCount; i += 2) {
				if (spans[i + 1].xMid - spans[i].xMid <= 0.0001f)
					continue;

				const float quadU[4] = {spans[i].xTop, spans[i + 1].xTop, spans[i + 1].xBottom, spans[i].xBottom};
				const float quadV[4] = {y0, y0, y1, y1};
				wallPolygon(corners, quadU, quadV, 4, fillColor);
			}
		}
	};

	if (macMode)
		_gfx->setWireframe(false);

	int offset = 1;
	for (int poly = 0; poly < data[0]; ++poly) {
		const int coordCount = data[offset++];
		int count = coordCount / 2;
		if (count > 32)
			count = 32;

		float u[32], v[32];
		for (int i = 0; i < count; ++i) {
			u[i] = (float)data[offset + i * 2] / 6.0f;
			v[i] = (float)data[offset + i * 2 + 1] / 6.0f;
		}

		drawFilledCharPolygon(u, v, count);
		for (int i = 0; i < count; ++i) {
			const int next = (i + 1) % count;
			wallLine(corners, u[i], v[i], u[next], v[next], lineColor);
		}

		offset += coordCount;
	}

	if (macMode) {
		const uint32 wallFill = _hasMacColors
			? packMacColor(_macColors[8 + _level - 1].fg)
			: (uint32)255;
		_gfx->setWireframe(true, wallFill);
	}
}

void ColonyEngine::getCellFace3D(int cellX, int cellY, bool ceiling, float corners[4][3]) {
	float z = ceiling ? 160.0f : -160.0f;
	float x0 = cellX * 256.0f;
	float y0 = cellY * 256.0f;
	float x1 = x0 + 256.0f;
	float y1 = y0 + 256.0f;
	const float eps = 0.1f;
	if (ceiling)
		z -= eps;
	else
		z += eps;

	corners[0][0] = x0; corners[0][1] = y0; corners[0][2] = z;
	corners[1][0] = x1; corners[1][1] = y0; corners[1][2] = z;
	corners[2][0] = x1; corners[2][1] = y1; corners[2][2] = z;
	corners[3][0] = x0; corners[3][1] = y1; corners[3][2] = z;
}

void ColonyEngine::drawCellFeature3D(int cellX, int cellY) {
	const uint8 *map = mapFeatureAt(cellX, cellY, kDirCenter);
	if (!map || map[0] == 0)
		return;

	float corners[4][3];
	bool ceiling = (map[0] == 3 || map[0] == 4); // SMHOLECEIL, LGHOLECEIL
	getCellFace3D(cellX, cellY, ceiling, corners);

	// DOS uses color_wall (PenColor) for hole outlines.
	// In our inverted 3D renderer: lit=black outlines on white fill, dark=white on black.
	bool lit = (_corePower[_coreIndex] > 0);
	uint32 holeColor = lit ? 0 : 7;

	const bool macMode = (_renderMode == Common::kRenderMacintosh);
	const bool macColors = (macMode && _hasMacColors);

	// Helper lambda: draw a filled hole polygon with Mac color or B&W fallback
	auto drawHolePoly = [&](const float *u, const float *v, int cnt, int macIdx) {
		if (macColors) {
			uint32 fg = packMacColor(_macColors[macIdx].fg);
			uint32 bg = packMacColor(_macColors[macIdx].bg);
			int pat = _macColors[macIdx].pattern;
			const byte *stipple = setupMacPattern(_gfx, pat, fg, bg);
			wallPolygon(corners, u, v, cnt, fg);
			if (stipple)
			_gfx->setStippleData(nullptr);
		} else if (macMode) {
			_gfx->setStippleData(kStippleGray);
			wallPolygon(corners, u, v, cnt, 0);
			_gfx->setStippleData(nullptr);
		} else {
			wallPolygon(corners, u, v, cnt, holeColor);
		}
	};

	switch (map[0]) {
	case 1: // SMHOLEFLR
	case 3: // SMHOLECEIL
	{
		float u[4] = {0.25f, 0.75f, 0.75f, 0.25f};
		float v[4] = {0.25f, 0.25f, 0.75f, 0.75f};
		drawHolePoly(u, v, 4, 30); // c_hole
		break;
	}
	case 2: // LGHOLEFLR
	case 4: // LGHOLECEIL
	{
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		drawHolePoly(u, v, 4, 30); // c_hole
		break;
	}
	case 5: // HOTFOOT
	{
		float u[4] = {0.0f, 1.0f, 1.0f, 0.0f};
		float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
		if (macMode) {
			drawHolePoly(u, v, 4, 31); // c_hotplate
		} else {
			// DOS non-polyfill: X pattern (two diagonals)
			wallLine(corners, 0.0f, 0.0f, 1.0f, 1.0f, holeColor);
			wallLine(corners, 1.0f, 0.0f, 0.0f, 1.0f, holeColor);
		}
		break;
	}
	default:
		break;
	}
}

static float stairStepHeight(const float *vf, const float *vc, int d, int s) {
	return vf[d] + (s + 1) / 8.0f * (vc[d] - vf[d]);
}

void ColonyEngine::drawWallFeature3D(int cellX, int cellY, int direction) {
	const uint8 *map = mapFeatureAt(cellX, cellY, direction);
	if (!map || map[0] == kWallFeatureNone)
		return;

	// Backface culling: only draw features for the side facing the camera.
	// This prevents backside decorations (like Level 2 lines) from bleeding through.
	// We use non-inclusive comparisons so features remain visible while standing on the boundary.
	switch (direction) {
	case kDirNorth:
		if (_me.yloc > (cellY + 1) * 256)
			return;
		break;
	case kDirSouth:
		if (_me.yloc < cellY * 256)
			return;
		break;
	case kDirWest:
		if (_me.xloc < cellX * 256)
			return;
		break;
	case kDirEast:
		if (_me.xloc > (cellX + 1) * 256)
			return;
		break;
	default: break;
	}

	float corners[4][3];
	getWallFace3D(cellX, cellY, direction, corners);
	const bool macMode = (_renderMode == Common::kRenderMacintosh);
	const bool macColors = (_renderMode == Common::kRenderMacintosh && _hasMacColors);
	const bool lit = (_corePower[_coreIndex] > 0);
	const uint32 wallFeatureFill = macColors
		? packMacColor(lit ? _macColors[8 + _level - 1].fg : _macColors[6].bg)
		: (lit ? (macMode ? 255u : 7u) : 0u);

	// Wall faces are already filled with level-specific color (c_char0+level-1.fg)
	// by the wall grid in renderCorridor3D(). Features are drawn on top.

	// Helper lambda: Mac color fill for a wall feature polygon
	auto macFillPoly = [&](const float *u, const float *v, int cnt, int macIdx) {
		uint32 fg = packMacColor(_macColors[macIdx].fg);
		uint32 bg = packMacColor(_macColors[macIdx].bg);
		const byte *stipple = setupMacPattern(_gfx, _macColors[macIdx].pattern, fg, bg);
		wallPolygon(corners, u, v, cnt, fg);
		if (stipple)
			_gfx->setStippleData(nullptr);
	};

	switch (map[0]) {
	case kWallFeatureDoor: {
		const uint32 doorColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
		bool shipLevel = (_level == 1 || _level == 5 || _level == 6);

		if (shipLevel) {
			static const float uSs[8] = { 0.375f, 0.250f, 0.250f, 0.375f, 0.625f, 0.750f, 0.750f, 0.625f };
			static const float vSs[8] = { 0.125f, 0.250f, 0.750f, 0.875f, 0.875f, 0.750f, 0.250f, 0.125f };
			const uint32 shipDoorColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : (_wireframe ? 8u : 0u));

			if (macMode) {
				if (map[1] != 0) {
					if (macColors) {
						macFillPoly(uSs, vSs, 8, 15); // c_bulkhead
					} else {
						_gfx->setStippleData(kStippleGray);
						wallPolygon(corners, uSs, vSs, 8, 0);
						_gfx->setStippleData(nullptr);
					}
				} else {
					// Open: fill with BLACK (passable opening)
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, uSs, vSs, 8, 0);
				}
			} else if (!_wireframe) {
				if (map[1] != 0) {
					const byte *stipple = setupMacPattern(_gfx, kPatternLtGray, 0, 15);
					wallPolygon(corners, uSs, vSs, 8, 0);
					if (stipple)
						_gfx->setStippleData(nullptr);
				} else {
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, uSs, vSs, 8, 0);
				}
			} else {
				_gfx->setWireframe(true);
				wallPolygon(corners, uSs, vSs, 8, 8);
			}

			for (int i = 0; i < 8; i++)
				wallLine(corners, uSs[i], vSs[i], uSs[(i + 1) % 8], vSs[(i + 1) % 8], shipDoorColor);

			if (map[1] != 0) {
				wallLine(corners, 0.375f, 0.25f, 0.375f, 0.75f, shipDoorColor);
				wallLine(corners, 0.375f, 0.75f, 0.625f, 0.75f, shipDoorColor);
				wallLine(corners, 0.625f, 0.75f, 0.625f, 0.25f, shipDoorColor);
				wallLine(corners, 0.625f, 0.25f, 0.375f, 0.25f, shipDoorColor);
			}
		} else {
			static const float xl = 0.25f, xr = 0.75f;
			static const float yb = 0.0f, yt = 0.875f;

			if (macMode) {
				float ud[4] = {xl, xr, xr, xl};
				float vd[4] = {yb, yb, yt, yt};
				if (map[1] != 0) {
					if (macColors) {
						macFillPoly(ud, vd, 4, 16); // c_door
					} else {
						_gfx->setStippleData(kStippleGray);
						wallPolygon(corners, ud, vd, 4, 0);
						_gfx->setStippleData(nullptr);
					}
				} else {
					// Open: fill with BLACK (passable opening)
					_gfx->setWireframe(true, 0);
					wallPolygon(corners, ud, vd, 4, 0);
					_gfx->setWireframe(true, 255);
				}
			}

			wallLine(corners, xl, yb, xl, yt, doorColor);
			wallLine(corners, xl, yt, xr, yt, doorColor);
			wallLine(corners, xr, yt, xr, yb, doorColor);
			wallLine(corners, xr, yb, xl, yb, doorColor);

			if (map[1] != 0) {
				wallLine(corners, 0.3125f, 0.4375f, 0.6875f, 0.4375f, doorColor);
			}
		}
		break;
	}
	case kWallFeatureWindow: {
		const uint32 winColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
		float xl = 0.25f, xr = 0.75f;
		float yb = 0.25f, yt = 0.75f;
		float xc = 0.5f, yc = 0.5f;

		// Mac: fill window pane
		if (macMode) {
			float uw[4] = {xl, xr, xr, xl};
			float vw[4] = {yb, yb, yt, yt};
			if (macColors) {
				macFillPoly(uw, vw, 4, 17); // c_window
			} else {
				_gfx->setStippleData(kStippleDkGray);
				wallPolygon(corners, uw, vw, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}

		wallLine(corners, xl, yb, xl, yt, winColor);
		wallLine(corners, xl, yt, xr, yt, winColor);
		wallLine(corners, xr, yt, xr, yb, winColor);
		wallLine(corners, xr, yb, xl, yb, winColor);
		wallLine(corners, xc, yb, xc, yt, winColor);
		wallLine(corners, xl, yc, xr, yc, winColor);
		break;
	}
	case kWallFeatureShelves: {
		// DOS drawbooks: recessed bookcase with 3D depth.
		const uint32 shelfColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);

		// Mac: fill shelves area
		if (macMode) {
			float us[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vs[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(us, vs, 4, 18); // c_shelves
			} else {
				_gfx->setStippleData(kStippleLtGray);
				wallPolygon(corners, us, vs, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}
		float bx = 0.1875f, bxr = 0.8125f;
		float by = 0.1875f, byt = 0.8125f;
		// Back face rectangle
		wallLine(corners, bx, by, bxr, by, shelfColor);
		wallLine(corners, bxr, by, bxr, byt, shelfColor);
		wallLine(corners, bxr, byt, bx, byt, shelfColor);
		wallLine(corners, bx, byt, bx, by, shelfColor);
		// Connecting lines (front corners to back corners)
		wallLine(corners, 0.0f, 0.0f, bx, by, shelfColor);
		wallLine(corners, 0.0f, 1.0f, bx, byt, shelfColor);
		wallLine(corners, 1.0f, 0.0f, bxr, by, shelfColor);
		wallLine(corners, 1.0f, 1.0f, bxr, byt, shelfColor);
		// 7 shelf lines across front face (DOS split7 at 1/8..7/8 intervals)
		for (int i = 1; i <= 7; i++) {
			float v = (float)i / 8.0f;
			wallLine(corners, 0.0f, v, 1.0f, v, shelfColor);
		}
		break;
	}
	case kWallFeatureUpStairs: {
		// DOS: draw_up_stairs  staircase ascending into the wall with perspective
		const uint32 col = macColors ? (uint32)0xFF000000 : 0; // vBLACK

		// Mac: fill entire wall face (c_upstairs)
		if (_renderMode == Common::kRenderMacintosh) {
			float uf[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vf2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(uf, vf2, 4, 19); // c_upstairs1
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, uf, vf2, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}

		// Perspective convergence: back of passage at ~1/3 width (1 cell deep)
		float ul[7], ur[7], vf[7], vc[7];
		for (int i = 0; i < 7; i++) {
			float f = (i + 1) / 8.0f;
			float inset = f * (1.0f / 3.0f);
			ul[i] = inset;
			ur[i] = 1.0f - inset;
			vf[i] = inset;        // floor rises toward center
			vc[i] = 1.0f - inset; // ceiling drops toward center
		}
		// Back of passage (full depth)
		float bi = 1.0f / 3.0f; // back inset
		float bu = bi, bur = 1.0f - bi, bvc = 1.0f - bi;

		// 1. Side wall verticals at back of passage
		wallLine(corners, bu, bvc, bu, 0.5f, col);
		wallLine(corners, bur, 0.5f, bur, bvc, col);

		// 2. Back wall landing (depth 6 to full depth)
		wallLine(corners, ul[6], stairStepHeight(vf, vc, 6, 6), bu, bvc, col);
		wallLine(corners, bu, bvc, bur, bvc, col);
		wallLine(corners, bur, bvc, ur[6], stairStepHeight(vf, vc, 6, 6), col);
		wallLine(corners, ur[6], stairStepHeight(vf, vc, 6, 6), ul[6], stairStepHeight(vf, vc, 6, 6), col);

		// 3. First step tread (floor from wall face to depth 0)
		wallLine(corners, 0.0f, 0.0f, ul[0], vf[0], col);
		wallLine(corners, ul[0], vf[0], ur[0], vf[0], col);
		wallLine(corners, ur[0], vf[0], 1.0f, 0.0f, col);
		wallLine(corners, 1.0f, 0.0f, 0.0f, 0.0f, col);

		// 4. First step riser (at depth 0)
		wallLine(corners, ul[0], stairStepHeight(vf, vc, 0, 0), ul[0], vf[0], col);
		wallLine(corners, ur[0], vf[0], ur[0], stairStepHeight(vf, vc, 0, 0), col);
		wallLine(corners, ur[0], stairStepHeight(vf, vc, 0, 0), ul[0], stairStepHeight(vf, vc, 0, 0), col);

		// 5. Step treads (i=3..0: depth i to depth i+1)
		for (int i = 3; i >= 0; i--) {
			wallLine(corners, ul[i], stairStepHeight(vf, vc, i, i), ul[i + 1], stairStepHeight(vf, vc, i + 1, i), col);
			wallLine(corners, ul[i + 1], stairStepHeight(vf, vc, i + 1, i), ur[i + 1], stairStepHeight(vf, vc, i + 1, i), col);
			wallLine(corners, ur[i + 1], stairStepHeight(vf, vc, i + 1, i), ur[i], stairStepHeight(vf, vc, i, i), col);
			wallLine(corners, ur[i], stairStepHeight(vf, vc, i, i), ul[i], stairStepHeight(vf, vc, i, i), col);
		}

		// 6. Step risers (i=5..0: vertical face at depth i+1)
		for (int i = 5; i >= 0; i--) {
			wallLine(corners, ul[i + 1], stairStepHeight(vf, vc, i + 1, i + 1), ul[i + 1], stairStepHeight(vf, vc, i + 1, i), col);
			wallLine(corners, ul[i + 1], stairStepHeight(vf, vc, i + 1, i), ur[i + 1], stairStepHeight(vf, vc, i + 1, i), col);
			wallLine(corners, ur[i + 1], stairStepHeight(vf, vc, i + 1, i), ur[i + 1], stairStepHeight(vf, vc, i + 1, i + 1), col);
			wallLine(corners, ur[i + 1], stairStepHeight(vf, vc, i + 1, i + 1), ul[i + 1], stairStepHeight(vf, vc, i + 1, i + 1), col);
		}

		// 7. Handrails: from center of wall edges up to near-ceiling at mid-depth
		wallLine(corners, 0.0f, 0.5f, ul[3], vc[0], col);
		wallLine(corners, 1.0f, 0.5f, ur[3], vc[0], col);
		break;
	}
	case kWallFeatureDnStairs: {
		// DOS: draw_dn_stairs  staircase descending into the wall with perspective
		const uint32 col = macColors ? (uint32)0xFF000000 : 0; // vBLACK

		// Mac: fill entire wall face (c_dnstairs)
		if (_renderMode == Common::kRenderMacintosh) {
			float uf[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vf2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(uf, vf2, 4, 21); // c_dnstairs
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, uf, vf2, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}

		float ul[7], ur[7], vf[7], vc[7];
		for (int i = 0; i < 7; i++) {
			float f = (i + 1) / 8.0f;
			float inset = f * (1.0f / 3.0f);
			ul[i] = inset;
			ur[i] = 1.0f - inset;
			vf[i] = inset;
			vc[i] = 1.0f - inset;
		}
		float bi = 1.0f / 3.0f;
		float bu = bi, bur = 1.0f - bi;

		// 1. Ceiling: front ceiling slopes down to mid-depth
		wallLine(corners, 0.0f, 1.0f, ul[3], vc[3], col);
		wallLine(corners, ul[3], vc[3], ur[3], vc[3], col);
		wallLine(corners, ur[3], vc[3], 1.0f, 1.0f, col);

		// 2. Slant: from mid-depth ceiling down to center at back
		wallLine(corners, ul[3], vc[3], bu, 0.5f, col);
		wallLine(corners, bu, 0.5f, bur, 0.5f, col);
		wallLine(corners, bur, 0.5f, ur[3], vc[3], col);

		// 3. Side wall verticals: from center at back down to floor level
		wallLine(corners, bu, 0.5f, bu, vf[0], col);
		wallLine(corners, bur, 0.5f, bur, vf[0], col);

		// 4. First step (floor from wall face to depth 0)
		wallLine(corners, 0.0f, 0.0f, ul[0], vf[0], col);
		wallLine(corners, ul[0], vf[0], ur[0], vf[0], col);
		wallLine(corners, ur[0], vf[0], 1.0f, 0.0f, col);
		wallLine(corners, 1.0f, 0.0f, 0.0f, 0.0f, col);

		// 5. Handrails: from center of wall edges down to floor at mid-depth
		wallLine(corners, 0.0f, 0.5f, ul[3], vf[3], col);
		wallLine(corners, 1.0f, 0.5f, ur[3], vf[3], col);
		break;
	}
	case kWallFeatureChar:
		wallChar(corners, map[1]);
		break;
	case kWallFeatureGlyph: {
		// DOS wireframe: PenColor(realcolor[vDKGRAY]) = 8
		const uint32 glyphColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);

		// Mac: fill glyph area
		if (macMode) {
			float ug[4] = {0.0f, 1.0f, 1.0f, 0.0f};
			float vg[4] = {0.0f, 0.0f, 1.0f, 1.0f};
			if (macColors) {
				macFillPoly(ug, vg, 4, 22); // c_glyph
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, ug, vg, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}

		for (int i = 0; i < 7; i++) {
			float v = 0.2f + i * 0.1f;
			wallLine(corners, 0.2f, v, 0.8f, v, glyphColor);
		}
		break;
	}
	case kWallFeatureElevator: {
		const uint32 elevColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
		float xl = 0.2f, xr = 0.8f;
		float yb = 0.1f, yt = 0.9f;

		// Mac: fill elevator door
		if (macMode) {
			float ue[4] = {xl, xr, xr, xl};
			float ve[4] = {yb, yb, yt, yt};
			if (macColors) {
				macFillPoly(ue, ve, 4, 23); // c_elevator
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, ue, ve, 4, 0);
				_gfx->setStippleData(nullptr);
			}
		}

		wallLine(corners, xl, yb, xl, yt, elevColor);
		wallLine(corners, xl, yt, xr, yt, elevColor);
		wallLine(corners, xr, yt, xr, yb, elevColor);
		wallLine(corners, xr, yb, xl, yb, elevColor);
		wallLine(corners, 0.5f, yb, 0.5f, yt, elevColor);
		break;
	}
	case kWallFeatureTunnel: {
		// Tunnel: hexagonal opening from Grid (0,0 0,5 1,6 5,6 6,5 6,0)
		static const float uT[6] = { 0.0f,    0.0f,    1/6.0f,  5/6.0f,  1.0f,    1.0f };
		static const float vT[6] = { 0.0f,    0.750f,  0.875f,  0.875f,  0.750f,  0.0f };
		if (_renderMode == Common::kRenderMacintosh) {
			if (macColors) {
				macFillPoly(uT, vT, 6, 24); // c_tunnel
			} else {
				_gfx->setStippleData(kStippleGray);
				wallPolygon(corners, uT, vT, 6, 0);
				_gfx->setStippleData(nullptr);
			}
		} else {
			wallPolygon(corners, uT, vT, 6, 0); // vBLACK outline
		}
		break;
	}
	case kWallFeatureAirlock: {
		// Direct port of drawALOpen/drawALClosed from WALLFTRS.C / wallftrs.c.
		// These are the exact split7x7 positions on the wall face.
		static const float u[8] = {0.125f, 0.25f, 0.5f, 0.75f, 0.875f, 0.75f, 0.5f, 0.25f};
		static const float v[8] = {0.5f, 0.75f, 0.875f, 0.75f, 0.5f, 0.25f, 0.125f, 0.25f};
		static const float spokeU[8] = {0.375f, 0.5f, 0.625f, 0.625f, 0.625f, 0.5f, 0.375f, 0.375f};
		static const float spokeV[8] = {0.625f, 0.625f, 0.625f, 0.5f, 0.375f, 0.375f, 0.375f, 0.5f};
		static const float centerU = 0.5f;
		static const float centerV = 0.5f;

		if (map[1] == 0) {
			// Original drawALOpen: solid black opening on both DOS and Mac.
			if (macMode || !_wireframe)
				_gfx->setWireframe(true, 0);
			else
				_gfx->setWireframe(true);
			wallPolygon(corners, u, v, 8, 0);
		} else {
			// Mac: fill airlock when closed
			if (macMode) {
				if (macColors) {
					macFillPoly(u, v, 8, 25); // c_airlock
				} else {
					_gfx->setStippleData(kStippleGray);
					wallPolygon(corners, u, v, 8, 0);
					_gfx->setStippleData(nullptr);
				}
			} else if (!_wireframe) {
				const byte *stipple = setupMacPattern(_gfx, kPatternLtGray, 0, 15);
				wallPolygon(corners, u, v, 8, 0);
				if (stipple)
					_gfx->setStippleData(nullptr);
			}

			const uint32 airlockColor = macColors ? (uint32)0xFF000000 : (macMode ? 0 : 8);
			for (int i = 0; i < 8; i++) {
				int n = (i + 1) % 8;
				wallLine(corners, u[i], v[i], u[n], v[n], airlockColor);
			}
			for (int i = 0; i < 8; i++) {
				wallLine(corners, u[i], v[i], spokeU[i], spokeV[i], airlockColor);
				wallLine(corners, spokeU[i], spokeV[i], centerU, centerV, airlockColor);
			}
		}
		break;
	}
	case kWallFeatureColor: {
		// Mac drawColor / DOS drawColor: 4 horizontal bands.
		// map[1..4] = pattern ID per band (0=WHITE, 1=LTGRAY, 2=GRAY, 3=DKGRAY, 4=BLACK).
		// Values >= 5 trigger animation: color = (map[i+1] + _displayCount) % 5.
		// Band 0 (top): v=0.75..1.0, Band 1: v=0.5..0.75, Band 2: v=0.25..0.5, Band 3: v=0..0.25.
		if (_renderMode == Common::kRenderMacintosh) {
			if (macColors) {
				// Mac drawColor: map[i+1] selects color (0→c_color0..3→c_color3, 4→BLACK).
				// Values >= 5: animated = (map[i+1] + _displayCount) % 5.
				for (int i = 0; i < 4; i++) {
					int val = map[i + 1];
					if (val > 4)
						val = (val + _displayCount / 6) % 5;
					float vb = (3 - i) / 4.0f;
					float vt = (4 - i) / 4.0f;
					float ub[4] = {0.0f, 1.0f, 1.0f, 0.0f};
					float vb4[4] = {vb, vb, vt, vt};
					if (val == 4) {
						// BLACK: solid black fill
						_gfx->setWireframe(true, (uint32)0xFF000000);
						wallPolygon(corners, ub, vb4, 4, 0xFF000000);
						_gfx->setWireframe(true, packMacColor(_macColors[8 + _level - 1].fg));
					} else {
						macFillPoly(ub, vb4, 4, 26 + val); // c_color0 + val
					}
				}
			} else {
				static const byte *stripPatterns[5] = {
					nullptr, kStippleLtGray, kStippleGray, kStippleDkGray, nullptr
				};
				for (int i = 0; i < 4; i++) {
					int pat = map[i + 1];
					if (pat > 4)
						pat = (pat + _displayCount / 6) % 5; // animated cycling
					float vb = (3 - i) / 4.0f;
					float vt = (4 - i) / 4.0f;
					float ub[4] = {0.0f, 1.0f, 1.0f, 0.0f};
					float vb4[4] = {vb, vb, vt, vt};
					if (pat == 4) {
						_gfx->setWireframe(true, 0);
						wallPolygon(corners, ub, vb4, 4, 0);
						_gfx->setWireframe(true, 255);
					} else if (pat == 0) {
						// WHITE: no fill needed (wall background is white)
					} else {
						_gfx->setStippleData(stripPatterns[pat]);
						wallPolygon(corners, ub, vb4, 4, 0);
						_gfx->setStippleData(nullptr);
					}
				}
			}
		}

		// EGA / Mac B&W: colored lines at band boundaries.
		// DOS non-polyfill draws 3 lines; we animate line colors for bands > 4.
		// Mac color mode uses macFillPoly for band fills instead.
		if (!macColors) {
			for (int i = 1; i <= 3; i++) {
				int val = map[i];
				if (val > 4)
					val = (val + _displayCount / 6) % 5; // animated cycling
				uint32 c = 120 + val * 20;
				if (c == 120 && val == 0 && !map[1] && !map[2] && !map[3] && !map[4])
					c = 100 + (_level * 15);
				float v = (float)i / 4.0f;
				wallLine(corners, 0.0f, v, 1.0f, v, c);
			}
		}
		break;
	}
	default:
		break;
	}

	_gfx->setStippleData(nullptr);
	_gfx->setWireframe(true, wallFeatureFill);
}

void ColonyEngine::drawWallFeatures3D() {
	if (_corePower[_coreIndex] == 0)
		return;

	for (int y = 0; y < 31; y++) {
		for (int x = 0; x < 31; x++) {
			if (!_visibleCell[x][y])
				continue;
			drawCellFeature3D(x, y);
			for (int dir = 0; dir < 4; dir++) {
				const uint8 *map = mapFeatureAt(x, y, dir);
				if (map && map[0] != kWallFeatureNone) {
					drawWallFeature3D(x, y, dir);
				}
			}
		}
	}
}

} // End of namespace Colony
