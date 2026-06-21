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

#ifndef AUDIO_EFFECTS_HMI_INTERFACES_RESONATOR_H
#define AUDIO_EFFECTS_HMI_INTERFACES_RESONATOR_H

#include "audio/effects/hmi/hmi_interface.h"

namespace Audio {

class HMIResonator : public HMIInterface {
public:
	HMIResonator();
	int init(HMIPreset *, HMIEffectNode *) override;
	int uninit(HMIPreset *, HMIEffectNode *) override;
	int processBlock(HMIPreset *, HMIEffectNode *) override;
	int initEffect(HMIPreset *, HMIEffectNode *) override;
	int getEffectParam(HMIEffectNode *, int, float *, int *) override;
	int setEffectParam(HMIEffectNode *, int, float) override;

private:
	static const char *const kResonatorParams[];
	static const float kResonatorMin[];
	static const float kResonatorMax[];
	static const float kResonatorDefault[];
};

} // End of namespace Audio

#endif
