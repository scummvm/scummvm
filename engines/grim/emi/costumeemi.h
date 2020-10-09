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

#ifndef GRIM_COSTUMEEMI_H
#define GRIM_COSTUMEEMI_H

#include "common/stream.h"

#include "engines/grim/object.h"
#include "engines/grim/costume.h"
#include "engines/grim/emi/costume/emichore.h"

namespace Grim {

typedef uint32 tag32;

class EMISkelComponent;
class EMIMeshComponent;
class Material;
class EMIModel;

class EMICostume : public Costume {
public:
	EMICostume(const Common::String &filename, Actor *owner, Costume *prevCost);

	void load(Common::SeekableReadStream *data) override;

	void draw() override;
	int update(uint time) override;

	void playChore(int num, uint msecs = 0) override;
	void playChoreLooping(int num, uint msecs = 0) override;

	void saveState(SaveGame *state) const override;
	bool restoreState(SaveGame *state) override;

	Material *loadMaterial(const Common::String &name, bool clamp);
	Material *findMaterial(const Common::String &name);

	void setHead(const char *joint, const Math::Vector3d &offset);
	void setHeadLimits(float yawRange, float maxPitch, float minPitch);

	EMIModel *getEMIModel() const;
	EMIModel *getEMIModel(int num) const;
public:
	EMIChore *_wearChore;
	EMISkelComponent *_emiSkel;
private:
	Common::List<ObjectPtr<Material> > _materials;
	static bool compareChores(const Chore *c1, const Chore *c2);
	Component *loadEMIComponent(Component *parent, int parentID, const char *name, Component *prevComponent);
	void setWearChore(EMIChore *chore);

	friend class Chore;
};

} // end of namespace Grim

#endif
