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
 * Subtracts off extra multiples of the given angle in degrees, and returns 
 * the same angle represented in the [-180, 180] range.
 */
static Math::Angle to180Range(Math::Angle angle)
{
	float deg = angle.getDegrees(-180.f);
	angle.setDegrees(deg);
	return angle;
}
	
/**
 * Returns val clamped to range [-mag, mag]. 
 */
static Math::Angle clampMagnitude(Math::Angle val, float mag)
{
	val = to180Range(val);
	if (val.getDegrees() >= mag)
		return mag;
	if (val.getDegrees() <= -mag)
		return -mag;
	return val;
}
	
void setCol(Math::Matrix4 &m, int col, const Math::Vector3d &vec)
{
	m.setValue(0, col, vec.x());
	m.setValue(1, col, vec.y());
	m.setValue(2, col, vec.z());
	m.setValue(3, col, col == 3 ? 1.f : 0.f);
}

void setRow(Math::Matrix4 &m, int row, const Math::Vector3d &vec)
{
	m.setValue(row, 0, vec.x());
	m.setValue(row, 1, vec.y());
	m.setValue(row, 2, vec.z());
	m.setValue(row, 3, row == 3 ? 1.f : 0.f);
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
	
	Math::Matrix4 m1;
	setCol(m1, 0, worldRight);
	setCol(m1, 1, perpWorldUp);
	setCol(m1, 2, targetDirection);
	setCol(m1, 3, Math::Vector3d(0,0,0));
	
	Math::Matrix4 m2;
	setRow(m2, 0, localRight);
	setRow(m2, 1, localUp);
	setRow(m2, 2, localForward);
	setRow(m2, 3, Math::Vector3d(0,0,0));
	
	return m1 * m2;
}

/** 
 * Decomposes the matrix M to form M = R_z * R_x * R_y (R_D being the cardinal rotation 
 * matrix about the axis +D), and outputs the angles of rotation in parameters Z, X and Y.
 * In the convention of the coordinate system used in Grim Fandango characters:
 *	+Z is the yaw rotation (up axis)
 *	+X is the pitch rotation (right axis)
 *	+Y is the roll rotation (forward axis)
 * This function was adapted from http://www.geometrictools.com/Documentation/EulerAngles.pdf
 * The matrix M must be orthonormal. 
 */
void extractEulerZXY(const Math::Matrix4 &m, Math::Angle &Z, Math::Angle &X, Math::Angle &Y) {
	float x,y,z;
	if (m.getValue(2, 1) < 1.f) {
		if (m.getValue(2, 1) > -1.f) {
			x = asin(m.getValue(2, 1));
			z = atan2(-m.getValue(0, 1), m.getValue(1, 1));
			y = atan2(-m.getValue(2, 0), m.getValue(2, 2));
		}
		else {
			// Not a unique solution. Pick an arbitrary one.
			x = -3.141592654f/2.f;
			z = -atan2(-m.getValue(0, 2), m.getValue(0, 0));
			y = 0;
		}
	}
	else {
		// Not a unique solution. Pick an arbitrary one.
		x = 3.141592654f/2.f;
		z = atan2(m.getValue(0, 2), m.getValue(0, 0));
		y = 0;
	}
	X = Math::Angle::fromRadians(x);
	Y = Math::Angle::fromRadians(y);
	Z = Math::Angle::fromRadians(z);
}

/** 
 * Inverts a matrix in place.
 *	This function avoid having to do generic Gaussian elimination on the matrix
 *	by assuming that the top-left 3x3 part of the matrix is orthonormal
 *	(columns and rows 0, 1 and 2 orthogonal and unit length).
 *	See e.g. Eric Lengyel's Mathematics for 3D Game Programming and Computer Graphics, p. 82. 
 */
void invertAffineOrthonormal(Math::Matrix4 &m) {
	Math::Matrix4 m2;
	m2.setValue(0, 0, m.getValue(0, 0));
	m2.setValue(0, 1, m.getValue(1, 0));
	m2.setValue(0, 2, m.getValue(2, 0));
	m2.setValue(1, 0, m.getValue(0, 1));
	m2.setValue(1, 1, m.getValue(1, 1));
	m2.setValue(1, 2, m.getValue(2, 1));
	m2.setValue(2, 0, m.getValue(0, 2));
	m2.setValue(2, 1, m.getValue(1, 2));
	m2.setValue(2, 2, m.getValue(2, 2));
	m2.setValue(3, 0, 0.f);
	m2.setValue(3, 1, 0.f);
	m2.setValue(3, 2, 0.f);
	m2.setValue(3, 3, 1.f);
	m2.setPosition(Math::Vector3d(0,0,0));
	
	Math::Matrix<4,1> v;
	v.setValue(0, -m.getValue(0, 3)); 
	v.setValue(1, -m.getValue(1, 3)); 
	v.setValue(2, -m.getValue(2, 3)); 
	v.setValue(3, 0.f); 
	
	m2.transformVector(&v);
	m2.setPosition(Math::Vector3d(v.getData()[0],v.getData()[1],v.getData()[2]));
	m = m2;
}
	
