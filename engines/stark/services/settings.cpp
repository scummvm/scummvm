/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/settings.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/archiveloader.h"

#include "common/config-manager.h"
#include "common/debug.h"

namespace Stark {

Settings::Settings(Engine *engine) :
		_engine(engine),
		_domainName(ConfMan.getActiveDomainName()) {
	// Load from the configure
	loadConf("high_model", _boolSettings[kHighModel], true);
	loadConf("subtitles", _boolSettings[kSubtitle], true);
	loadConf("specialfx", _boolSettings[kSpecialFX], true);
	loadConf("shadow", _boolSettings[kShadow], true);
	loadConf("high_fmv", _boolSettings[kHighFMV], true);
	loadConf("enable_time_skip", _boolSettings[kTimeSkip], false);

	loadConf("speech_volume", _intSettings[kVoice], 256);
	loadConf("music_volume", _intSettings[kMusic], 256);
	loadConf("sfx_volume", _intSettings[kSfx], 256);

	// Use the FunCom logo video to check low-resolution fmv
	_hasLowRes = StarkArchiveLoader->getExternalFile("1402_lo_res.bbb", "Global/");
}

void Settings::save() const {
	saveConf("high_model", _boolSettings[kHighModel]);
	saveConf("subtitles", _boolSettings[kSubtitle]);
	saveConf("specialfx", _boolSettings[kSpecialFX]);
	saveConf("shadow", _boolSettings[kShadow]);
	saveConf("high_fmv",  _boolSettings[kHighFMV]);
	saveConf("enable_time_skip", _boolSettings[kTimeSkip]);

	saveConf("speech_volume",_intSettings[kVoice]);
	saveConf("music_volume", _intSettings[kMusic]);
	saveConf("sfx_volume", _intSettings[kSfx]);

	ConfMan.flushToDisk();
}

} // End of namespace Stark
