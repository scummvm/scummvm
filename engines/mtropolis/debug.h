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

#ifndef MTROPOLIS_DEBUG_H
#define MTROPOLIS_DEBUG_H

#include "common/str.h"
#include "common/ptr.h"
#include "common/hashmap.h"

#define MTROPOLIS_DEBUG_VTHREAD_STACKS

#define MTROPOLIS_DEBUG_ENABLE
#if defined(MTROPOLIS_DEBUG_ENABLE) && !defined(MTROPOLIS_DEBUG_VTHREAD_STACKS)
// VThread stack debugging is mandatory when debugging
#define MTROPOLIS_DEBUG_VTHREAD_STACKS
#endif

namespace MTropolis {

#ifdef MTROPOLIS_DEBUG_ENABLE

class Runtime;
class Window;
class Structural;
class Modifier;
class DebugToolWindowBase;

struct IDebuggable;

enum SupportStatus {
	kSupportStatusNone,
	kSupportStatusPartial,
	kSupportStatusDone,
};

class DebugInspector {
public:
	explicit DebugInspector(IDebuggable *debuggable);
	virtual ~DebugInspector();

	virtual void onDestroyed();

	void onDebuggableRelocated(IDebuggable *debuggable);

private:
	IDebuggable *_debuggable;
};

class DebugPrimaryTaskList {
public:
	explicit DebugPrimaryTaskList(const Common::String &name);

	void addItem(IDebuggable *debuggable);
	const Common::Array<IDebuggable *> &getItems() const;

	const Common::String &getName() const;

private:
	Common::String _name;
	Common::Array<IDebuggable *> _primaryTasks;
};

enum DebugSeverity {
	kDebugSeverityInfo,
	kDebugSeverityWarning,
	kDebugSeverityError,
};

class Notification {
public:

private:
	Runtime *_runtime;
};

enum DebuggerTool {
	kDebuggerToolSceneTree,
	kDebuggerToolInspector,
	kDebuggerToolStepThrough,

	kDebuggerToolCount,
};

class Debugger {
public:
	explicit Debugger(Runtime *runtime);
	~Debugger();

	// runFrame runs after the frame, before rendering, and before event processing for the following frame
	void runFrame(uint32 msec);

	void setPaused(bool paused);
	bool isPaused() const;

	Runtime *getRuntime() const;

	void notify(DebugSeverity severity, const Common::String &str);
	void notifyFmt(DebugSeverity severity, const char *fmt, ...);
	void vnotifyFmt(DebugSeverity severity, const char *fmt, va_list args);

	void refreshSceneStatus();
	void complainAboutUnfinished(Structural *structural);

	void openToolWindow(DebuggerTool tool);
	void closeToolWindow(DebuggerTool tool);

private:
	Debugger();

	static void scanStructuralStatus(Structural *structural, Common::HashMap<Common::String, SupportStatus> &unfinishedModifiers, Common::HashMap<Common::String, SupportStatus> &unfinishedElements);
	static void scanModifierStatus(Modifier *modifier, Common::HashMap<Common::String, SupportStatus> &unfinishedModifiers, Common::HashMap<Common::String, SupportStatus> &unfinishedElements);
	static void scanDebuggableStatus(IDebuggable *debuggable, Common::HashMap<Common::String, SupportStatus> &unfinished);

	struct ToastNotification {
		Common::SharedPtr<Window> window;
		uint64 dismissTime;
	};

	bool _paused;
	Runtime *_runtime;
	Common::SharedPtr<Window> _sceneStatusWindow;
	Common::SharedPtr<Window> _toolsWindow;
	Common::SharedPtr<DebugToolWindowBase> _toolWindows[kDebuggerToolCount];
	Common::Array<ToastNotification> _toastNotifications;
};

#endif /* !MTROPOLIS_DEBUG_ENABLE */

struct IDebuggable {
#ifdef MTROPOLIS_DEBUG_ENABLE
	virtual SupportStatus debugGetSupportStatus() const = 0;
	virtual const char *debugGetTypeName() const = 0;
	virtual const Common::String &debugGetName() const = 0;
	virtual Common::SharedPtr<DebugInspector> debugGetInspector() const = 0;
#endif
};

} // End of namespace MTropolis

#endif /* MTROPOLIS_DEBUG_H */
