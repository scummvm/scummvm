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

#ifndef AUDIO_EFFECTS_HMI_INTERFACE_H
#define AUDIO_EFFECTS_HMI_INTERFACE_H

#include "audio/effects/hmi/hmi_types.h"

namespace Audio {

typedef const char *const *HMIParamNames;

class HMIInterface {
public:
	virtual ~HMIInterface() {}

	virtual int init(HMIPreset *preset, HMIEffectNode *base) = 0;
	virtual int uninit(HMIPreset *preset, HMIEffectNode *base) = 0;
	virtual int getMinDuration(HMIEffectNode *n, uint32 *out);
	virtual int processBlock(HMIPreset *preset, HMIEffectNode *base) = 0;
	virtual int initEffect(HMIPreset *preset, HMIEffectNode *base) = 0;
	virtual int getEffectParam(HMIEffectNode *base, int param, float *value, int *type) = 0;
	virtual int setEffectParam(HMIEffectNode *base, int param, float value) = 0;

	int effectStructSize() const { return _effectStructSize; }
	int outChannels() const { return _outChannels; }
	const char *interfaceName() const { return _interfaceName; }
	HMIParamNames effectParams() const { return _effectParams; }
	const float *paramMinValues() const { return _paramMinValues; }
	const float *paramMaxValues() const { return _paramMaxValues; }
	const float *paramDefaultValues() const { return _paramDefaultValues; }
	const char *presetName() const { return _presetName; }
	int presetId() const { return _presetId; }
	int paramCount() const { return _paramCount; }

protected:
	static const double kHmiPi;
	static const double kHmiTwoPi;
	static const double kHmiMillis;

	HMIInterface(int size, int channels,
				 const char *dialogName, HMIParamNames params, const float *mins,
				 const float *maxs, const float *defaults, const char *name, int id,
				 int paramCount);

private:
	int _effectStructSize = 0;
	int _outChannels = 0;
	const char *_interfaceName = nullptr;
	HMIParamNames _effectParams = nullptr;
	const float *_paramMinValues = nullptr;
	const float *_paramMaxValues = nullptr;
	const float *_paramDefaultValues = nullptr;
	char _presetName[64] = {0};
	int _presetId = 0;
	int _paramCount = 0;
};

} // End of namespace Audio

#endif
