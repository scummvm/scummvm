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

#ifndef BAGEL_DETECTION_H
#define BAGEL_DETECTION_H

#include "engines/advancedDetector.h"

namespace Bagel {

extern const PlainGameDescriptor bagelGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Bagel

class BagelMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {

public:
	BagelMetaEngineDetection();
	~BagelMetaEngineDetection() override {}

	const char *getName() const override {
		return "bagel";
	}

	const char *getEngineName() const override {
		return "BAGEL"; // the Boffo Adventure Game Engine and Libraries
	}

	const char *getOriginalCopyright() const override {
		return "Copyright(C) 1996 Boffo Games, Inc.";
	}
};

#endif // BAGEL_DETECTION_H
