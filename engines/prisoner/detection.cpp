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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "prisoner/detection.h"
#include "prisoner/detection_tables.h"

PrisonerMetaEngineDetection::PrisonerMetaEngineDetection() : AdvancedMetaEngineDetection(Prisoner::gameDescriptions, Prisoner::prisonerGames) {
}

REGISTER_PLUGIN_STATIC(PRISONER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PrisonerMetaEngineDetection);
