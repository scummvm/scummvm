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

#include "common/config-manager.h"
#include "common/frac.h"
#include "common/rotationmode.h"
#include "common/system.h"
#include "graphics/hotspot_renderer.h"
#include "hollywood/hotspots.h"

namespace Hollywood {
namespace {

class HollywoodHotspotRenderer : public Graphics::HotspotRenderer {
protected:
	void prepareRender(const Common::Array<Graphics::HotspotInfo> &hotspots,
			int gameWidth, int gameHeight, int overlayWidth, int overlayHeight) override {
		_screenSize = Common::Point(gameWidth, gameHeight);
		_gameArea = Common::Rect(overlayWidth, overlayHeight);

		const int stretchMode = g_system->getStretchMode();
		const OSystem::GraphicsMode *mode = g_system->getSupportedStretchModes();
		while (mode && mode->name && mode->id != stretchMode) {
			++mode;
		}
		if (!mode || !mode->name) {
			return;
		}

		const bool fit = !strcmp(mode->name, "fit");
		const bool center = !strcmp(mode->name, "center");
		const bool integral = !strcmp(mode->name, "pixel-perfect") || !strcmp(mode->name, "even-pixels");
		if (!fit && !center && !integral) {
			return;
		}

		if (g_system->hasFeature(OSystem::kFeatureRotationMode) &&
				Common::parseRotationMode(ConfMan.getInt("rotation_mode")) != Common::kRotationNormal) {
			return;
		}

		// Desktop backends can center the game inside a larger overlay.
		const Common::Rect safeArea = g_system->getSafeOverlayArea();
		if (safeArea.isEmpty()) {
			return;
		}

		int width = safeArea.width();
		int height = safeArea.height();
		const frac_t aspect = intToFrac(gameWidth) / gameHeight;
		if (fit) {
			const frac_t overlayAspect = intToFrac(width) / height;
			if (overlayAspect < aspect) {
				height = intToFrac(width) / aspect;
			} else if (overlayAspect > aspect) {
				width = fracToInt(height * aspect);
			}
		} else {
			width = gameWidth * g_system->getScaleFactor();
			height = intToFrac(width) / aspect;
			if (width > safeArea.width() || height > safeArea.height()) {
				const int divisor = 1 + MAX((width - 1) / safeArea.width(), (height - 1) / safeArea.height());
				width /= divisor;
				height /= divisor;
			} else if (integral) {
				const int factor = MIN(safeArea.width() / width, safeArea.height() / height);
				width *= factor;
				height *= factor;
			}
		}

		int16 left = (overlayWidth - width) / 2;
		int16 top = (overlayHeight - height) / 2;
		safeArea.constrain(left, top, width, height);
		_gameArea = Common::Rect(left, top, left + width, top + height);
		_sizeScale = _style.scaleWithGame ? MIN((float)width / gameWidth, (float)height / gameHeight) : 1.0f;
		_markerSize = MAX(2, (int)(_style.markerSize * _sizeScale));
		_pointRadius = MAX(1, (int)(_style.pointRadius * _sizeScale));
		_lineThickness = MAX(1, (int)(_style.lineThickness * _sizeScale));
	}

	Common::Point placeHotspot(const Graphics::HotspotInfo &hotspot, Common::Point position) override {
		return Common::Point(
			_gameArea.left + hotspot.position.x * _gameArea.width() / _screenSize.x,
			_gameArea.top + hotspot.position.y * _gameArea.height() / _screenSize.y);
	}

private:
	Common::Point _screenSize;
	Common::Rect _gameArea;
};

} // End of anonymous namespace

void HotspotSource::getHotspotPositions(Common::Array<Graphics::HotspotInfo> &hotspots, const Common::Rect &viewport) const {
	Common::Rect bounds[256];
	for (int y = viewport.top; y < viewport.bottom; ++y) {
		for (int x = viewport.left; x < viewport.right; ++x) {
			const byte id = hotspotAt(Common::Point(x, y));
			if (!id) {
				continue;
			}
			const Common::Rect pixel(x, y, x + 1, y + 1);
			if (bounds[id].isEmpty()) {
				bounds[id] = pixel;
			} else {
				bounds[id].extend(pixel);
			}
		}
	}

	for (uint id = 1; id < ARRAYSIZE(bounds); ++id) {
		if (bounds[id].isEmpty()) {
			continue;
		}
		const Common::U32String name = hotspotName(id);
		if (name.empty()) {
			continue;
		}
		const Common::Point center = bounds[id].center();
		Common::Point position = center;
		// Keep markers inside irregular or disconnected click regions.
		if (hotspotAt(center) != id) {
			uint bestDistance = viewport.width() * viewport.width() + viewport.height() * viewport.height();
			for (int y = bounds[id].top; y < bounds[id].bottom; ++y) {
				for (int x = bounds[id].left; x < bounds[id].right; ++x) {
					const Common::Point point(x, y);
					const uint distance = point.sqrDist(center);
					if (distance < bestDistance && hotspotAt(point) == id) {
						position = point;
						bestDistance = distance;
					}
				}
			}
		}
		hotspots.push_back(Graphics::HotspotInfo(position, name));
	}
}

Graphics::HotspotRenderer *createHollywoodHotspotRenderer() {
	return new HollywoodHotspotRenderer();
}

} // End of namespace Hollywood
