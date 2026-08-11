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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AUDIO_EFFECTS_HMI_INTERFACES_FILTER1_H
#define AUDIO_EFFECTS_HMI_INTERFACES_FILTER1_H

#include "audio/effects/hmi/hmi_interface.h"

namespace Audio {

class HMIFilter1 : public HMIInterface {
public:
	HMIFilter1();
	int init(HMIPreset *preset, HMIEffectNode *base) override;
	int uninit(HMIPreset *preset, HMIEffectNode *base) override;
	int processBlock(HMIPreset *preset, HMIEffectNode *base) override;
	int initEffect(HMIPreset *preset, HMIEffectNode *base) override;
	int getEffectParam(HMIEffectNode *base, int param, float *value, int *type) override;
	int setEffectParam(HMIEffectNode *base, int param, float value) override;

private:
	static const char *const kFilter1Params[];
	static const float kFilter1Min[];
	static const float kFilter1Max[];
	static const float kFilter1Default[];
};

} // End of namespace Audio

#endif
