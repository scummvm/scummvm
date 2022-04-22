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

#include "mtropolis/debug.h"
#include "mtropolis/render.h"
#include "mtropolis/runtime.h"

#include "gui/dialog.h"

#include "graphics/fontman.h"

namespace MTropolis {

DebugInspector::DebugInspector(IDebuggable *debuggable) {
}

DebugInspector::~DebugInspector() {
}

void DebugInspector::onDestroyed() {
	_debuggable = nullptr;
}

Debugger::Debugger(Runtime *runtime) : _paused(false), _runtime(runtime) {
	refreshSceneStatus();
}

Debugger::~Debugger() {
	if (_runtime)
		_runtime->removeWindow(_sceneStatusWindow.get());
}

void Debugger::runFrame(uint32 msec) {
	for (size_t ri = _toastNotifications.size(); ri > 0; ri--) {
		size_t i = ri - 1;

		ToastNotification &toastNotification = _toastNotifications[i];

		uint64 realTime = _runtime->getRealTime();
		Window &window = *toastNotification.window;

		if (realTime >= toastNotification.dismissTime) {
			_runtime->removeWindow(&window);
			_toastNotifications.remove_at(i);
		}
		else {
			uint64 timeRemaining = toastNotification.dismissTime - realTime;
			uint32 dismissDuration = 250;
			if (timeRemaining < dismissDuration) {
				int32 offset = window.getSurface()->w * static_cast<int32>(dismissDuration - timeRemaining) / static_cast<int32>(dismissDuration);
				window.setPosition(-offset, window.getY());
			}
		}
	}
}

void Debugger::setPaused(bool paused) {
	_paused = paused;
}

bool Debugger::isPaused() const {
	return _paused;
}

void Debugger::notify(DebugSeverity severity, const Common::String& str) {
	const int toastNotificationHeight = 15;

	uint16 displayWidth, displayHeight;
	_runtime->getDisplayResolution(displayWidth, displayHeight);

	int horizPadding = 10;
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	int width = font->getStringWidth(str) + horizPadding * 2;
	if (width > displayWidth)
		width = displayWidth;

	const Graphics::PixelFormat pixelFmt = _runtime->getRenderPixelFormat();

	ToastNotification toastNotification;
	toastNotification.window.reset(new Window(0, displayHeight, width, toastNotificationHeight, pixelFmt));

	byte fillColor[3] = {255, 255, 255};
	if (severity == kDebugSeverityError) {
		fillColor[0] = 255;
		fillColor[1] = 100;
		fillColor[2] = 100;
	} else if (severity == kDebugSeverityWarning) {
		fillColor[0] = 255;
		fillColor[1] = 225;
		fillColor[2] = 120;
	}

	Graphics::ManagedSurface &managedSurface = *toastNotification.window->getSurface();
	managedSurface.fillRect(Common::Rect(0, 0, width, toastNotificationHeight), Render::resolveRGB(fillColor[0], fillColor[1], fillColor[2], pixelFmt));

	font->drawString(&managedSurface, str, 10, (toastNotificationHeight - font->getFontAscent()) / 2, width - horizPadding * 2, Render::resolveRGB(0, 0, 0, pixelFmt));

	toastNotification.dismissTime = _runtime->getRealTime() + 5250;

	_toastNotifications.push_back(toastNotification);
	_runtime->addWindow(toastNotification.window);

	for (size_t i = 0; i < _toastNotifications.size(); i++) {
		Window &window = *_toastNotifications[i].window;
		window.setPosition(window.getX(), window.getY() - toastNotificationHeight);
	}
}

void Debugger::notifyFmt(DebugSeverity severity, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	this->vnotifyFmt(severity, fmt, args);
	va_end(args);
}

void Debugger::vnotifyFmt(DebugSeverity severity, const char* fmt, va_list args) {
	Common::String str(Common::String::vformat(fmt, args));
	this->notify(severity, str);
}

void Debugger::refreshSceneStatus() {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);

	Common::Array<Common::String> sceneStrs;
	Structural *sharedScene = _runtime->getActiveSharedScene().get();
	if (sharedScene)
		sceneStrs.push_back(Common::String("Shar: ") + sharedScene->debugGetName());
	Structural *mainScene = _runtime->getActiveMainScene().get();
	if (mainScene)
		sceneStrs.push_back(Common::String("Main: ") + mainScene->debugGetName());

