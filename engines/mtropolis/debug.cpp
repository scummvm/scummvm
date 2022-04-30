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

#include "common/hash-ptr.h"



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

	virtual void update() {}
	void render();

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
	virtual void toolRenderSurface(int32 subAreaWidth, int32 subAreaHeight) {}

	void setDirty();

	Common::SharedPtr<Graphics::ManagedSurface> _toolSurface;

	Debugger *_debugger;

private:
	void refreshChrome();

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
	bool _isDirty;
	int _scrollOffset;
};

DebugToolWindowBase::DebugToolWindowBase(DebuggerTool tool, const Common::String &title, Debugger *debugger, const WindowParameters &windowParams)
	: Window(windowParams), _debugger(debugger), _tool(tool), _title(title), _activeWidget(kToolWindowWidgetNone), _isMouseCaptured(false), _isDirty(true), _scrollOffset(0) {

	refreshChrome();
}

void DebugToolWindowBase::render() {
	if (_isDirty) {
		_isDirty = false;

		bool needChromeUpdate = false;
		int oldWidth = 0;
		int oldHeight = 0;
		if (_toolSurface) {
			oldWidth = _toolSurface->w;
			oldHeight = _toolSurface->h;
			needChromeUpdate = true;
		}

		int32 renderWidth = getWidth() - kScrollBarWidth;
		int32 renderHeight = getHeight() - kTopBarHeight;
		toolRenderSurface(renderWidth, renderHeight);

		if (_toolSurface && !needChromeUpdate) {
			if (oldWidth != _toolSurface->w || oldHeight != _toolSurface->h)
				needChromeUpdate = true;
		}

		if (needChromeUpdate)
			refreshChrome();

		if (_toolSurface) {
			int32 contentsBottom = _toolSurface->h - _scrollOffset;
			if (contentsBottom < renderHeight) {
				_scrollOffset -= (renderHeight - contentsBottom);
			}
			if (_scrollOffset < 0)
				_scrollOffset = 0;

			int32 srcLeft = 0;
			int32 srcTop = 0;
			int32 srcRight = _toolSurface->w;
			int32 srcBottom = _toolSurface->h;
			int32 destLeft = 0;
			int32 destRight = _toolSurface->w;
			int32 destTop = -_scrollOffset;
			int32 destBottom = _toolSurface->h - _scrollOffset;

			if (srcTop < 0) {
				int32 adjust = -srcTop;
				destTop += adjust;
				srcTop += adjust;
			}
			if (destTop < 0) {
				int32 adjust = -destTop;
				destTop += adjust;
				srcTop += adjust;
			}
			if (srcBottom > _toolSurface->h) {
				int32 adjust = srcBottom - _toolSurface->h;
				destBottom += adjust;
				srcBottom += adjust;
			}
			if (destBottom > renderHeight) {
				int32 adjust = destBottom - renderHeight;
				destBottom += adjust;
				srcBottom += adjust;
			}
			if (srcLeft < 0) {
				int32 adjust = -srcLeft;
				destLeft += adjust;
				srcLeft += adjust;
			}
			if (destLeft < 0) {
				int32 adjust = -destLeft;
				destLeft += adjust;
				srcLeft += adjust;
			}
			if (srcRight > _toolSurface->w) {
				int32 adjust = srcRight - _toolSurface->w;
				destRight += adjust;
				srcRight += adjust;
			}
			if (destRight > renderWidth) {
				int32 adjust = destRight - renderWidth;
				destRight += adjust;
				srcRight += adjust;
			}

			if (srcLeft >= srcRight || srcTop >= srcBottom)
				return;

			getSurface()->fillRect(Common::Rect(0, kTopBarHeight, renderWidth, getHeight()), getSurface()->format.RGBToColor(255, 255, 255));
			getSurface()->rawBlitFrom(*_toolSurface.get(), Common::Rect(srcLeft, srcTop, srcRight, srcBottom), Common::Point(destLeft, destTop + kTopBarHeight), nullptr);
		}
	}
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
				_toolSurface.reset();
				resizeWindow(newWidth, newHeight);
				_isDirty = true;
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

void DebugToolWindowBase::setDirty() {
	_isDirty = true;
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

class DebugSceneTreeWindow : public DebugToolWindowBase {
public:
	DebugSceneTreeWindow(Debugger *debugger, const WindowParameters &windowParams);

	void update() override;
	void toolRenderSurface(int32 subAreaWidth, int32 subAreaHeight) override;

	void toolOnMouseDown(int32 x, int32 y, int mouseButton) override;

private:
	static const int kRowHeight = 12;
	static const int kBaseLeftPadding = 14;
	static const int kExpanderLeftOffset = 8;
	static const int kPerLevelSpacing = 14;

	struct SceneTreeEntryUIState {
		SceneTreeEntryUIState();

		bool expanded;
	};

	struct SceneTreeEntry {
		SceneTreeEntryUIState uiState;
		size_t parentIndex;
		int level;
		bool hasChildren;
		Common::WeakPtr<RuntimeObject> object;
	};

	struct RenderEntry {
		size_t treeIndex;
		size_t parentRenderIndex;
	};

	static void recursiveBuildTree(int level, size_t parentIndex, RuntimeObject *object, Common::Array<SceneTreeEntry> &tree);

	Common::Array<SceneTreeEntry> _tree;
	Common::Array<RenderEntry> _renderEntries;
	bool _forceRender;
};

DebugSceneTreeWindow::SceneTreeEntryUIState::SceneTreeEntryUIState() : expanded(false) {
}

DebugSceneTreeWindow::DebugSceneTreeWindow(Debugger *debugger, const WindowParameters &windowParams)
	: DebugToolWindowBase(kDebuggerToolSceneTree, "Project", debugger, windowParams), _forceRender(true) {
}

void DebugSceneTreeWindow::update() {
	bool needRerender = _forceRender;

	// This is super expensive but still less expensive than a redraw and we're only using it to debug,
	// so kind of just eating the massive perf hit...
	Common::HashMap<RuntimeObject *, SceneTreeEntryUIState> stateCache;
	for (const SceneTreeEntry &treeEntry : _tree) {
		Common::SharedPtr<RuntimeObject> obj = treeEntry.object.lock();
		if (obj) {
			stateCache[obj.get()] = treeEntry.uiState;
		} else {
			needRerender = true;
			continue;
		}
	}

	size_t oldSize = _tree.size();

	// Keep existing reserve
	_tree.resize(0);

	Project *project = _debugger->getRuntime()->getProject();
	if (project)
		recursiveBuildTree(0, 0, project, _tree);

	if (_tree.size() != oldSize)
		needRerender = true;

	for (SceneTreeEntry &treeEntry : _tree) {
		Common::HashMap<RuntimeObject *, SceneTreeEntryUIState>::const_iterator oldStateIt = stateCache.find(treeEntry.object.lock().get());
		if (oldStateIt != stateCache.end())
			treeEntry.uiState = oldStateIt->_value;
	}

	if (needRerender) {
		setDirty();
		_renderEntries.clear();
		_forceRender = false;
	}
}

void DebugSceneTreeWindow::toolRenderSurface(int32 subAreaWidth, int32 subAreaHeight) {
	Common::HashMap<const SceneTreeEntry *, size_t> treeToRenderIndex;
	_renderEntries.clear();

	treeToRenderIndex[&_tree[0]] = 0;

	size_t lastParentIndex = _tree.size(); // So we can skip some hash map lookups, yuck
	size_t lastParentRenderIndex = 0;
	bool lastParentExpanded = true;

	size_t numTreeNodes = _tree.size();
	for (size_t i = 0; i < numTreeNodes; i++) {
		const SceneTreeEntry &entry = _tree[i];
		size_t parentIndex = entry.parentIndex;
		size_t parentRenderIndex = 0;
		bool isParentExpanded = false;
		if (i == 0) {
			isParentExpanded = true;
			parentRenderIndex = 0;
		} else if (parentIndex == lastParentIndex) {
			isParentExpanded = lastParentExpanded;
			parentRenderIndex = lastParentRenderIndex;
		}else {
			const SceneTreeEntry *parent = &_tree[entry.parentIndex];
			if (parent->uiState.expanded) {
				// Parent is expanded, figure out if it's actually rendered
				Common::HashMap<const SceneTreeEntry *, size_t>::const_iterator t2r = treeToRenderIndex.find(parent);
				if (t2r != treeToRenderIndex.end()) {
					isParentExpanded = true;
					parentRenderIndex = t2r->_value;
				}
			}

			lastParentIndex = entry.parentIndex;
			lastParentRenderIndex = parentRenderIndex;
			lastParentExpanded = isParentExpanded;
		}

		if (isParentExpanded) {
			treeToRenderIndex[&entry] = _renderEntries.size();

			RenderEntry renderEntry;
			renderEntry.treeIndex = i;
			renderEntry.parentRenderIndex = parentRenderIndex;

			_renderEntries.push_back(renderEntry);
		}
	}

	Graphics::PixelFormat fmt = getSurface()->format;

	int32 width = subAreaWidth;
	int32 height = static_cast<int32>(_renderEntries.size()) * kRowHeight;
	if (!_toolSurface || (height != _toolSurface->h || width != _toolSurface->w)) {
		_toolSurface.reset();
		_toolSurface.reset(new Graphics::ManagedSurface(subAreaWidth, height, fmt));
	}

	uint32 whiteColor = fmt.RGBToColor(255, 255, 255);
	uint32 lightGrayColor = fmt.RGBToColor(192, 192, 192);
	uint32 blackColor = fmt.RGBToColor(0, 0, 0);

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);

	_toolSurface->fillRect(Common::Rect(0, 0, width, height), whiteColor);

	for (size_t row = 0; row < _renderEntries.size(); row++) {
		const RenderEntry &renderEntry = _renderEntries[row];
		const SceneTreeEntry &entry = _tree[renderEntry.treeIndex];

		Common::SharedPtr<RuntimeObject> obj = entry.object.lock();
		if (!obj)
			continue;	// ???

		int32 y = static_cast<int32>(row) * kRowHeight + (kRowHeight - font->getFontAscent()) / 2;
		int32 x = kBaseLeftPadding + kPerLevelSpacing * entry.level;

		Common::String name;
		if (obj->isModifier())
			name = static_cast<const Modifier *>(obj.get())->getName();
		else if (obj->isStructural())
			name = static_cast<const Structural *>(obj.get())->getName();

		font->drawString(_toolSurface.get(), name, x, y, width - x, blackColor, Graphics::kTextAlignLeft, 0, true);

		if (entry.hasChildren) {
			int32 expanderCenterX = x - kExpanderLeftOffset;
			int32 expanderCenterY = static_cast<int32>(row) * kRowHeight + (kRowHeight / 2);
			_toolSurface->frameRect(Common::Rect(expanderCenterX - 4, expanderCenterY - 4, expanderCenterX + 5, expanderCenterY + 5), blackColor);
			_toolSurface->drawLine(expanderCenterX - 2, expanderCenterY, expanderCenterX + 2, expanderCenterY, blackColor);
			if (!entry.uiState.expanded)
				_toolSurface->drawLine(expanderCenterX, expanderCenterY - 2, expanderCenterX, expanderCenterY + 2, blackColor);
		}
	}

	Common::Array<bool> haveRenderedParentTracers;
	haveRenderedParentTracers.resize(_renderEntries.size());
	for (size_t i = 0; i < _renderEntries.size(); i++)
		haveRenderedParentTracers[i] = false;

	for (size_t ri = 0; ri < _renderEntries.size(); ri++) {
		size_t row = _renderEntries.size() - 1 - ri;
		const RenderEntry &renderEntry = _renderEntries[row];

		if (row == 0)
			continue;

		const RenderEntry &parentRenderEntry = _renderEntries[renderEntry.parentRenderIndex];
		const SceneTreeEntry &treeEntry = _tree[renderEntry.treeIndex];
		const SceneTreeEntry &parentTreeEntry = _tree[parentRenderEntry.treeIndex];

		int32 x = kBaseLeftPadding + kPerLevelSpacing * treeEntry.level;

		int32 parentTracerRightX = x - 2;
		int32 parentTracerY = static_cast<int32>(row) * kRowHeight + (kRowHeight / 2);
		if (treeEntry.hasChildren)
			parentTracerRightX -= kExpanderLeftOffset + 7;

		int32 parentTracerLeftX = kBaseLeftPadding + kPerLevelSpacing * parentTreeEntry.level - kExpanderLeftOffset;

		_toolSurface->drawLine(parentTracerRightX, parentTracerY, parentTracerLeftX, parentTracerY, lightGrayColor);
		if (!haveRenderedParentTracers[renderEntry.parentRenderIndex]) {
			haveRenderedParentTracers[renderEntry.parentRenderIndex] = true;

			int32 parentTracerTopY = static_cast<int32>(renderEntry.parentRenderIndex + 1) * kRowHeight;
			_toolSurface->drawLine(parentTracerLeftX, parentTracerY, parentTracerLeftX, parentTracerTopY, lightGrayColor);
		}
	}
}

void DebugSceneTreeWindow::toolOnMouseDown(int32 x, int32 y, int mouseButton) {
	if (mouseButton != Actions::kMouseButtonLeft)
		return;

	if (y < 0)
		return;

	int32 row = y / kRowHeight;

	if (row >= _renderEntries.size())
		return;

	const RenderEntry &renderEntry = _renderEntries[row];
	SceneTreeEntry &treeEntry = _tree[renderEntry.treeIndex];

	int32 expanderCenterX = kBaseLeftPadding - kExpanderLeftOffset;
	int32 expanderCenterY = row * kRowHeight + kRowHeight / 2;

	if (x >= expanderCenterX - 5 && x <= expanderCenterX + 5 && y >= expanderCenterX - 5 && y <= expanderCenterY + 5) {
		// Clicked the expander
		treeEntry.uiState.expanded = !treeEntry.uiState.expanded;
		_forceRender = true;
		return;
	}
}

void DebugSceneTreeWindow::recursiveBuildTree(int level, size_t parentIndex, RuntimeObject *object, Common::Array<SceneTreeEntry> &tree) {
	SceneTreeEntry treeEntry;
	treeEntry.level = level;
	treeEntry.object = object->getSelfReference();
	treeEntry.parentIndex = parentIndex;
	treeEntry.hasChildren = false;

	size_t thisIndex = tree.size();
	tree.push_back(treeEntry);

	if (object->isStructural()) {
		Structural *structural = static_cast<Structural *>(object);
		for (const Common::SharedPtr<Modifier> &modifier : structural->getModifiers())
			recursiveBuildTree(level + 1, thisIndex, modifier.get(), tree);
		for (const Common::SharedPtr<Structural> &child : structural->getChildren())
			recursiveBuildTree(level + 1, thisIndex, child.get(), tree);
	} else if (object->isModifier()) {
		IModifierContainer *childContainer = static_cast<Modifier *>(object)->getChildContainer();
		if (childContainer) {
			for (const Common::SharedPtr<Modifier> &child : childContainer->getModifiers())
				recursiveBuildTree(level + 1, thisIndex, child.get(), tree);
		}
	}

	if (tree.size() - thisIndex > 1)
		tree[thisIndex].hasChildren = true;
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

	for (const Common::SharedPtr<DebugToolWindowBase> &toolWindow : _toolWindows) {
		if (toolWindow) {
			toolWindow->update();
			toolWindow->render();
		}
	}
}

void Debugger::setPaused(bool paused) {
	_paused = paused;
}

bool Debugger::isPaused() const {
	return _paused;
}

Runtime *Debugger::getRuntime() const {
	return _runtime;
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

	Common::SharedPtr<DebugToolWindowBase> &windowRef = _toolWindows[tool];
	if (windowRef)
		return;

	switch (tool) {
	case kDebuggerToolSceneTree:
		windowRef.reset(new DebugSceneTreeWindow(this, WindowParameters(_runtime, 32, 32, 100, 320, _runtime->getRenderPixelFormat())));
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
