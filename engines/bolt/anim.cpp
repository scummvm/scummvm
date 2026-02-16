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

#include "bolt/bolt.h"

namespace Bolt {

void *BoltEngine::openRTF(const char *fileName) {
	return nullptr;
}

void BoltEngine::closeRTF(void *rtf) {
}

bool BoltEngine::playRTF(void *rtfFile, int16 animIndex, void *ringBuffer, int32 bufferSize) {
	return false;
}

bool BoltEngine::fillRTFBuffer() {
	return false;
}

void BoltEngine::flushRTFSoundQueue() {
}

bool BoltEngine::maintainRTF(int16 mode, void *outFrameData) {
	return false;
}

bool BoltEngine::isRTFPlaying() {
	return false;
}

void BoltEngine::killRTF() {
}

void BoltEngine::readPacket() {

}

void BoltEngine::preProcessPacket() {
}

void BoltEngine::queuePacket() {
}

void BoltEngine::deQueuePacket() {
}

void BoltEngine::allocPacket() {
}

void BoltEngine::freePacket() {
}

void BoltEngine::resetPlaybackState() {
}

void BoltEngine::sub_12980() {
}

void BoltEngine::prepareAV() {
}

void BoltEngine::maintainAV() {
}

void BoltEngine::stopAV() {
}

bool BoltEngine::playAV(void *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff) {
	return false;
}

void BoltEngine::processPacket() {
}

void BoltEngine::processRL7() {
}

void BoltEngine::processPLTE() {
}

void BoltEngine::initAV() {
}

void BoltEngine::cleanUpAV() {
}

void BoltEngine::startAnimation() {
}

void BoltEngine::maintainAudioPlay() {
}

void BoltEngine::initAnim() {
}

void BoltEngine::cleanUpAnim() {
}

} // End of namespace Bolt
