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

#ifndef STARK_MOVEMENT_FOLLOW_PATH_LIGHT_H
#define STARK_MOVEMENT_FOLLOW_PATH_LIGHT_H

#include "engines/stark/movement/movement.h"

namespace Stark {

namespace Resources {
class Light;
class Path;
}

/**
 * Make a light follow pre-computed path
 */
class FollowPathLight : public Movement {
public:
	FollowPathLight(Resources::ItemVisual *item);
	virtual ~FollowPathLight();

	// Movement API
	void start() override;
	void onGameLoop() override;
	void stop(bool force = false) override;
	uint32 getType() const override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Set the path to follow */
	void setPath(Resources::Path *path);

	/** Set the light to move */
	void setLight(Resources::Light *light);

	/** Set the movement speed on the path */
	void setSpeed(float speed);

private:
	Resources::Path *_path;
	Resources::Light *_light;
	float _speed;

	float _position;
	bool _previouslyEnabled;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_FOLLOW_PATH_LIGHT_H
