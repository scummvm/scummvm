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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

int IMuseDigital::waveInit() {
	if (tracksInit())
		return -1;
	return 0;
}

int IMuseDigital::waveTerminate() {
	return 0;
}

int IMuseDigital::wavePause() {
	Common::StackLock lock(_mutex);
	tracksPause();
	return 0;
}

int IMuseDigital::waveResume() {
	Common::StackLock lock(_mutex);
	tracksResume();
	return 0;
}

void IMuseDigital::waveSaveLoad(Common::Serializer &ser) {
	Common::StackLock lock(_mutex);
	tracksSaveLoad(ser);
}

void IMuseDigital::waveUpdateGroupVolumes() {
	Common::StackLock lock(_mutex);
	tracksSetGroupVol();
}

int IMuseDigital::waveStartSound(int soundId, int priority) {
	Common::StackLock lock(_mutex);
	return tracksStartSound(soundId, priority, 0);
}

int IMuseDigital::waveStopSound(int soundId) {
	Common::StackLock lock(_mutex);
	return tracksStopSound(soundId);
}

int IMuseDigital::waveStopAllSounds() {
	Common::StackLock lock(_mutex);
	return tracksStopAllSounds();
}

int IMuseDigital::waveGetNextSound(int soundId) {
	return tracksGetNextSound(soundId);
}

int IMuseDigital::waveSetParam(int soundId, int opcode, int value) {
	Common::StackLock lock(_mutex);
	return tracksSetParam(soundId, opcode, value);
}

int IMuseDigital::waveGetParam(int soundId, int opcode) {
	Common::StackLock lock(_mutex);
	return tracksGetParam(soundId, opcode);
}

int IMuseDigital::waveSetHook(int soundId, int hookId) {
	return tracksSetHook(soundId, hookId);
}

int IMuseDigital::waveGetHook(int soundId) {
	return tracksGetHook(soundId);
}

int IMuseDigital::waveStartStream(int soundId, int priority, int bufferId) {
	if (soundId == 0)
		return -1;

	Common::StackLock lock(_mutex);
	return tracksStartSound(soundId, priority, bufferId);
}

int IMuseDigital::waveSwitchStream(int oldSoundId, int newSoundId, int fadeLengthMs, int fadeSyncFlag2, int fadeSyncFlag1) {
	Common::StackLock lock(_mutex);
	return dispatchSwitchStream(oldSoundId, newSoundId, fadeLengthMs, fadeSyncFlag2, fadeSyncFlag1);
}

int IMuseDigital::waveSwitchStream(int oldSoundId, int newSoundId, uint8 *crossfadeBuffer, int crossfadeBufferSize, int vocLoopFlag) {
	Common::StackLock lock(_mutex);
	return dispatchSwitchStream(oldSoundId, newSoundId, crossfadeBuffer, crossfadeBufferSize, vocLoopFlag);
}

int IMuseDigital::waveProcessStreams() {
	Common::StackLock lock(_mutex);
	return streamerProcessStreams();
}

void IMuseDigital::waveQueryStream(int soundId, int32 &bufSize, int32 &criticalSize, int32 &freeSpace, int &paused) {
	Common::StackLock lock(_mutex);
	tracksQueryStream(soundId, bufSize, criticalSize, freeSpace, paused);
}

int IMuseDigital::waveFeedStream(int soundId, uint8 *srcBuf, int32 sizeToFeed, int paused) {
	Common::StackLock lock(_mutex);
	return tracksFeedStream(soundId, srcBuf, sizeToFeed, paused);
}

int IMuseDigital::waveLipSync(int soundId, int syncId, int msPos, int32 &width, int32 &height) {
	return tracksLipSync(soundId, syncId, msPos, width, height);
}

} // End of namespace Scumm
