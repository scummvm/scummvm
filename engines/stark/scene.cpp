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

#include "engines/stark/scene.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/renderentry.h"

#include "math/glmath.h"

namespace Stark {

Scene::Scene(GfxDriver *gfx) :
		_gfx(gfx),
		_fov(45.0),
		_nearClipPlane(100.0),
		_farClipPlane(64000.0),
		_scollXFactor(0.0),
		_scollYFactor(0.0) {
}

Scene::~Scene() {
}

void Scene::initCamera(const Math::Vector3d &position, const Math::Vector3d &lookDirection,
		float fov, Common::Rect viewSize, float nearClipPlane, float farClipPlane) {
	_cameraPosition = position;
	_cameraLookDirection = lookDirection;
	_fov = fov;
	_viewSize = viewSize;
	_nearClipPlane = nearClipPlane;
	_farClipPlane = farClipPlane;

	float xmax, ymax;
	computeSymmetricPerspectiveRect(nullptr, &xmax, nullptr, &ymax);

	// The amounts by which translate to clipping planes to account for one pixel
	// of camera scrolling movement
	_scollXFactor = xmax / 640.0 * 2;
	_scollYFactor = ymax / 365.0 * 2;

	_lookAtMatrix = Math::makeLookAtMatrix(_cameraPosition, _cameraPosition + _cameraLookDirection, Math::Vector3d(0.0, 0.0, 1.0));
}

void Scene::scrollCamera(const Common::Rect &viewport) {
	_viewport = viewport;

	// The perspective matrix is a symmetric perspective matrix as returned
	// by Math::makePerspectiveMatrix with the clipping rect translated
	// to account for the camera scrolling.

	float xmin, xmax, ymin, ymax;
	computeSymmetricPerspectiveRect(&xmin, &xmax, &ymin, &ymax);

	int32 distanceToCenterX = _viewport.left + _viewport.width() / 2 - _viewSize.width() / 2;
	int32 distanceToCenterY = _viewport.top + _viewport.height() / 2 - _viewSize.height() / 2;

	xmin += distanceToCenterX * _scollXFactor;
	xmax += distanceToCenterX * _scollXFactor;
	ymin += distanceToCenterY * _scollYFactor;
	ymax += distanceToCenterY * _scollYFactor;

	_perspectiveMatrix = Math::makeFrustumMatrix(xmin, xmax, ymin, ymax, _nearClipPlane, _farClipPlane);
}

void Scene::computeSymmetricPerspectiveRect(float *xmin, float *xmax, float *ymin, float *ymax) {
	float aspectRatio = 1.0;
	float ymaxValue = _nearClipPlane * tan(_fov * M_PI / 360.0);
	float yminValue = -ymaxValue;
	float xminValue = yminValue / aspectRatio;
	float xmaxValue = ymaxValue / aspectRatio;

	if (xmin) *xmin = xminValue;
	if (xmax) *xmax = xmaxValue;
	if (ymin) *ymin = yminValue;
	if (ymax) *ymax = ymaxValue;
}

void Scene::render(RenderEntryArray renderEntries) {
	// setup cam
	_gfx->setupPerspective(_perspectiveMatrix);
	_gfx->setupCamera(_cameraPosition, _lookAtMatrix);

	// Draw bg

	// Draw other things

	// Render all the scene elements
	RenderEntryArray::iterator element = renderEntries.begin();
	while (element != renderEntries.end()) {
		// Draw the current element
		(*element)->render(_gfx);

		// Go for the next one
		element++;
	}

	//_gfx->set3DMode();

	// setup lights

	// draw actors

	// draw overlay
}

} // End of namespace Stark
