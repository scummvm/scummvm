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

static const byte g_sceneTreeGraphic[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte g_inspectorGraphic[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
	0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte g_stepThroughGraphic[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0,
	0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0,
	0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0,
	0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const byte g_resizeGraphic[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

class DebugToolWindowBase : public Window {
public:
	DebugToolWindowBase(DebuggerTool tool, const Common::String &title, Debugger *debugger, const WindowParameters &windowParams);

protected:
	const int kTopBarHeight = 12;
	const int kScrollBarWidth = 12;
	const int kCloseWidth = 12;
	const int kResizeHeight = 12;

	void onMouseDown(int32 x, int32 y, int mouseButton) override;
	void onMouseMove(int32 x, int32 y) override;
	void onMouseUp(int32 x, int32 y, int mouseButton) override;

	virtual void toolOnMouseDown(int32 x, int32 y, int mouseButton) {}
	virtual void toolOnMouseMove(int32 x, int32 y) {}
	virtual void toolOnMouseUp(int32 x, int32 y, int mouseButton) {}

	void refreshChrome();

	Debugger *_debugger;
	Common::SharedPtr<Graphics::ManagedSurface> _toolSurface;

private:
	enum ToolWindowWidget {
		kToolWindowWidgetNone,

		kToolWindowWidgetClose,
		kToolWindowWidgetScroll,
		kToolWindowWidgetResize,
		kToolWindowWidgetMove,
	};
	ToolWindowWidget _activeWidget;
	bool _isMouseCaptured;
	int32 _dragStartX;
	int32 _dragStartY;
	int32 _resizeStartWidth;
	int32 _resizeStartHeight;
	DebuggerTool _tool;

	Common::String _title;
};

DebugToolWindowBase::DebugToolWindowBase(DebuggerTool tool, const Common::String &title, Debugger *debugger, const WindowParameters &windowParams)
	: Window(windowParams), _debugger(debugger), _tool(tool), _title(title), _activeWidget(kToolWindowWidgetNone), _isMouseCaptured(false) {

	refreshChrome();
}

void DebugToolWindowBase::onMouseDown(int32 x, int32 y, int mouseButton) {
	if (mouseButton != Actions::kMouseButtonLeft)
		return;

	if (_isMouseCaptured)
		return;

	_isMouseCaptured = true;
	_dragStartX = x;
	_dragStartY = y;

	if (y < kTopBarHeight) {
		if (x < kCloseWidth)
			_activeWidget = kToolWindowWidgetClose;
		else
			_activeWidget = kToolWindowWidgetMove;

		_dragStartX = x;
		_dragStartY = y;
	} else if (x >= getWidth() - kScrollBarWidth) {
		if (y >= getHeight() - kResizeHeight) {
			_activeWidget = kToolWindowWidgetResize;
			_resizeStartWidth = getWidth();
			_resizeStartHeight = getHeight();
		}
		else
			_activeWidget = kToolWindowWidgetScroll;
	} else {
		_activeWidget = kToolWindowWidgetNone;
		toolOnMouseDown(x, y - kTopBarHeight, mouseButton);
	}
}

void DebugToolWindowBase::onMouseMove(int32 x, int32 y) {
	if (_activeWidget == kToolWindowWidgetNone)
		toolOnMouseMove(x, y - kTopBarHeight);
	else {
		if (_activeWidget == kToolWindowWidgetMove) {
			int32 relX = x - _dragStartX;
			int32 relY = y - _dragStartY;
			setPosition(getX() + relX, getY() + relY);
		} else if (_activeWidget == kToolWindowWidgetResize) {
			int32 relX = x - _dragStartX;
			int32 relY = y - _dragStartY;
			int32 newWidth = _resizeStartWidth + relX;
			int32 newHeight = _resizeStartHeight + relY;

			if (newWidth < 100)
				newWidth = 100;
			if (newHeight < 100)
				newHeight = 100;

			if (newWidth != getWidth() || newHeight != getHeight()) {
				this->resizeWindow(newWidth, newHeight);
				refreshChrome();
			}
		}
	}
}

void DebugToolWindowBase::onMouseUp(int32 x, int32 y, int mouseButton) {
	if (mouseButton != Actions::kMouseButtonLeft)
		return;

	if (!_isMouseCaptured)
		return;

	_isMouseCaptured = false;

	if (_activeWidget == kToolWindowWidgetNone)
		toolOnMouseUp(x, y - kTopBarHeight, mouseButton);
	else {
		if (_activeWidget == kToolWindowWidgetClose) {
			if (x < kCloseWidth && y < kTopBarHeight) {
				_debugger->closeToolWindow(_tool);
				return;
			}
		}

		_activeWidget = kToolWindowWidgetNone;
	}
}

void DebugToolWindowBase::refreshChrome() {
	Graphics::ManagedSurface *surface = getSurface().get();

	const Graphics::PixelFormat &fmt = surface->rawSurface().format;

	uint32 blackColor = fmt.RGBToColor(0, 0, 0);
	uint32 whiteColor = fmt.RGBToColor(255, 255, 255);
	uint32 closeColor = fmt.RGBToColor(255, 0, 0);

	uint32 topBarColor = fmt.RGBToColor(192, 192, 192);
	uint32 topTextColor = blackColor;

	uint32 inactiveScrollColor = fmt.RGBToColor(225, 225, 225);

	int width = surface->w;
	int height = surface->h;

	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 12; x++) {
			uint32 pixelColor = (g_resizeGraphic[y * 12 + x] == 0) ? blackColor : whiteColor;
			surface->setPixel(width - 12 + x, height - 12 + y, pixelColor);
		}
	}

	surface->fillRect(Common::Rect(0, 0, width, kTopBarHeight), topBarColor);

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	int titleWidth = font->getStringWidth(_title);
	int titleAvailableWidth = width - kCloseWidth;
	if (titleWidth < titleAvailableWidth)
		titleWidth = titleAvailableWidth;

	int titleY = (kTopBarHeight - font->getFontAscent()) / 2;

	font->drawString(surface, _title, kCloseWidth, titleY, titleAvailableWidth, topTextColor, Graphics::kTextAlignCenter, 0, true);

	surface->fillRect(Common::Rect(width - kScrollBarWidth, kTopBarHeight, width, height - kResizeHeight), inactiveScrollColor);
	surface->fillRect(Common::Rect(0, 0, kCloseWidth, kTopBarHeight), closeColor);
	surface->drawThickLine(2, 2, kCloseWidth - 4, kTopBarHeight - 4, 2, 2, whiteColor);
	surface->drawThickLine(kCloseWidth - 4, 2, 2, kTopBarHeight - 4, 2, 2, whiteColor);
}

class DebugToolsWindow : public Window {
public:
	DebugToolsWindow(Debugger *debugger, const WindowParameters &windowParams);

	void onMouseDown(int32 x, int32 y, int mouseButton) override;

private:
	Debugger *_debugger;
};

DebugToolsWindow::DebugToolsWindow(Debugger *debugger, const WindowParameters &windowParams)
	: Window(windowParams), _debugger(debugger) {
}

void DebugToolsWindow::onMouseDown(int32 x, int32 y, int mouseButton) {
	int tool = 0;
	if (y > 1)
		tool = (y - 1) / 17;
	_debugger->openToolWindow(static_cast<DebuggerTool>(tool));
}


DebugInspector::DebugInspector(IDebuggable *debuggable) {
}

DebugInspector::~DebugInspector() {
}

void DebugInspector::onDestroyed() {
	_debuggable = nullptr;
}

Debugger::Debugger(Runtime *runtime) : _paused(false), _runtime(runtime) {
	refreshSceneStatus();

	const Graphics::PixelFormat renderFmt = runtime->getRenderPixelFormat();

	const byte *toolGraphics[kDebuggerToolCount] = {
		g_sceneTreeGraphic,
		g_inspectorGraphic,
		g_stepThroughGraphic,
	};

	_toolsWindow.reset(new DebugToolsWindow(this, WindowParameters(runtime, 0, 0, 18, 1 + kDebuggerToolCount * 17, renderFmt)));
	Graphics::ManagedSurface *toolWindowSurface = _toolsWindow->getSurface().get();

	uint32 whiteColor = renderFmt.RGBToColor(255, 255, 255);
	uint32 blackColor = renderFmt.RGBToColor(0, 0, 0);

	const uint32 toolGraphicPalette[] = {blackColor, whiteColor};

	for (int y = 0; y < 1 + kDebuggerToolCount * 17; y++) {
		for (int x = 0; x < 18; x++) {
			toolWindowSurface->setPixel(x, y, whiteColor);
		}
	}

	for (int tool = 0; tool < kDebuggerToolCount; tool++) {
		const byte *toolGraphic = toolGraphics[tool];

		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				toolWindowSurface->setPixel(x + 1, tool * 17 + 1 + y, toolGraphicPalette[toolGraphic[y * 16 + x]]);
			}
		}
	}

	_toolsWindow->setStrata(1);
	runtime->addWindow(_toolsWindow);
}

