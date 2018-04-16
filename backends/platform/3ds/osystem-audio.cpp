/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "osystem.h"
#include "audio/mixer.h"

namespace _3DS {

static bool hasAudio = false;

static void audioThreadFunc(void *arg) {
	Audio::MixerImpl *mixer = (Audio::MixerImpl *)arg;
	OSystem_3DS *osys = (OSystem_3DS *)g_system;

	int i;
	const int channel = 0;
	int bufferIndex = 0;
	const int bufferCount = 3;
	const int bufferSize = 80000; // Can't be too small, based on delayMillis duration
	const int sampleRate = mixer->getOutputRate();
	int sampleLen = 0;
	uint32 lastTime = osys->getMillis(true);
	uint32 time = lastTime;
	ndspWaveBuf buffers[bufferCount];

	for (i = 0; i < bufferCount; ++i) {
		memset(&buffers[i], 0, sizeof(ndspWaveBuf));
		buffers[i].data_vaddr = linearAlloc(bufferSize);
		buffers[i].looping = false;
		buffers[i].status = NDSP_WBUF_FREE;
	}

	ndspChnReset(channel);
	ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
	ndspChnSetRate(channel, sampleRate);
	ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

	while (!osys->exiting) {
		osys->delayMillis(100); // Note: Increasing the delay requires a bigger buffer

		time = osys->getMillis(true);
		sampleLen = (time - lastTime) * 22 * 4; // sampleRate / 1000 * channelCount * sizeof(int16);
		lastTime = time;

		if (!osys->sleeping && sampleLen > 0) {
			bufferIndex++;
			bufferIndex %= bufferCount;
			ndspWaveBuf *buf = &buffers[bufferIndex];

			buf->nsamples = mixer->mixCallback(buf->data_adpcm, sampleLen);
			if (buf->nsamples > 0) {
				DSP_FlushDataCache(buf->data_vaddr, bufferSize);
				ndspChnWaveBufAdd(channel, buf);
			}
		}
	}

	for (i = 0; i < bufferCount; ++i)
		linearFree(buffers[i].data_pcm8);
}

void OSystem_3DS::initAudio() {
	_mixer = new Audio::MixerImpl(this, 22050);

	hasAudio = R_SUCCEEDED(ndspInit());
	_mixer->setReady(false);

	if (hasAudio) {
		s32 prio = 0;
		svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
		audioThread = threadCreate(&audioThreadFunc, _mixer, 32 * 1048, prio - 1, -2, false);
	}
}

void OSystem_3DS::destroyAudio() {
	if (hasAudio) {
		threadJoin(audioThread, U64_MAX);
		threadFree(audioThread);
		ndspExit();
	}

	delete _mixer;
	_mixer = 0;
}

Audio::Mixer *OSystem_3DS::getMixer() {
	assert(_mixer);
	return _mixer;
}

} // namespace _3DS
