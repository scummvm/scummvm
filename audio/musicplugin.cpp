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

#include "audio/musicplugin.h"
#include "common/str.h"
#include "common/hash-str.h"
#include "common/config-manager.h"
#include "gui/message.h"
#include "common/translation.h"

// If detectDevice() detects MT32 and we have a preferred MT32 device
// we use this to force getMusicType() to return MT_MT32 so that we don't
// have to rely on the 'True Roland MT-32' config manager setting (since nobody
// would possibly think about activating 'True Roland MT-32' when he has set
// 'Music Driver' to '<default>')
static bool forceTypeMT32 = false;

MusicDevice::MusicDevice(MusicPluginObject const *musicPlugin, Common::String name, MusicType mt) :
	_plugin(musicPlugin),
	_name(name), _type(mt) {
}

const Common::String &MusicDevice::getName() const {
	return _name;
}

const Common::String MusicDevice::getMusicDriverName() const {
	return _plugin->getName();
}

const Common::String MusicDevice::getMusicDriverId() const {
	return _plugin->getId();
}

MusicType MusicDevice::getMusicType() const {
	if (forceTypeMT32)
		return MT_MT32;
	return _type;
}

Common::String MusicDevice::getCompleteName() const {
	Common::String name;

	if (_name.empty()) {
		// Default device, just show the driver name
		name = getMusicDriverName();
	} else {
		// Show both device and driver names
		name = _name;
		name += " [";
		name += getMusicDriverName();
		name += "]";
	}

	return name;
}

Common::String MusicDevice::getCompleteId() const {
	Common::String id = getMusicDriverId();
	if (!_name.empty()) {
		id += "_";
		id += _name;
	}

	return id;
}

namespace Common {
DECLARE_SINGLETON(MusicManager);
}

const PluginList &MusicManager::getPlugins() const {
	return PluginManager::instance().getPlugins(PLUGIN_TYPE_MUSIC);
}

MusicManager::MusicManager() {
	const PluginList &plugins=getPlugins();

	if (plugins.begin() == plugins.end())
		error("MusicMan.getDevices: Music plugins must be loaded prior to calling this method");

	for (PluginList::const_iterator m = plugins.begin(); m != plugins.end(); ++m) {
		MusicDevices i = (*m)->get<MusicPluginObject>().getDevices();
		_devices.insert(_devices.end(), i.begin(), i.end());
	}
}

MusicDevice *MusicManager::getDevice(const Common::String &identifier) {
	for (MusicDevices::iterator d = _devices.begin(); d != _devices.end(); ++d) {
		// The music driver id isn't unique, but it will match
		// driver's first device. This is useful when selecting
		// the driver from the command line.
		if (identifier.equals(d->getMusicDriverId()) || identifier.equals(d->getCompleteId()) || identifier.equals(d->getCompleteName())) {
			return &*d;
		}
	}

	return 0;
}

MidiDriver *MusicManager::createMidi(MusicDevice *d) const {
	MidiDriver *driver;
	Common::Error err=d->_plugin->createInstance(&driver, d);
	if (err.getCode() == Common::kNoError)
		return driver;
	return nullptr;
}

Common::String MusicDevice::getDeviceString(DeviceStringType type) const {
	if (type == kDriverName)
		return getMusicDriverName();
	else if (type == kDriverId)
		return getMusicDriverId();
	else if (type == kDeviceName)
		return getCompleteName();
	else if (type == kDeviceId)
		return getCompleteId();
	else
		return Common::String("auto");
}


