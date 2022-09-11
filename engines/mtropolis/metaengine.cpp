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

#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "graphics/scaler.h"
#include "graphics/surface.h"

#include "mtropolis/actions.h"
#include "mtropolis/debug.h"
#include "mtropolis/detection.h"
#include "mtropolis/mtropolis.h"

#include "mtropolis/mtropolis.h"

namespace Graphics {

struct Surface;

} // End of namespace Graphics

namespace MTropolis {

uint32 MTropolisEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform MTropolisEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace MTropolis

class MTropolisMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "mtropolis";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;

	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

bool MTropolisMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSupportsListSaves:
	case kSupportsDeleteSave:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSavesSupportPlayTime:
	case kSimpleSavesNames:
	case kSavesUseExtendedFormat:
		return true;
	default:
		return false;
	}
}

Common::Error MTropolisMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new MTropolis::MTropolisEngine(syst, reinterpret_cast<const MTropolis::MTropolisGameDescription *>(desc));
	return Common::kNoError;
}


Common::Array<Common::Keymap *> MTropolisMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "mtropolis", "mTropolis");

	Common::Action *act;
	act = new Common::Action("DEBUG_TOGGLE_OVERLAY", _("Toggle debug overlay"));
	act->setCustomEngineActionEvent(MTropolis::Actions::kDebugToggleOverlay);
	act->addDefaultInputMapping("F10");
	keymap->addAction(act);

	return Common::Keymap::arrayOf(keymap);
}

void MTropolisMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	const Graphics::Surface *savegameScreenshot = static_cast<MTropolis::MTropolisEngine *>(g_engine)->getSavegameScreenshot();

	if (savegameScreenshot) {
		int thumbnailWidth = kThumbnailWidth;
		int thumbnailHeight = thumbnailWidth * savegameScreenshot->h / savegameScreenshot->w;
		if (thumbnailHeight > kThumbnailHeight2) {
			thumbnailHeight = kThumbnailHeight2;
			thumbnailWidth = thumbnailHeight * savegameScreenshot->w / savegameScreenshot->h;
		}

		Common::SharedPtr<Graphics::Surface> outSurface(new Graphics::Surface());
		outSurface->create(savegameScreenshot->w, savegameScreenshot->h, Graphics::createPixelFormat<888>());

		for (int y = 0; y < savegameScreenshot->h; y++) {
			for (int x = 0; x < savegameScreenshot->w; x++) {
				uint8 r, g, b;
				savegameScreenshot->format.colorToRGB(savegameScreenshot->getPixel(x, y), r, g, b);
				outSurface->setPixel(x, y, outSurface->format.RGBToColor(r, g, b));
			}
		}

		while (outSurface->w >= thumbnailWidth * 2) {
			Common::SharedPtr<Graphics::Surface> temp(new Graphics::Surface());
			temp->create(outSurface->w / 2, outSurface->h, Graphics::createPixelFormat<888>());

			for (int y = 0; y < temp->h; y++) {
				for (int x = 0; x < temp->w; x++) {
					uint32 px1 = outSurface->getPixel(x * 2, y);
					uint32 px2 = outSurface->getPixel(x * 2 + 1, y);

					uint8 r1, g1, b1;
					outSurface->format.colorToRGB(px1, r1, g1, b1);

					uint8 r2, g2, b2;
					outSurface->format.colorToRGB(px2, r2, g2, b2);

					temp->setPixel(x, y, temp->format.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1));
				}
			}

			outSurface = temp;
		}

		while (outSurface->h >= thumbnailHeight * 2) {
			Common::SharedPtr<Graphics::Surface> temp(new Graphics::Surface());
			temp->create(outSurface->w, outSurface->h / 2, Graphics::createPixelFormat<888>());

			for (int y = 0; y < temp->h; y++) {
				for (int x = 0; x < temp->w; x++) {
					uint32 px1 = outSurface->getPixel(x, y * 2);
					uint32 px2 = outSurface->getPixel(x, y * 2 + 1);

					uint8 r1, g1, b1;
					outSurface->format.colorToRGB(px1, r1, g1, b1);

					uint8 r2, g2, b2;
					outSurface->format.colorToRGB(px2, r2, g2, b2);

					temp->setPixel(x, y, temp->format.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1));
				}
			}

			outSurface = temp;
		}

		// TODO: Fix this for weird sizes
		Common::SharedPtr<Graphics::Surface> changeTo16Temp = outSurface;
		outSurface.reset(new Graphics::Surface());
		outSurface->create(changeTo16Temp->w, changeTo16Temp->h, Graphics::createPixelFormat<565>());

		for (int y = 0; y < outSurface->h; y++) {
			for (int x = 0; x < outSurface->w; x++) {
				uint32 px = changeTo16Temp->getPixel(x, y);

				uint8 r, g, b;
				changeTo16Temp->format.colorToRGB(px, r, g, b);

				outSurface->setPixel(x, y, outSurface->format.RGBToColor(r, g, b));
			}
		}

		changeTo16Temp.reset();

		thumb.copyFrom(*outSurface);
	} else {
		AdvancedMetaEngine::getSavegameThumbnail(thumb);
	}
}

#if PLUGIN_ENABLED_DYNAMIC(MTROPOLIS)
REGISTER_PLUGIN_DYNAMIC(MTROPOLIS, PLUGIN_TYPE_ENGINE, MTropolisMetaEngine);
#else
REGISTER_PLUGIN_STATIC(MTROPOLIS, PLUGIN_TYPE_ENGINE, MTropolisMetaEngine);
#endif
