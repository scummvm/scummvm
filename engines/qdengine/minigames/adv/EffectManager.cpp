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
	const char *effectName = runtime->parameter("effect_name", "effect");
	if (runtime->testObject(effectName)) {
		effect_ = runtime->getObject(effectName);
		data_.crd = effect_->R();
		effect_->set_screen_scale(mgVect2f(0.01f, 0.01f), mgVect2f(10000.f, 10000.f));
		runtime->hide(effect_);
	}

	data.process(data_);

	effectTime_ = clamp(getParameter("effect_time", 3.f), 0.5f, 10.f);
	phaseTime_ = clamp(getParameter("effect_phase_time", effectTime_ / 20.f), 0.03f, 1.f);
	phaseSpeed_ = clamp(getParameter("effect_phase_speed", 1.5f), 1.05f, 10.f);

	current_ = EFFECT_COUNT;

}

EffectManager::~EffectManager() {
	runtime->release(effect_);

}

void EffectManager::quant(float dt) {
	if (current_ == EFFECT_COUNT)
		return;

	if (runtime->getTime() > effectTimer_) {
		stop(current_);
		return;
	}

	if (runtime->getTime() > phaseTimer_) {
		phaseTimer_ = runtime->getTime() + phaseTime_;
		mgVect2f scale = effect_->screen_scale();
		mgVect2f speed = scale;
		scale *= phaseSpeed_;
		speed = scale - speed;
		speed /= phaseTime_;
		effect_->set_screen_scale(scale, speed);
	}

}

void EffectManager::start(EffectType id) {
	if (current_ != EFFECT_COUNT || !effect_)
		return;
	effectTimer_ = runtime->getTime() + effectTime_;
	current_ = id;
	phaseTimer_ = runtime->getTime();
	effect_->set_screen_scale(mgVect2f(0.02f, 0.02f), mgVect2f(10000.f, 10000.f));
	effect_->set_R(data_.crd);

}

void EffectManager::stop(EffectType id) {
	if (current_ == EFFECT_COUNT)
		return;
	runtime->hide(effect_);
	effect_->set_screen_scale(mgVect2f(0.01f, 0.01f), mgVect2f(10000.f, 10000.f));
	current_ = EFFECT_COUNT;
}

} // namespace QDEngine
