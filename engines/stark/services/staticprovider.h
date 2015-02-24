/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_STATIC_PROVIDER_H
#define STARK_STATIC_PROVIDER_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Stark {

namespace Resources {
class Anim;
class Level;
}

class ArchiveLoader;
class Global;
class VisualImageXMG;

/**
 * Static Resource provider.
 *
 * Maintains the static resource trees.
 * Maintained trees are the level and the location ones.
 */
class StaticProvider {
public:
	StaticProvider(ArchiveLoader *archiveLoader, Global *global);

	/** Load the static level archive */
	void init();

	/** Release the static resources */
	void shutdown();

	/** Obtain the static level */
	Resources::Level *getLevel() const { return _level; }

	VisualImageXMG *getCursorImage(uint32 stockAnim);

	enum StaticItems {
		kQuit = 8,
		kDiaryNormal = 15,
		kInventory = 16,
		kTextBackgroundActive = 20,
		kTextBackgroundPassive = 21
	};

	// TODO: This is awaiting subItem2-too.
	Resources::Anim *getUIItem(StaticItems stockAnim);

private:
	ArchiveLoader *_archiveLoader;
	Global *_global;

	Resources::Level *_level;
	Common::Array<Resources::Anim *> _stockAnims;
};

} // End of namespace Stark

#endif // STARK_STATIC_PROVIDER_H
