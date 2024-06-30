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

#ifndef TETRAEDGE_METAENGINE_H
#define TETRAEDGE_METAENGINE_H

#include "engines/achievements.h"
#include "engines/advancedDetector.h"

#define GAMEOPTION_CORRECT_MOVIE_ASPECT GUIO_GAMEOPTIONS1
#define GAMEOPTION_RESTORE_SCENES GUIO_GAMEOPTIONS2

class TetraedgeMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	void getSavegameThumbnail(Graphics::Surface &thumb) override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	bool hasFeature(MetaEngineFeature f) const override;

};

#endif
