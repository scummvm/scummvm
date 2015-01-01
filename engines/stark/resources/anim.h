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

#ifndef STARK_RESOURCES_ANIM_H
#define STARK_RESOURCES_ANIM_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class XRCReadStream;

class Anim : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kAnim;

	enum SubType {
		kAnimSub1 = 1,
		kAnimSub2 = 2,
		kAnimSub3 = 3,
		kAnimSub4 = 4
	};

	/** Anim factory */
	static Resource *construct(Resource *parent, byte subType, uint16 index, const Common::String &name);

	Anim(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Anim();

	void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	uint32 _field_30;
	uint32 _numFrames;

};

class AnimSub1 : public Anim {
public:
	AnimSub1(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimSub1();

	virtual void readData(XRCReadStream *stream) override;

protected:
	void printData() override;

	float _field_3C;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_H
