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

#ifndef AUDIO_EFFECTS_HMI_INTERFACES_REVERB2_H
#define AUDIO_EFFECTS_HMI_INTERFACES_REVERB2_H

#include "audio/effects/hmi/hmi_interface.h"

namespace Audio {

class HMIReverb2 : public HMIInterface {
public:
	HMIReverb2();
	int init(HMIPreset *preset, HMIEffectNode *base) override;
	int uninit(HMIPreset *preset, HMIEffectNode *base) override;
	int getMinDuration(HMIEffectNode *base, uint32 *duration) override;
	int processBlock(HMIPreset *preset, HMIEffectNode *base) override;
	int initEffect(HMIPreset *preset, HMIEffectNode *base) override;
	int getEffectParam(HMIEffectNode *base, int param, float *value, int *type) override;
	int setEffectParam(HMIEffectNode *base, int param, float value) override;

private:
	static const char *const kReverb2Params[];
	static const float kReverb2Min[];
	static const float kReverb2Max[];
	static const float kReverb2Default[];
	static const float kReverb2Delay[6];
	static const float kReverb2Decay[6];
};

} // End of namespace Audio

#endif
