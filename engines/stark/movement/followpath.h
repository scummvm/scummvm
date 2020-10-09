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

#ifndef STARK_MOVEMENT_FOLLOW_PATH_H
#define STARK_MOVEMENT_FOLLOW_PATH_H

#include "engines/stark/movement/movement.h"

namespace Stark {

namespace Resources {
class Anim;
class Path;
}

/**
 * Make an item follow pre-computed path
 *
 * Works for 2D and 3D items, with respectively 2D and 3D paths
 */
class FollowPath : public Movement {
public:
	FollowPath(Resources::ItemVisual *item);
	virtual ~FollowPath();

	// Movement API
	void start() override;
	void onGameLoop() override;
	void stop(bool force = false) override;
	uint32 getType() const override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Set the path to follow */
	void setPath(Resources::Path *path);

	/** Set the movement speed on the path */
	void setSpeed(float speed);

	/** Override the animation to play while the item follows the path */
	void setAnim(Resources::Anim *anim);

private:
	void changeItemAnim();
	void updateItemPosition(uint currentEdge, float positionInEdge) const;
	bool is3D() const;

	Resources::Path *_path;
	float _speed;

	float _position;
	bool _previouslyEnabled;

	Resources::Anim *_anim;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_FOLLOW_PATH_H
