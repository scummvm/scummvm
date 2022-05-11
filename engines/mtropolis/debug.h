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
class DebugInspector;
class DebugToolWindowBase;
class RuntimeObject;

struct IDebuggable;

enum SupportStatus {
	kSupportStatusNone,
	kSupportStatusPartial,
	kSupportStatusDone,
};

struct IDebugInspectionReport {
	// Attempts to declare a row with static contents.  If this returns true, then declareStaticContents must be called.
	virtual bool declareStatic(const char *name) = 0;

	// Declares the contents of a static row
	virtual void declareStaticContents(const Common::String &data) = 0;

	// Declares the contents of a dynamic row
	virtual void declareDynamic(const char *name, const Common::String &data) = 0;

	// Declares the contents of a loose row
	virtual void declareLoose(const Common::String &data) = 0;
};

struct IDebuggable {
	virtual SupportStatus debugGetSupportStatus() const = 0;
	virtual const char *debugGetTypeName() const = 0;
	virtual const Common::String &debugGetName() const = 0;
	virtual const Common::SharedPtr<DebugInspector> &debugGetInspector() = 0;
	virtual void debugInspect(IDebugInspectionReport *report) const = 0;
};

// The debug inspector link goes inside of a debuggable, it contains an inspector and
// will notify the inspector of the object's destruction
class Debuggable : public IDebuggable {
public:
	Debuggable();
	Debuggable(const Debuggable &other);
	Debuggable(Debuggable &&other);
	~Debuggable();

private:
	const Common::SharedPtr<DebugInspector> &debugGetInspector() override;

	Common::SharedPtr<DebugInspector> _inspector;
};

class DebugInspector {
public:
	explicit DebugInspector(IDebuggable *debuggable);
	virtual ~DebugInspector();

	void onDestroyed(IDebuggable *instance);
	void changePrimaryInstance(IDebuggable *instance);

	IDebuggable *getDebuggable() const;

private:
	IDebuggable *_instance;
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

	void inspectObject(IDebuggable *debuggable);
	void tryInspectObject(RuntimeObject *object);

	const Common::SharedPtr<DebugInspector> &getInspector() const;

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
	Common::SharedPtr<DebugInspector> _inspector;
};

#else

struct IDebuggable {
};

struct Debuggable : public IDebuggable {
};

#endif /* !MTROPOLIS_DEBUG_ENABLE */

} // End of namespace MTropolis

#endif /* MTROPOLIS_DEBUG_H */
