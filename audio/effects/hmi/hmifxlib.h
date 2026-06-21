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

#ifndef AUDIO_EFFECTS_HMI_HMIFXLIB_H
#define AUDIO_EFFECTS_HMI_HMIFXLIB_H

#include "audio/effects/hmi/hmifxfp.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class HMIFxLib {
public:
	HMIFxLib();
	~HMIFxLib();

	HMIFxFp *effects();

	int hmiFXInitSystem(HMIInitData *initData);
	int hmiFXUnInitSystem(HMIInitData *initData);
	int hmiFXGetInterface(int interfaceId, HMIInterface **outInterface);
	unsigned int hmiFXGenerateQueryString(char *buffer, const char *prefix, unsigned int id);
	int hmiFXGetFirstInterface(HMIInterface **outInterface);
	int hmiFXGetNextInterface(HMIInterface **outInterface);

	int hmiFXPresetCreate(HMIPreset *preset);
	int hmiFXPresetDestroy(HMIPreset *preset);
	int hmiFXPresetAddEffect(HMIPreset *preset, HMIEffectNode *newNode);
	int hmiFXPresetLoadLibrary(HMILibrary **outLib, const char *fileName);
	int hmiFXPresetLoadLibrary(HMILibrary **outLib, Common::SeekableReadStream &stream);
	int hmiFXPresetFreeLibrary(HMILibrary *library);
	int hmiFXPresetProcess(HMILibrary *library, HMIProcessRequest *request);
	int hmiFXPresetGetByName(HMILibrary *library, const char *name, HMIPreset **outPreset);
	int hmiFXPresetLock(HMIPreset *preset);
	int hmiFXPresetRelease(HMIPreset *preset);
	int hmiFXPresetGetRouting(HMIPreset *preset, uint32 *routingOut);
	void hmiFXSetWaveFmtEx(HMIFormat *format, int sampleRate, uint16 bitsPerSample, uint16 channels);
	int hmiFXPresetCreateInstance(HMIPreset *source, HMIPreset **outInstance);
	int hmiFXPresetDestroyInstance(HMIPreset *instance);
	int hmiFXPresetProcessEx(HMIProcessRequest *request);

	void *hmiFXHeapAlloc(int size);
	void hmiFXHeapFree(void *ptr);
	int hmiFXHeapInit(HMIInitData *initData);
	void *hmiFXHeapReset();

private:
	int convertPCMToFloat(HMIPreset *preset, const uint8 *buffer, int samples);
	int convertFloatToPCM(HMIPreset *preset, uint8 *buffer, int samples, int clampOutput);
	int processPreset(HMIPreset *preset, HMIProcessRequest *request);
	void resetMixBuffers(HMIPreset *preset);
	void processEffectChain(HMIPreset *preset);

	int readFile(HMIFileDescriptor *desc, Common::SeekableReadStream &stream);
	int flushFile(HMIFileDescriptor *desc);
	int findSection(HMIFileDescriptor *desc, const char *section);
	int findKey(HMIFileDescriptor *desc, const char *key);
	int readInt(HMIFileDescriptor *desc, int *value);
	int readCSV(HMIFileDescriptor *desc, char *buffer, int size);
	int readSectionInt(HMIFileDescriptor *desc, const char *key, int *value);
	int readSectionString(HMIFileDescriptor *desc, const char *key, char *value, int size);

	HMIFxFp *_fxFp = nullptr;
	HMIInterface *_loadedInterfaces[1024];
	unsigned int _numLoadedInterfaces = 0;
	unsigned int _curInterfaceIndex = 0;

	uint8 *_systemHeap = nullptr;
	uint8 *_heapCursor = nullptr;
	uint8 *_heapEnd = nullptr;
};

} // End of namespace Audio

#endif