	const uint horizPadding = 10;
	const uint vertSpacing = 15;
	int width = 0;
	for (uint i = 0; i < sceneStrs.size(); i++) {
		int lineWidth = font->getStringWidth(sceneStrs[i]);
		if (lineWidth > width)
			width = lineWidth;
	}

	if (_sceneStatusWindow.get()) {
		_runtime->removeWindow(_sceneStatusWindow.get());
		_sceneStatusWindow.reset();
	}

	const Graphics::PixelFormat pixelFmt = _runtime->getRenderPixelFormat();

	_sceneStatusWindow.reset(new Window(0, 0, horizPadding * 2 + width, vertSpacing * sceneStrs.size(), pixelFmt));
	_runtime->addWindow(_sceneStatusWindow);

	for (uint i = 0; i < sceneStrs.size(); i++) {
		font->drawString(_sceneStatusWindow->getSurface().get(), sceneStrs[i], horizPadding, vertSpacing * i + (vertSpacing - font->getFontAscent()) / 2, width, Render::resolveRGB(255, 255, 255, pixelFmt));
	}
}

void Debugger::complainAboutUnfinished(Structural *structural) {
	Common::HashMap<Common::String, SupportStatus> unfinishedModifiers;
	Common::HashMap<Common::String, SupportStatus> unfinishedElements;

	scanStructuralStatus(structural, unfinishedModifiers, unfinishedElements);

	const SupportStatus supportStatusBins[2] = {kSupportStatusPartial,
												kSupportStatusNone};
	const char *supportStatusNames[2] = {"partially-finished", "unimplemented"};

	const Common::HashMap<Common::String, SupportStatus> *typeBins[2] = {&unfinishedModifiers, &unfinishedElements};
	const char *typeNames[2] = {"modifier", "element"};

	for (int ssi = 0; ssi < 2; ssi++) {
		for (int ti = 0; ti < 2; ti++) {
			Common::Array<Common::String> names;
			for (Common::HashMap<Common::String, SupportStatus>::const_iterator it = typeBins[ti]->begin(), itEnd = typeBins[ti]->end(); it != itEnd; ++it) {
				if (it->_value == supportStatusBins[ssi])
					names.push_back(it->_key);
			}

			Common::sort(names.begin(), names.end());

			for (size_t i = 0; i < names.size(); i++) {
				Common::String message = "Scene '" + structural->debugGetName() + "' contains " + supportStatusNames[ssi] + " " + typeNames[ti] + ": " + names[i];
				this->notify(DebugSeverity::kDebugSeverityWarning, message);
			}
		}
	}
}

void Debugger::scanStructuralStatus(Structural *structural, Common::HashMap<Common::String, SupportStatus> &unfinishedModifiers, Common::HashMap<Common::String, SupportStatus> &unfinishedElements) {
	for (Common::Array<Common::SharedPtr<Structural>>::const_iterator it = structural->getChildren().begin(), itEnd = structural->getChildren().end(); it != itEnd; ++it) {
		scanStructuralStatus(it->get(), unfinishedModifiers, unfinishedElements);
	}

	for (Common::Array<Common::SharedPtr<Modifier> >::const_iterator it = structural->getModifiers().begin(), itEnd = structural->getModifiers().end(); it != itEnd; ++it) {
		scanModifierStatus(it->get(), unfinishedModifiers, unfinishedElements);
	}

	scanDebuggableStatus(structural, unfinishedElements);
}

void Debugger::scanModifierStatus(Modifier *modifier, Common::HashMap<Common::String, SupportStatus> &unfinishedModifiers, Common::HashMap<Common::String, SupportStatus> &unfinishedElements) {
	IModifierContainer *children = modifier->getChildContainer();
	if (children) {
		for (Common::Array<Common::SharedPtr<Modifier> >::const_iterator it = children->getModifiers().begin(), itEnd = children->getModifiers().end(); it != itEnd; ++it) {
			scanModifierStatus(it->get(), unfinishedModifiers, unfinishedElements);
		}
	}

	scanDebuggableStatus(modifier, unfinishedModifiers);
}

void Debugger::scanDebuggableStatus(IDebuggable* debuggable, Common::HashMap<Common::String, SupportStatus>& unfinished) {
	SupportStatus supportStatus = debuggable->debugGetSupportStatus();
	if (supportStatus != kSupportStatusDone)
		unfinished[Common::String(debuggable->debugGetTypeName())] = supportStatus;
}


} // End of namespace MTropolis
