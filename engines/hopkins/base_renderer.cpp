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

// Portable indexed-colour ACK-style renderer for Hopkins WBASE.

#include "hopkins/base_renderer.h"

#include "common/algorithm.h"
#include "common/util.h"

namespace Hopkins {

namespace {

struct VisibleObject {
	int id;
	int distance;
	int centerColumn;
	int halfSize;
};

class BaseSoftwareRenderer : public BaseRenderer {
public:
	BaseSoftwareRenderer() {
		_wallDistance.resize(kBaseViewWidth);
		_floorWallDistance.resize(kBaseViewWidth);
		_hits.resize(kBaseViewWidth);
	}

	void render(const BaseEngine &engine, byte *framebuffer) override;

private:
	void buildWallColumns(const BaseEngine &engine);
	void renderFloorAndCeiling(const BaseEngine &engine, byte *framebuffer) const;
	void renderWalls(const BaseEngine &engine, byte *framebuffer) const;
	void renderObjects(const BaseEngine &engine, byte *framebuffer) const;
	void renderHud(const BaseEngine &engine, byte *framebuffer) const;
	void drawCenteredTextureColumn(const BaseBitmap &bitmap, int textureColumn, int distance,
			int screenColumn, byte *framebuffer) const;
	void drawSprite(const BaseBitmap &bitmap, int destX, int destY, byte *framebuffer) const;

