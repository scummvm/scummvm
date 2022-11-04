/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_TE_TE_CAMERA_H
#define TETRAEDGE_TE_TE_CAMERA_H

#include "common/str.h"
#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_references_counter.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeCamera : public Te3DObject2, public TeReferencesCounter {
public:
	TeCamera();

	void apply();
	void applyProjection();
	void applyTransformations();
	void buildOrthoMatrix();
	void buildPerspectiveMatrix();
	void buildPerspectiveMatrix2();
	void buildPerspectiveMatrix3();
	void draw() override;

	void getRay(const TeVector2s32 &param_1, TeVector3f32 &out1, TeVector3f32 &out2);

	void loadBin(const Common::String &path);
	void loadBin(const Common::ReadStream &stream);

	//void lookAt(const TeVector3f32 &point) {} // empty and unused?

	void orthogonalParams(float f1, float f2, float f3, float f4);
	TeMatrix4x4 projectionMatrix();

	TeVector3f32 projectPoint(const TeVector3f32 &pt);
	TeVector3f32 projectPoint3f32(const TeVector3f32 &pt);

	static void restore();
	TeMatrix4x4 transformationMatrix();
	TeVector3f32 transformCoord(const TeVector3f32 &pt);
	TeVector3f32 transformPoint2Dto3D(const TeVector2f32 &pt);
	void updateProjectionMatrix();

	void viewport(int x, int y, uint width, uint height);
	TeVector2f32 viewportSize() const { return TeVector2f32(_viewportW, _viewportH); }

	TeSignal0Param &onViewportChangedSignal() { return _onViewportChangedSignal; }

	int _projectionMatrixType;
	float _orthNearVal;
	float _orthFarVal;
	float _fov;
	float _somePerspectiveVal;

private:
	int _viewportX;
	int _viewportY;
	uint _viewportW;
	uint _viewportH;

	int _transformA;
	int _transformB;

	float _orthogonalParamL;
	float _orthogonalParamR;
	float _orthogonalParamT;
	float _orthogonalParamB;

	TeMatrix4x4 _projectionMatrix;

	TeSignal0Param _onViewportChangedSignal;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CAMERA_H
