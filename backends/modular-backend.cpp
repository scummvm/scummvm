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

#include "backends/modular-backend.h"

#include "backends/audiocd/audiocd.h"
#include "backends/graphics/graphics.h"
#include "backends/graphics3d/graphics3d.h"
#include "backends/mixer/mixer.h"
#include "backends/mutex/mutex.h"
#include "gui/EventRecorder.h"

#include "common/timer.h"
#include "graphics/pixelformat.h"
#include "graphics/pixelbuffer.h"

ModularGraphicsBackend::ModularGraphicsBackend()
	:
	_graphicsManager(0) {

}

ModularGraphicsBackend::~ModularGraphicsBackend() {
	delete _graphicsManager;
	_graphicsManager = 0;
}

bool ModularGraphicsBackend::hasFeature(Feature f) {
	return _graphicsManager->hasFeature(f);
}

void ModularGraphicsBackend::setFeatureState(Feature f, bool enable) {
	_graphicsManager->setFeatureState(f, enable);
}

bool ModularGraphicsBackend::getFeatureState(Feature f) {
	return _graphicsManager->getFeatureState(f);
}

GraphicsManager *ModularGraphicsBackend::getGraphicsManager() {
	assert(_graphicsManager);
	return (GraphicsManager *)_graphicsManager;
}

const OSystem::GraphicsMode *ModularGraphicsBackend::getSupportedGraphicsModes() const {
	return _graphicsManager->getSupportedGraphicsModes();
}

int ModularGraphicsBackend::getDefaultGraphicsMode() const {
	return _graphicsManager->getDefaultGraphicsMode();
}

bool ModularGraphicsBackend::setGraphicsMode(int mode, uint flags) {
	return _graphicsManager->setGraphicsMode(mode, flags);
}

int ModularGraphicsBackend::getGraphicsMode() const {
	return _graphicsManager->getGraphicsMode();
}

const OSystem::GraphicsMode *ModularGraphicsBackend::getSupportedShaders() const {
	return _graphicsManager->getSupportedShaders();
}

int ModularGraphicsBackend::getDefaultShader() const {
	return _graphicsManager->getDefaultShader();
}

bool ModularGraphicsBackend::setShader(int id) {
	return _graphicsManager->setShader(id);
}

int ModularGraphicsBackend::getShader() const {
	return _graphicsManager->getShader();
}

const OSystem::GraphicsMode *ModularGraphicsBackend::getSupportedStretchModes() const {
	return _graphicsManager->getSupportedStretchModes();
}

int ModularGraphicsBackend::getDefaultStretchMode() const {
	return _graphicsManager->getDefaultStretchMode();
}

bool ModularGraphicsBackend::setStretchMode(int mode) {
	return _graphicsManager->setStretchMode(mode);
}

int ModularGraphicsBackend::getStretchMode() const {
	return _graphicsManager->getStretchMode();
}

#ifdef USE_RGB_COLOR

Graphics::PixelFormat ModularGraphicsBackend::getScreenFormat() const {
	return _graphicsManager->getScreenFormat();
}

Common::List<Graphics::PixelFormat> ModularGraphicsBackend::getSupportedFormats() const {
	return _graphicsManager->getSupportedFormats();
}

#endif

void ModularGraphicsBackend::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	_graphicsManager->initSize(w, h, format);
}

void ModularGraphicsBackend::initSizeHint(const Graphics::ModeList &modes) {
	_graphicsManager->initSizeHint(modes);
}

int ModularGraphicsBackend::getScreenChangeID() const {
	return _graphicsManager->getScreenChangeID();
}

void ModularGraphicsBackend::beginGFXTransaction() {
	_graphicsManager->beginGFXTransaction();
}

OSystem::TransactionError ModularGraphicsBackend::endGFXTransaction() {
	return _graphicsManager->endGFXTransaction();
}

int16 ModularGraphicsBackend::getHeight() {
	return _graphicsManager->getHeight();
}

int16 ModularGraphicsBackend::getWidth() {
	return _graphicsManager->getWidth();
}

PaletteManager *ModularGraphicsBackend::getPaletteManager() {
	return _graphicsManager;
}

void ModularGraphicsBackend::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_graphicsManager->copyRectToScreen(buf, pitch, x, y, w, h);
}

Graphics::Surface *ModularGraphicsBackend::lockScreen() {
	return _graphicsManager->lockScreen();
}

