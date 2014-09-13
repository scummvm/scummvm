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

#include "engines/grim/model.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/costume/head.h"

namespace Grim {

Head::Joint::Joint() :
		_node(nullptr), _pitch(0.f), _yaw(0.f), _roll(0.f) {
}

void Head::Joint::init(ModelNode *node) {
	_node = node;
}

void Head::Joint::orientTowards(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix,
								float maxPitch, float maxYaw, float maxRoll, float constrain) {
	float step = g_grim->getPerSecond(rate);
	float yawStep = step;
	float pitchStep = step / 3.0f;
	float rollStep = step / 3.0f;

	if (!_node)
		return;

	// Make sure we have up-to-date world transform matrices computed for the joint nodes of this character.
	_node->_needsUpdate = true;
	ModelNode *p = _node;
	while (p->_parent) {
		p = p->_parent;
		p->_needsUpdate = true;
	}
	p->setMatrix(matrix);
	p->update();

	Math::Vector3d modelFront; // the modeling convention for the forward direction.
	Math::Vector3d modelUp; // the modeling convention for the upward direction.
	Math::Vector3d frontDir; // Character front facing direction vector in world space (global scene coordinate space)

	// the character head coordinate frame is: +Y forward, +Z up, +X right.
	frontDir = Math::Vector3d(_node->_matrix(0, 1), _node->_matrix(1, 1), _node->_matrix(2, 1)); // Look straight ahead. (+Y)
	modelFront = Math::Vector3d(0, 1, 0);
	modelUp = Math::Vector3d(0, 0, 1);

	// v is the world space direction vector this character should be looking towards.
	Math::Vector3d targetDir = point - _node->_pivotMatrix.getPosition();
	if (!entering)
		targetDir = frontDir;
	if (targetDir.isZero())
		return;

	targetDir.normalize();

	// The vector v is in world space, so generate the world space lookat matrix for the desired head facing
	// orientation.
	Math::Matrix4 lookAtTM;
	lookAtTM.setToIdentity();
	const Math::Vector3d worldUp(0, 0, 1); // The Residual scene convention: +Z is world space up.
	if (Math::Vector3d::dotProduct(targetDir, worldUp) >= 0.98f) // Avoid singularity if trying to look straight up.
		lookAtTM.buildFromTargetDir(modelFront, targetDir, modelUp, -frontDir); // Instead of orienting head towards scene up, orient head towards character "back",
	                                                                // i.e. when you look straight up, your head up vector tilts/arches to point straight backwards.
	else if (Math::Vector3d::dotProduct(targetDir, worldUp) <= -0.98f) // Avoid singularity if trying to look straight down.
		lookAtTM.buildFromTargetDir(modelFront, targetDir, modelUp, frontDir); // Instead of orienting head towards scene down, orient head towards character "front",
																   // i.e. when you look straight down, your head up vector tilts/arches to point straight forwards.
	else
		lookAtTM.buildFromTargetDir(modelFront, targetDir, modelUp, worldUp);
	// The above specifies the world space orientation of this bone, but we need to output
	// the orientation in parent space (as yaw/pitch/roll).

	// Get the coordinate frame in which we need to produce the character head yaw/pitch/roll values.
	Math::Matrix4 parentWorldTM;
	if (_node->_parent)
		parentWorldTM = _node->_parent->_matrix;

	// While we could compute the desired lookat direction directly in the above coordinate frame,
	// it is preferrable to compute the lookat direction with respect to the head orientation in
	// the keyframe animation. This is because the LUA scripts specify the maximum head yaw, pitch and
	// roll values with respect to those keyframe animations. If the lookat was simply computed
	// directly in the space of the parent, we couldn't apply the head maxYaw/Pitch/Roll constraints
	// properly. So, compute the coordinate frame of this bone in the keyframe animation.
	Math::Matrix4 animFrame = _node->_localMatrix;
	parentWorldTM = parentWorldTM * animFrame;
	parentWorldTM.invertAffineOrthonormal();

	// Convert lookAtTM orientation from world space to parent-with-keyframe-animation space.
	lookAtTM = parentWorldTM * lookAtTM;

	// Decompose to yaw-pitch-roll (+Z, +X, +Y).
	// In this space, Yaw is +Z. Pitch is +X. Roll is +Y.
	Math::Angle y, pt, r;
	lookAtTM.getEuler(&y, &pt, &r, Math::EO_ZXY);

	y = y * constrain;
	pt = pt * constrain;
	r = r * constrain;

	// Constrain the maximum head movement, as desired by the game LUA scripts.
	y.clampDegrees(maxYaw);
	pt.clampDegrees(maxPitch);
	r.clampDegrees(maxRoll);

	// Also limit yaw, pitch and roll to make at most a movement as large as the given max step size during this frame.
	// This will produce a slow head-turning animation instead of immediately snapping to the
	// target lookat orientation.
	if (y - _yaw > yawStep)
		y = _yaw + yawStep;
	if (_yaw - y > yawStep)
		y = _yaw - yawStep;

	if (pt - _pitch > pitchStep)
		pt = _pitch + pitchStep;
	if (_pitch - pt > pitchStep)
		pt = _pitch - pitchStep;

	if (r - _roll > rollStep)
		r = _roll + rollStep;
	if (_roll - r > rollStep)
		r = _roll - rollStep;

	// Remember how far we animated the head this frame, and we'll continue from here the next frame.
	_pitch = pt;
	_yaw = y;
	_roll = r;

	// Assemble ypr to a quaternion.
	// This is the head orientation with respect to parent-with-keyframe-animation space.
	Math::Quaternion lookAtQuat = Math::Quaternion::fromEuler(y, pt, r, Math::EO_ZXY);

	_node->_animRot = _node->_animRot * lookAtQuat;
}

void Head::Joint::saveState(SaveGame *state) const {
	state->writeFloat(_pitch.getDegrees());
	state->writeFloat(_yaw.getDegrees());
	state->writeFloat(_roll.getDegrees());
}

void Head::Joint::restoreState(SaveGame *state) {
	_pitch = state->readFloat();
	_yaw = state->readFloat();
	_roll = state->readFloat();
}

Head::Head() :
	_maxPitch(0), _maxYaw(0), _maxRoll(0),
	_joint1Node(-1), _joint2Node(-1), _joint3Node(-1) {

}

void Head::setJoints(int joint1, int joint2, int joint3) {
	_joint1Node = joint1;
	_joint2Node = joint2;
	_joint3Node = joint3;
}

void Head::loadJoints(ModelNode *nodes) {
	if (_joint1Node >= 0 && _joint2Node >= 0 && _joint3Node >= 0 && nodes) {
		_joint1.init(nodes + _joint1Node);
		_joint2.init(nodes + _joint2Node);
		_joint3.init(nodes + _joint3Node);
	}
}

void Head::setMaxAngles(float maxPitch, float maxYaw, float maxRoll) {
	_maxRoll = maxRoll;
	_maxPitch = maxPitch;
	_maxYaw = maxYaw;
}

void Head::lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) {
	if (_joint1Node != -1) {
		// NOTE: By default, the _head.maxRoll for Manny's head is constrained to 165 degrees, which
		// comes in from the orignal Lua data scripts. (also, maxYaw == 80, maxPitch == 28).
		// The very small maxPitch angle, and a very large maxRoll angle causes problems when Manny
		// is trying to look straight up to an object, in which case the euler roll angles vary
		// wildly compared to the pitch angles, which get clamped to a much smaller interval. Therefore,
		// restrict the maximum roll angle to a smaller value than 165 degrees to avoid this behavior.
		// If you want to change this, good places to test are:
		// A) Year 1, outside the Department of Death, run/walk up & down the stairs, there's a sign
		//    right above the stairs, and Manny looks dead up.
		// B) Year 3, when Manny and Meche are imprisoned in the vault. Walk inside the room where Meche
		//    is in, to look straight up to the sprinklers.

		if (_joint1Node == _joint2Node && _joint1Node == _joint3Node) {
			// Most characters only have one head joint instead of three, so we can orient the head
			// with a single call.
			_joint3.orientTowards(entering, point, rate, matrix, _maxPitch, _maxYaw, 30.f, 1.0f);
		} else {
			// For characters like Manny, we'll have to orient each of the three head joints.
			_joint1.orientTowards(entering, point, rate / 3, matrix, _maxPitch / 3, _maxYaw / 3, 10.f, 0.333f);
			_joint2.orientTowards(entering, point, rate / 3, matrix, _maxPitch / 3, _maxYaw / 3, 10.f, 0.666f);
			_joint3.orientTowards(entering, point, rate / 3, matrix, _maxPitch / 3, _maxYaw / 3, 10.f, 1.000f);
		}
	}
}

void Head::saveState(SaveGame *state) const {
	state->writeLESint32(_joint1Node);
	state->writeLESint32(_joint2Node);
	state->writeLESint32(_joint3Node);
	state->writeFloat(_maxPitch);
	state->writeFloat(_maxYaw);
	state->writeFloat(_maxRoll);

	_joint1.saveState(state);
	_joint2.saveState(state);
	_joint3.saveState(state);
}

void Head::restoreState(SaveGame *state) {
	_joint1Node = state->readLESint32();
	_joint2Node = state->readLESint32();
	_joint3Node = state->readLESint32();
	_maxPitch = state->readFloat();
	_maxYaw = state->readFloat();
	_maxRoll = state->readFloat();

	if (state->saveMinorVersion() < 2) {
		state->readFloat();
		state->readFloat();
	} else {
		_joint1.restoreState(state);
		_joint2.restoreState(state);
		_joint3.restoreState(state);
	}
}

} // end of namespace Grim