Debugger::~Debugger() {
	if (_runtime) {
		_runtime->removeWindow(_sceneStatusWindow.get());
		_runtime->removeWindow(_toolsWindow.get());
		for (int i = 0; i < kDebuggerToolCount; i++)
			_runtime->removeWindow(_toolWindows[i].get());
	}
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
	toastNotification.window.reset(new Window(WindowParameters(_runtime, 0, displayHeight, width, toastNotificationHeight, pixelFmt)));
	toastNotification.window->setStrata(3);
	toastNotification.window->setMouseTransparent(true);

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

	debug(1, "%s", str.c_str());
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

	_sceneStatusWindow.reset(new Window(WindowParameters(_runtime, 0, 0, horizPadding * 2 + width, vertSpacing * sceneStrs.size(), pixelFmt)));
	_sceneStatusWindow->setMouseTransparent(true);
	_sceneStatusWindow->setStrata(1);

	_runtime->addWindow(_sceneStatusWindow);

	for (uint i = 0; i < sceneStrs.size(); i++) {
		font->drawString(_sceneStatusWindow->getSurface().get(), sceneStrs[i], horizPadding, vertSpacing * i + (vertSpacing - font->getFontAscent()) / 2, width, Render::resolveRGB(255, 255, 255, pixelFmt));
	}

	if (_toolsWindow)
		_toolsWindow->setPosition(0, _sceneStatusWindow->getHeight());
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

void Debugger::openToolWindow(DebuggerTool tool) {
	if (tool < 0 || tool >= kDebuggerToolCount)
		return;	// This should never happen

	Common::SharedPtr<Window> &windowRef = _toolWindows[tool];
	if (windowRef)
		return;

	switch (tool) {
	case kDebuggerToolSceneTree:
		windowRef.reset(new DebugToolWindowBase(kDebuggerToolSceneTree, "SceneTree", this, WindowParameters(_runtime, 32, 32, 100, 320, _runtime->getRenderPixelFormat())));
		break;
	case kDebuggerToolInspector:
		windowRef.reset(new DebugToolWindowBase(kDebuggerToolInspector, "Inspector", this, WindowParameters(_runtime, 32, 32, 100, 320, _runtime->getRenderPixelFormat())));
		break;
	case kDebuggerToolStepThrough:
		windowRef.reset(new DebugToolWindowBase(kDebuggerToolStepThrough, "Debugger", this, WindowParameters(_runtime, 32, 32, 100, 320, _runtime->getRenderPixelFormat())));
		break;
	default:
		assert(false);
		return;
	}

	_runtime->addWindow(windowRef);
}

void Debugger::closeToolWindow(DebuggerTool tool) {
	_runtime->removeWindow(_toolWindows[tool].get());
	_toolWindows[tool].reset();
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
