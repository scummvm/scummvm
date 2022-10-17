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

#include "common/math.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

TeCamera::TeCamera() : _projectionMatrixType(0), _orthogonalParamL(1.0f),
	_orthogonalParamR(0.0f), _orthogonalParamT(1.0f), _orthogonalParamB(0.0f),
	_orthNearVal(10.0f), _orthFarVal(4000.0f), _transformA(0), _transformB(0),
	_focalLenMaybe(40.0f), _somePerspectiveVal(1.0f)
{
}

void TeCamera::applyProjection() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setCurrentCamera(this);
	renderer->setViewport(_viewportX, _viewportY,
						  (uint)(_viewportW * _scale.x()), (uint)(_viewportH * _scale.y()));
	renderer->setMatrixMode(TeRenderer::MatrixMode::MM_GL_PROJECTION);
	updateProjectionMatrix();
	renderer->setMatrixMode(TeRenderer::MatrixMode::MM_GL_PROJECTION);
	renderer->loadCurrentMatrixToGL();
	renderer->setMatrixMode(TeRenderer::MatrixMode::MM_GL_MODELVIEW);
}

void TeCamera::applyTransformations() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setMatrixMode(TeRenderer::MatrixMode::MM_GL_MODELVIEW);
	TeMatrix4x4 matrix = transformationMatrix();
	matrix.inverse();
	renderer->loadMatrix(matrix);
	renderer->loadCurrentMatrixToGL();
}

void TeCamera::buildOrthoMatrix() {
	float widthNorm = FLT_MAX;
	if ((_orthogonalParamR - _orthogonalParamL) != 0.0) {
	  widthNorm = 1.0 / (_orthogonalParamR - _orthogonalParamL);
	}
	float heightNorm = FLT_MAX;
	if (_orthogonalParamB - _orthogonalParamT != 0.0) {
	  heightNorm = 1.0 / (_orthogonalParamB - _orthogonalParamT);
	}
	float depthNorm = FLT_MAX;
	if ((_orthFarVal - _orthNearVal) != 0.0) {
	  depthNorm = 1.0 / (_orthFarVal - _orthNearVal);
	}

	_projectionMatrix.setValue(0, 0, widthNorm * 2.0f);
	_projectionMatrix.setValue(0, 1, 0.0);
	_projectionMatrix.setValue(0, 2, 0.0);
	_projectionMatrix.setValue(0, 3, -((_orthogonalParamR + _orthogonalParamL) * widthNorm));

	_projectionMatrix.setValue(1, 0, 0.0);
	_projectionMatrix.setValue(1, 1, heightNorm * 2.0f);
	_projectionMatrix.setValue(1, 2, 0.0);
	_projectionMatrix.setValue(1, 3, -((_orthogonalParamB + _orthogonalParamT) * heightNorm));

	_projectionMatrix.setValue(2, 0, 0.0);
	_projectionMatrix.setValue(2, 1, 0.0);
	_projectionMatrix.setValue(2, 2, depthNorm * -2.0f);
	_projectionMatrix.setValue(2, 3, -((_orthFarVal + _orthNearVal) * depthNorm));

	_projectionMatrix.setValue(3, 0, 0.0);
	_projectionMatrix.setValue(3, 1, 0.0);
	_projectionMatrix.setValue(3, 2, 0.0);
	_projectionMatrix.setValue(3, 3, 1.0);
}

void TeCamera::buildPerspectiveMatrix() {
	error("TODO: Implement TeCamera::buildPerspectiveMatrix");
}

void TeCamera::buildPerspectiveMatrix2() {
	error("TODO: Implement TeCamera::buildPerspectiveMatrix2");
}

void TeCamera::buildPerspectiveMatrix3() {
	error("TODO: Implement TeCamera::buildPerspectiveMatrix3");
}

void TeCamera::draw() {
	error("TODO: Implement TeCamera::draw");
}

void TeCamera::getRay(const TeVector2s32 &param_1, TeVector3f32 &out1, TeVector3f32 &out2) {
	error("TODO: Implement TeCamera::getRay");
}

void TeCamera::loadBin(const Common::String &path) {
	error("TODO: Implement TeCamera::loadBin");
}

void TeCamera::loadBin(const Common::ReadStream &stream) {
	error("TODO: Implement TeCamera::loadBin");
}

void TeCamera::orthogonalParams(float left, float right, float top, float bottom) {
	_orthogonalParamL = left;
	_orthogonalParamR = right;
	_orthogonalParamT = top;
	_orthogonalParamB = bottom;
}

TeMatrix4x4 TeCamera::projectionMatrix() {
	switch(_projectionMatrixType) {
	case 1:
		buildPerspectiveMatrix();
		break;
	case 2:
		buildPerspectiveMatrix2();
		break;
	case 3:
		buildPerspectiveMatrix3();
		break;
	case 4:
		buildOrthoMatrix();
		break;
	}
	return _projectionMatrix;
}

TeVector3f32 TeCamera::projectPoint(const TeVector3f32 &pt) {
	error("TODO: Implement TeCamera::projectPoint");
}

TeVector3f32 TeCamera::projectPoint3f32(const TeVector3f32 &pt) {
	error("TODO: Implement TeCamera::projectPoint3f32");
}

void TeCamera::restore() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setCurrentColor(TeColor(255, 255, 255, 255));
	renderer->setCurrentCamera(nullptr);
}

TeMatrix4x4 TeCamera::transformationMatrix() {
	if (!_transformA)
		return Te3DObject2::transformationMatrix();

	TeMatrix4x4 retval;
	warning("TODO: TeCamera::transformationMatrix Implement me.");

	retval.setToIdentity();
	return retval;
}

TeVector3f32 TeCamera::transformCoord(const TeVector3f32 &pt) {
	warning("TODO: TeCamera::transformCoord Implement me.");
	return pt;
}

TeVector3f32 TeCamera::transformPoint2Dto3D(const TeVector2f32 &pt) {
	TeVector3f32 retval;
	TeVector3f32 vp_br(_viewportX + _viewportW, _viewportY + _viewportH, 0.0f);
	float x = (pt.getX() - _viewportX) / (vp_br.x() - _viewportX);
	float y = (pt.getY() - _viewportY) / (vp_br.y() - _viewportY);
	return TeVector3f32(x * 2 - 1.0f, -(y * 2 - 1.0f), 0.0);
}

void TeCamera::updateProjectionMatrix() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setMatrixMode(TeRenderer::MatrixMode::MM_GL_PROJECTION);
	renderer->loadProjectionMatrix(projectionMatrix());
}

void TeCamera::viewport(int x, int y, uint w, uint h) {
	_viewportX = x;
	_viewportY = y;
	_viewportW = w;
	_viewportH = h;
	_onViewportChangedSignal.call();
}


} // end namespace Tetraedge
