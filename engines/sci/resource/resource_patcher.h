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

#ifndef SCI_RESOURCE_RESOURCE_PATCHER_H
#define SCI_RESOURCE_RESOURCE_PATCHER_H

#include "common/language.h"
#include "sci/sci.h"
#include "sci/resource/resource.h"
#include "sci/resource/resource_intern.h"

namespace Sci {

enum ResourcePatchOp {
	// Using high bytes to make it less likely that accidental raw data will be
	// treated like an operator instead of an error
	kSkipBytes = 0xF0,
	kReplaceBytes,
	kInsertBytes,
	kReplaceNumber,
	kAdjustNumber,
	kInsertNumber,
	kReplaceFill,
	kInsertFill,
	kEndOfPatch
};

struct GameResourcePatch {
	/**
	 * The game to patch.
	 */
	SciGameId gameId;

	/**
	 * The language to patch. Use `Common::UNK_LANG` to apply the patch to all
	 * languages.
	 */
	Common::Language gameLanguage;

	/**
	 * The resource ID to patch.
	 */
	ResourceId resourceId;

	/**
	 * Patch instructions to apply to the resource.
	 */
	const byte *patchData;

	/**
	 * Set to true if the patch resource is actually a new resource, rather than
	 * a patch for an existing resource.
	 */
	bool isNewResource;
};

/**
 * A basic class for generating patched resource data at runtime.
 */
class ResourcePatcher : public ResourceSource {
public:
	ResourcePatcher(const SciGameId gameId, const Common::Language gameLanguage);

	~ResourcePatcher() override {}

	/**
	 * Finds and applies a patch to the given resource.
	 *
	 * @returns true if a patch was applied.
	 */
	bool applyPatch(Resource &resource) const;

	/**
	 * Adds new resources from the patch table to the resource manager. This
	 * is needed since otherwise tests for these resources via `kResCheck`
	 * would fail, and so they would never actually be loaded.
	 */
	void scanSource(ResourceManager *resMan) override;

	/**
	 * Load a resource. Since resources using this source are patched explicitly
	 * after they get loaded by any other resource source, this method does
	 * nothing.
	 */
	void loadResource(ResourceManager *resMan, Resource *res) override {}

private:
	struct PatchSizes {
		/**
		 * The minimum number of bytes required in the source data.
		 */
		uint32 expected;

		/**
		 * The difference in size between the original data and the patched
		 * data.
		 */
		int32 delta;

		PatchSizes(uint32 exp, int32 d) {
			expected = exp;
			delta = d;
		}
	};

	typedef Common::Array<GameResourcePatch> PatchList;

	/**
	 * The list of patches that should apply to the currently loaded game.
	 */
	PatchList _patches;

	/**
	 * Patches the given Resource using patch data from the given
	 * GameResourcePatch.
	 */
	void patchResource(Resource &resource, const GameResourcePatch &patch) const;

	/**
	 * Calculates expected and extra data sizes from the patch data.
	 */
	PatchSizes calculatePatchSizes(const byte *patchData) const;

	/**
	 * Reads an block size from the patch data, validates it, and advances the
	 * patch data pointer.
	 */
	int32 readBlockSize(const byte * &patchData) const;
};
} // End of namespace Sci

#endif	// SCI_RESOURCE_RESOURCE_PATCHER_H
