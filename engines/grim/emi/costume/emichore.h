/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_EMICHORE_H
#define GRIM_EMICHORE_H

#include "engines/grim/costume/chore.h"
#include "engines/grim/pool.h"
#include "engines/grim/emi/costume/emimesh_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"

namespace Grim {

class EMIChore : public PoolObject<EMIChore>, public Chore {
public:
	EMIChore(char name[32], int id, Costume *owner, int length, int numTracks);
	static int32 getStaticTag() { return MKTAG('C', 'H', 'O', 'R'); }

	void update(uint msecs) override;
	void stop(uint msecs) override;
	void addComponent(Component *component);
	bool isWearChore() { return _mesh && _skeleton; }
	void saveState(SaveGame *state) const override;
	void restoreState(SaveGame *state) override;
	EMIMeshComponent *getMesh() { return _mesh; }
	EMISkelComponent *getSkeleton() { return _skeleton; }

private:
	void fade(Animation::FadeMode mode, uint msecs) override;

	Animation::FadeMode _fadeMode;
	float _fade;
	float _startFade;
	int _fadeLength;

	EMIMeshComponent *_mesh;
	EMISkelComponent *_skeleton;
};

} // end of namespace Grim

#endif
