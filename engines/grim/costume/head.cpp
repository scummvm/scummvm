/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/model.h"
#include "engines/grim/grim.h"
#include "engines/grim/costume/head.h"

namespace Grim {

Head::Head() :
	_maxPitch(0),
	_joint1(-1), _joint2(-1), _joint3(-1),
	_joint1Node(NULL), _joint2Node(NULL), _joint3Node(NULL),
	_headYaw(0), _headPitch(0) {

}

void Head::setJoints(int joint1, int joint2, int joint3) {
	_joint1 = joint1;
	_joint2 = joint2;
	_joint3 = joint3;
}

void Head::loadJoints(ModelNode *nodes) {
	if (_joint1 >= 0 && _joint2 >= 0 && _joint3 >= 0 && nodes) {
		_joint1Node = nodes + _joint1;
		_joint2Node = nodes + _joint2;
		_joint3Node = nodes + _joint3;
	}
}

void Head::setMaxAngles(float maxPitch, float maxYaw, float maxRoll) {
	_maxRoll = maxRoll;
	_maxPitch = maxPitch;
	_maxYaw = maxYaw;
}

/** 
 * Generates a lookat matrix with position at origin. For reference, see 
 * http://clb.demon.fi/MathGeoLib/docs/float3x3_LookAt.php 
 */
Math::Matrix4 lookAtMatrix(const Math::Vector3d &localForward, const Math::Vector3d &targetDirection, 
						   const Math::Vector3d &localUp, const Math::Vector3d &worldUp)
{
	Math::Vector3d localRight = Math::Vector3d::crossProduct(localUp, localForward);
	localRight.normalize();
	Math::Vector3d worldRight = Math::Vector3d::crossProduct(worldUp, targetDirection);
	worldRight.normalize();
	Math::Vector3d perpWorldUp = Math::Vector3d::crossProduct(targetDirection, worldRight);
	perpWorldUp.normalize();
	
	Math::Matrix3 m1;
	m1.getRow(0) << worldRight.x() << worldRight.y() << worldRight.z();
	m1.getRow(1) << perpWorldUp.x() << perpWorldUp.y() << perpWorldUp.z();
	m1.getRow(2) << targetDirection.x() << targetDirection.y() << targetDirection.z();
	m1.transpose();
	
	Math::Matrix3 m2;
	m2.getRow(0) << localRight.x() << localRight.y() << localRight.z();
	m2.getRow(1) << localUp.x() << localUp.y() << localUp.z();
	m2.getRow(2) << localForward.x() << localForward.y() << localForward.z();
	
	Math::Matrix4 m3;
	m3.setToIdentity();
	m3.setRotation(m1 * m2);
	
	return m3;
}

	
void Head::lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) {
	if (_joint1Node) {
		float step = g_grim->getPerSecond(rate);
		float yawStep = step;
		float pitchStep = step / 3.f;
		float rollStep = step / 3.f;
		
		// Make sure we have up-to-date world transform matrices computed for every bone node of this character.
		ModelNode *p = _joint3Node;
		while (p->_parent) {
			p = p->_parent;
		}
		p->setMatrix(matrix);
		p->update();
			
		Math::Vector3d localFront; // Character front direction vector in local space.
		Math::Vector3d localUp; // Character up direction vector in local space.
		Math::Vector3d frontDir; // Character front facing direction vector in world space (global scene coordinate space)

		// the character head coordinate frame is: +Y forward, +Z up, +X right.
		frontDir = Math::Vector3d(_joint3Node->_matrix(0,1), _joint3Node->_matrix(1,1), _joint3Node->_matrix(2,1)); // Look straight ahead. (+Y)
		localFront = Math::Vector3d(0,1,0);
		localUp = Math::Vector3d(0,0,1);
				
		// v is the world space direction vector this character should be looking towards.
		Math::Vector3d v = point - _joint3Node->_pivotMatrix.getPosition();
		if (!entering)
			v = frontDir;
		if (v.isZero())
			return;

		v.normalize();
		
		// The vector v is in world space, so generate the world space lookat matrix for the desired head facing
		// orientation.
		Math::Matrix4 lookAtTM;
		const Math::Vector3d worldUp(0,0,1); // The Residual scene convention: +Z is world space up.
		if (Math::Vector3d::dotProduct(v, worldUp) >= 0.98f) // Avoid singularity if trying to look straight up.
			lookAtTM = lookAtMatrix(localFront, v, localUp, -frontDir); // Instead of orienting head towards scene up, orient head towards character "back",
		                                                                // i.e. when you look straight up, your head up vector tilts/arches to point straight backwards.
		else if (Math::Vector3d::dotProduct(v, worldUp) <= -0.98f) // Avoid singularity if trying to look straight down.
			lookAtTM = lookAtMatrix(localFront, v, localUp, frontDir); // Instead of orienting head towards scene down, orient head towards character "front",
																	   // i.e. when you look straight down, your head up vector tilts/arches to point straight forwards.
		else
			lookAtTM = lookAtMatrix(localFront, v, localUp, worldUp);
		// The above specifies the world space orientation of this bone, but we need to output
		// the orientation in parent space (as yaw/pitch/roll). 
		
		// Get the coordinate frame in which we need to produce the character head yaw/pitch/roll values.
		Math::Matrix4 parentWorldTM = _joint3Node->_parent->_matrix;
		
		// While we could compute the desired lookat direction directly in the above coordinate frame,
		// it is preferrable to compute the lookat direction with respect to the head orientation in
		// the keyframe animation. This is because the LUA scripts specify the maximum head yaw, pitch and
		// roll values with respect to those keyframe animations. If the lookat was simply computed 
		// directly in the space of the parent, we couldn't apply the head maxYaw/Pitch/Roll constraints 
		// properly. So, compute the coordinate frame of this bone in the keyframe animation.
		Math::Matrix4 animFrame;
		animFrame.buildFromPitchYawRoll(_joint3Node->_pitch, _joint3Node->_yaw, _joint3Node->_roll);
		animFrame.setPosition(Math::Vector3d(0, 0 ,0));
		parentWorldTM = parentWorldTM * animFrame;
		parentWorldTM.invertAffineOrthonormal();
		
		// Convert lookAtTM orientation from world space to parent-with-keyframe-animation space.
		lookAtTM = parentWorldTM * lookAtTM;
		
		// Decompose to yaw-pitch-roll (+Z, +X, +Y).
		// In this space, Yaw is +Z. Pitch is +X. Roll is +Y.
		Math::Angle y, pt, r;
		lookAtTM.getPitchYawRoll(&pt, &y, &r);
		
		// Constrain the maximum head movement, as desired by the game LUA scripts.
		y.clampDegrees(_maxYaw);
		pt.clampDegrees(_maxPitch);
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
		r.clampDegrees(30);
		//		r.clampDegrees(_head.maxRoll); // For original, use this.
		
		// Also limit yaw, pitch and roll to make at most a movement as large as the given max step size during this frame.
		// This will produce a slow head-turning animation instead of immediately snapping to the
		// target lookat orientation.
		if (y - _headYaw > yawStep)
			y = _headYaw + yawStep;
		if (_headYaw - y > yawStep)
			y = _headYaw - yawStep;
		
		if (pt - _headPitch > pitchStep)
			pt = _headPitch + pitchStep;
		if (_headPitch - pt > pitchStep)
			pt = _headPitch - pitchStep;
		
		if (r - _headRoll > rollStep)
			r = _headRoll + rollStep;
		if (_headRoll - r > rollStep)
			r = _headRoll - rollStep;
		
		// Remember how far we animated the head this frame, and we'll continue from here the next frame.
		_headPitch = pt;
		_headYaw = y;
		_headRoll = r;
		
		// Assemble ypr back to a matrix.
		// This matrix is the head orientation with respect to parent-with-keyframe-animation space.
		lookAtTM.buildFromPitchYawRoll(pt, y, r);
		
		// What follows is a hack: Since translateObject(ModelNode *node, bool reset) in this file,
		// and GfxOpenGL/GfxTinyGL::drawHierachyNode concatenate transforms incorrectly, by summing up
		// euler angles, do a hack here where we do the proper transform here already, and *subtract off*
		// the YPR scalars from the animYPR scalars to cancel out the values that those pieces of code 
		// will later accumulate. After those pieces of code have been fixed, the following lines can
		// be deleted, and this function can simply output the contents of pt, y and r variables above. 
		lookAtTM = animFrame * lookAtTM;
		
		lookAtTM.getPitchYawRoll(&pt, &y, &r);
		_joint3Node->_animYaw = y - _joint3Node->_yaw;
		_joint3Node->_animPitch = pt - _joint3Node->_pitch;
		_joint3Node->_animRoll = r - _joint3Node->_roll;
		
	}
}

void Head::saveState(SaveGame *state) const {
	state->writeLESint32(_joint1);
	state->writeLESint32(_joint2);
	state->writeLESint32(_joint3);
	state->writeFloat(_maxPitch);
	state->writeFloat(_maxYaw);
	state->writeFloat(_maxRoll);
	state->writeFloat(_headPitch.getDegrees());
	state->writeFloat(_headYaw.getDegrees());
}

void Head::restoreState(SaveGame *state) {
	_joint1 = state->readLESint32();
	_joint2 = state->readLESint32();
	_joint3 = state->readLESint32();
	_maxPitch = state->readFloat();
	_maxYaw = state->readFloat();
	_maxRoll = state->readFloat();

	_headPitch = state->readFloat();
	_headYaw = state->readFloat();
}

} // end of namespace Grim
