/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "graphics/surface.h"

#include "trecision/trecision.h"

class TrecisionMetaEngine : public AdvancedMetaEngine {
	const char *getName() const override {
		return "trecision";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

Common::Error TrecisionMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Trecision::TrecisionEngine(syst, desc);
	return Common::kNoError;
}

void TrecisionMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	// We are referencing g_engine here, but this should be safe, as this
	// method is only used while the engine is running.
	// TODO: Is there a better way to do this?
	thumb.copyFrom(((Trecision::TrecisionEngine *)g_engine)->_thumbnail);
}

bool Trecision::TrecisionEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSubtitleOptions) ||
//		   (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime) ||
		   (f == kSupportsChangingOptionsDuringRuntime);
}

#if PLUGIN_ENABLED_DYNAMIC(TRECISION)
	REGISTER_PLUGIN_DYNAMIC(TRECISION, PLUGIN_TYPE_ENGINE, TrecisionMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TRECISION, PLUGIN_TYPE_ENGINE, TrecisionMetaEngine);
#endif
