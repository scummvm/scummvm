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

#ifndef GRIM_ANIMATION_H
#define GRIM_ANIMATION_H

#include "engines/grim/keyframe.h"

namespace Grim {

class SaveGame;
class AnimManager;

class Animation {
public:
	enum RepeatMode {
		Once = 0,
		Looping = 1,
		PauseAtEnd = 2,
		FadeAtEnd = 3
	};
	enum FadeMode {
		None = 0,
		FadeIn = 1,
		FadeOut = 2
	};

	Animation(const Common::String &keyframe, AnimManager *manager, int pr1, int pr2);
	~Animation();
	void activate();
	void deactivate();
	void play(RepeatMode repeatMode);
	void fade(FadeMode fadeMode, int fadeLength);
	void pause(bool pause);
	void stop();
	bool getIsActive() const;
	FadeMode getFadeMode() const;

	int update(uint time);

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

private:
	AnimManager *_manager;
	ObjectPtr<KeyframeAnim> _keyframe;
	int _priority1;
	int _priority2;
	bool _paused;
	bool _active;
	int _time;
	float _fade;
	RepeatMode _repeatMode;
	FadeMode _fadeMode;
	int _fadeLength;

	friend class AnimManager;
};

class AnimManager {
public:
	AnimManager();
	~AnimManager();
	void addAnimation(Animation *anim, int pr1, int pr2);
	void removeAnimation(const Animation *anim);

	void animate(ModelNode *hier, int numNodes);

private:
	struct AnimationEntry {
		Animation *_anim;
		int _priority;
		bool _tagged;
	};

	Common::List<AnimationEntry> _activeAnims;
};

}

#endif
