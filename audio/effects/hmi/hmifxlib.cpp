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

#include "audio/effects/hmi/hmifxlib.h"

#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"

namespace Audio {

/*
 * This deliberately is not a plain integer cast. With the DLL's default x87
 * control word, FISTP rounds to nearest with ties going to the even integer:
 * 1.6 becomes 2, 2.5 stays 2, and 3.5 becomes 4. A C++ cast would instead
 * truncate toward zero. FISTP also produces its integer-indefinite value for
 * NaN and out-of-range inputs, so preserve that behavior here as well.
 * AFAIK, we have no rounding helper that provides this exact combination of
 * nearest-even rounding and fixed-width x87 overflow behavior. AFAIK.
 */
static int32 hmiFistp32(float value) {
	if (value != value || value < -2147483648.0f || value >= 2147483648.0f)
		return -2147483647 - 1;

	int32 result = (int32)value;
	float fraction = value - (float)result;

	if (fraction > 0.5f || (fraction == 0.5f && (result % 2))) {
		++result;
	} else if (fraction < -0.5f || (fraction == -0.5f && (result % 2))) {
		--result;
	}

	return result;
}

static int16 hmiFistp16(float value) {
	int32 result = hmiFistp32(value);
	if (result < -32768 || result > 32767)
		return -32768;

	return (int16)result;
}

HMIFxLib::HMIFxLib()
	: _numLoadedInterfaces(0), _curInterfaceIndex(0), _systemHeap(nullptr), _heapCursor(nullptr), _heapEnd(nullptr) {
	memset(_loadedInterfaces, 0, sizeof(_loadedInterfaces));

	_fxFp = new HMIFxFp();
	
	HMIInterface **list = nullptr;
	_fxFp->hmiFXGetInterfaceList(&list);

	while (_numLoadedInterfaces != 1024 && list[_numLoadedInterfaces]) {
		_loadedInterfaces[_numLoadedInterfaces] = list[_numLoadedInterfaces];
		++_numLoadedInterfaces;
	}

	if (_numLoadedInterfaces != 1024)
		_loadedInterfaces[_numLoadedInterfaces] = nullptr;
}

HMIFxLib::~HMIFxLib() {
	delete _fxFp;
}

HMIFxFp *HMIFxLib::effects() {
	return _fxFp;
}

int HMIFxLib::hmiFXInitSystem(HMIInitData *initData) {
	initData->heapPtr = malloc((size_t)initData->heapSize);
	if (!initData->heapPtr)
		return 1;

	hmiFXHeapInit(initData);
	return 0;
}

int HMIFxLib::hmiFXUnInitSystem(HMIInitData *initData) {
	if (initData->heapPtr)
		free(initData->heapPtr);

	initData->heapPtr = nullptr;
	_systemHeap = _heapCursor = _heapEnd = nullptr;
	return 0;
}

int HMIFxLib::hmiFXGetInterface(int interfaceId, HMIInterface **outInterface) {
	for (unsigned int i = 0; i != _numLoadedInterfaces; ++i) {
		if (_loadedInterfaces[i]->presetId() == interfaceId) {
			*outInterface = _loadedInterfaces[i];
			return 0;
		}
	}

	return 7;
}

unsigned int HMIFxLib::hmiFXGenerateQueryString(char *buffer, const char *prefix, unsigned int id) {
	if (id < 10)
		return (unsigned int)snprintf(buffer, 32, "%s000%d", prefix, id);

	if (id < 100)
		return (unsigned int)snprintf(buffer, 32, "%s00%d", prefix, id);

	if (id < 1000)
		return (unsigned int)snprintf(buffer, 32, "%s0%d", prefix, id);

	if (id < 10000)
		return (unsigned int)snprintf(buffer, 32, "%s%d", prefix, id);

	return id;
}

int HMIFxLib::hmiFXGetFirstInterface(HMIInterface **outInterface) {
	if (!_numLoadedInterfaces)
		return 7;

	_curInterfaceIndex = 1;
	*outInterface = _loadedInterfaces[0];
	return 0;
}

int HMIFxLib::hmiFXGetNextInterface(HMIInterface **outInterface) {
	if (_curInterfaceIndex == _numLoadedInterfaces)
		return 7;

	*outInterface = _loadedInterfaces[_curInterfaceIndex++];
	return 0;
}

void *HMIFxLib::hmiFXHeapAlloc(int size) {
	uint8 *result = _heapCursor;
	if (!result || result + size > _heapEnd)
		return nullptr;

	_heapCursor += size;
	return result;
}

void HMIFxLib::hmiFXHeapFree(void *) {}

int HMIFxLib::hmiFXHeapInit(HMIInitData *data) {
	_systemHeap = (uint8 *)data->heapPtr;
	_heapCursor = _systemHeap;
	_heapEnd = _systemHeap + data->heapSize;
	return 0;
}

void *HMIFxLib::hmiFXHeapReset() {
	_heapCursor = _systemHeap;
	return _systemHeap;
}

int HMIFxLib::convertPCMToFloat(HMIPreset *preset, const uint8 *buffer, int samples) {
	float *left = preset->floatBufLeftActive;
	float *right = preset->floatBufRightActive;

	if (preset->inputFmt->bitsPerSample == 8) {
		if (preset->inputFmt->numChannels == 1) {
			for (int i = 0; i != samples; ++i) {
				left[i] = (float)((double)buffer[i] - 127.0);
			}
		} else {
			for (int i = 0; i != samples; ++i) {
				left[i] = (float)((double)buffer[i * 2] - 127.0);
				right[i] = (float)((double)buffer[i * 2 + 1] - 127.0);
			}
		}
	} else if (preset->inputFmt->bitsPerSample == 16) {
		const int16 *input = (const int16 *)buffer;
		if (preset->inputFmt->numChannels == 1) {
			for (int i = 0; i != samples; ++i) {
				left[i] = (float)input[i];
			}
		} else {
			for (int i = 0; i != samples; ++i) {
				left[i] = (float)input[i * 2];
				right[i] = (float)input[i * 2 + 1];
			}
		}
	}

	return 0;
}

int HMIFxLib::convertFloatToPCM(HMIPreset *preset, uint8 *buffer, int samples, int clampOutput) {
	float *left = preset->floatBufLeftAlt;
	float *right = preset->floatBufRightAlt;

	if (preset->outputFmt->bitsPerSample == 8) {
		if (preset->outputFmt->numChannels == 1) {
			for (int i = 0; i != samples; ++i) {
				if (left[i] > 127.0f) {
					buffer[i] = 255;
				} else if (left[i] <= -128.0f) {
					buffer[i] = 0;
				} else {
					buffer[i] = (uint8)((int64)((double)left[i] + 128.0));
				}
			}
		} else {
			for (int i = 0; i != samples; ++i) {
				float values[2] = {left[i], right[i]};
				for (int c = 0; c != 2; ++c) {
					if (values[c] > 127.0f) {
						buffer[i * 2 + c] = 255;
					} else if (values[c] <= -128.0f) {
						buffer[i * 2 + c] = 0;
					} else {
						buffer[i * 2 + c] = (uint8)(((int64)values[c]) ^ 0x80);
					}
				}
			}
		}
	} else if (preset->outputFmt->bitsPerSample == 16) {
		int16 *output = (int16 *)buffer;
		int channels = preset->outputFmt->numChannels == 1 ? 1 : 2;
		for (int i = 0; i != samples; ++i) {
			for (int c = 0; c != channels; ++c) {
				float value = c ? right[i] : left[i];
				if (!clampOutput) {
					output[i * channels + c] = hmiFistp16(value);
					continue;
				}

				int converted = hmiFistp32(value);
				if (converted > 32767) {
					converted = 32767;
				} else if (converted < -32767) {
					converted = -32767;
				}

				output[i * channels + c] = (int16)converted;
			}
		}
	}

	return 0;
}

int HMIFxLib::hmiFXPresetCreate(HMIPreset *preset) {
	hmiFXHeapReset();

	uint32 samples = preset->inputFmt->sampleRate * preset->chunkSizeInMs / 1000U;
	int segmentBytes = 4 * (int)samples + 1024;
	preset->chunkSizeInSamples = samples;
	float *block = (float *)((preset->flags & HMI_PRESET_FLAG_USE_MALLOC) ? malloc(4 * segmentBytes)
																		  : hmiFXHeapAlloc(4 * segmentBytes));
	if (!block)
		return 1;

	preset->floatBufLeft0 = block;
	preset->floatBufLeft1 = (float *)((uint8 *)block + segmentBytes);
	preset->floatBufRight0 = (float *)((uint8 *)block + segmentBytes * 2);
	preset->floatBufRight1 = (float *)((uint8 *)block + segmentBytes * 3);

	resetMixBuffers(preset);
	preset->effectChain = nullptr;
	return 0;
}

void HMIFxLib::resetMixBuffers(HMIPreset *preset) {
	preset->floatBufLeftActive = preset->floatBufLeft0;
	preset->floatBufRightActive = preset->floatBufRight0;
	preset->floatBufLeftAlt = preset->floatBufLeft1;
	preset->floatBufRightAlt = preset->floatBufRight1;
	preset->pingPongIndex = 0;
}

int HMIFxLib::hmiFXPresetDestroy(HMIPreset *preset) {
	for (HMIEffectNode *node = preset->effectChain; node; node = node->next)
		node->interface->uninit(preset, node);

	if (preset->flags & HMI_PRESET_FLAG_USE_MALLOC)
		free(preset->floatBufLeft0);

	return 0;
}

int HMIFxLib::hmiFXPresetAddEffect(HMIPreset *preset, HMIEffectNode *newNode) {
	int result = newNode->interface->init(preset, newNode);
	if (result)
		return result;

	HMIEffectNode *tail = preset->effectChain;
	if (!tail) {
		preset->effectChain = newNode;
	} else {
		while (tail->next)
			tail = tail->next;

		tail->next = newNode;
	}
	
	newNode->next = nullptr;
	preset->allocatedHeapSize = _systemHeap ? (int)(_heapCursor - _systemHeap) : 0;
	return 0;
}

int HMIFxLib::hmiFXPresetGetByName(HMILibrary *library, const char *name, HMIPreset **outPreset) {
	HMIPreset *preset = library->effectPtr;
	if (!preset)
		return 12;

	*outPreset = nullptr;
	while (preset && scumm_stricmp(preset->name, name)) {
		preset = preset->next;
	}

	if (!preset)
		return 13;

	*outPreset = preset;
	return 0;
}

int HMIFxLib::hmiFXPresetLock(HMIPreset *preset) {
	if (!preset)
		return 15;

	if (preset->lockLevel) {
		++preset->lockLevel;
		return 0;
	}

	preset->lockLevel = 1;

	preset->flags &= ~(HMI_PRESET_FLAG_USE_MALLOC | HMI_PRESET_FLAG_EFFECTS_INITIALIZED);
	for (HMIEffectNode *n = preset->effectChain; n; n = n->next)
		n->interface->uninit(preset, n);

	preset->flags |= HMI_PRESET_FLAG_USE_MALLOC;
	for (HMIEffectNode *n = preset->effectChain; n; n = n->next)
		n->interface->init(preset, n);

	return 0;
}

int HMIFxLib::hmiFXPresetRelease(HMIPreset *preset) {
	if (!preset)
		return 15;

	if (--preset->lockLevel == 0) {
		for (HMIEffectNode *n = preset->effectChain; n; n = n->next)
			n->interface->uninit(preset, n);

		HMIEffectNode *chain = preset->effectChain;
		preset->flags &= ~HMI_PRESET_FLAG_USE_MALLOC;
		hmiFXPresetCreate(preset);
		preset->effectChain = chain;

		for (HMIEffectNode *n = chain; n; n = n->next)
			n->interface->init(preset, n);
	}

	return 0;
}

int HMIFxLib::hmiFXPresetGetRouting(HMIPreset *preset, uint32 *routingOut) {
	if (!preset)
		return 15;

	if (!preset->effectChain)
		return 6;

	HMIEffectNode *n = preset->effectChain;
	while (n->next)
		n = n->next;

	*routingOut = (n->channelMode & HMI_EFFECT_STEREO_ROUTING_MASK) ? 2 : 1;
	return 0;
}

void HMIFxLib::hmiFXSetWaveFmtEx(HMIFormat *f, int rate, uint16 bits, uint16 channels) {
	f->numChannels = channels;
	f->bitsPerSample = bits;
	f->sampleRate = rate;
	f->blockAlign = (uint16)(bits * channels / 8);
	f->byteRate = f->blockAlign * rate;
	f->fmtBeginSection = 1;
	f->fmtEndSection = 0;
}

int HMIFxLib::hmiFXPresetCreateInstance(HMIPreset *source, HMIPreset **outInstance) {
	if (!source)
		return 15;

	HMIPreset *copy = (HMIPreset *)malloc(sizeof(HMIPreset));
	if (!copy) {
		*outInstance = nullptr;
		return 1;
	}

	memcpy(copy, source, sizeof(HMIPreset));
	copy->effectChain = nullptr;
	copy->flags = (copy->flags & ~(HMI_PRESET_FLAG_USE_MALLOC | HMI_PRESET_FLAG_EFFECTS_INITIALIZED | HMI_PRESET_FLAG_UNKNOWN_28)) | HMI_PRESET_FLAG_USE_MALLOC;

	HMIEffectNode *previous = nullptr;
	for (HMIEffectNode *src = source->effectChain; src; src = src->next) {
		HMIEffectNode *node = (HMIEffectNode *)malloc(src->interface->effectStructSize());
		if (!node) {
			hmiFXPresetDestroyInstance(copy);
			*outInstance = nullptr;
			return 1;
		}

		memcpy(node, src, src->interface->effectStructSize());
		node->next = nullptr;
		node->interface->init(copy, node);

		if (previous) {
			previous->next = node;
		} else {
			copy->effectChain = node;
		}

		previous = node;
	}

	*outInstance = copy;
	return 0;
}

int HMIFxLib::hmiFXPresetDestroyInstance(HMIPreset *preset) {
	if (!preset)
		return 15;

	HMIEffectNode *node = preset->effectChain;
	while (node) {
		node->interface->uninit(preset, node);
		HMIEffectNode *next = node->next;
		free(node);
		node = next;
	}

	free(preset);
	return 0;
}

void HMIFxLib::processEffectChain(HMIPreset *preset) {
	HMIEffectNode *node = preset->effectChain;
	while (node) {
		node->interface->processBlock(preset, node);
		node = node->next;

		if (node) {
			preset->pingPongIndex ^= 1;
			int active = preset->pingPongIndex;
			preset->floatBufLeftActive = active ? preset->floatBufLeft1 : preset->floatBufLeft0;
			preset->floatBufRightActive = active ? preset->floatBufRight1 : preset->floatBufRight0;
			preset->floatBufLeftAlt = active ? preset->floatBufLeft0 : preset->floatBufLeft1;
			preset->floatBufRightAlt = active ? preset->floatBufRight0 : preset->floatBufRight1;
		}
	}
}

int HMIFxLib::processPreset(HMIPreset *preset, HMIProcessRequest *req) {
	if (!preset->effectChain)
		return 6;

	HMIFormat *savedInput = preset->inputFmt;
	HMIFormat *savedOutput = preset->outputFmt;

	if (req->flags & HMI_PROCESS_OVERRIDE_INPUT_FORMAT)
		preset->inputFmt = req->inputFmt;

	if (req->flags & HMI_PROCESS_OVERRIDE_OUTPUT_FORMAT)
		preset->outputFmt = req->outputFmt;

	if (req->flags & HMI_PROCESS_DERIVE_OUTPUT_FORMAT) {
		uint32 routing;
		hmiFXPresetGetRouting(preset, &routing);
		hmiFXSetWaveFmtEx(req->outputFmt, preset->inputFmt->sampleRate,
						  preset->inputFmt->bitsPerSample, (uint16)routing);
		preset->outputFmt = req->outputFmt;
	}

	preset->chunkSizeInSamples = preset->inputFmt->sampleRate * preset->chunkSizeInMs / 1000U;
	uint32 sourceSamples = req->srcLen / preset->inputFmt->blockAlign;
	uint32 tailMs = 0;

	for (HMIEffectNode *n = preset->effectChain; n; n = n->next) {
		uint32 value = 0;
		n->interface->getMinDuration(n, &value);
		if (tailMs <= value)
			tailMs = value;
	}

	uint32 tailSamples = 0;
	if (!(preset->flags & HMI_PRESET_FLAG_USE_MALLOC) || (req->flags & HMI_PROCESS_INCLUDE_TAIL))
		tailSamples = (uint32)((double)preset->inputFmt->sampleRate * tailMs * 0.001);

	uint32 outputBytes = (sourceSamples + tailSamples) * preset->outputFmt->blockAlign;
	if (req->flags & HMI_PROCESS_QUERY_OUTPUT_SIZE) {
		req->dstLen = outputBytes;

		if (req->flags & HMI_PROCESS_OVERRIDE_INPUT_FORMAT)
			preset->inputFmt = savedInput;

		if (req->flags & HMI_PROCESS_OVERRIDE_OUTPUT_FORMAT)
			preset->outputFmt = savedOutput;

		if (req->flags & HMI_PROCESS_DERIVE_OUTPUT_FORMAT)
			preset->outputFmt = savedOutput;

		return 0;
	}

	if (req->flags & HMI_PROCESS_ALLOCATE_OUTPUT) {
		req->dstBuf = (uint8 *)malloc(outputBytes);
		if (!req->dstBuf)
			return 1;

		req->dstLen = outputBytes;
	}

	resetMixBuffers(preset);

	if (!(preset->flags & HMI_PRESET_FLAG_USE_MALLOC) ||
		!(preset->flags & HMI_PRESET_FLAG_EFFECTS_INITIALIZED)) {
		for (HMIEffectNode *n = preset->effectChain; n; n = n->next) {
			n->outputBufSize = (int)(sourceSamples + tailSamples + preset->chunkSizeInSamples);
			n->interface->initEffect(preset, n);
		}

		if (preset->flags & HMI_PRESET_FLAG_USE_MALLOC)
			preset->flags |= HMI_PRESET_FLAG_EFFECTS_INITIALIZED;
	}

	uint8 *src = req->srcBuf;
	uint8 *dst = req->dstBuf;
	uint32 remaining = sourceSamples;
	int clamp = (req->flags & HMI_PROCESS_DISABLE_CLAMPING) == 0;

	while (remaining) {
		uint32 count = remaining < preset->chunkSizeInSamples ? remaining : preset->chunkSizeInSamples;
		preset->chunkSize = count;
		convertPCMToFloat(preset, src, count);
		processEffectChain(preset);
		convertFloatToPCM(preset, dst, count, clamp);
		src += count * preset->inputFmt->blockAlign;
		dst += count * preset->outputFmt->blockAlign;
		remaining -= count;
	}

	remaining = tailSamples;
	while (remaining) {
		uint32 count = remaining < preset->chunkSizeInSamples ? remaining : preset->chunkSizeInSamples;
		preset->chunkSize = count;
		memset(preset->floatBufLeftActive, 0, count * sizeof(float));
		memset(preset->floatBufRightActive, 0, count * sizeof(float));
		processEffectChain(preset);
		convertFloatToPCM(preset, dst, count, clamp);
		dst += count * preset->outputFmt->blockAlign;
		remaining -= count;
	}

	if (req->flags & HMI_PROCESS_OVERRIDE_INPUT_FORMAT)
		preset->inputFmt = savedInput;

	if (req->flags & HMI_PROCESS_OVERRIDE_OUTPUT_FORMAT)
		preset->outputFmt = savedOutput;

	if (req->flags & HMI_PROCESS_DERIVE_OUTPUT_FORMAT)
		preset->outputFmt = savedOutput;

	return 0;
}

int HMIFxLib::hmiFXPresetProcess(HMILibrary *library, HMIProcessRequest *req) {
	if (req->flags & HMI_PROCESS_LOOKUP_ONLY) {
		req->presetPtr = nullptr;
		return hmiFXPresetGetByName(library, req->effectName, &req->presetPtr);
	}

	HMIPreset *preset = nullptr;
	if (req->flags & HMI_PROCESS_USE_PRESET) {
		preset = req->presetPtr;
	} else if (req->flags & HMI_PROCESS_FIND_PRESET_BY_NAME) {
		int result = hmiFXPresetGetByName(library, req->effectName, &preset);
		if (result)
			return result;

	} else {
		return 14;
	}

	return processPreset(preset, req);
}

int HMIFxLib::hmiFXPresetProcessEx(HMIProcessRequest *req) {
	if (req->flags & HMI_PROCESS_LOOKUP_ONLY)
		return 15;

	if (!(req->flags & HMI_PROCESS_USE_PRESET))
		return 14;

	return processPreset(req->presetPtr, req);
}

int HMIFxLib::readFile(HMIFileDescriptor *d, Common::SeekableReadStream &stream) {
	memset(d, 0, sizeof(*d));
	const int64 streamSize = stream.size();
	if (streamSize < 0 || streamSize > 0x7FFFFBFF)
		return 0;

	d->fileSize = (int)streamSize;
	d->fileSizeExp = d->fileSize + 1024;
	if (!stream.seek(0))
		return 0;

	d->fileBuffer = (uint8 *)malloc(d->fileSizeExp);
	if (!d->fileBuffer)
		return 0;

	if ((int)stream.read(d->fileBuffer, d->fileSize) != d->fileSize) {
		free(d->fileBuffer);
		d->fileBuffer = nullptr;
		return 0;
	}

	d->ptrToDescription = d->fileBuffer;
	return 1;
}

int HMIFxLib::flushFile(HMIFileDescriptor *d) {
	free(d->fileBuffer);
	d->fileBuffer = nullptr;
	return 1;
}

int HMIFxLib::findSection(HMIFileDescriptor *d, const char *section) {
	uint8 *end = d->fileBuffer + d->fileSize;
	for (uint8 *p = d->fileBuffer; p < end; ++p) {
		if (*p != '[')
			continue;

		uint8 *name = p + 1;
		const char *s = section;
		while (name < end && *s && *name == (uint8)*s) {
			++name;
			++s;
		}

		if (!*s && name < end && *name == ']') {
			while (name < end && *name != '\n')
				++name;

			if (name < end)
				++name;

			d->firstPresetPos = (int)(name - d->fileBuffer);
			d->ptrToDescription = name;
			d->anotherPtrToDescription = name;
			d->ptrToFirstPresetSection = p;
			return 1;
		}
	}

	return 0;
}

int HMIFxLib::findKey(HMIFileDescriptor *d, const char *key) {
	uint8 *p = d->ptrToDescription;
	uint8 *end = d->fileBuffer + d->fileSize;

	while (p < end && *p != '[') {
		if (*p == (uint8)*key) {
			uint8 *start = p;
			const char *s = key;

			while (p < end && *s && *p == (uint8)*s) {
				++p;
				++s;
			}

			if (!*s) {
				while (p < end && *p != '=' && *p != '\r') {
					++p;
				}

				d->ptrToEffectParams = p < end && *p == '=' ? p + 1 : nullptr;
				d->currentPos = nullptr;
				d->ptrToFirstEffectSection = start;
				return 1;
			}

			p = start;
		}

		++p;
	}

	return 0;
}

int HMIFxLib::readInt(HMIFileDescriptor *d, int *value) {
	uint8 *p = d->currentPos ? d->currentPos : d->ptrToEffectParams;
	if (!p)
		return 0;

	while (*p == ' ')
		++p;

	if (*p == '\r' || *p == '\n' || !*p)
		return 0;

	char token[32];
	int n = 0;
	while (*p != ',' && *p != ' ' && *p != '\r' && *p != '\n' && n != 31)
		token[n++] = (char)*p++;

	token[n] = 0;

	while (*p == ' ')
		++p;

	if (*p == ',')
		++p;

	d->currentPos = p;

	if (!n)
		return 0;

	*value = (int)strtol(token, 0, token[0] == '0' && (token[1] == 'x' || token[1] == 'X') ? 16 : 10);
	return 1;
}

int HMIFxLib::readCSV(HMIFileDescriptor *d, char *buffer, int size) {
	uint8 *p = d->currentPos ? d->currentPos : d->ptrToEffectParams;
	if (!p)
		return 0;

	while (*p == ' ')
		++p;

	int n = 0;
	if (*p == '"') {
		++p;
		while (*p != '\r' && *p != '\n' && n != size - 1) {
			if (*p == '"') {
				if (p[1] != '"') {
					++p;
					break;
				}

				++p;
			}

			buffer[n++] = (char)*p++;
		}
	} else {
		while (*p != ',' && *p != '\r' && *p != '\n' && n != size - 1)
			buffer[n++] = (char)*p++;
	}

	buffer[n] = 0;

	while (*p != ',' && *p != '\r' && *p != '\n')
		++p;

	if (*p == ',')
		++p;

	d->currentPos = p;
	return 1;
}

int HMIFxLib::readSectionInt(HMIFileDescriptor *d, const char *key, int *value) {
	return findKey(d, key) && readInt(d, value);
}
int HMIFxLib::readSectionString(HMIFileDescriptor *d, const char *key, char *value, int size) {
	return findKey(d, key) && readCSV(d, value, size);
}

int HMIFxLib::hmiFXPresetLoadLibrary(HMILibrary **outLib, const char *fileName) {
	Common::File file;

	if (!file.open(fileName))
		return 10;

	return hmiFXPresetLoadLibrary(outLib, file);
}

int HMIFxLib::hmiFXPresetLoadLibrary(HMILibrary **outLib, Common::SeekableReadStream &stream) {
	HMIFileDescriptor d;
	if (!readFile(&d, stream))
		return 10;

	HMILibrary *library = (HMILibrary *)malloc(sizeof(HMILibrary));
	if (!library) {
		flushFile(&d);
		return 1;
	}

	memset(library, 0, sizeof(*library));
	*outLib = library;
	if (findSection(&d, "LIBRARY")) {
		readSectionString(&d, "Description", library->description, 64);
		readSectionString(&d, "Author", library->authorName, 64);
	}

	int value;
	if (findSection(&d, "INPUTFMT")) {
		readSectionInt(&d, "Rate", &value);
		library->inputFmt.sampleRate = value;
		readSectionInt(&d, "BitsPerSample", &value);
		library->inputFmt.bitsPerSample = (uint16)value;
		readSectionInt(&d, "Channels", &value);
		library->inputFmt.numChannels = (uint16)value;
		hmiFXSetWaveFmtEx(&library->inputFmt, library->inputFmt.sampleRate,
						  library->inputFmt.bitsPerSample, library->inputFmt.numChannels);
	}

	if (findSection(&d, "OUTPUTFMT")) {
		readSectionInt(&d, "Rate", &value);
		library->outputFmt.sampleRate = value;
		readSectionInt(&d, "BitsPerSample", &value);
		library->outputFmt.bitsPerSample = (uint16)value;
		readSectionInt(&d, "Channels", &value);
		library->outputFmt.numChannels = (uint16)value;
		hmiFXSetWaveFmtEx(&library->outputFmt, library->outputFmt.sampleRate,
						  library->outputFmt.bitsPerSample, library->outputFmt.numChannels);
	}

	HMIPreset *head = nullptr;
	char query[64];

	for (unsigned int presetId = 0;; ++presetId) {
		hmiFXGenerateQueryString(query, "PRESET", presetId);
		if (!findSection(&d, query))
			break;

		HMIPreset *preset = (HMIPreset *)malloc(sizeof(HMIPreset));
		if (!preset) {
			flushFile(&d);
			return 1;
		}

		memset(preset, 0, sizeof(*preset));
		preset->next = head;
		readSectionString(&d, "Description", preset->name, 64);
		readSectionInt(&d, "Flags", (int *)&preset->flags);
		preset->inputFmt = &library->inputFmt;
		preset->outputFmt = &library->outputFmt;
		preset->chunkSizeInMs = 200;

		if (hmiFXPresetCreate(preset)) {
			flushFile(&d);
			return 1;
		}

		for (unsigned int effectId = 0;; ++effectId) {
			hmiFXGenerateQueryString(query, "Effect", effectId);
			if (!findKey(&d, query))
				break;

			int interfaceId;
			if (!readInt(&d, &interfaceId))
				break;

			HMIInterface *interface;
			if (hmiFXGetInterface(interfaceId, &interface)) {
				flushFile(&d);
				return 7;
			}

			HMIEffectNode *node = (HMIEffectNode *)malloc(interface->effectStructSize());
			if (!node) {
				flushFile(&d);
				return 1;
			}

			memset(node, 0, interface->effectStructSize());
			node->interface = interface;
			readInt(&d, &node->outputBus);
			readInt(&d, (int *)&node->channelMode);
			readInt(&d, &node->sendChannel);

			char token[64];
			int param = 0;

			while (readCSV(&d, token, 64) && token[0])
				interface->setEffectParam(node, param++, (float)atof(token));

			int result = hmiFXPresetAddEffect(preset, node);
			if (result) {
				flushFile(&d);
				return result;
			}
		}

		head = preset;
	}

	library->effectPtr = head;
	flushFile(&d);
	return head ? 0 : 12;
}

int HMIFxLib::hmiFXPresetFreeLibrary(HMILibrary *library) {
	if (!library)
		return 12;

	HMIPreset *preset = library->effectPtr;
	while (preset) {
		HMIEffectNode *node = preset->effectChain;
		while (node) {
			node->interface->uninit(preset, node);
			HMIEffectNode *next = node->next;
			free(node);
			node = next;
		}

		if (preset->flags & HMI_PRESET_FLAG_USE_MALLOC)
			free(preset->floatBufLeft0);

		HMIPreset *next = preset->next;
		free(preset);
		preset = next;
	}

	free(library);
	return 0;
}

} // End of namespace Audio