	Common::Array<BaseRayHit> _hits;
	Common::Array<int> _wallDistance;
	Common::Array<int> _floorWallDistance;
};

static int signedAngleDelta(int from, int to) {
	int delta = normalizeBaseAngle(to - from);
	if (delta > kBaseHalfTurn)
		delta -= kBaseAngleCount;
	return delta;
}

void BaseSoftwareRenderer::render(const BaseEngine &engine, byte *framebuffer) {
	if (!framebuffer)
		return;

	Common::fill(framebuffer, framebuffer + kBaseFrameWidth * kBaseFrameHeight, 0);
	buildWallColumns(engine);
	renderFloorAndCeiling(engine, framebuffer);
	renderWalls(engine, framebuffer);
	renderObjects(engine, framebuffer);
	renderHud(engine, framebuffer);
}

void BaseSoftwareRenderer::buildWallColumns(const BaseEngine &engine) {
	for (int column = 0; column < kBaseViewWidth; ++column) {
		const int angle = normalizeBaseAngle(engine.playerAngle() - kBaseAngleHalfFov + column);
		_hits[column] = engine.castRay(angle, column);
		_wallDistance[column] = _hits[column].hit ? _hits[column].distance : kBaseMaximumDistance - 1;

		// Hopkins' Windows ACK fork stores the pre-fisheye distance with six
		// fractional bits remaining at the floor renderer handoff.
		int floorDistance = _hits[column].hit ? (int)(_hits[column].rawDistance >> 6) : kBaseMaximumDistance - 1;
		_floorWallDistance[column] = CLIP(floorDistance, 1, kBaseMaximumDistance - 1);
	}
}

void BaseSoftwareRenderer::renderFloorAndCeiling(const BaseEngine &engine, byte *framebuffer) const {
	const BaseData &data = engine.data();
	if (!engine.texturesEnabled()) {
		for (int y = 0; y < kBaseHorizon; ++y)
			Common::fill(framebuffer + y * kBaseFrameWidth,
					framebuffer + y * kBaseFrameWidth + kBaseViewWidth, 4);
		for (int y = kBaseHorizon; y < kBaseViewHeight; ++y)
			Common::fill(framebuffer + y * kBaseFrameWidth,
					framebuffer + y * kBaseFrameWidth + kBaseViewWidth, 5);
		return;
	}

	const BaseBitmap &floorBitmap = data.wallBitmap(5);
	const BaseBitmap &ceilingBitmap = data.wallBitmap(4);
	// Hopkins retains ACK's initialized camera ViewHeight value of 31.
	// This is distinct from the 180-pixel viewport height.
	const int floorHeight = 89 - 31;
	const int scaleHeight = floorHeight * (floorHeight * 5);
	// The Hopkins fork uses a one-row-biased screen base and scans rows 7..91.
	const int firstScanRow = 7;
	const int lastScanRow = kBaseHorizon + 1;
	const int floorScanOrigin = kBaseHorizon - 2;
	const int ceilingScanOrigin = kBaseHorizon + 2;

	for (int column = 0; column < kBaseViewWidth; column += 2) {
		const int angle = normalizeBaseAngle(engine.playerAngle() - kBaseAngleHalfFov + column);
		const int32 cv = data.cosQ16(angle);
		const int32 sv = data.sinQ16(angle);
		const int32 floorCos = data.floorCos(column);
		const int wallDistance = _floorWallDistance[column];
		int previousDistance = -1;
		int worldX = 0;
		int worldY = 0;

		for (int row = firstScanRow; row <= lastScanRow; ++row) {
			const int scan = scaleHeight / row;
			const int distance = (int)(((int64)floorCos * scan) >> 15);
			if (distance >= wallDistance)
				continue;

			if (distance != previousDistance) {
				worldX = engine.playerX() + (int)(((int64)cv * distance) >> 16);
				worldY = engine.playerY() + (int)(((int64)sv * distance) >> 16);
				previousDistance = distance;
			}
			const int mapPos = baseWorldMapIndex(worldX, worldY);
			if (mapPos < 0 || mapPos >= kBaseMapCellCount)
				continue;

			const uint textureX = worldX & 63;
			const uint textureY = worldY & 63;
			const byte floorPixel = floorBitmap.sample(textureX, textureY);
			const byte ceilingPixel = ceilingBitmap.sample(textureX, textureY);
			const int floorY = floorScanOrigin + row;
			const int ceilingY = ceilingScanOrigin - row;

			if (floorY >= 0 && floorY < kBaseViewHeight) {
				framebuffer[floorY * kBaseFrameWidth + column] = floorPixel;
				if (column + 1 < kBaseViewWidth)
					framebuffer[floorY * kBaseFrameWidth + column + 1] = floorPixel;
			}
			if (ceilingY >= 0 && ceilingY < kBaseViewHeight) {
				framebuffer[ceilingY * kBaseFrameWidth + column] = ceilingPixel;
				if (column + 1 < kBaseViewWidth)
					framebuffer[ceilingY * kBaseFrameWidth + column + 1] = ceilingPixel;
			}
		}
	}
}

void BaseSoftwareRenderer::renderWalls(const BaseEngine &engine, byte *framebuffer) const {
	const BaseData &data = engine.data();
	for (int column = 0; column < kBaseViewWidth; ++column) {
		const BaseRayHit &hit = _hits[column];
		if (!hit.hit)
			continue;

		const uint bitmapId = hit.code & 0xff;
		const BaseBitmap &bitmap = data.wallBitmap(bitmapId);
		if (!bitmap.valid())
			continue;
		drawCenteredTextureColumn(bitmap, hit.textureColumn, hit.distance, column, framebuffer);
	}
}

void BaseSoftwareRenderer::renderObjects(const BaseEngine &engine, byte *framebuffer) const {
	const BaseData &data = engine.data();
	Common::Array<VisibleObject> visible;

	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		const BaseObject &object = engine.object(id);
		if (!object.active)
			continue;

		const int deltaX = object.x - engine.playerX();
		const int deltaY = object.y - engine.playerY();
		const int angle = engine.objectAngle(deltaX, deltaY);
		const int relative = signedAngleDelta(engine.playerAngle(), angle);
		const int distance = engine.objectDistance(object);
		if (distance <= 0 || distance >= kBaseMaximumDistance - 10)
			continue;

		const int halfSize = data.distanceHeight(distance);
		if (halfSize < 3 || halfSize > 300)
			continue;
		if (relative + halfSize < -kBaseAngleHalfFov || relative - halfSize > kBaseAngleHalfFov)
			continue;

		VisibleObject item;
		item.id = id;
		item.distance = distance;
		item.centerColumn = kBaseViewHalfWidth + relative;
		item.halfSize = halfSize;

		uint insertAt = 0;
		while (insertAt < visible.size() && visible[insertAt].distance > distance)
			++insertAt;
		visible.push_back(item);
		for (uint pos = visible.size() - 1; pos > insertAt; --pos)
			visible[pos] = visible[pos - 1];
		visible[insertAt] = item;
	}

