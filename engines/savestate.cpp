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

#include "engines/savestate.h"
#include "engines/engine.h"
#include "engines/metaengine.h"
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"

SaveStateDescriptor::SaveStateDescriptor()
	// FIXME: default to 0 (first slot) or to -1 (invalid slot) ?
	: _slot(-1), _description(), _isDeletable(true), _isWriteProtected(false),
	  _isLocked(false), _saveDate(), _saveTime(), _playTime(), _playTimeMSecs(0),
	_thumbnail(), _saveType(kSaveTypeUndetermined) {
}

SaveStateDescriptor::SaveStateDescriptor(const MetaEngine *metaEngine, int slot, const Common::U32String &d)
	: _slot(slot), _description(d), _isLocked(false), _playTimeMSecs(0), _saveType(kSaveTypeUndetermined) {
	initSaveSlot(metaEngine);
}

SaveStateDescriptor::SaveStateDescriptor(const MetaEngine *metaEngine, int slot, const Common::String &d)
	: _slot(slot), _description(Common::U32String(d)), _isLocked(false), _playTimeMSecs(0), _saveType(kSaveTypeUndetermined) {
	initSaveSlot(metaEngine);
}

void SaveStateDescriptor::initSaveSlot(const MetaEngine *metaEngine) {
	if (!metaEngine && g_engine)
		metaEngine = g_engine->getMetaEngine();
	int autosaveSlot = metaEngine ? metaEngine->getAutosaveSlot() : -1;
	
	if (autosaveSlot >= 0 && _slot == autosaveSlot) {
		// Do not allow autosave slot to be deleted or overwritten
		_isWriteProtected = true;
		_isDeletable = false;	
	} else {
		_isWriteProtected = false;
		_isDeletable = true;		
	}
}

void SaveStateDescriptor::setThumbnail(Graphics::Surface *t) {
	if (_thumbnail.get() == t)
		return;

	_thumbnail = Common::SharedPtr<Graphics::Surface>(t, Graphics::SurfaceDeleter());
}

void SaveStateDescriptor::setSaveDate(int year, int month, int day) {
	_saveDate = Common::String::format("%.4d-%.2d-%.2d", year, month, day);
}

void SaveStateDescriptor::setSaveTime(int hour, int min) {
	_saveTime = Common::String::format("%.2d:%.2d", hour, min);
}

void SaveStateDescriptor::setPlayTime(int hours, int minutes) {
	_playTimeMSecs = ((hours * 60 + minutes) * 60) * 1000;
	_playTime = Common::String::format("%.2d:%.2d", hours, minutes);
}

void SaveStateDescriptor::setPlayTime(uint32 msecs) {
	_playTimeMSecs = msecs;
	uint minutes = msecs / 60000;
	setPlayTime(minutes / 60, minutes % 60);
}

void SaveStateDescriptor::setAutosave(bool autosave) {
	_saveType = autosave ? kSaveTypeAutosave : kSaveTypeRegular;
}

bool SaveStateDescriptor::isAutosave() const {
	return hasAutosaveName() || _saveType == kSaveTypeAutosave;
}

bool SaveStateDescriptor::hasAutosaveName() const
{
	const Common::U32String &autosave = _("Autosave");

	// if the save file name is long enough, just check if it starts with "Autosave"
	if (_description.size() >= autosave.size())
		return _description.substr(0, autosave.size()) == autosave;

	// if the save name has been trimmed, as long as it isn't too short, use fallback logic
	if (_description.size() < 14)
		return false;
	return autosave.substr(0, _description.size()) == _description;
}

bool SaveStateDescriptor::isValid() const
{
	return _slot >= 0 && !_description.empty();
}
