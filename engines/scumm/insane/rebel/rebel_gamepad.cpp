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

#include "common/config-manager.h"
#include "common/system.h"

#include "scumm/insane/rebel/rebel_gamepad.h"

namespace Scumm {

namespace {

#ifdef IPHONE
const char *const kIOSGamepadControllerKey = "gamepad_controller";
const char *const kIOSGamepadControllerMinimalLayoutKey = "gamepad_controller_minimal_layout";
const char *const kIOSGamepadControllerDirectionalInputKey = "gamepad_controller_directional_input";
const int kIOSGamepadControllerDirectionalInputDpad = 1;

void saveDomainSetting(RebelIOSGamepadControllerState::SavedSetting &setting,
		const Common::ConfigManager::Domain *domain, const char *key) {
	setting.present = domain && domain->contains(key);
	setting.value = setting.present ? domain->getVal(key) : Common::String();
}

void restoreDomainSetting(const RebelIOSGamepadControllerState::SavedSetting &setting,
		const Common::String &domainName, const char *key) {
	Common::ConfigManager::Domain *domain = ConfMan.getDomain(domainName);
	if (!domain)
		return;

	if (setting.present)
		ConfMan.set(key, setting.value, domainName);
	else if (domain->contains(key))
		ConfMan.removeKey(key, domainName);
}
#endif

void clearSavedSetting(RebelIOSGamepadControllerState::SavedSetting &setting) {
	setting.present = false;
	setting.value.clear();
}

} // End of anonymous namespace

RebelIOSGamepadControllerState::RebelIOSGamepadControllerState() :
		_active(false),
		_gamepadController(),
		_gamepadControllerMinimalLayout(),
		_gamepadControllerDirectionalInput() {
	clearSavedSetting(_gamepadController);
	clearSavedSetting(_gamepadControllerMinimalLayout);
	clearSavedSetting(_gamepadControllerDirectionalInput);
}

void RebelIOSGamepadControllerState::enable() {
#ifdef IPHONE
	if (_active)
		return;

	_domainName = ConfMan.getActiveDomainName();
	if (_domainName.empty())
		return;

	const Common::ConfigManager::Domain *domain = ConfMan.getDomain(_domainName);
	saveDomainSetting(_gamepadController, domain, kIOSGamepadControllerKey);
	saveDomainSetting(_gamepadControllerMinimalLayout, domain, kIOSGamepadControllerMinimalLayoutKey);
	saveDomainSetting(_gamepadControllerDirectionalInput, domain, kIOSGamepadControllerDirectionalInputKey);

	// Same iOS virtual-controller profile used by Freescape: compact d-pad overlay.
	ConfMan.setBool(kIOSGamepadControllerKey, true, _domainName);
	ConfMan.setBool(kIOSGamepadControllerMinimalLayoutKey, true, _domainName);
	ConfMan.setInt(kIOSGamepadControllerDirectionalInputKey, kIOSGamepadControllerDirectionalInputDpad, _domainName);
	g_system->applyBackendSettings();
	_active = true;
#endif
}

void RebelIOSGamepadControllerState::restore() {
#ifdef IPHONE
	if (!_active)
		return;

	restoreDomainSetting(_gamepadController, _domainName, kIOSGamepadControllerKey);
	restoreDomainSetting(_gamepadControllerMinimalLayout, _domainName, kIOSGamepadControllerMinimalLayoutKey);
	restoreDomainSetting(_gamepadControllerDirectionalInput, _domainName, kIOSGamepadControllerDirectionalInputKey);
	g_system->applyBackendSettings();

	clearSavedSetting(_gamepadController);
	clearSavedSetting(_gamepadControllerMinimalLayout);
	clearSavedSetting(_gamepadControllerDirectionalInput);
	_domainName.clear();
	_active = false;
#endif
}

} // End of namespace Scumm
