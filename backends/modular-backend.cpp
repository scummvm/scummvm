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
#include "gui/message.h"

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
	if (_eventManager != 0)
		delete _eventManager;
	if (_graphicsManager != 0)
		delete _graphicsManager;
	if (_mixer != 0)
		delete _mixer;
	if (_audiocdManager != 0)
		delete _audiocdManager;
	if (_savefileManager != 0)
		delete _savefileManager;
	if (_timerManager != 0)
		delete _timerManager;
	if (_mutexManager != 0)
		delete _mutexManager;
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

const OSystem::GraphicsMode *ModularBackend::getSupportedGraphicsModes() const {
	return _graphicsManager->getSupportedGraphicsModes();
}

int ModularBackend::getDefaultGraphicsMode() const {
	return _graphicsManager->getDefaultGraphicsMode();
}

bool ModularBackend::setGraphicsMode(int mode) {
	return _graphicsManager->setGraphicsMode(mode);
}

int ModularBackend::getGraphicsMode() const {
	return _graphicsManager->getGraphicsMode();
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat ModularBackend::getScreenFormat() const {
	return _graphicsManager->getScreenFormat();
}

Common::List<Graphics::PixelFormat> ModularBackend::getSupportedFormats() {
	return _graphicsManager->getSupportedFormats();
}
#endif

void ModularBackend::initSize(uint w, uint h, const Graphics::PixelFormat *format ) {
	_graphicsManager->initSize(w, h, format);
}

int ModularBackend::getScreenChangeID() const {
	return _graphicsManager->getScreenChangeID();
}

void ModularBackend::beginGFXTransaction() {
	_graphicsManager->beginGFXTransaction();
}

OSystem::TransactionError ModularBackend::endGFXTransaction() {
	return _graphicsManager->endGFXTransaction();
}

int16 ModularBackend::getHeight() {
	return _graphicsManager->getHeight();
}

int16 ModularBackend::getWidth() {
	return _graphicsManager->getWidth();
}

void ModularBackend::setPalette(const byte *colors, uint start, uint num) {
	_graphicsManager->setPalette(colors, start, num);
}

void ModularBackend::grabPalette(byte *colors, uint start, uint num) {
	_graphicsManager->grabPalette(colors, start, num);
}

void ModularBackend::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	_graphicsManager->copyRectToScreen(buf, pitch, x, y, w, h);
}

Graphics::Surface *ModularBackend::lockScreen() {
	return _graphicsManager->lockScreen();
}

void ModularBackend::unlockScreen() {
	_graphicsManager->unlockScreen();
}

void ModularBackend::fillScreen(uint32 col) {
	_graphicsManager->fillScreen(col);
}

void ModularBackend::updateScreen() {
	_graphicsManager->updateScreen();
}

void ModularBackend::setShakePos(int shakeOffset) {
	_graphicsManager->setShakePos(shakeOffset);
}
void ModularBackend::setFocusRectangle(const Common::Rect& rect) {
	_graphicsManager->setFocusRectangle(rect);
}

void ModularBackend::clearFocusRectangle() {
	_graphicsManager->clearFocusRectangle();
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

void ModularBackend::setCursorPalette(const byte *colors, uint start, uint num) {
	_graphicsManager->setCursorPalette(colors, start, num);
}

void ModularBackend::disableCursorPalette(bool disable) {
	_graphicsManager->disableCursorPalette(disable);
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
#ifdef USE_OSD
	_graphicsManager->displayMessageOnOSD(msg);
#else
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
#endif
}

Common::SaveFileManager *ModularBackend::getSavefileManager() {
	assert(_savefileManager);
	return _savefileManager;
}

FilesystemFactory *ModularBackend::getFilesystemFactory() {
	assert(_fsFactory);
	return _fsFactory;
}