void ModularGraphicsBackend::unlockScreen() {
	_graphicsManager->unlockScreen();
}

void ModularGraphicsBackend::fillScreen(uint32 col) {
	_graphicsManager->fillScreen(col);
}

void ModularGraphicsBackend::updateScreen() {
#ifdef ENABLE_EVENTRECORDER
	g_eventRec.preDrawOverlayGui();
#endif

	_graphicsManager->updateScreen();

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.postDrawOverlayGui();
#endif
}

void ModularGraphicsBackend::setShakePos(int shakeXOffset, int shakeYOffset) {
	_graphicsManager->setShakePos(shakeXOffset, shakeYOffset);
}
void ModularGraphicsBackend::setFocusRectangle(const Common::Rect& rect) {
	_graphicsManager->setFocusRectangle(rect);
}

void ModularGraphicsBackend::clearFocusRectangle() {
	_graphicsManager->clearFocusRectangle();
}

void ModularGraphicsBackend::showOverlay() {
	_graphicsManager->showOverlay();
}

void ModularGraphicsBackend::hideOverlay() {
	_graphicsManager->hideOverlay();
}

bool ModularGraphicsBackend::isOverlayVisible() const {
	return _graphicsManager->isOverlayVisible();
}

Graphics::PixelFormat ModularGraphicsBackend::getOverlayFormat() const {
	return _graphicsManager->getOverlayFormat();
}

void ModularGraphicsBackend::clearOverlay() {
	_graphicsManager->clearOverlay();
}

void ModularGraphicsBackend::grabOverlay(void *buf, int pitch) {
	_graphicsManager->grabOverlay(buf, pitch);
}

void ModularGraphicsBackend::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_graphicsManager->copyRectToOverlay(buf, pitch, x, y, w, h);
}

int16 ModularGraphicsBackend::getOverlayHeight() {
	return _graphicsManager->getOverlayHeight();
}

int16 ModularGraphicsBackend::getOverlayWidth() {
	return _graphicsManager->getOverlayWidth();
}

bool ModularGraphicsBackend::showMouse(bool visible) {
	return _graphicsManager->showMouse(visible);
}

bool ModularGraphicsBackend::lockMouse(bool visible) {
	return _graphicsManager->lockMouse(visible);
}

void ModularGraphicsBackend::warpMouse(int x, int y) {
	_eventManager->purgeMouseEvents();
	_graphicsManager->warpMouse(x, y);
}

void ModularGraphicsBackend::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	_graphicsManager->setMouseCursor(buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format);
}

void ModularGraphicsBackend::setCursorPalette(const byte *colors, uint start, uint num) {
	_graphicsManager->setCursorPalette(colors, start, num);
}

void ModularGraphicsBackend::displayMessageOnOSD(const Common::U32String &msg) {
	_graphicsManager->displayMessageOnOSD(msg);
}

void ModularGraphicsBackend::displayActivityIconOnOSD(const Graphics::Surface *icon) {
	_graphicsManager->displayActivityIconOnOSD(icon);
}

void ModularGraphicsBackend::saveScreenshot() {
	_graphicsManager->saveScreenshot();
}


ModularMixerBackend::ModularMixerBackend()
	:
	_mixerManager(0) {

}

ModularMixerBackend::~ModularMixerBackend() {
	// _audiocdManager needs to be deleted before _mixerManager to avoid a crash.
	delete _audiocdManager;
	_audiocdManager = 0;
	delete _mixerManager;
	_mixerManager = 0;
}

MixerManager *ModularMixerBackend::getMixerManager() {
	assert(_mixerManager);
	return _mixerManager;
}

Audio::Mixer *ModularMixerBackend::getMixer() {
	assert(_mixerManager);
	return getMixerManager()->getMixer();
}


ModularMutexBackend::ModularMutexBackend()
	:
	_mutexManager(0) {

}

ModularMutexBackend::~ModularMutexBackend() {
	// _timerManager needs to be deleted before _mutexManager to avoid a crash.
	delete _timerManager;
	_timerManager = 0;
	delete _mutexManager;
	_mutexManager = 0;
}

OSystem::MutexRef ModularMutexBackend::createMutex() {
	assert(_mutexManager);
	return _mutexManager->createMutex();
}

void ModularMutexBackend::lockMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->lockMutex(mutex);
}

void ModularMutexBackend::unlockMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->unlockMutex(mutex);
}

void ModularMutexBackend::deleteMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->deleteMutex(mutex);
}
