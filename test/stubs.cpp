#include "backends/events/default/default-events.h"
#include "backends/timer/default/default-timer.h"
#include "backends/saves/default/default-saves.h"
#include "common/savefile.h"
#include "gui/message.h"
#include "backends/base-backend.h"
#include "backends/mixer/null/null-mixer.h"
#include "engines/engine.h"

Engine *g_engine = 0;

GUI::Debugger *Engine::getOrCreateDebugger() {
	assert(0);
}

NullMixerManager::NullMixerManager() : MixerManager() {
	_outputRate = 22050;
	_callsCounter = 0;
	_samples = 8192;
	while (_samples * 16 > _outputRate * 2)
		_samples >>= 1;
	_samplesBuf = new uint8[_samples * 4];
}

NullMixerManager::~NullMixerManager() {
	delete []_samplesBuf;
}

void NullMixerManager::update(uint8 callbackPeriod) {
	if (_audioSuspended) {
		return;
	}

	assert(0);
}

void NullMixerManager::init() {
	assert(0);
}

void NullMixerManager::suspendAudio() {
	_audioSuspended = true;
}

int NullMixerManager::resumeAudio() {
	if (!_audioSuspended) {
		return -2;
	}
	_audioSuspended = false;
	return 0;
}

DefaultEventManager::DefaultEventManager(Common::EventSource *boss) :
	_buttonState(0),
	_modifierState(0),
	_shouldQuit(false),
	_shouldReturnToLauncher(false),
	_confirmExitDialogActive(false) {
}

DefaultEventManager::~DefaultEventManager() {
}

void DefaultEventManager::init() {
}

bool DefaultEventManager::pollEvent(Common::Event &event) {
	assert(0);
}

void DefaultEventManager::pushEvent(const Common::Event &event) {
	assert(0);
}

void DefaultEventManager::purgeMouseEvents() {
	assert(0);
}

Common::Keymap *DefaultEventManager::getGlobalKeymap() {
	assert(0);
}

DefaultTimerManager::DefaultTimerManager() :
	_timerCallbackNext(0),
	_head(0) {
}

DefaultTimerManager::~DefaultTimerManager() {
}

void DefaultTimerManager::handler() {
	assert(0);
}

void DefaultTimerManager::checkTimers(uint32 interval) {
	assert(0);
}

bool DefaultTimerManager::installTimerProc(TimerProc callback, int32 interval, void *refCon, const Common::String &id) {
	assert(0);
}

void DefaultTimerManager::removeTimerProc(TimerProc callback) {
	assert(0);
}

DefaultSaveFileManager::DefaultSaveFileManager() {
}

DefaultSaveFileManager::DefaultSaveFileManager(const Common::String &defaultSavepath) {
}


void DefaultSaveFileManager::checkPath(const Common::FSNode &dir) {
	assert(0);
}

void DefaultSaveFileManager::updateSavefilesList(Common::StringArray &lockedFiles) {
	assert(0);
}

Common::StringArray DefaultSaveFileManager::listSavefiles(const Common::String &pattern) {
	assert(0);
}

Common::InSaveFile *DefaultSaveFileManager::openRawFile(const Common::String &filename) {
	assert(0);
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const Common::String &filename) {
	assert(0);
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const Common::String &filename, bool compress) {
	assert(0);
}

bool DefaultSaveFileManager::removeSavefile(const Common::String &filename) {
	assert(0);
}

Common::String DefaultSaveFileManager::getSavePath() const {
	assert(0);
}

void DefaultSaveFileManager::assureCached(const Common::String &savePathName) {
	assert(0);
}

Common::String DefaultSaveFileManager::concatWithSavesPath(Common::String name) {
	assert(0);
}

bool Common::SaveFileManager::copySavefile(const Common::String &oldFilename, const Common::String &newFilename, bool compress) {
	assert(0);
}

bool Common::SaveFileManager::renameSavefile(const Common::String &oldFilename, const Common::String &newFilename, bool compress) {
	assert(0);
}

Common::String Common::SaveFileManager::popErrorDesc() {
	Common::String err = _errorDesc;
	clearError();

	return err;
}

void BaseBackend::displayMessageOnOSD(const Common::U32String &msg) {
}

void BaseBackend::initBackend() {
	OSystem::initBackend();
}

void BaseBackend::fillScreen(uint32 col) {
}

void EventsBaseBackend::initBackend() {
	BaseBackend::initBackend();
}
