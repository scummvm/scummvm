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

#ifndef QDENGINE_MINIGAMES_ADV_EFFECT_MANAGER_H
#define QDENGINE_MINIGAMES_ADV_EFFECT_MANAGER_H

#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/HoldData.h"

namespace QDEngine {

enum EffectType {
	EFFECT_1,
	EFFECT_COUNT
};

class EffectManager {
public:
	EffectManager(HoldData<EffectManagerData> &data);
	~EffectManager();

	void quant(float dt);

	void start(EffectType id);
	void stop(EffectType id);

private:
	EffectType current_;
	EffectManagerData data_;
	float phaseTime_;
	float effectTime_;
	float phaseSpeed_;

	float effectTimer_;
	float phaseTimer_;
	QDObject effect_;

};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_EFFECT_MANAGER_H
