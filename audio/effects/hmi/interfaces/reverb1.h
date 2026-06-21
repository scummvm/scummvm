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

#ifndef AUDIO_EFFECTS_HMI_INTERFACES_REVERB1_H
#define AUDIO_EFFECTS_HMI_INTERFACES_REVERB1_H

#include "audio/effects/hmi/hmi_interface.h"

namespace Audio {

class HMIReverb1 : public HMIInterface {
public:
	HMIReverb1();
	int init(HMIPreset *, HMIEffectNode *) override;
	int uninit(HMIPreset *, HMIEffectNode *) override;
	int getMinDuration(HMIEffectNode *, uint32 *) override;
	int processBlock(HMIPreset *, HMIEffectNode *) override;
	int initEffect(HMIPreset *, HMIEffectNode *) override;
	int getEffectParam(HMIEffectNode *, int, float *, int *) override;
	int setEffectParam(HMIEffectNode *, int, float) override;

private:
	static const char *const kReverb1Params[];
	static const float kReverb1Min[];
	static const float kReverb1Max[];
	static const float kReverb1Default[];
	static const float kReverb1Delay[4];
};

} // End of namespace Audio

#endif
