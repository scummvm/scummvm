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

#include "backends/base-backend.h"

#include "graphics/scalerplugin.h"

#ifndef DISABLE_DEFAULT_EVENT_MANAGER
#include "backends/events/default/default-events.h"
#endif

#ifndef DISABLE_DEFAULT_AUDIOCD_MANAGER
#include "backends/audiocd/default/default-audiocd.h"
#endif


#include "gui/message.h"

bool BaseBackend::setScaler(const char *name, int factor) {
	if (!name)
		return false;

	if (!scumm_stricmp(name, "default"))
		return setScaler(getDefaultScaler(), factor);

	const PluginList &scalerPlugins = ScalerMan.getPlugins();

	for (uint scalerIndex = 0; scalerIndex < scalerPlugins.size(); scalerIndex++) {
		if (!scumm_stricmp(scalerPlugins[scalerIndex]->get<ScalerPluginObject>().getName(), name)) {
			return setScaler(scalerIndex, factor);
		}
	}

	return false;
}

void BaseBackend::displayMessageOnOSD(const Common::U32String &msg) {
	// Display the message for 1.5 seconds
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
}

void BaseBackend::initBackend() {
	// Init audio CD manager
#ifndef DISABLE_DEFAULT_AUDIOCD_MANAGER
	if (!_audiocdManager)
		_audiocdManager = new DefaultAudioCDManager();
#endif
	_cpuFeatures = kCpuNoFeatures;
#if defined(__x86_64__) || defined(__i686__) || defined(_M_X86) || defined(_M_X64)
	uint32 ext_edx1 = 0, ext_ebx7 = 0, ext_ecx1 = 0;
#  ifdef __GNUC__
	asm ("mov $1, %%eax\n\t"
		 "cpuid\n\t"
		 "mov %%edx, %0\n\t"
		 "mov %%ecx, %2\n\t"
		 "mov $7, %%eax\n\t"
		 "cpuid\n\t"
		 "mov %%ebx, %1\n\t"
		 : "=rm" (ext_edx1), "=rm" (ext_ebx7), "=rm" (ext_ecx1)
		 :
		 : "eax", "ebx", "ecx", "edx");
#  elif _MSC_VER
	__asm
	{
		mov eax,1
		cpuid
		mov ext_edx1,edx
		mov ext_ecx1,ecx
		mov ebx,7
		cpuid
		mov ext_ebx7,ebx
	}
#  endif // __GNUC__ and _MSC_VER
	_cpuFeatures |= (ext_edx1 & (1 << 26)) ? kCpuFeatureSSE2 : kCpuNoFeatures;
	_cpuFeatures |= (ext_ebx7 & (1 << 5)) ? kCpuFeatureAVX2 : kCpuNoFeatures;
	_cpuFeatures |= (ext_ecx1 & (1 << 19)) ? kCpuFeatureSSE41 : kCpuNoFeatures;
#endif // __x86_64__ and __i686__
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
	_cpuFeatures |= kCpuFeatureNEON;
#endif
	OSystem::initBackend();
}

bool BaseBackend::hasFeature(Feature f) {
	if (f == kFeatureSSE2) return _cpuFeatures & kCpuFeatureSSE2;
	if (f == kFeatureAVX2) return _cpuFeatures & kCpuFeatureAVX2;
	if (f == kFeatureNEON) return _cpuFeatures & kCpuFeatureNEON;
	return false;
}

void BaseBackend::fillScreen(uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(Common::Rect(screen->w, screen->h), col);
	unlockScreen();
}

void BaseBackend::fillScreen(const Common::Rect &r, uint32 col) {
	Graphics::Surface *screen = lockScreen();
	if (screen)
		screen->fillRect(r, col);
	unlockScreen();
}

void EventsBaseBackend::initBackend() {
	// Init Event manager
#ifndef DISABLE_DEFAULT_EVENT_MANAGER
	if (!_eventManager)
		_eventManager = new DefaultEventManager(this);
#endif

	BaseBackend::initBackend();
}
