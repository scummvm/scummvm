/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/sidefx/distort_node.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/render_table.h"

#include "common/stream.h"


namespace ZVision {

DistortNode::DistortNode(ZVision *engine, uint32 key, int16 speed, float st_angl, float en_angl, float st_lin, float en_lin)
	: SideFX(engine, key, SIDEFX_DISTORT) {

	_angle = _engine->getRenderManager()->getRenderTable()->getAngle();
	_linScale = _engine->getRenderManager()->getRenderTable()->getLinscale();

	_speed = speed;
	_incr = true;
	_st_angl = st_angl;
	_en_angl = en_angl;
	_st_lin = st_lin;
	_en_lin = en_lin;

	_curFrame = 1.0;

	_diff_angl = en_angl - st_angl;
	_diff_lin = en_lin - st_lin;

	_frmSpeed = (float)speed / 15.0;
	_frames = ceil((5.0 - _frmSpeed * 2.0) / _frmSpeed);
	if (_frames <= 0)
		_frames = 1;

	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 1);
}

DistortNode::~DistortNode() {
	setParams(_angle, _linScale);
}

bool DistortNode::process(uint32 deltaTimeInMillis) {

	float updTime = deltaTimeInMillis / (1000.0 / 60.0);

	if (_incr)
		_curFrame += updTime;
	else
		_curFrame -= updTime;

	if (_curFrame < 1.0) {
		_curFrame = 1.0;
		_incr = true;
	} else if (_curFrame > _frames) {
		_curFrame = _frames;
		_incr = false;
	}

	float diff = (1.0 / (5.0 - (_curFrame * _frmSpeed))) / (5.0 - _frmSpeed);


	setParams(_st_angl + diff * _diff_angl, _st_lin + diff * _diff_lin);

	return false;
}

void DistortNode::setParams(float angl, float linScale) {
	RenderTable *table = _engine->getRenderManager()->getRenderTable();
	if (table->getRenderState() == RenderTable::PANORAMA) {
		table->setPanoramaFoV(angl);
		table->setPanoramaScale(linScale);
		table->generateRenderTable();
		_engine->getRenderManager()->markDirty();
	} else if (table->getRenderState() == RenderTable::TILT) {
		table->setTiltFoV(angl);
		table->setTiltScale(linScale);
		table->generateRenderTable();
		_engine->getRenderManager()->markDirty();
	}
}


} // End of namespace ZVision
