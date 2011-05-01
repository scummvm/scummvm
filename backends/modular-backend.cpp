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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/modular-backend.h"

#include "backends/fs/fs-factory.h"
#include "backends/audiocd/audiocd.h"
#include "backends/graphics/graphics.h"
#include "backends/mutex/mutex.h"

#include "audio/mixer.h"
#include "common/events.h"
#include "gui/message.h"
#include "graphics/pixelformat.h"

ModularBackend::ModularBackend()
	:
	_fsFactory(0),
	_eventManager(0),
	_savefileManager(0),
	_timerManager(0),
	_mutexManager(0),
	_graphicsManager(0),
	_mixer(0),
	_audiocdManager(0) {

}

ModularBackend::~ModularBackend() {
	delete _fsFactory;
	_fsFactory = 0;
	delete _graphicsManager;
	_graphicsManager = 0;
	delete _eventManager;
	_eventManager = 0;
	delete _mixer;
	_mixer = 0;
	delete _audiocdManager;
	_audiocdManager = 0;
	delete _savefileManager;
	_savefileManager = 0;
	delete _timerManager;
	_timerManager = 0;
	delete _mutexManager;
	_mutexManager = 0;
}

bool ModularBackend::hasFeature(Feature f) {
	return _graphicsManager->hasFeature(f);
}

void ModularBackend::setFeatureState(Feature f, bool enable) {
	return _graphicsManager->setFeatureState(f, enable);
}

bool ModularBackend::getFeatureState(Feature f) {
	return _graphicsManager->getFeatureState(f);
}

GraphicsManager *ModularBackend::getGraphicsManager() {
	assert(_graphicsManager);
	return (GraphicsManager *)_graphicsManager;
}

void ModularBackend::launcherInitSize(uint w, uint h) {
	_graphicsManager->launcherInitSize(w, h);
}

byte *ModularBackend::setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d) {
	return _graphicsManager->setupScreen(screenW, screenH, fullscreen, accel3d);
}

int16 ModularBackend::getHeight() {
	return _graphicsManager->getHeight();
}

int16 ModularBackend::getWidth() {
	return _graphicsManager->getWidth();
}

void ModularBackend::updateScreen() {
	_graphicsManager->updateScreen();
}

void ModularBackend::showOverlay() {
	_graphicsManager->showOverlay();
}

void ModularBackend::hideOverlay() {
	_graphicsManager->hideOverlay();
}

Graphics::PixelFormat ModularBackend::getOverlayFormat() const {
	return _graphicsManager->getOverlayFormat();
}

void ModularBackend::clearOverlay() {
	_graphicsManager->clearOverlay();
}

void ModularBackend::grabOverlay(OverlayColor *buf, int pitch) {
	_graphicsManager->grabOverlay(buf, pitch);
}

void ModularBackend::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	_graphicsManager->copyRectToOverlay(buf, pitch, x, y, w, h);
}

int16 ModularBackend::getOverlayHeight() {
	return _graphicsManager->getOverlayHeight();
}

int16 ModularBackend::getOverlayWidth() {
	return _graphicsManager->getOverlayWidth();
}

bool ModularBackend::showMouse(bool visible) {
	return _graphicsManager->showMouse(visible);
}

void ModularBackend::warpMouse(int x, int y) {
	_graphicsManager->warpMouse(x, y);
}

void ModularBackend::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	_graphicsManager->setMouseCursor(buf, w, h, hotspotX, hotspotY, keycolor, cursorTargetScale, format);
}

Common::TimerManager *ModularBackend::getTimerManager() {
	assert(_timerManager);
	return _timerManager;
}

Common::EventManager *ModularBackend::getEventManager() {
	assert(_eventManager);
	return _eventManager;
}

OSystem::MutexRef ModularBackend::createMutex() {
	assert(_mutexManager);
	return _mutexManager->createMutex();
}

void ModularBackend::lockMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->lockMutex(mutex);
}

void ModularBackend::unlockMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->unlockMutex(mutex);
}

void ModularBackend::deleteMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->deleteMutex(mutex);
}

Audio::Mixer *ModularBackend::getMixer() {
	assert(_mixer);
	return (Audio::Mixer *)_mixer;
}

AudioCDManager *ModularBackend::getAudioCDManager() {
	assert(_audiocdManager);
	return _audiocdManager;
}

void ModularBackend::displayMessageOnOSD(const char *msg) {
	_graphicsManager->displayMessageOnOSD(msg);
}

Common::SaveFileManager *ModularBackend::getSavefileManager() {
	assert(_savefileManager);
	return _savefileManager;
}

FilesystemFactory *ModularBackend::getFilesystemFactory() {
	assert(_fsFactory);
	return _fsFactory;
}
