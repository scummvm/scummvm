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

#include "tetraedge/te/te_clip_layout.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

TeClipLayout::TeClipLayout() {
}

void TeClipLayout::draw() {
	if (!worldVisible())
		return;

	TeRenderer *renderer = g_engine->getRenderer();
	bool prevScissorEnabled = renderer->scissorEnabled();
	const TeVector2f32 prevScissorPos(renderer->scissorX(), renderer->scissorY());
	const TeVector2f32 prevScissorSize(renderer->scissorWidth(), renderer->scissorHeight());

	const TeMatrix4x4 worldTransform = worldTransformationMatrix();

	const TeVector3f32 v1 = worldTransform * TeVector3f32(0, 0, 0);
	const TeVector3f32 v2 = worldTransform * TeVector3f32(1, 0, 0);
	const TeVector3f32 v3 = worldTransform * TeVector3f32(0, 1, 0);

	const TeVector3f32 offset((v2 - v1).length(), (v3 - v1).length(), 1);
	const TeVector3f32 thisSize(xSize(), ySize(), 0);
	const TeVector3f32 newScissorSize = thisSize * offset;
	const TeVector3f32 transformedSize(v1.x() - newScissorSize.x() / 2, v1.y() + newScissorSize.y() / 2, 0);

	const TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();

	const TeVector3f32 newScissorOff(transformedSize.x() + winSize.x() / 2, winSize.y() - (transformedSize.y() + winSize.y() / 2), 0);

	renderer->setScissorEnabled(true);
	renderer->setScissor(newScissorOff.x(), newScissorOff.y(), newScissorSize.x(), newScissorSize.y());

	TeLayout::draw();

	renderer->setScissor(prevScissorPos.getX(), prevScissorPos.getY(), prevScissorSize.getX(), prevScissorSize.getY());
	renderer->setScissorEnabled(prevScissorEnabled);
}

} // end namespace Tetraedge
