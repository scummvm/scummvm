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

#include "common/file.h"
#include "common/md5.h"

#include "cryomni3d/detection.h"

namespace CryOmni3D {

static const PlainGameDescriptor cryomni3DGames[] = {
	{"versailles", "Versailles 1685"},
	{0, 0}
};

} // End of namespace CryOmni3D

#include "cryomni3d/detection_tables.h"

namespace CryOmni3D {

static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CryOmni3DMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CryOmni3DMetaEngineDetection() : AdvancedMetaEngineDetection(CryOmni3D::gameDescriptions,
				sizeof(CryOmni3DGameDescription), cryomni3DGames, optionsList) {
		_directoryGlobs = directoryGlobs;
		_maxScanDepth = 5;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles,
								  const Common::FSList &fslist) const override {
		return detectGameFilebased(allFiles, fileBased);
	}

	const char *getEngineId() const override {
		return "cryomni3d";
	}

	const char *getName() const override {
		return "Cryo Omni3D";
	}

	const char *getOriginalCopyright() const override {
		return "Cryo game Engine (C) 1997-2002 Cryo Interactive";
	}
};

} // End of Namespace CryOmni3D

REGISTER_PLUGIN_STATIC(CRYOMNI3D_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CryOmni3D::CryOmni3DMetaEngineDetection);
