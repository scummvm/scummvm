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

#ifndef STARK_MODEL_ANIM_HANDLER_H
#define STARK_MODEL_ANIM_HANDLER_H

#include "common/scummsys.h"

namespace Stark {

class Model;
class BoneNode;
class SkeletonAnim;

/**
 * Animate a skeletal model's bones according to an animation
 */
class AnimHandler {
public:
	AnimHandler();
	~AnimHandler();

	/**
	 * Increment the animation timestamp, and apply bone animations if required
	 */
	void animate(uint32 time);

	/** Set the skeletal model to animate */
	void setModel(Model *model);

	/** Set the skeletal animation to use */
	void setAnim(SkeletonAnim *anim);

	/** Stop blending and forget about the previous animation */
	void resetBlending();

private:
	void enactCandidate();
	void startBlending();
	void updateBlending(int32 deltaTime);
	void stopBlending();

	void setNode(uint32 time, BoneNode *bone, const BoneNode *parent);

	static const uint32 _blendDuration = 300; // ms

	SkeletonAnim *_anim;
	int32 _animTime;

	int32 _framesBeforeCandidateReady;
	SkeletonAnim *_candidateAnim;
	int32 _candidateAnimTime;

	SkeletonAnim *_blendAnim;
	int32 _blendAnimTime;
	int32 _blendTimeRemaining;

	Model *_model;
};

} // End of namespace Stark

#endif // STARK_MODEL_ANIM_HANDLER_H
