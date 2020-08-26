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

#ifndef CRYOMNI3D_DETECTION_H
#define CRYOMNI3D_DETECTION_H

#include "engines/advancedDetector.h"

namespace CryOmni3D {

enum CryOmni3DGameType {
	GType_VERSAILLES
};

enum CryOmni3DGameFeatures {
	GF_VERSAILLES_FONTS_MASK               = (3 << 0), // Fonts flag mask
	GF_VERSAILLES_FONTS_NUMERIC            = (0 << 0), // Fonts are font01.crf, ...
	GF_VERSAILLES_FONTS_SET_A              = (1 << 0), // Fonts are for French Macintosh (development version)
	GF_VERSAILLES_FONTS_SET_B              = (2 << 0), // Standard set (Helvet12 is used for debugging docs)
	GF_VERSAILLES_FONTS_SET_C              = (3 << 0), // Fonts for Italian version (Helvet12 is used for docs texts)

	GF_VERSAILLES_AUDIOPADDING_NO          = (0 << 2), // Audio files have underscore padding before extension
	GF_VERSAILLES_AUDIOPADDING_YES         = (1 << 2), // Audio files have underscore padding before extension

	GF_VERSAILLES_LINK_STANDARD            = (0 << 3), // Links file is lien_doc.txt
	GF_VERSAILLES_LINK_LOCALIZED           = (1 << 3)  // Links file is taken from cryomni3d.dat
};

struct CryOmni3DGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
};

} // End of namespace CryOmni3D

#endif // CRYOMNI3D_DETECTION_H
