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

	virtual int init(HMIPreset *, HMIEffectNode *) = 0;
	virtual int uninit(HMIPreset *, HMIEffectNode *) = 0;
	virtual int getMinDuration(HMIEffectNode *, uint32 *);
	virtual int processBlock(HMIPreset *, HMIEffectNode *) = 0;
	virtual int initEffect(HMIPreset *, HMIEffectNode *) = 0;
	virtual int getEffectParam(HMIEffectNode *, int, float *, int *) = 0;
	virtual int setEffectParam(HMIEffectNode *, int, float) = 0;

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
	int _effectStructSize;
	int _outChannels;
	const char *_interfaceName;
	HMIParamNames _effectParams;
	const float *_paramMinValues;
	const float *_paramMaxValues;
	const float *_paramDefaultValues;
	char _presetName[64];
	int _presetId;
	int _paramCount;
};

} // End of namespace Audio

#endif
