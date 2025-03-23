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

#ifndef ALG_DETECTION_H
#define ALG_DETECTION_H

#include "engines/advancedDetector.h"

namespace Alg {

enum AlgGameType {
	GType_CPATROL_SS_DOS,
	GType_CPATROL_DS_DOS,
	GType_CPATROL_DEMO_DOS,
	GType_DWARS_SS_DOS,
	GType_DWARS_DS_DOS,
	GType_DWARS_DEMO_DOS,
	GType_JOHNROC_SS_DOS,
	GType_JOHNROC_DS_DOS,
	GType_LBHUNTER_DOS,
	GType_LBHUNTER_DEMO_DOS,
	GType_MADDOG_DOS,
	GType_MADDOG2_SS_DOS,
	GType_MADDOG2_DS_DOS,
	GType_SPIRATES_SS_DOS,
	GType_SPIRATES_DS_DOS,
	GType_SPIRATES_DEMO_DOS,
};

struct AlgGameDescription {
	ADGameDescription desc;
	uint8 gameType;

	uint32 sizeBuffer() const {
		return desc.sizeBuffer();
	}

	void *toBuffer(void *buffer) {
		return desc.toBuffer(buffer);
	}
};

} // End of namespace Alg

#endif // ALG_DETECTION_H
