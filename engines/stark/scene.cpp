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

Scene::Scene(Gfx::GfxDriver *gfx) :
		_gfx(gfx),
		_fov(45.0),
		_nearClipPlane(100.0),
		_farClipPlane(64000.0) {
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

	_lookAtMatrix = Math::makeLookAtMatrix(_cameraPosition, _cameraPosition + _cameraLookDirection, Math::Vector3d(0.0, 0.0, 1.0));
}

void Scene::scrollCamera(const Common::Rect &viewport) {
	_viewport = viewport;

	float xmin, xmax, ymin, ymax;
	computeClippingRect(&xmin, &xmax, &ymin, &ymax);

	// The amounts by which translate to clipping planes to account for one pixel
	// of camera scrolling movement
	float scollXFactor = (xmax - xmin) / _viewport.width();
	float scollYFactor = (ymax - ymin) / _viewport.height();

	int32 distanceToRight = _viewport.right - _viewSize.width();
	int32 distanceToTop = -_viewport.top;

	xmin += distanceToRight * scollXFactor;
	xmax += distanceToRight * scollXFactor;
	ymin += distanceToTop * scollYFactor;
	ymax += distanceToTop * scollYFactor;

	_perspectiveMatrix = Math::makeFrustumMatrix(xmin, xmax, ymin, ymax, _nearClipPlane, _farClipPlane);
}

void Scene::computeClippingRect(float *xmin, float *xmax, float *ymin, float *ymax) {
	float aspectRatio = _viewSize.width() / (float) _viewSize.height();
	float xmaxValue = _nearClipPlane * tan(_fov * M_PI / 360.0);
	float ymaxValue = xmaxValue / aspectRatio;

	float xminValue = xmaxValue - 2 * xmaxValue * (_viewport.width() / (float) _viewSize.width());
	float yminValue = ymaxValue - 2 * ymaxValue * (_viewport.height() / (float) _viewSize.height());

	if (xmin) *xmin = xminValue;
	if (xmax) *xmax = xmaxValue;
	if (ymin) *ymin = yminValue;
	if (ymax) *ymax = ymaxValue;
}

void Scene::render(Gfx::RenderEntryArray renderEntries) {
	// setup cam
	_gfx->setGameViewport();
	_gfx->setupPerspective(_perspectiveMatrix);
	_gfx->setupCamera(_cameraPosition, _lookAtMatrix);

	// Draw bg

	// Draw other things

	// Render all the scene elements
	Gfx::RenderEntryArray::iterator element = renderEntries.begin();
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
	_gfx->setScreenViewport();
}

} // End of namespace Stark
