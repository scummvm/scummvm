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

#include "base/plugins.h"

#include "harvester/detection.h"
#include "harvester/detection_tables.h"

const DebugChannelDef HarvesterMetaEngineDetection::debugFlagList[] = {
	{ Harvester::kDebugGeneral, "general", "General debug level" },
	{ Harvester::kDebugCursor, "cursor", "Cursor animation, hover, and placement debug" },
	{ Harvester::kDebugRoom, "room", "Room setup, scene population, and interaction debug" },
	{ Harvester::kDebugPlayer, "player", "Player animation, movement, and spawn debug" },
	{ Harvester::kDebugPathfinding, "pathfinding", "Player pathfinding target, blocker, and movement-band debug" },
	{ Harvester::kDebugCombat, "combat", "Room combat pursuit, attack, and damage debug" },
	{ Harvester::kDebugResources, "resources", "Resource loading" },
	{ Harvester::kDebugScene, "scene", "Scene object selection and startup script debug" },
	{ Harvester::kDebugDialogue, "dialogue", "Startup dialogue and NPC interaction debug" },
	{ Harvester::kDebugInventory, "inventory", "Inventory hover, click, and secondary-action debug" },
	DEBUG_CHANNEL_END
};

HarvesterMetaEngineDetection::HarvesterMetaEngineDetection() : AdvancedMetaEngineDetection(
	Harvester::gameDescriptions, Harvester::harvesterGames) {
	_maxScanDepth = 1;
}

REGISTER_PLUGIN_STATIC(HARVESTER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HarvesterMetaEngineDetection);
