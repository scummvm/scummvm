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

#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/EffectManager.h"
#include "qdengine/minigames/adv/qdMath.h"

namespace QDEngine {

EffectManager::EffectManager(HoldData<EffectManagerData> &data) {
	const char *effectName = g_runtime->parameter("effect_name", "effect");
	if (g_runtime->testObject(effectName)) {
		_effect = g_runtime->getObject(effectName);
		_data.crd = _effect->R();
		_effect->set_screen_scale(mgVect2f(0.01f, 0.01f), mgVect2f(10000.f, 10000.f));
		g_runtime->hide(_effect);
	}

	data.process(_data);

	_effectTime = clamp(getParameter("effect_time", 3.f), 0.5f, 10.f);
	_phaseTime = clamp(getParameter("effect_phase_time", _effectTime / 20.f), 0.03f, 1.f);
	_phaseSpeed = clamp(getParameter("effect_phase_speed", 1.5f), 1.05f, 10.f);

	_current = EFFECT_COUNT;

	_effectTimer = 0;
	_phaseTimer = 0;
}

EffectManager::~EffectManager() {
	g_runtime->release(_effect);

}

void EffectManager::quant(float dt) {
	if (_current == EFFECT_COUNT)
		return;

	if (g_runtime->getTime() > _effectTimer) {
		stop(_current);
		return;
	}

	if (g_runtime->getTime() > _phaseTimer) {
		_phaseTimer = g_runtime->getTime() + _phaseTime;
		mgVect2f scale = _effect->screen_scale();
		mgVect2f speed = scale;
		scale *= _phaseSpeed;
		speed = scale - speed;
		speed /= _phaseTime;
		_effect->set_screen_scale(scale, speed);
	}

}

void EffectManager::start(EffectType id) {
	if (_current != EFFECT_COUNT || !_effect)
		return;
	_effectTimer = g_runtime->getTime() + _effectTime;
	_current = id;
	_phaseTimer = g_runtime->getTime();
	_effect->set_screen_scale(mgVect2f(0.02f, 0.02f), mgVect2f(10000.f, 10000.f));
	_effect->set_R(_data.crd);

}

void EffectManager::stop(EffectType id) {
	if (_current == EFFECT_COUNT)
		return;
	g_runtime->hide(_effect);
	_effect->set_screen_scale(mgVect2f(0.01f, 0.01f), mgVect2f(10000.f, 10000.f));
	_current = EFFECT_COUNT;
}

} // namespace QDEngine
