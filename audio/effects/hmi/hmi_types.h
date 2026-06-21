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

#ifndef AUDIO_EFFECTS_HMI_TYPES_H
#define AUDIO_EFFECTS_HMI_TYPES_H

#include "common/scummsys.h"

namespace Audio {

/* HMIPreset::flags */
#define HMI_PRESET_FLAG_UNKNOWN_28          0x10000000U
#define HMI_PRESET_FLAG_EFFECTS_INITIALIZED 0x20000000U
#define HMI_PRESET_FLAG_USE_MALLOC          0x80000000U

/* HMIProcessRequest::flags */
#define HMI_PROCESS_USE_PRESET              0x00000001U
#define HMI_PROCESS_FIND_PRESET_BY_NAME     0x00000002U
#define HMI_PROCESS_ALLOCATE_OUTPUT         0x00000004U
#define HMI_PROCESS_OVERRIDE_INPUT_FORMAT   0x00000008U
#define HMI_PROCESS_OVERRIDE_OUTPUT_FORMAT  0x00000010U
#define HMI_PROCESS_LOOKUP_ONLY             0x00000020U
#define HMI_PROCESS_QUERY_OUTPUT_SIZE       0x00000040U
#define HMI_PROCESS_INCLUDE_TAIL            0x00000080U
#define HMI_PROCESS_DERIVE_OUTPUT_FORMAT    0x00000200U
#define HMI_PROCESS_DISABLE_CLAMPING        0x00000400U

/* HMIEffectNode::channelMode */
#define HMI_EFFECT_CHANNEL_MONO             0x00000001U
#define HMI_EFFECT_CHANNEL_STEREO           0x00000002U
#define HMI_EFFECT_STEREO_ROUTING_MASK      0x0000000AU

/* HMIFileDescriptor::fileFlags */
#define HMI_FILE_FLAG_DIRTY                 0x00008000U

class HMIInterface;

struct HMIFormat;
struct HMIEffectNode;
struct HMIPreset;

struct HMIFormat {
	uint16 fmtBeginSection;
	uint16 numChannels;
	uint32 sampleRate;
	uint32 byteRate;
	uint16 blockAlign;
	uint16 bitsPerSample;
	uint16 fmtEndSection;
};

struct HMIEffectNode {
	uint32 flags;
	int outputBus;
	int sendChannel;
	uint32 channelMode;
	int outputBufSize;
	HMIInterface *interface;
	HMIEffectNode *next;
};

#define HMI_EFFECT_NODE_HEADER \
	uint32 flags;              \
	int outputBus;             \
	int sendChannel;           \
	uint32 channelMode;        \
	int outputBufSize;         \
	HMIInterface *interface;   \
	HMIEffectNode *next

struct HMIPreset {
	uint32 flags;
	char name[64];
	int chunkSizeInMs;
	float *floatBufLeftActive;
	float *floatBufRightActive;
	float *floatBufLeftAlt;
	float *floatBufRightAlt;
	float *floatBufLeft0;
	float *floatBufLeft1;
	float *floatBufRight0;
	float *floatBufRight1;
	int pingPongIndex;
	uint32 chunkSizeInSamples;
	uint32 chunkSize;
	int lockLevel;
	int allocatedHeapSize;
	HMIEffectNode *effectChain;
	HMIFormat *outputFmt;
	HMIFormat *inputFmt;
	HMIPreset *next;
};

struct HMIInitData {
	int heapSize;
	void *heapPtr;
};

struct HMILibrary {
	char description[64];
	char authorName[64];
	uint8 data[20];
	HMIPreset *effectPtr;
	HMIFormat inputFmt;
	HMIFormat outputFmt;
};

struct HMIProcessRequest {
	uint32 flags;
	HMIPreset *presetPtr;
	char *effectName;
	uint8 *dstBuf;
	uint32 dstLen;
	uint8 *srcBuf;
	uint32 srcLen;
	HMIFormat *inputFmt;
	HMIFormat *outputFmt;
};

struct HMIFileDescriptor {
	int fileFlags;
	char filename[128];
	uint8 *fileBuffer;
	int fileSize;
	int fileSizeExp;
	uint8 *ptrToDescription;
	int firstPresetPos;
	uint8 *ptrToFirstPresetSection;
	uint8 *ptrToFirstEffectSection;
	uint8 *ptrToEffectParams;
	uint8 *currentPos;
	uint8 *anotherPtrToDescription;
};

struct HMIFilter1Node {
	HMI_EFFECT_NODE_HEADER;
	int filterType;
	float cutoffFrequency;
	float centerFrequency;
	float bandWidth;
	float coeff_b0, coeff_b1, coeff_b2, coeff_a1, coeff_a2;
	float state_x1, state_x2, state_x3, state_y1;
	float state_x1b, state_x2b, state_x3b, state_y1b;
};

struct HMIRingModulatorNode {
	HMI_EFFECT_NODE_HEADER;
	float frequency;
	float modulatorType;
	float modulateOutOfPhase;
	float dryOut;
	float wetOut;
	float phase;
};

struct HMIResonatorNode {
	HMI_EFFECT_NODE_HEADER;
	float cutoffFrequency;
	float bandWidth;
	float resonance;
	float coeff1;
	float coeff2;
	float stateX1;
	float stateX2;
};

struct HMIReverbEchoStage {
	float *buffer;
	int writeHead;
	int readHead;
	int bufSizeInSamples;
	float lastOutput;
	float decayCoeff;
};

struct HMIReverb1Node {
	HMI_EFFECT_NODE_HEADER;
	float reverbMax;
	float reverbTime;
	float preDelay;
	int preDelaySamples;
	int bufSizeBytes;
	HMIReverbEchoStage echoStages[4];
	float dryOut;
	float wetOut;
};

struct HMIReverb2Node {
	HMI_EFFECT_NODE_HEADER;
	float reverbMax;
	float reverbTime;
	float preDelay;
	int preDelaySamples;
	int bufSizeBytes;
	HMIReverbEchoStage echoStages[6];
	float dryOut;
	float wetOut;
};

struct HMIPhasorNode {
	HMI_EFFECT_NODE_HEADER;
	float rate;
	float depth;
	float centerFrequency;
	float feedback;
	float dryOut;
	float wetOut;
	float phase;
	float b0, b1, a1;
	float state_x1L, state_y1L, state_x1R, state_y1R;
	float state_x2L, state_y2L, state_x2R, state_y2R;
	float coeff;
	float modPhase;
};

struct HMIEnvelopeNode {
	HMI_EFFECT_NODE_HEADER;
	float envPoints;
	float pointTime[8];
	float pointAmp[8];
	float initGuard;
	int segRemaining[8];
	int currentSegment;
	float currentAmp;
	float segSlopes[8];
};

struct HMIMonoDelayNode {
	HMI_EFFECT_NODE_HEADER;
	float maxDelayTime;
	float delayTime;
	float unused0;
	float feedback;
	float dryOut;
	float wetOut;
	float *delayBuf;
	int unused1;
	int bufSizeBytes;
	int unused2;
	int writeIndex;
	int pad;
};

struct HMIStereoDelayNode {
	HMI_EFFECT_NODE_HEADER;
	float maxDelayTime;
	float delayTimeL;
	float delayTimeR;
	float feedback;
	float dryOut;
	float wetOut;
	float *delayBufL;
	float *delayBufR;
	int bufSizeBytesL;
	int bufSizeBytesR;
	int writeIndexL;
	int writeIndexR;
};

#undef HMI_EFFECT_NODE_HEADER

} // End of namespace Audio

#endif