MusicDevice *MusicManager::detectDevice(int flags) {
	// Query the selected music device (defaults to MT_AUTO device).
	Common::String selDevStr = ConfMan.hasKey("music_driver") ? ConfMan.get("music_driver") : Common::String("auto");
	if ((flags & MDT_PREFER_FLUID) && selDevStr == "auto") {
		selDevStr = "fluidsynth";
	}
	MusicDevice *device = getDevice(selDevStr.empty() ? Common::String("auto") : selDevStr);
	MusicDevice *reslt = 0;

	forceTypeMT32 = false;

	// Check whether the selected music driver is compatible with the
	// given flags.
	switch (device->getMusicType()) {
	case MT_PCSPK:
		if (flags & MDT_PCSPK)
			reslt = device;
		break;

	case MT_PCJR:
		if (flags & MDT_PCJR)
			reslt = device;
		break;

	case MT_CMS:
		if (flags & MDT_CMS)
			reslt = device;
		break;

	case MT_ADLIB:
		if (flags & MDT_ADLIB)
			reslt = device;
		break;

	case MT_C64:
		if (flags & MDT_C64)
			reslt = device;
		break;

	case MT_AMIGA:
		if (flags & MDT_AMIGA)
			reslt = device;
		break;

	case MT_APPLEIIGS:
		if (flags & MDT_APPLEIIGS)
			reslt = device;
		break;

	case MT_TOWNS:
		if (flags & MDT_TOWNS)
			reslt = device;
		break;

	case MT_PC98:
		if (flags & MDT_PC98)
			reslt = device;
		break;

	case MT_SEGACD:
		if (flags & MDT_SEGACD)
			reslt = device;
		break;

	case MT_GM:
	case MT_GS:
	case MT_MT32:
		if (flags & MDT_MIDI)
			reslt = device;
		break;

	case MT_NULL:
		reslt = device;
		break;

	default:
		break;
	}

	Common::String failedDevStr;
	if (device->getMusicType() == MT_INVALID) {
		// If the expressly selected driver or device cannot be found (no longer compiled in, turned off, etc.)
		// we display a warning and continue.
		failedDevStr = selDevStr;
		Common::U32String warningMsg = Common::U32String::format(
										   _("The selected audio device '%s' was not found (e.g. might be turned off or disconnected)."), failedDevStr.c_str()) +
									   Common::U32String(" ") + _("Attempting to fall back to the next available device...");
		GUI::MessageDialog dialog(warningMsg);
		dialog.runModal();
	}

	if (reslt && reslt->getMusicType() != MT_AUTO) {
		if (reslt->checkDevice()) {
			return reslt;
		} else {
			// If the expressly selected device cannot be used we display a warning and continue.
			failedDevStr = reslt->getDeviceString(MusicDevice::kDeviceName);
			Common::U32String warningMsg = Common::U32String::format(
											   _("The selected audio device '%s' cannot be used. See log file for more information."), failedDevStr.c_str()) +
										   Common::U32String(" ") + _("Attempting to fall back to the next available device...");
			GUI::MessageDialog dialog(warningMsg);
			dialog.runModal();
		}
	}

	// If the selected driver did not match the flags setting,
	// we try to determine a suitable and "optimal" music driver.
	const PluginList p = MusicMan.getPlugins();
	// If only MDT_MIDI but not MDT_PREFER_MT32 or MDT_PREFER_GM is set we prefer the other devices (which will always be
	// detected since they are hard coded and cannot be disabled).
	bool skipMidi = !(flags & (MDT_PREFER_GM | MDT_PREFER_MT32));
	while (flags != MDT_NONE) {
		if ((flags & MDT_MIDI) && !skipMidi) {
			// If a preferred MT32 or GM device has been selected that device gets returned if available.
			Common::String devStr;
			if (flags & MDT_PREFER_MT32)
				devStr = ConfMan.hasKey("mt32_device") ? ConfMan.get("mt32_device") : Common::String("null");
			else if (flags & MDT_PREFER_GM)
				devStr = ConfMan.hasKey("gm_device") ? ConfMan.get("gm_device") : Common::String("null");
			else
				devStr = "auto";

			// Default to Null device here, since we also register a default null setting for
			// the MT32 or GM device in the config manager.
			device = getDevice(devStr.empty() ? Common::String("null") : devStr);
			const MusicType type = device->getMusicType();

			// If we have a "Don't use GM/MT-32" setting we skip this part and jump
			// to AdLib, PC Speaker etc. detection right away.
			if (type != MT_NULL) {
				if (type == MT_INVALID) {
					// If the preferred (expressly requested) selected driver or device cannot be found (no longer compiled in, turned off, etc.)
					// we display a warning and continue. Don't warn about the missing device if we did already (this becomes relevant if the
					// missing device is selected as preferred device and also as GM or MT-32 device).
					if (failedDevStr != devStr) {
						Common::U32String warningMsg = Common::U32String::format(
														   _("The preferred audio device '%s' was not found (e.g. might be turned off or disconnected)."), devStr.c_str()) +
													   Common::U32String(" ") + _("Attempting to fall back to the next available device...");
						GUI::MessageDialog dialog(warningMsg);
						dialog.runModal();
					}
				} else if (type != MT_AUTO) {
					if (device->checkDevice()) {
						if (flags & MDT_PREFER_MT32)
							// If we have a preferred MT32 device we disable the gm/mt32 mapping.
							forceTypeMT32 = true;
						return device;
					} else {
						// If the preferred (expressly requested) device cannot be used we display a warning and continue.
						// Don't warn about the failing device if we did already (this becomes relevant if the failing
						// device is selected as preferred device and also as GM or MT-32 device).
						if (failedDevStr != device->getDeviceString(MusicDevice::kDeviceName)) {
							Common::U32String warningMsg = Common::U32String::format(
															   _("The preferred audio device '%s' cannot be used. See log file for more information."), device->getDeviceString(MusicDevice::kDeviceName).c_str()) +
														   Common::U32String(" ") + _("Attempting to fall back to the next available device...");
							GUI::MessageDialog dialog(warningMsg);
							dialog.runModal();
						}
					}
				}

				// If no specific device is selected (neither in the scummvm nor in the game domain)
				// and there is no preferred MT32 or GM device selected either or if the detected device is unavailable we arrive here.
				// If MT32 is preferred we try for the first available device with music type 'MT_MT32' (usually the mt32 emulator).
				if (flags & MDT_PREFER_MT32) {
					for (MusicDevices::iterator d = _devices.begin(); d != _devices.end(); ++d) {
						if (d->getMusicType() != MT_MT32)
							continue;
						return &*d;
					}
				}

				// Now we default to the first available device with music type 'MT_GM' if not
				// MT-32 is preferred or if MT-32 is preferred but all other devices have failed.
				if (!(flags & MDT_PREFER_MT32) || flags == (MDT_PREFER_MT32 | MDT_MIDI)) {
					for (MusicDevices::iterator d = _devices.begin(); d != _devices.end(); ++d) {
						if (!d->checkDevice())
							continue;
						if (d->getMusicType() == MT_GM || d->getMusicType() == MT_GS) {
							return &*d;
						}
					}
					// Detection flags get removed after final detection attempt to avoid further attempts.
					flags &= ~(MDT_MIDI | MDT_PREFER_GM | MDT_PREFER_MT32);
				}
			}
		}

		MusicType tp;
		// The order in this list is important, since this is the order of preference
		// (e.g. MT_ADLIB is checked before MT_PCJR and MT_PCSPK for a good reason).
		// Detection flags get removed after detection attempt to avoid further attempts.
		if (flags & MDT_TOWNS) {
			tp = MT_TOWNS;
			flags &= ~MDT_TOWNS;
		} else if (flags & MDT_PC98) {
			tp = MT_PC98;
			flags &= ~MDT_PC98;
		} else if (flags & MDT_SEGACD) {
			tp = MT_SEGACD;
			flags &= ~MDT_SEGACD;
		} else if (flags & MDT_ADLIB) {
			tp = MT_ADLIB;
			flags &= ~MDT_ADLIB;
		} else if (flags & MDT_PCJR) {
			tp = MT_PCJR;
			flags &= ~MDT_PCJR;
		} else if (flags & MDT_PCSPK) {
			tp = MT_PCSPK;
			flags &= ~MDT_PCSPK;
		} else if (flags & MDT_C64) {
			tp = MT_C64;
			flags &= ~MDT_C64;
		} else if (flags & MDT_AMIGA) {
			tp = MT_AMIGA;
			flags &= ~MDT_AMIGA;
		} else if (flags & MDT_APPLEIIGS) {
			tp = MT_APPLEIIGS;
			flags &= ~MDT_APPLEIIGS;
		} else if (flags & MDT_MIDI) {
			// If we haven't tried to find a MIDI device yet we do this now.
			skipMidi = false;
			continue;
		} else if (flags) {
			// Invalid flags. Leave the detection loop.
			break;
		}

		for (MusicDevices::iterator d = _devices.begin(); d != _devices.end(); ++d) {

			if (!d->checkDevice())
				continue;
			if (d->getMusicType() == tp) {
					return &*d;
			}
		}

	}

	return 0;
}
