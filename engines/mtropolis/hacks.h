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

#ifndef MTROPOLIS_HACKS_H
#define MTROPOLIS_HACKS_H

#include "common/rect.h"
#include "common/ptr.h"
#include "common/hashmap.h"

namespace MTropolis {

class AssetHooks;
class ModifierHooks;
class SaveLoadHooks;
class SceneTransitionHooks;
class StructuralHooks;
struct IAutoSaveProvider;
struct MTropolisGameDescription;

struct Hacks {
	Hacks();
	~Hacks();

	void addStructuralHooks(uint32 guid, const Common::SharedPtr<StructuralHooks> &hooks);
	void addModifierHooks(uint32 guid, const Common::SharedPtr<ModifierHooks> &hooks);
	void addAssetHooks(const Common::SharedPtr<AssetHooks> &hooks);
	void addSceneTransitionHooks(const Common::SharedPtr<SceneTransitionHooks> &hooks);
	void addSaveLoadHooks(const Common::SharedPtr<SaveLoadHooks> &hooks);

	bool ignoreMismatchedProjectNameInObjectLookups;
	uint midiVolumeScale;	// 256 = 1.0

	uint32 minTransitionDuration;

	Common::Point reportDisplaySize;	// If X or Y is non-zero, report this as the display size
	Common::Point mainWindowOffset;		// Coordinate offset of the main window

	Common::HashMap<uint32, Common::SharedPtr<StructuralHooks> > structuralHooks;
	Common::HashMap<uint32, Common::SharedPtr<ModifierHooks> > modifierHooks;
	Common::Array<Common::SharedPtr<SceneTransitionHooks> > sceneTransitionHooks;
	Common::Array<Common::SharedPtr<AssetHooks> > assetHooks;
	Common::Array<Common::SharedPtr<SaveLoadHooks> > saveLoadHooks;
};

namespace HackSuites {

void addObsidianBugFixes(const MTropolisGameDescription &desc, Hacks &hacks);
void addObsidianAutoSaves(const MTropolisGameDescription &desc, Hacks &hacks, IAutoSaveProvider *autoSaveProvider);
void addObsidianImprovedWidescreen(const MTropolisGameDescription &desc, Hacks &hacks);

} // End of namespace HackSuites

} // End of namespace MTropolis

#endif
