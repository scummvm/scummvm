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

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/message.h"
#include "gui/saveload.h"

#include "mtropolis/mtropolis.h"
#include "mtropolis/render.h"
#include "mtropolis/runtime.h"

namespace MTropolis {


CompoundVarSaver::CompoundVarSaver(Runtime *runtime, RuntimeObject *object) : _runtime(runtime), _object(object) {
}

bool CompoundVarSaver::writeSave(Common::WriteStream *stream) {
	if (_object == nullptr || !_object->isModifier())
		return false;

	Modifier *modifier = static_cast<Modifier *>(_object);
	Common::SharedPtr<ModifierSaveLoad> saveLoad = modifier->getSaveLoad(_runtime);
	if (!saveLoad)
		return false;

	saveLoad->save(modifier, stream);
	return !stream->err();
}

SaveLoadHooks::~SaveLoadHooks() {
}

void SaveLoadHooks::onLoad(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) {
}

void SaveLoadHooks::onSave(Runtime *runtime, Modifier *saveLoadModifier, Modifier *varModifier) {
}

SaveLoadMechanismHooks::~SaveLoadMechanismHooks() {
}

bool SaveLoadMechanismHooks::canSaveNow(Runtime *runtime) {
	return false;
}

Common::SharedPtr<ISaveWriter> SaveLoadMechanismHooks::createSaveWriter(Runtime *runtime) {
	return nullptr;
}

bool MTropolisEngine::promptSave(ISaveWriter *writer, const Graphics::Surface *screenshotOverride) {
	Common::String desc;
	int slot;

	Common::SharedPtr<GUI::SaveLoadChooser> dialog(new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true));

	slot = dialog->runModalWithCurrentTarget();
	desc = dialog->getResultString();

	if (desc.empty()) {
		// create our own description for the saved game, the user didn't enter it
		desc = dialog->createDefaultSaveDescription(slot);
	}

	if (slot < 0)
		return true;

	Common::String saveFileName = getSaveStateName(slot);

	return save(writer, screenshotOverride, saveFileName, desc);
}

bool MTropolisEngine::save(ISaveWriter *writer, const Graphics::Surface *screenshotOverride, const Common::String &saveFileName, const Common::String &desc) {

	Common::SharedPtr<Common::OutSaveFile> out(_saveFileMan->openForSaving(saveFileName, false));

	ISaveWriter *oldWriter = _saveWriter;

	_saveWriter = writer;

	saveGameStream(out.get(), false);

	_saveWriter = oldWriter;

	getMetaEngine()->appendExtendedSave(out.get(), getTotalPlayTime(), desc, false);

	return true;
}

bool MTropolisEngine::namedSave(ISaveWriter *writer, const Graphics::Surface *screenshotOverride, const Common::String &fileName) {
	return save(writer, screenshotOverride, getUnpromptedSaveFileName(fileName), fileName);
}

bool MTropolisEngine::promptLoad(ISaveReader *reader) {
	Common::String desc;
	int slot;

	{
		Common::SharedPtr<GUI::SaveLoadChooser> dialog(new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false));
		slot = dialog->runModalWithCurrentTarget();
	}

	if (slot < 0)
		return true;

	Common::String saveFileName = getSaveStateName(slot);

	return load(reader, saveFileName);
}

