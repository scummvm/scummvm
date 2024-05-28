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

#ifndef M4_METAENGINE_H
#define M4_METAENGINE_H

#include "graphics/surface.h"
#include "engines/advancedDetector.h"

#include "engines/m4/detection.h"

class M4MetaEngine : public AdvancedMetaEngine<M4::M4GameDescription> {
private:
	Common::InSaveFile *getOriginalSave(const Common::String &saveName) const;

public:
	Graphics::Surface *_thumbnail = nullptr;

public:
	~M4MetaEngine() override {}

	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const M4::M4GameDescription *desc) const override;

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	/**
	 * Convert the current screen contents to a thumbnail. Can be overriden by individual
	 * engine meta engines to provide their own thumb, such as hiding any on-screen save
	 * dialog so that it won't appear in the thumbnail.
	 */
	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

#endif // M4_METAENGINE_H