/** 
 * Returns the scalar 'val' animated towards zero, at most by the given maximum step.
 * If val is nearer to zero than maxStep, 0 is returned.
 * Note: The angle is in degrees, but assuming here that it is nicely in the range [-180, 180],
 * or otherwise the shortest route to zero angle wouldn't be like animated here. 
 */
static Math::Angle moveTowardsZero(Math::Angle val, float maxStep)
{
	if (val > maxStep)
		return val - maxStep;
	if (val < -maxStep)
		return val + maxStep;
	return 0;
}

void Head::lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) {
	if (_joint1Node) {
		float step = g_grim->getPerSecond(rate);
		float yawStep = step;
		float pitchStep = step / 3.f;
		if (!entering) {

			// The character isn't looking at a target.
			// Animate _headYaw and _headPitch slowly towards zero
			// so that the character will turn to look straight ahead.
			_headYaw = moveTowardsZero(_headYaw, yawStep);
			_headPitch = moveTowardsZero(_headPitch, pitchStep);

			_joint1Node->_animYaw = _headYaw;
			Math::Angle pi = _headPitch / 3.f;
			_joint1Node->_animPitch += pi;
			_joint2Node->_animPitch += pi;
			_joint3Node->_animPitch += pi;
			_joint1Node->_animRoll = (_joint1Node->_animYaw.getDegrees() / 20.f) * _headPitch.getDegrees() / -5.f;
			_joint1Node->_animRoll = clampMagnitude(_joint1Node->_animRoll, _maxRoll);
			return;
		}
		
		// Make sure we have up-to-date world transform matrices computed for every bone node of this character.
		ModelNode *p = _joint3Node;
		while (p->_parent) {
			p = p->_parent;
		}
		p->setMatrix(matrix);
		p->update();

		// v is the world space direction vector this character should be looking towards.
		Math::Vector3d v =  point - _joint1Node->_matrix.getPosition();
		if (v.isZero()) {
			return;
		}

		v.normalize();
		
		// The vector v is in world space, so generate the world space lookat matrix for the desired head facing
		// orientation.
		Math::Matrix4 lookAtTM = lookAtMatrix(Math::Vector3d(0,1,0), v, Math::Vector3d(0,0,1), Math::Vector3d(0,0,1));
		// The above specifies the world space orientation of this bone, but we need to output
		// the orientation in parent space (as yaw/pitch/roll). 
		
		// Get the coordinate frame in which we need to produce the character head yaw/pitch/roll values.
		Math::Matrix4 parentWorldTM = _joint1Node->_parent->_matrix;
		
		// While we could compute the desired lookat direction directly in the above coordinate frame,
		// it is preferrable to compute the lookat direction with respect to the head orientation in
		// the keyframe animation. This is because the LUA scripts specify the maximum head yaw, pitch and
		// roll values with respect to those keyframe animations. If the lookat was simply computed 
		// directly in the space of the parent, we couldn't apply the head maxYaw/Pitch/Roll constraints 
		// properly. So, compute the coordinate frame of this bone in the keyframe animation.
		Math::Matrix4 animFrame;
		animFrame.buildFromPitchYawRoll(_joint1Node->_pitch, _joint1Node->_yaw, _joint1Node->_roll);
		animFrame.setPosition(Math::Vector3d(0,0,0)); // NOTE: Could take _joint1Node->_pos into account here, but the 
		                                              // vector v would need to be fixed accordingly above.
		parentWorldTM = parentWorldTM * animFrame;
		invertAffineOrthonormal(parentWorldTM);
		
		// Convert lookAtTM orientation from world space to parent-with-keyframe-animation space.
		lookAtTM = parentWorldTM * lookAtTM;
		
		// Decompose to yaw-pitch-roll (+Z, +X, +Y).
		// In this space, Yaw is +Z. Pitch is +X. Roll is +Y.
		Math::Angle y, pt, r;
		extractEulerZXY(lookAtTM, y, pt, r);
		
		// Constrain the maximum head movement, as desired by the game LUA scripts.
		y = clampMagnitude(y, _maxYaw);
		pt = clampMagnitude(pt, _maxPitch);
		r = clampMagnitude(r, _maxRoll);
		
		// Also limit yaw and pitch to make at most a movement as large as yawStep/pitchStep during this frame.
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
		
		// Remember how far we animated the head this frame, and we'll continue from here the next frame.
		_headPitch = pt;
		_headYaw = y;
		
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
		
		extractEulerZXY(lookAtTM, y, pt, r);
		_joint1Node->_animYaw = y;
		_joint1Node->_animPitch = pt;
		_joint1Node->_animRoll = r;
		
		// hack hack:
		_joint1Node->_animYaw -= _joint1Node->_yaw;
		_joint1Node->_animPitch -= _joint1Node->_pitch;
		_joint1Node->_animRoll -= _joint1Node->_roll;

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