bool MTropolisEngine::load(ISaveReader *reader, const Common::String &saveFileName) {
	Common::SharedPtr<Common::InSaveFile> in(_saveFileMan->openForLoading(saveFileName));
	if (!in) {
		warning("An error occurred while attempting to open save file '%s'", saveFileName.c_str());
		return false;
	}

	uint32 signature = in->readUint32BE();
	uint32 saveFileVersion = in->readUint32BE();
	if (in->err()) {
		GUI::MessageDialog dialog(_("Failed to read version information from save file"));
		dialog.runModal();

		warning("An error occurred while reading the save file version from '%s'", saveFileName.c_str());
		return false;
	}

	if (signature != kSavegameSignature) {
		GUI::MessageDialog dialog(_("Failed to load save, the save file doesn't contain valid version information."));
		dialog.runModal();

		warning("Save file '%s' version is above the current save file version", saveFileName.c_str());
		return false;
	}

	if (saveFileVersion > kCurrentSaveFileVersion) {
		GUI::MessageDialog dialog(_("Saved game was created with a newer version of ScummVM. Unable to load."));
		dialog.runModal();

		warning("Save file '%s' version is above the current save file version", saveFileName.c_str());
		return false;
	}

	if (saveFileVersion < kEarliestSupportedSaveFileVersion) {
		GUI::MessageDialog dialog(_("Saved game was created with an earlier, incompatible version of ScummVM. Unable to load."));
		dialog.runModal();

		warning("An error occurred while reading file '%s'", saveFileName.c_str());
		return false;
	}

	if (!reader->readSave(in.get(), saveFileVersion)) {
		GUI::MessageDialog dialog(_("Failed to load save, an error occurred when reading the save game data."));
		dialog.runModal();

		warning("An error occurred while reading file '%s'", saveFileName.c_str());
		return false;
	}

	return true;
}

Common::String MTropolisEngine::getUnpromptedSaveFileName(const Common::String &fileName) {
	return _targetName + "." + toCaseInsensitive(fileName);
}

bool MTropolisEngine::namedLoad(ISaveReader *reader, const Common::String &fileName) {
	return load(reader, getUnpromptedSaveFileName(fileName));
}

bool MTropolisEngine::autoSave(ISaveWriter *writer) {
	ISaveWriter *oldWriter = _saveWriter;
	bool oldIsTriggeredAutosave = _isTriggeredAutosave;

	_saveWriter = writer;
	_isTriggeredAutosave = true;

	saveAutosaveIfEnabled();

	_saveWriter = oldWriter;
	_isTriggeredAutosave = oldIsTriggeredAutosave;

	return true;
}

const Graphics::Surface *MTropolisEngine::getSavegameScreenshot() const {
	const Graphics::ManagedSurface *screenshotOverride = _runtime->getSaveScreenshotOverride().get();
	if (screenshotOverride)
		return &screenshotOverride->rawSurface();
	else {
		Window *mainWindow = _runtime->getMainWindow().lock().get();
		if (!mainWindow)
			return nullptr;
		return mainWindow->getSurface().get()->surfacePtr();
	}
}

Common::Error MTropolisEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	ISaveWriter *saveWriter = _saveWriter;

	Common::SharedPtr<ISaveWriter> mechanismHookWriter;
	if (!saveWriter) {
		for (Common::SharedPtr<SaveLoadMechanismHooks> &hooks : _runtime->getHacks().saveLoadMechanismHooks) {
			if (hooks->canSaveNow(_runtime.get())) {
				mechanismHookWriter = hooks->createSaveWriter(_runtime.get());
				saveWriter = mechanismHookWriter.get();
				break;
			}
		}
	}

	if (!saveWriter)
		return Common::Error(Common::kWritingFailed, Common::convertFromU32String(_("An internal error occurred while attempting to write save game data")));

	assert(saveWriter);

	stream->writeUint32BE(kSavegameSignature);
	stream->writeUint32BE(kCurrentSaveFileVersion);

	if (!saveWriter->writeSave(stream) || stream->err())
		return Common::Error(Common::kWritingFailed, Common::convertFromU32String(_("An error occurred while writing the save game")));

	return Common::kNoError;
}

bool MTropolisEngine::canSaveAutosaveCurrently() {
	// Triggered autosaves are always safe
	if (_isTriggeredAutosave)
		return true;

	return canSaveGameStateCurrently();
}

bool MTropolisEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (!_runtime->isIdle())
		return false;

	for (Common::SharedPtr<SaveLoadMechanismHooks> &hooks : _runtime->getHacks().saveLoadMechanismHooks) {
		if (hooks->canSaveNow(_runtime.get()))
			return true;
	}

	return false;
}

} // End of namespace MTropolis
