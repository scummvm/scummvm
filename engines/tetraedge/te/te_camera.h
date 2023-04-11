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
#include "math/ray.h"

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

	Math::Ray getRay(const TeVector2s32 &pxloc);

	// Syberia 2 redefines loadBin to actually load XML.
	// We just have a separate function.
	void loadXml(const Common::Path &path);

	// Unused in Syberia 1.
	//void loadBin(const Common::Path &path);
	//void loadBin(const Common::ReadStream &stream);

	//void lookAt(const TeVector3f32 &point) {} // empty and unused?

	void orthogonalParams(float f1, float f2, float f3, float f4);
	TeMatrix4x4 projectionMatrix();

	TeVector2f32 projectPoint(const TeVector3f32 &pt);
	TeVector3f32 projectPoint3f32(const TeVector3f32 &pt);

	static void restore();
	TeMatrix4x4 transformationMatrix();
	TeVector3f32 transformCoord(const TeVector3f32 &pt);
	TeVector3f32 transformPoint2Dto3D(const TeVector3f32 &pt);

	void viewport(int x, int y, uint width, uint height);
	TeVector2f32 viewportSize() const { return TeVector2f32(_viewportW, _viewportH); }

	TeSignal0Param &onViewportChangedSignal() { return _onViewportChangedSignal; }

	void setFov(float fov) { _fov = fov; }
	void setOrthoPlanes(float near, float far) {
		_orthFarVal = far;
		_orthNearVal = near;
	}
	void setProjMatrixType(int matrixType) { _projectionMatrixType = matrixType; }
	int projMatrixType() const { return _projectionMatrixType; }
	void setAspectRatio(float val) { _aspectRatio = val; }
	float orthoNearPlane() const { return _orthNearVal; }
	float orthoFarPlane() const { return _orthFarVal; }
	void setOrthoNear(float f) { _orthNearVal = f; }
	void setOrthoFar(float f) { _orthFarVal = f; }
	float getViewportHeight() const { return _viewportH; }
	float getViewportWidth() const { return _viewportW; }

private:
	void updateProjectionMatrix();

	int _projectionMatrixType; // TODO: Should be an enum.
	float _orthNearVal;
	float _orthFarVal;
	float _fov;
	float _aspectRatio;

	int _viewportX;
	int _viewportY;
	uint _viewportW;
	uint _viewportH;

	int _transformA;
	//int _transformB; // never used?

	float _orthogonalParamL;
	float _orthogonalParamR;
	float _orthogonalParamT;
	float _orthogonalParamB;

	TeMatrix4x4 _projectionMatrix;

	TeSignal0Param _onViewportChangedSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CAMERA_H