	for (uint index = 0; index < visible.size(); ++index) {
		const VisibleObject &item = visible[index];
		const BaseObject &object = engine.object(item.id);
		const BaseBitmap &bitmap = data.objectBitmap(object.bitmap);
		if (!bitmap.valid())
			continue;

		const int left = item.centerColumn - item.halfSize;
		const int right = item.centerColumn + item.halfSize;
		const int fullSize = MAX(2, item.halfSize * 2);
		for (int x = MAX(0, left); x < MIN(kBaseViewWidth, right); ++x) {
			if (item.distance >= _wallDistance[x])
				continue;
			const int textureX = CLIP(((x - left) * 64) / fullSize, 0, 63);
			if (bitmap.blankColumns.size() == bitmap.width && bitmap.blankColumns[textureX])
				continue;
			drawCenteredTextureColumn(bitmap, textureX, item.distance, x, framebuffer);
		}
	}
}

void BaseSoftwareRenderer::drawCenteredTextureColumn(const BaseBitmap &bitmap, int textureColumn,
		int distance, int screenColumn, byte *framebuffer) const {
	if (!bitmap.valid() || !bitmap.columnMajor || distance <= 0 ||
			screenColumn < 0 || screenColumn >= kBaseViewWidth)
		return;
	const byte *sourceColumn = bitmap.pixels.begin() +
			((uint)textureColumn % bitmap.width) * bitmap.height;

	// Hopkins' non-shaded transfer starts at source rows 31 and 32, then
	// advances both halves away from the horizon with an 8.8 accumulator.
	// This preserves ACK's integer rounding and its asymmetric center pair.
	uint32 sourcePosition = 0;
	for (int row = 0; row < kBaseViewHeight; ++row) {
		const uint sourceOffset = sourcePosition >> 8;
		if (sourceOffset >= 32)
			break;

		const int upperY = kBaseHorizon - row;
		const int lowerY = kBaseHorizon + row + 1;
		if (upperY >= 0) {
			const byte pixel = sourceColumn[31 - sourceOffset];
			if (pixel)
				framebuffer[upperY * kBaseFrameWidth + screenColumn] = pixel;
		}
		if (lowerY < kBaseViewHeight) {
			const byte pixel = sourceColumn[32 + sourceOffset];
			if (pixel)
				framebuffer[lowerY * kBaseFrameWidth + screenColumn] = pixel;
		}
		if (upperY < 0 && lowerY >= kBaseViewHeight)
			break;
		sourcePosition += distance;
	}
}

void BaseSoftwareRenderer::drawSprite(const BaseBitmap &bitmap, int destX, int destY, byte *framebuffer) const {
	if (!bitmap.valid())
		return;
	for (uint y = 0; y < bitmap.height; ++y) {
		const int screenY = destY + (int)y;
		if (screenY < 0 || screenY >= kBaseFrameHeight)
			continue;
		for (uint x = 0; x < bitmap.width; ++x) {
			const int screenX = destX + (int)x;
			if (screenX < 0 || screenX >= kBaseFrameWidth)
				continue;
			const byte pixel = bitmap.sample(x, y);
			if (pixel)
				framebuffer[screenY * kBaseFrameWidth + screenX] = pixel;
		}
	}
}

void BaseSoftwareRenderer::renderHud(const BaseEngine &engine, byte *framebuffer) const {
	const BaseData &data = engine.data();
	const uint weaponFrame = engine.weaponCounter() > 3 ? 1 : 0;
	drawSprite(data.weaponFrame(weaponFrame), 128, 75, framebuffer);

	const int health = CLIP(engine.health() / 10, 0, 999);
	const int digits[3] = { health / 100, (health / 10) % 10, health % 10 };
	for (int i = 0; i < 3; ++i)
		drawSprite(data.fontFrame(digits[i]), 13 + i * 12, 160, framebuffer);
}

} // End of anonymous namespace

BaseRenderer *BaseRenderer::createSoftware() {
	return new BaseSoftwareRenderer();
}

} // End of namespace Hopkins
