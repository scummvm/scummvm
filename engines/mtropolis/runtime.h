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

#ifndef MTROPOLIS_RUNTIME_H
#define MTROPOLIS_RUNTIME_H

#include "common/array.h"
#include "common/events.h"
#include "common/language.h"
#include "common/platform.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "graphics/pixelformat.h"

#include "mtropolis/actions.h"
#include "mtropolis/core.h"
#include "mtropolis/data.h"
#include "mtropolis/debug.h"
#include "mtropolis/hacks.h"
#include "mtropolis/subtitles.h"
#include "mtropolis/vthread.h"

class OSystem;

namespace Audio {

class Mixer;

} // End of namespace Audio

namespace Common {

class RandomSource;

} // End of namespace Common

namespace Graphics {

struct WinCursorGroup;
class MacCursor;
class MacFontManager;
class ManagedSurface;
class Cursor;
struct PixelFormat;
struct Surface;

} // End of namespace Graphics

namespace MTropolis {

class Asset;
class AssetManagerInterface;
class CursorGraphic;
class CursorGraphicCollection;
class Element;
class GraphicModifier;
class KeyboardInputEvent;
class MessageDispatch;
class MiniscriptThread;
class Modifier;
class ObjectLinkingScope;
class PlugInModifier;
class RuntimeObject;
class PlugIn;
class Project;
class Runtime;
class Structural;
class SystemInterface;
class VisualElement;
class Window;
class WorldManagerInterface;
struct DynamicValue;
struct DynamicValueWriteProxy;
struct IBoundaryDetector;
struct ICollider;
struct ILoadUIProvider;
struct IMessageConsumer;
struct IModifierContainer;
struct IPlugInModifierFactory;
struct IPlugInModifierFactoryAndDataFactory;
struct IPostEffect;
struct ISaveUIProvider;
struct ISaveWriter;
struct IStructuralReferenceVisitor;
struct MessageProperties;
struct ModifierLoaderContext;
struct PlugInModifierLoaderContext;
struct SIModifierFactory;
template<typename TElement, typename TElementData> class ElementFactory;

enum MiniscriptInstructionOutcome {
	kMiniscriptInstructionOutcomeContinue,					// Continue executing next instruction
	kMiniscriptInstructionOutcomeYieldToVThreadNoRetry,		// Instruction pushed a VThread task and should be retried when the task completes
	kMiniscriptInstructionOutcomeYieldToVThreadAndRetry,	// Instruction pushed a VThread task and completed
	kMiniscriptInstructionOutcomeFailed,					// Instruction errored
};

#ifdef MTROPOLIS_DEBUG_ENABLE
class DebugPrimaryTaskList;
#endif

char invariantToLower(char c);
Common::String toCaseInsensitive(const Common::String &str);
bool caseInsensitiveEqual(const Common::String &str1, const Common::String &str2);
size_t caseInsensitiveFind(const Common::String &stringToSearch, const Common::String &stringToFind);

enum ColorDepthMode {
	kColorDepthMode1Bit,
	kColorDepthMode2Bit,
	kColorDepthMode4Bit,
	kColorDepthMode8Bit,
	kColorDepthMode16Bit,
	kColorDepthMode32Bit,

	kColorDepthModeCount,

	kColorDepthModeInvalid,
};

namespace SceneTransitionTypes {

enum SceneTransitionType {
	kNone,
	kPatternDissolve,
	kRandomDissolve, // No steps
	kFade,
	kSlide, // Directional
	kPush,  // Directional
	kZoom,
	kWipe, // Directional
};

bool loadFromData(SceneTransitionType &transType, int32 data);

} // End of namespace SceneTransitionTypes

namespace SceneTransitionDirections {

enum SceneTransitionDirection {
	kUp,
	kDown,
	kLeft,
	kRight,
};

bool loadFromData(SceneTransitionDirection &transDir, int32 data);

} // End of namespace SceneTransitionDirections

enum ConstraintDirection {
	kConstraintDirectionNone,
	kConstraintDirectionHorizontal,
	kConstraintDirectionVertical,
};

enum MouseInteractivityTestType {
	kMouseInteractivityTestAnything,
	kMouseInteractivityTestMouseClick,
};

namespace DynamicValueTypes {

enum DynamicValueType {
	kInvalid,

	kNull,
	kInteger,
	kFloat,
	kPoint,
	kIntegerRange,
	kBoolean,
	kVector,
	kLabel,
	kEvent,
	kVariableReference,
	kIncomingData,
	kString,
	kList,
	kObject,
	kWriteProxy,

	kEmpty,
};

} // End of namespace DynamicValuesTypes

namespace AttributeIDs {

enum AttributeID {
	kAttribCache = 55,
	kAttribDirect = 56,
	kAttribVisible = 58,
	kAttribLayer = 24,
	kAttribPaused = 25,
	kAttribLoop = 57,
	kAttribPosition = 1,
	kAttribWidth = 2,
	kAttribHeight = 3,
	kAttribRate = 4,
	kAttribRange = 5,
	kAttribCel = 6,
	kAttribLoopBackForth = 59,
	kAttribPlayEveryFrame = 60,
	kAttribTimeValue = 16,
	kAttribTrackDisable = 51,
	kAttribTrackEnable = 50,
	kAttribVolume = 13,
	kAttribBalance = 26,
	kAttribText = 7,
	kAttribMasterVolume = 18,
	kAttribUserTimeout = 19,
};

} // End of namespace AttributeIDs

namespace EventIDs {

enum EventID {
	kNothing = 0,

	kElementEnableEdit = 207,
	kElementDisableEdit = 220,
	kElementSelect = 209,
	kElementDeselect = 210,
	kElementToggleSelect = 213,
	kElementUpdatedCalculated = 219,
	kElementShow = 222,
	kElementHide = 223,

	kElementScrollUp = 1001,
	kElementScrollDown = 1002,
	kElementScrollRight = 1005,
	kElementScrollLeft = 1006,

	kMotionStarted = 501,
	kMotionEnded = 502,

	kTransitionStarted = 503,
	kTransitionEnded = 504,

	kMouseDown = 301,
	kMouseUp = 302,
	kMouseOver = 303,
	kMouseOutside = 304,
	kMouseTrackedInside = 305,
	kMouseTrackedOutside = 306,
	kMouseTracking = 307,
	kMouseUpInside = 309,
	kMouseUpOutside = 310,

	kSceneStarted = 101,
	kSceneEnded = 102,
	kSceneDeactivated = 103,
	kSceneReactivated = 104,
	kSceneTransitionEnded = 506,

	kSharedSceneReturnedToScene = 401,
	kSharedSceneSceneChanged = 402,
	kSharedSceneNoNextScene = 403,
	kSharedSceneNoPrevScene = 404,

	kParentEnabled = 2001,
	kParentDisabled = 2002,
	kParentChanged = 227,

	kPreloadMedia = 1701,
	kFlushMedia = 1703,
	kPrerollMedia = 1704,

	kCloseProject = 1601,

	kUserTimeout = 1801,
	kProjectStarted = 1802,
	kProjectEnded = 1803,
	kFlushAllMedia = 1804,

	kAttribGet = 1300,
	kAttribSet = 1200,

	kClone = 226,
	kKill = 228,

	kPlay = 201,
	kStop = 202,
	kPause = 801,
	kUnpause = 802,
	kTogglePause = 803,
	kAtFirstCel = 804,
	kAtLastCel = 805,


	kAuthorMessage = 900,
};

bool isCommand(EventID eventID);

} // End of namespace EventIDs

MiniscriptInstructionOutcome pointWriteRefAttrib(Common::Point &point, MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib);
Common::String pointToString(const Common::Point &point);

struct IntRange {
	IntRange();
	IntRange(int32 pmin, int32 pmax);

	int32 min;
	int32 max;

	bool load(const Data::IntRange &range);

	inline bool operator==(const IntRange &other) const {
		return min == other.min && max == other.max;
	}

	inline bool operator!=(const IntRange &other) const {
		return !((*this) == other);
	}

	MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib);
	Common::String toString() const;
};

struct Label {
	Label();
	Label(int32 psuperGroupID, int32 pid);

	uint32 superGroupID;
	uint32 id;

	bool load(const Data::Label &label);

	inline bool operator==(const Label &other) const {
		return superGroupID == other.superGroupID && id == other.id;
	}

	inline bool operator!=(const Label &other) const {
		return !((*this) == other);
	}
};

struct Event {
	Event();
	Event(EventIDs::EventID peventType, uint32 peventInfo);

	EventIDs::EventID eventType;
	uint32 eventInfo;

	// Returns true if this event, interpreted as a filter, recognizes another event.
	// Handles cases where eventInfo is ignored (hopefully).
	bool respondsTo(const Event &otherEvent) const;

	bool load(const Data::Event &data);

	inline bool operator==(const Event &other) const {
		return eventType == other.eventType && eventInfo == other.eventInfo;
	}

	inline bool operator!=(const Event &other) const {
		return !((*this) == other);
	}
};

struct VarReference {
	VarReference();
	VarReference(uint32 pguid, const Common::String &psource);

	uint32 guid;
	Common::String source;

	inline bool operator==(const VarReference &other) const {
		return guid == other.guid && source == other.source;
	}

	inline bool operator!=(const VarReference &other) const {
		return !((*this) == other);
	}

	bool resolve(Structural *structuralScope, Common::WeakPtr<RuntimeObject> &outObject) const;
	bool resolve(Modifier *modifierScope, Common::WeakPtr<RuntimeObject> &outObject) const;

private:
	bool resolveContainer(IModifierContainer *modifierContainer, Common::WeakPtr<RuntimeObject> &outObject) const;
	bool resolveSingleModifier(Modifier *modifier, Common::WeakPtr<RuntimeObject> &outObject) const;
};

struct ObjectReference {
	Common::WeakPtr<RuntimeObject> object;

	inline ObjectReference() {
	}

	inline explicit ObjectReference(const Common::WeakPtr<RuntimeObject> objectPtr) : object(objectPtr) {
	}

	inline bool operator==(const ObjectReference &other) const {
		return !object.owner_before(other.object) && !other.object.owner_before(object);
	}

	inline bool operator!=(const ObjectReference &other) const {
		return !((*this) == other);
	}

	inline void reset() {
		object.reset();
	}
};

struct AngleMagVector {
	AngleMagVector();

	double angleDegrees; // These are stored as radians in the data but scripts treat them as degrees so it's just pointless constantly doing conversion...
	double magnitude;

	inline bool operator==(const AngleMagVector &other) const {
		return angleDegrees == other.angleDegrees && magnitude == other.magnitude;
	}

	inline bool operator!=(const AngleMagVector &other) const {
		return !((*this) == other);
	}

	inline static AngleMagVector createRadians(double angleRadians, double magnitude) {
		return AngleMagVector(angleRadians * (180.0 / M_PI), magnitude);
	}

	inline static AngleMagVector createDegrees(double angleDegrees, double magnitude) {
		return AngleMagVector(angleDegrees, magnitude);
	}

	MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib);
	Common::String toString() const;

private:
	AngleMagVector(double angleDegrees, double magnitude);
};

struct ColorRGB8 {
	ColorRGB8();
	ColorRGB8(uint8 pr, uint8 pg, uint8 pb);

	uint8 r;
	uint8 g;
	uint8 b;

	bool load(const Data::ColorRGB16 &color);

	inline bool operator==(const ColorRGB8 &other) const {
		return r == other.r && g == other.g && b == other.b;
	}

	inline bool operator!=(const ColorRGB8 &other) const {
		return !((*this) == other);
	}
};

struct MessageFlags {
	MessageFlags();

	bool relay : 1;
	bool cascade : 1;
	bool immediate : 1;
};

struct DynamicValue;
struct DynamicList;

// This should be an interface, but since this exists to make global singletons that JUST supply a vtable,
// GCC complains about there being a global destructor, unless we delete the destructor, in which case
// it complains about a class having virtual functions but not having a virtual destructor, so we have to
// do this dispatch table stuff just to make GCC be quiet.
struct DynamicValueWriteInterface {
	typedef MiniscriptInstructionOutcome (*writeFunc_t)(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
	typedef MiniscriptInstructionOutcome (*refAttribFunc_t)(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
	typedef MiniscriptInstructionOutcome (*refAttribIndexedFunc_t)(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);

	writeFunc_t write;
	refAttribFunc_t refAttrib;
	refAttribIndexedFunc_t refAttribIndexed;
};

template<class T>
class DynamicValueWriteInterfaceGlue {
public:
	static const DynamicValueWriteInterface *getInstance();

private:
	static DynamicValueWriteInterface _instance;
};

template<class T>
inline const DynamicValueWriteInterface *DynamicValueWriteInterfaceGlue<T>::getInstance() {
	return &_instance;
}

template<class T>
DynamicValueWriteInterface DynamicValueWriteInterfaceGlue<T>::_instance = {
	static_cast<DynamicValueWriteInterface::writeFunc_t>(T::write),
	static_cast<DynamicValueWriteInterface::refAttribFunc_t>(T::refAttrib),
	static_cast<DynamicValueWriteInterface::refAttribIndexedFunc_t>(T::refAttribIndexed),
};



struct DynamicValueWriteProxyPOD {
	uintptr ptrOrOffset;
	void *objectRef;
	const DynamicValueWriteInterface *ifc;

	static DynamicValueWriteProxyPOD createDefault();
};

struct DynamicValueWriteProxy {
	DynamicValueWriteProxy();

	DynamicValueWriteProxyPOD pod;
	Common::SharedPtr<DynamicList> containerList;
};

struct Point16POD {
	int16 x;
	int16 y;

	Common::Point toScummVMPoint() const;
};


class DynamicListContainerBase {
public:
	virtual ~DynamicListContainerBase();
	virtual bool setAtIndex(size_t index, const DynamicValue &dynValue) = 0;
	virtual bool getAtIndex(size_t index, DynamicValue &dynValue) const = 0;
	virtual void truncateToSize(size_t sz) = 0;
	virtual bool expandToMinimumSize(size_t sz) = 0;
	virtual void setFrom(const DynamicListContainerBase &other) = 0; // Only supports setting same type!
	virtual const void *getConstArrayPtr() const = 0;
	virtual void *getArrayPtr() = 0;
	virtual size_t getSize() const = 0;
	virtual bool compareEqual(const DynamicListContainerBase &other) const = 0;
	virtual DynamicListContainerBase *clone() const = 0;
};

struct DynamicListDefaultSetter {
	static void defaultSet(int32 &value);
	static void defaultSet(double &value);
	static void defaultSet(Common::Point &value);
	static void defaultSet(IntRange &value);
	static void defaultSet(bool &value);
	static void defaultSet(AngleMagVector &value);
	static void defaultSet(Label &value);
	static void defaultSet(Event &value);
	static void defaultSet(Common::String &value);
	static void defaultSet(Common::SharedPtr<DynamicList> &value);
	static void defaultSet(ObjectReference &value);
};

template<class T>
struct DynamicListValueConverter {
	typedef T DynamicValuePODType_t;

	static const T &dereference(const T *source) { return *source; }
};

struct DynamicListValueImporter {
	static bool importValue(const DynamicValue &dynValue, const int32 *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const double *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const Common::Point *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const IntRange *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const bool *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const AngleMagVector *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const Label *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const Event *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const Common::String *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const Common::SharedPtr<DynamicList> *&outPtr);
	static bool importValue(const DynamicValue &dynValue, const ObjectReference *&outPtr);
};

struct DynamicListValueExporter {
	static void exportValue(DynamicValue &dynValue, const int32 &value);
	static void exportValue(DynamicValue &dynValue, const double &value);
	static void exportValue(DynamicValue &dynValue, const Common::Point &value);
	static void exportValue(DynamicValue &dynValue, const IntRange &value);
	static void exportValue(DynamicValue &dynValue, const bool &value);
	static void exportValue(DynamicValue &dynValue, const AngleMagVector &value);
	static void exportValue(DynamicValue &dynValue, const Label &value);
	static void exportValue(DynamicValue &dynValue, const Event &value);
	static void exportValue(DynamicValue &dynValue, const Common::String &value);
	static void exportValue(DynamicValue &dynValue, const Common::SharedPtr<DynamicList> &value);
	static void exportValue(DynamicValue &dynValue, const ObjectReference &value);
};

template<class T>
class DynamicListContainer : public DynamicListContainerBase {
public:
	bool setAtIndex(size_t index, const DynamicValue &dynValue) override;
	bool getAtIndex(size_t index, DynamicValue &dynValue) const override;
	void truncateToSize(size_t sz) override;
	bool expandToMinimumSize(size_t sz) override;
	void setFrom(const DynamicListContainerBase &other) override;
	const void *getConstArrayPtr() const override;
	void *getArrayPtr() override;
	size_t getSize() const override;
	bool compareEqual(const DynamicListContainerBase &other) const override;
	DynamicListContainerBase *clone() const override;

private:
	Common::Array<T> _array;
};

template<>
class DynamicListContainer<void> : public DynamicListContainerBase {
public:
	DynamicListContainer();

	bool setAtIndex(size_t index, const DynamicValue &dynValue) override;
	bool getAtIndex(size_t index, DynamicValue &dynValue) const override;
	void truncateToSize(size_t sz) override;
	bool expandToMinimumSize(size_t sz) override;
	void setFrom(const DynamicListContainerBase &other) override;
	const void *getConstArrayPtr() const override;
	void *getArrayPtr() override;
	size_t getSize() const override;
	bool compareEqual(const DynamicListContainerBase &other) const override;
	DynamicListContainerBase *clone() const override;

public:
	size_t _size;
};

template<>
class DynamicListContainer<VarReference> : public DynamicListContainerBase {
public:
	bool setAtIndex(size_t index, const DynamicValue &dynValue) override;
	bool getAtIndex(size_t index, DynamicValue &dynValue) const override;
	void truncateToSize(size_t sz) override;
	bool expandToMinimumSize(size_t sz) override;
	void setFrom(const DynamicListContainerBase &other) override;
	const void *getConstArrayPtr() const override;
	void *getArrayPtr() override;
	size_t getSize() const override;
	bool compareEqual(const DynamicListContainerBase &other) const override;
	DynamicListContainerBase *clone() const override;

private:
	void rebuildStringPointers();

	Common::Array<VarReference> _array;
};

template<class T>
bool DynamicListContainer<T>::setAtIndex(size_t index, const DynamicValue &dynValue) {
	const typename DynamicListValueConverter<T>::DynamicValuePODType_t *valuePtr = nullptr;
	if (!DynamicListValueImporter::importValue(dynValue, valuePtr))
		return false;

	_array.reserve(index + 1);
	if (_array.size() <= index) {
		if (_array.size() < index) {
			T defaultValue;
			DynamicListDefaultSetter::defaultSet(defaultValue);
			while (_array.size() < index) {
				_array.push_back(defaultValue);
			}
		}
		_array.push_back(DynamicListValueConverter<T>::dereference(valuePtr));
	} else {
		_array[index] = DynamicListValueConverter<T>::dereference(valuePtr);
	}

	return true;
}

template<class T>
void DynamicListContainer<T>::truncateToSize(size_t sz) {
	if (_array.size() > sz)
		_array.resize(sz);
}

template<class T>
bool DynamicListContainer<T>::expandToMinimumSize(size_t sz) {
	_array.reserve(sz);
	if (_array.size() < sz) {
		T defaultValue;
		DynamicListDefaultSetter::defaultSet(defaultValue);
		while (_array.size() < sz) {
			_array.push_back(defaultValue);
		}
	}

	return true;
}

template<class T>
bool DynamicListContainer<T>::getAtIndex(size_t index, DynamicValue &dynValue) const {
	if (index >= _array.size())
		return false;

	DynamicListValueExporter::exportValue(dynValue, _array[index]);
	return true;
}

template<class T>
void DynamicListContainer<T>::setFrom(const DynamicListContainerBase &other) {
	_array = static_cast<const DynamicListContainer<T> &>(other)._array;
}

template<class T>
const void *DynamicListContainer<T>::getConstArrayPtr() const {
	return &_array;
}

template<class T>
void *DynamicListContainer<T>::getArrayPtr() {
	return &_array;
}

template<class T>
size_t DynamicListContainer<T>::getSize() const {
	return _array.size();
}

template<class T>
bool DynamicListContainer<T>::compareEqual(const DynamicListContainerBase &other) const {
	const DynamicListContainer<T> &otherTyped = static_cast<const DynamicListContainer<T> &>(other);
	return _array == otherTyped._array;
}

template<class T>
DynamicListContainerBase *DynamicListContainer<T>::clone() const {
	return new DynamicListContainer<T>(*this);
}

struct DynamicList {
	DynamicList();
	DynamicList(const DynamicList &other);
	~DynamicList();

	DynamicValueTypes::DynamicValueType getType() const;

	const Common::Array<int32> &getInt() const;
	const Common::Array<double> &getFloat() const;
	const Common::Array<Common::Point> &getPoint() const;
	const Common::Array<IntRange> &getIntRange() const;
	const Common::Array<AngleMagVector> &getVector() const;
	const Common::Array<Label> &getLabel() const;
	const Common::Array<Event> &getEvent() const;
	const Common::Array<VarReference> &getVarReference() const;
	const Common::Array<Common::String> &getString() const;
	const Common::Array<bool> &getBool() const;
	const Common::Array<Common::SharedPtr<DynamicList> > &getList() const;
	const Common::Array<ObjectReference> &getObjectReference() const;

	Common::Array<int32> &getInt();
	Common::Array<double> &getFloat();
	Common::Array<Common::Point> &getPoint();
	Common::Array<IntRange> &getIntRange();
	Common::Array<AngleMagVector> &getVector();
	Common::Array<Label> &getLabel();
	Common::Array<Event> &getEvent();
	Common::Array<VarReference> &getVarReference();
	Common::Array<Common::String> &getString();
	Common::Array<bool> &getBool();
	Common::Array<Common::SharedPtr<DynamicList> > &getList();
	Common::Array<ObjectReference> &getObjectReference();

	bool getAtIndex(size_t index, DynamicValue &value) const;
	bool setAtIndex(size_t index, const DynamicValue &value);
	void truncateToSize(size_t sz);
	void expandToMinimumSize(size_t sz);
	size_t getSize() const;

	static bool dynamicValueToIndex(size_t &outIndex, const DynamicValue &value);

	DynamicList &operator=(const DynamicList &other);

	bool operator==(const DynamicList &other) const;
	inline bool operator!=(const DynamicList &other) const {
		return !((*this) == other);
	}

	void swap(DynamicList &other);

	Common::SharedPtr<DynamicList> clone() const;

	void createWriteProxyForIndex(size_t index, DynamicValueWriteProxy &proxy);

private:
	struct WriteProxyInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};

	void clear();
	void initFromOther(const DynamicList &other);
	bool changeToType(DynamicValueTypes::DynamicValueType type);

	DynamicValueTypes::DynamicValueType _type;
	DynamicListContainerBase *_container;
};

// Dynamic value container.  Somewhat importantly, lists stored in dynamic values
// are BY REFERENCE and must be cloned as necessary.
struct DynamicValue {
	DynamicValue();
	DynamicValue(const DynamicValue &other);
	~DynamicValue();

	bool load(const Data::InternalTypeTaggedValue &data, const Common::String &varSource, const Common::String &varString);
	bool load(const Data::PlugInTypeTaggedValue &data);

	DynamicValueTypes::DynamicValueType getType() const;

	const int32 &getInt() const;
	const double &getFloat() const;
	const Common::Point &getPoint() const;
	const IntRange &getIntRange() const;
	const AngleMagVector &getVector() const;
	const Label &getLabel() const;
	const Event &getEvent() const;
	const VarReference &getVarReference() const;
	const Common::String &getString() const;
	const bool &getBool() const;
	const Common::SharedPtr<DynamicList> &getList() const;
	const ObjectReference &getObject() const;
	const DynamicValueWriteProxy &getWriteProxy() const;

	void clear();

	void setInt(int32 value);
	void setFloat(double value);
	void setPoint(const Common::Point &value);
	void setIntRange(const IntRange &value);
	void setVector(const AngleMagVector &value);
	void setLabel(const Label &value);
	void setEvent(const Event &value);
	void setVarReference(const VarReference &value);
	void setString(const Common::String &value);
	void setBool(bool value);
	void setList(const Common::SharedPtr<DynamicList> &value);
	void setObject(const ObjectReference &value);
	void setObject(const Common::WeakPtr<RuntimeObject> &value);
	void setWriteProxy(const DynamicValueWriteProxy &writeProxy);

	bool roundToInt(int32 &outInt) const;

	bool convertToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const;

	DynamicValue &operator=(const DynamicValue &other);

	bool operator==(const DynamicValue &other) const;
	inline bool operator!=(const DynamicValue &other) const {
		return !((*this) == other);
	}

private:
	union ValueUnion {
		ValueUnion();
		~ValueUnion();

		double asFloat;
		int32 asInt;
		IntRange asIntRange;
		AngleMagVector asVector;
		Label asLabel;
		VarReference asVarReference;
		Event asEvent;
		Common::Point asPoint;
		bool asBool;
		DynamicValueWriteProxy asWriteProxy;
		Common::String asString;
		Common::SharedPtr<DynamicList> asList;
		ObjectReference asObj;

		uint64 asUnset;

		template<class T, T(ValueUnion::*TMember)>
		void construct(const T &value);

		template<class T, T(ValueUnion::*TMember)>
		void construct(T &&value);

		template<class T, T(ValueUnion::*TMember)>
		void destruct();
	};

	template<class T>
	void internalSwap(T &a, T &b) {
		T temp = a;
		a = b;
		b = temp;
	}

	bool convertIntToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const;
	bool convertFloatToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const;
	bool convertBoolToType(DynamicValueTypes::DynamicValueType targetType, DynamicValue &result) const;

	void setFromOther(const DynamicValue &other);

	DynamicValueTypes::DynamicValueType _type;
	ValueUnion _value;
};

template<class TFloat>
struct DynamicValueWriteFloatHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
		TFloat &dest = *static_cast<TFloat *>(objectRef);
		switch (value.getType()) {
		case DynamicValueTypes::kFloat:
			dest = static_cast<TFloat>(value.getFloat());
			return kMiniscriptInstructionOutcomeContinue;
		case DynamicValueTypes::kInteger:
			dest = static_cast<TFloat>(value.getInt());
			return kMiniscriptInstructionOutcomeContinue;
		default:
			return kMiniscriptInstructionOutcomeFailed;
		}
	}
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
		return kMiniscriptInstructionOutcomeFailed;
	}
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
		return kMiniscriptInstructionOutcomeFailed;
	}

	static void create(TFloat *floatValue, DynamicValueWriteProxy &proxy) {
		proxy.pod.ptrOrOffset = 0;
		proxy.pod.objectRef = floatValue;
		proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteFloatHelper<TFloat> >::getInstance();
	}
};

template<class TInteger>
struct DynamicValueWriteIntegerHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
		TInteger &dest = *static_cast<TInteger *>(objectRef);
		switch (value.getType()) {
		case DynamicValueTypes::kFloat:
			dest = static_cast<TInteger>(floor(value.getFloat() + 0.5));
			return kMiniscriptInstructionOutcomeContinue;
		case DynamicValueTypes::kInteger:
			dest = static_cast<TInteger>(value.getInt());
			return kMiniscriptInstructionOutcomeContinue;
		default:
			return kMiniscriptInstructionOutcomeFailed;
		}
	}
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
		return kMiniscriptInstructionOutcomeFailed;
	}
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
		return kMiniscriptInstructionOutcomeFailed;
	}

	static void create(TInteger *intValue, DynamicValueWriteProxy &proxy) {
		proxy.pod.ptrOrOffset = 0;
		proxy.pod.objectRef = intValue;
		proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteIntegerHelper<TInteger> >::getInstance();
	}
};

struct DynamicValueWritePointHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset);
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);

	static void create(Common::Point *pointValue, DynamicValueWriteProxy &proxy);
};

struct DynamicValueWriteBoolHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset);
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);

	static void create(bool *boolValue, DynamicValueWriteProxy &proxy);
};

struct DynamicValueWriteStringHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset);
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);

	static void create(Common::String *strValue, DynamicValueWriteProxy &proxy);
};

template<class TClass, MiniscriptInstructionOutcome (TClass::*TWriteMethod)(MiniscriptThread *thread, const DynamicValue &dest), MiniscriptInstructionOutcome (TClass::*TRefAttribMethod)(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib)>
struct DynamicValueWriteOrRefAttribFuncHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) {
		return (static_cast<TClass *>(objectRef)->*TWriteMethod)(thread, dest);
	}
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
		return (static_cast<TClass *>(objectRef)->*TRefAttribMethod)(thread, proxy, attrib);
	}
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
		return kMiniscriptInstructionOutcomeFailed;
	}

	static void create(TClass *obj, DynamicValueWriteProxy &proxy) {
		proxy.pod.ptrOrOffset = 0;
		proxy.pod.objectRef = obj;
		proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteOrRefAttribFuncHelper<TClass, TWriteMethod, TRefAttribMethod> >::getInstance();
	}
};

template<class TClass, MiniscriptInstructionOutcome (TClass::*TWriteMethod)(MiniscriptThread *thread, const DynamicValue &dest)>
struct DynamicValueWriteFuncHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset) {
		return (static_cast<TClass *>(objectRef)->*TWriteMethod)(thread, dest);
	}
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
		return kMiniscriptInstructionOutcomeFailed;
	}
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
		return kMiniscriptInstructionOutcomeFailed;
	}

	static void create(TClass *obj, DynamicValueWriteProxy &proxy) {
		proxy.pod.ptrOrOffset = 0;
		proxy.pod.objectRef = obj;
		proxy.pod.ifc = DynamicValueWriteInterfaceGlue<DynamicValueWriteFuncHelper<TClass, TWriteMethod> >::getInstance();
	}
};

struct DynamicValueWriteObjectHelper {
	static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset);
	static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
	static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);

	static void create(RuntimeObject *obj, DynamicValueWriteProxy &proxy);
};

struct MessengerSendSpec {
	MessengerSendSpec();

	bool load(const Data::Event &dataEvent, uint32 dataMessageFlags, const Data::InternalTypeTaggedValue &dataLocator, const Common::String &dataWithSource, const Common::String &dataWithString, uint32 dataDestination);
	bool load(const Data::PlugInTypeTaggedValue &dataEvent, const MessageFlags &dataMessageFlags, const Data::PlugInTypeTaggedValue &dataWith, uint32 dataDestination);

	void linkInternalReferences(ObjectLinkingScope *outerScope);
	void visitInternalReferences(IStructuralReferenceVisitor *visitor);
	void resolveDestination(Runtime *runtime, Modifier *sender, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest, RuntimeObject *customDestination) const;

	static void resolveVariableObjectType(RuntimeObject *obj, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest);

	void sendFromMessenger(Runtime *runtime, Modifier *sender, const DynamicValue &incomingData, RuntimeObject *customDestination) const;
	void sendFromMessengerWithCustomData(Runtime *runtime, Modifier *sender, const DynamicValue &data, RuntimeObject *customDestination) const;

	enum LinkType {
		kLinkTypeNotYetLinked,
		kLinkTypeStructural,
		kLinkTypeModifier,
		kLinkTypeCoded,
		kLinkTypeUnresolved,
	};

	Event send;
	MessageFlags messageFlags;
	DynamicValue with;
	uint32 destination; // May be a MessageDestination or GUID

	LinkType _linkType;
	Common::WeakPtr<Structural> _resolvedStructuralDest;
	Common::WeakPtr<Modifier> _resolvedModifierDest;
	Common::WeakPtr<Modifier> _resolvedVarSource;

private:
	void resolveHierarchyStructuralDestination(Runtime *runtime, Modifier *sender, Common::WeakPtr<Structural> &outStructuralDest, Common::WeakPtr<Modifier> &outModifierDest, bool (*compareFunc)(Structural *structural)) const;
	static bool isSceneFilter(Structural *section);
	static bool isSectionFilter(Structural *section);
	static bool isSubsectionFilter(Structural *section);
	static bool isElementFilter(Structural *section);
};

enum MessageDestination {
	kMessageDestNone = 0,

	kMessageDestSharedScene = 0x65,
	kMessageDestScene = 0x66,
	kMessageDestSection = 0x67,
	kMessageDestProject = 0x68,
	kMessageDestActiveScene = 0x69,
	kMessageDestElementsParent = 0x6a,
	kMessageDestChildren = 0x6b,	// Saw this somewhere but can't find it any more?
	kMessageDestModifiersParent = 0x6c,
	kMessageDestSubsection = 0x6d,

	kMessageDestElement = 0xc9,
	kMessageDestSourcesParent = 0xcf,

	kMessageDestBehavior = 0xd4,
	kMessageDestNextElement = 0xd1,
	kMessageDestPrevElement = 0xd2,
	kMessageDestBehaviorsParent = 0xd3,
};

struct SegmentDescription {
	SegmentDescription();

	int volumeID;
	Common::String filePath;
	Common::SeekableReadStream *stream;
};

struct IPlugInModifierRegistrar : public IInterfaceBase {
	virtual void registerPlugInModifier(const char *name, const Data::IPlugInModifierDataFactory *loader, const IPlugInModifierFactory *factory) = 0;
	void registerPlugInModifier(const char *name, const IPlugInModifierFactoryAndDataFactory *loaderFactory);
};

class PlugIn {
public:
	virtual ~PlugIn();

	virtual void registerModifiers(IPlugInModifierRegistrar *registrar) const = 0;
};

class ProjectPersistentResource {
public:
	virtual ~ProjectPersistentResource();
};

struct ProjectResources {
	virtual ~ProjectResources();

	Common::Array<Common::SharedPtr<ProjectPersistentResource> > persistentResources;
};

class CursorGraphic {
public:
	virtual ~CursorGraphic();

	virtual Graphics::Cursor *getCursor() const = 0;
};

class MacCursorGraphic : public CursorGraphic {
public:
	explicit MacCursorGraphic(const Common::SharedPtr<Graphics::MacCursor> &macCursor);

	Graphics::Cursor *getCursor() const override;

private:
	Common::SharedPtr<Graphics::MacCursor> _macCursor;
};

class WinCursorGraphic : public CursorGraphic {
public:
	explicit WinCursorGraphic(const Common::SharedPtr<Graphics::WinCursorGroup> &winCursorGroup, Graphics::Cursor *cursor);

	Graphics::Cursor *getCursor() const override;

private:
	Common::SharedPtr<Graphics::WinCursorGroup> _winCursorGroup;
	Graphics::Cursor *_cursor;
};

class CursorGraphicCollection {
public:
	CursorGraphicCollection();
	~CursorGraphicCollection();

	void addWinCursorGroup(uint32 cursorGroupID, const Common::SharedPtr<Graphics::WinCursorGroup> &cursorGroup);
	void addMacCursor(uint32 cursorID, const Common::SharedPtr<Graphics::MacCursor> &cursor);

	Common::SharedPtr<CursorGraphic> getGraphicByID(uint32 id) const;

private:
	Common::HashMap<uint32, Common::SharedPtr<CursorGraphic> > _cursorGraphics;
};

enum ProjectPlatform {
	kProjectPlatformUnknown,

	kProjectPlatformWindows,
	kProjectPlatformMacintosh,
	KProjectPlatformCrossPlatform,
};

class ProjectDescription {
public:
	explicit ProjectDescription(ProjectPlatform platform);
	~ProjectDescription();

	void addSegment(int volumeID, const char *filePath);
	void addSegment(int volumeID, Common::SeekableReadStream *stream);
	const Common::Array<SegmentDescription> &getSegments() const;

	void addPlugIn(const Common::SharedPtr<PlugIn> &plugIn);
	const Common::Array<Common::SharedPtr<PlugIn> > &getPlugIns() const;

	void setResources(const Common::SharedPtr<ProjectResources> &resources);
	const Common::SharedPtr<ProjectResources> &getResources() const;

	void setCursorGraphics(const Common::SharedPtr<CursorGraphicCollection> &cursorGraphics);
	const Common::SharedPtr<CursorGraphicCollection> &getCursorGraphics() const;

	void setLanguage(const Common::Language &language);
	const Common::Language &getLanguage() const;

	ProjectPlatform getPlatform() const;

	const SubtitleTables &getSubtitles() const;
	void getSubtitles(const SubtitleTables &subs);

private:
	Common::Array<SegmentDescription> _segments;
	Common::Array<Common::SharedPtr<PlugIn> > _plugIns;
	Common::SharedPtr<ProjectResources> _resources;
	Common::SharedPtr<CursorGraphicCollection> _cursorGraphics;
	Common::Language _language;
	SubtitleTables _subtitles;
	ProjectPlatform _platform;
};

struct VolumeState {
	VolumeState();

	Common::String name;
	int volumeID;
	bool isMounted;
};

class ObjectLinkingScope {
public:
	ObjectLinkingScope();
	~ObjectLinkingScope();

	void setParent(ObjectLinkingScope *parent);
	void addObject(uint32 guid, const Common::String &name, const Common::WeakPtr<RuntimeObject> &object);
	Common::WeakPtr<RuntimeObject> resolve(uint32 staticGUID) const;
	Common::WeakPtr<RuntimeObject> resolve(const Common::String &name, bool isNameAlreadyInsensitive) const;
	Common::WeakPtr<RuntimeObject> resolve(uint32 staticGUID, const Common::String &name, bool isNameAlreadyInsensitive) const;

	void reset();

private:
	Common::HashMap<uint32, Common::WeakPtr<RuntimeObject> > _guidToObject;
	Common::HashMap<Common::String, Common::WeakPtr<RuntimeObject> > _nameToObject;
	ObjectLinkingScope *_parent;
};

struct LowLevelSceneStateTransitionAction {
	enum ActionType {
		kLoad,
		kUnload,
		kSendMessage,
		kAutoResetCursor,
	};

	explicit LowLevelSceneStateTransitionAction(const Common::SharedPtr<MessageDispatch> &msg);
	explicit LowLevelSceneStateTransitionAction(ActionType actionType);
	LowLevelSceneStateTransitionAction(const LowLevelSceneStateTransitionAction &other);
	LowLevelSceneStateTransitionAction(const Common::SharedPtr<Structural> &scene, ActionType actionType);

	ActionType getActionType() const;
	const Common::SharedPtr<Structural> &getScene() const;
	const Common::SharedPtr<MessageDispatch> &getMessage() const;

	LowLevelSceneStateTransitionAction &operator=(const LowLevelSceneStateTransitionAction &other);

private:
	ActionType _actionType;
	Common::SharedPtr<Structural> _scene;
	Common::SharedPtr<MessageDispatch> _msg;
};

struct HighLevelSceneTransition {
	enum Type {
		kTypeReturn,
		kTypeChangeToScene,
	};

	HighLevelSceneTransition(const Common::SharedPtr<Structural> &hlst_scene, Type hlst_type, bool hlst_addToDestinationScene, bool hlst_addToReturnList);

	Common::SharedPtr<Structural> scene;
	Type type;
	bool addToDestinationScene;
	bool addToReturnList;
};

struct SceneTransitionEffect {
	SceneTransitionEffect();

	uint32 _duration; // 6000000 is maximum
	uint16 _steps;
	SceneTransitionTypes::SceneTransitionType _transitionType;
	SceneTransitionDirections::SceneTransitionDirection _transitionDirection;
};

class MessageDispatch {
public:
	MessageDispatch(const Common::SharedPtr<MessageProperties> &msgProps, Structural *root, bool cascade, bool relay, bool couldBeCommand);
	MessageDispatch(const Common::SharedPtr<MessageProperties> &msgProps, Modifier *root, bool cascade, bool relay, bool couldBeCommand);

	bool isTerminated() const;
	VThreadState continuePropagating(Runtime *runtime);

	const Common::SharedPtr<MessageProperties> &getMsg() const;
	RuntimeObject *getRootPropagator() const;

	bool isCascade() const;
	bool isRelay() const;

private:
	struct PropagationStack {
		union Ptr {
			Structural *structural;
			Modifier *modifier;
			IModifierContainer *modifierContainer;
		};

		enum PropagationStage {
			kStageSendToModifier,
			kStageSendToModifierContainer,

			kStageSendToStructuralSelf,
			kStageSendToStructuralModifiers,
			kStageSendToStructuralChildren,

			kStageCheckAndSendToModifier,
			kStageCheckAndSendToStructural,
			kStageCheckAndSendCommand,

			kStageSendCommand,
		};

		PropagationStage propagationStage;
		size_t index;
		Ptr ptr;
	};

	Common::Array<PropagationStack> _propagationStack;
	Common::SharedPtr<MessageProperties> _msg;

	Common::WeakPtr<RuntimeObject> _root;

	bool _cascade; // Traverses structure tree
	bool _relay;   // Fire on multiple modifiers
	bool _terminated;
	bool _isCommand;
};

class KeyEventDispatch {
public:
	explicit KeyEventDispatch(const Common::SharedPtr<KeyboardInputEvent> &evt);

	Common::Array<Common::WeakPtr<RuntimeObject> > &getKeyboardMessengerArray();
	static bool keyboardMessengerFilterFunc(void *userData, RuntimeObject *object);

	bool isTerminated() const;
	VThreadState continuePropagating(Runtime *runtime);

private:
	Common::Array<Common::WeakPtr<RuntimeObject> > _keyboardMessengers;
	size_t _dispatchIndex;
	const Common::SharedPtr<KeyboardInputEvent> _evt;
};

class Scheduler;

class ScheduledEvent : Common::NonCopyable {
	friend class Scheduler;

public:
	void cancel();
	uint64 getScheduledTime() const;
	void activate(Runtime *runtime) const;

private:
	ScheduledEvent(void *obj, void (*activateFunc)(void *, Runtime *), uint64 scheduledTime, Scheduler *scheduler);

	void *_obj;
	void (*_activateFunc)(void *obj, Runtime *runtime);

	uint64 _scheduledTime;
	Scheduler *_scheduler;
};

class Scheduler {
	friend class ScheduledEvent;

public:
	Scheduler();
	~Scheduler();

	template<class T, void (T::*TMethodPtr)(Runtime *)>
	Common::SharedPtr<ScheduledEvent> scheduleMethod(uint64 scheduledTime, T* obj) {
		Common::SharedPtr<ScheduledEvent> evt(new ScheduledEvent(obj, Scheduler::methodActivateHelper<T, TMethodPtr>, scheduledTime, this));
		insertEvent(evt);
		return evt;
	}

	Common::SharedPtr<ScheduledEvent> getFirstEvent() const;
	void descheduleFirstEvent();

private:
	template<class T, void (T::*TMethodPtr)(Runtime *)>
	static void methodActivateHelper(void *obj, Runtime *runtime) {
		(static_cast<T *>(obj)->*TMethodPtr)(runtime);
	}

	void insertEvent(const Common::SharedPtr<ScheduledEvent> &evt);
	void removeEvent(const ScheduledEvent *evt);

	Common::Array<Common::SharedPtr<ScheduledEvent>> _events;
};

enum OSEventType {
	kOSEventTypeMouseDown,
	kOSEventTypeMouseUp,
	kOSEventTypeMouseMove,

	kOSEventTypeKeyboard,
};

class OSEvent {
public:
	explicit OSEvent(OSEventType eventType);
	virtual ~OSEvent();

	OSEventType getEventType() const;

private:
	OSEventType _eventType;
};

class MouseInputEvent : public OSEvent {
public:
	explicit MouseInputEvent(OSEventType eventType, int32 x, int32 y, Actions::MouseButton button);

	int32 getX() const;
	int32 getY() const;
	Actions::MouseButton getButton() const;

private:
	int32 _x;
	int32 _y;
	Actions::MouseButton _button;
};

class KeyboardInputEvent : public OSEvent {
public:
	explicit KeyboardInputEvent(OSEventType osEventType, Common::EventType keyEventType, bool repeat, const Common::KeyState &keyEvt);

	Common::EventType getKeyEventType() const;
	bool isRepeat() const;
	const Common::KeyState &getKeyState() const;

private:
	Common::EventType _keyEventType;
	bool _repeat;
	const Common::KeyState _keyEvt;
};

struct DragMotionProperties {
	DragMotionProperties();

	ConstraintDirection constraintDirection;
	Common::Rect constraintMargin;
	bool constrainToParent;
};

class SceneTransitionHooks {
public:
	virtual ~SceneTransitionHooks();

	virtual void onSceneTransitionSetup(Runtime *runtime, const Common::WeakPtr<Structural> &oldScene, const Common::WeakPtr<Structural> &newScene);
	virtual void onSceneTransitionEnded(Runtime *runtime, const Common::WeakPtr<Structural> &newScene);
};

class Runtime {
public:
	explicit Runtime(OSystem *system, Audio::Mixer *mixer, ISaveUIProvider *saveProvider, ILoadUIProvider *loadProvider, const Common::SharedPtr<SubtitleRenderer> &subRenderer);
	~Runtime();

	bool runFrame();
	void drawFrame();

	void queueProject(const Common::SharedPtr<ProjectDescription> &desc);
	void closeProject();

	void addVolume(int volumeID, const char *name, bool isMounted);
	bool getVolumeState(const Common::String &name, int &outVolumeID, bool &outIsMounted) const;
	void setDefaultVolumeState(bool defaultState);

	void addSceneStateTransition(const HighLevelSceneTransition &transition);

	void setSceneTransitionEffect(bool isInDestinationScene, SceneTransitionEffect *effect);

	Project *getProject() const;

	void postConsumeMessageTask(IMessageConsumer *msgConsumer, const Common::SharedPtr<MessageProperties> &msg);
	void postConsumeCommandTask(Structural *structural, const Common::SharedPtr<MessageProperties> &msg);

	uint32 allocateRuntimeGUID();

	void addWindow(const Common::SharedPtr<Window> &window);
	void removeWindow(Window *window);

	// Sets up a supported display mode
	void setupDisplayMode(ColorDepthMode displayMode, const Graphics::PixelFormat &pixelFormat);

	bool isDisplayModeSupported(ColorDepthMode displayMode) const;

	// Switches to a specified display mode.  Returns true if the mode was actually changed.  If so, all windows will need
	// to be recreated.
	bool switchDisplayMode(ColorDepthMode realDisplayMode, ColorDepthMode fakeDisplayMode);

	void setDisplayResolution(uint16 width, uint16 height);
	void getDisplayResolution(uint16 &outWidth, uint16 &outHeight) const;

	ColorDepthMode getRealColorDepth() const;
	ColorDepthMode getFakeColorDepth() const;	// Fake color depth that will be reported to scripts

	const Graphics::PixelFormat &getRenderPixelFormat() const;

	const Common::SharedPtr<Graphics::MacFontManager> &getMacFontManager() const;

	const Common::SharedPtr<Structural> &getActiveMainScene() const;
	const Common::SharedPtr<Structural> &getActiveSharedScene() const;
	void getSceneStack(Common::Array<Common::SharedPtr<Structural> > &sceneStack) const;

	bool mustDraw() const;

	uint64 getRealTime() const;
	uint64 getPlayTime() const;

	VThread &getVThread() const;

	// Sending a message on the VThread means "immediately"
	void sendMessageOnVThread(const Common::SharedPtr<MessageDispatch> &dispatch);
	void queueMessage(const Common::SharedPtr<MessageDispatch> &dispatch);

	void queueOSEvent(const Common::SharedPtr<OSEvent> &osEvent);

	Scheduler &getScheduler();

	void getScenesInRenderOrder(Common::Array<Structural *> &scenes) const;

	void instantiateIfAlias(Common::SharedPtr<Modifier> &modifier, const Common::WeakPtr<RuntimeObject> &relinkParent);

	Common::SharedPtr<Window> findTopWindow(int32 x, int32 y) const;

	void setVolume(double volume);

	ProjectPlatform getPlatform() const;

	void onMouseDown(int32 x, int32 y, Actions::MouseButton mButton);
	void onMouseMove(int32 x, int32 y);
	void onMouseUp(int32 x, int32 y, Actions::MouseButton mButton);
	void onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt);

	const Common::Point &getCachedMousePosition() const;
	void setModifierCursorOverride(uint32 cursorID);
	void clearModifierCursorOverride();
	void forceCursorRefreshOnce();
	void setAutoResetCursor(bool enabled);

	bool isAwaitingSceneTransition() const;

	Common::RandomSource *getRandom() const;
	WorldManagerInterface *getWorldManagerInterface() const;
	AssetManagerInterface *getAssetManagerInterface() const;
	SystemInterface *getSystemInterface() const;

	ISaveUIProvider *getSaveProvider() const;
	ILoadUIProvider *getLoadProvider() const;

	Audio::Mixer *getAudioMixer() const;

	Hacks &getHacks();
	const Hacks &getHacks() const;

	void setSceneGraphDirty();
	void clearSceneGraphDirty();
	bool isSceneGraphDirty() const;

	void addCollider(ICollider *collider);
	void removeCollider(ICollider *collider);
	void checkCollisions();

	void addBoundaryDetector(IBoundaryDetector *boundaryDetector);
	void removeBoundaryDetector(IBoundaryDetector *boundaryDetector);
	void checkBoundaries();

	void addPostEffect(IPostEffect *postEffect);
	void removePostEffect(IPostEffect *postEffect);
	const Common::Array<IPostEffect *> &getPostEffects() const;

	const Common::String *resolveAttributeIDName(uint32 attribID) const;

	const Common::WeakPtr<Window> &getMainWindow() const;

	const Common::SharedPtr<Graphics::Surface> &getSaveScreenshotOverride() const;
	void setSaveScreenshotOverride(const Common::SharedPtr<Graphics::Surface> &screenshot);

	bool isIdle() const;

	const Common::SharedPtr<SubtitleRenderer> &getSubtitleRenderer() const;

#ifdef MTROPOLIS_DEBUG_ENABLE
	void debugSetEnabled(bool enabled);
	void debugBreak();
	Debugger *debugGetDebugger() const;

	void debugGetPrimaryTaskList(Common::Array<Common::SharedPtr<DebugPrimaryTaskList> > &primaryTaskLists);
#endif

private:
	enum SceneTransitionState {
		kSceneTransitionStateNotTransitioning,
		kSceneTransitionStateWaitingForDraw,
		kSceneTransitionStateTransitioning,
	};

	struct SceneStackEntry {
		SceneStackEntry();

		Common::SharedPtr<Structural> scene;
	};

	struct Teardown {
		Teardown();

		Common::WeakPtr<Structural> structural;
		bool onlyRemoveChildren;
	};

	struct SceneReturnListEntry {
		SceneReturnListEntry();

		Common::SharedPtr<Structural> scene;
		bool isAddToDestinationSceneTransition;
	};

	struct DispatchMethodTaskData {
		Common::SharedPtr<MessageDispatch> dispatch;
	};

	struct DispatchKeyTaskData {
		Common::SharedPtr<KeyEventDispatch> dispatch;
	};

	struct ConsumeMessageTaskData {
		ConsumeMessageTaskData();

		IMessageConsumer *consumer;
		Common::SharedPtr<MessageProperties> message;
	};

	struct ConsumeCommandTaskData {
		ConsumeCommandTaskData();

		Structural *structural;
		Common::SharedPtr<MessageProperties> message;
	};

	struct UpdateMouseStateTaskData {
		UpdateMouseStateTaskData();

		bool mouseDown;
	};

	struct UpdateMousePositionTaskData {
		UpdateMousePositionTaskData();

		int32 x;
		int32 y;
	};

	struct CollisionCheckState {
		CollisionCheckState();

		Common::Array<Common::WeakPtr<VisualElement> > activeElements;
		ICollider *collider;
	};

	struct BoundaryCheckState {
		BoundaryCheckState();

		IBoundaryDetector *detector;
		uint currentContacts;
		Common::Point position;
		bool positionResolved;
	};

	struct ColliderInfo {
		ColliderInfo();

		size_t sceneStackDepth;
		uint16 layer;
		VisualElement *element;
		Common::Rect absRect;
	};

	static Common::SharedPtr<Structural> findDefaultSharedSceneForScene(Structural *scene);
	void executeTeardown(const Teardown &teardown);
	void executeLowLevelSceneStateTransition(const LowLevelSceneStateTransitionAction &transitionAction);
	void executeHighLevelSceneTransition(const HighLevelSceneTransition &transition);
	void executeCompleteTransitionToScene(const Common::SharedPtr<Structural> &scene);
	void executeSharedScenePostSceneChangeActions();

	void recursiveAutoPlayMedia(Structural *structural);
	void recursiveDeactivateStructural(Structural *structural);
	void recursiveActivateStructural(Structural *structural);

	static bool isStructuralMouseInteractive(Structural *structural, MouseInteractivityTestType testType);
	static bool isModifierMouseInteractive(Modifier *modifier, MouseInteractivityTestType testType);
	static void recursiveFindMouseCollision(Structural *&bestResult, int32 &bestLayer, int32 &bestStackHeight, bool &bestDirect, Structural *candidate, int32 stackHeight, int32 relativeX, int32 relativeY, MouseInteractivityTestType testType);

	void queueEventAsLowLevelSceneStateTransitionAction(const Event &evt, Structural *root, bool cascade, bool relay);

	void loadScene(const Common::SharedPtr<Structural> &scene);

	void ensureMainWindowExists();

	void unloadProject();
	void refreshPlayTime();	// Updates play time to be in sync with the system clock.  Used so that events occurring after storage access don't skip.

	VThreadState dispatchMessageTask(const DispatchMethodTaskData &data);
	VThreadState dispatchKeyTask(const DispatchKeyTaskData &data);
	VThreadState consumeMessageTask(const ConsumeMessageTaskData &data);
	VThreadState consumeCommandTask(const ConsumeCommandTaskData &data);
	VThreadState updateMouseStateTask(const UpdateMouseStateTaskData &data);
	VThreadState updateMousePositionTask(const UpdateMousePositionTaskData &data);

	void updateMainWindowCursor();

	static void recursiveFindColliders(Structural *structural, size_t sceneStackDepth, Common::Array<ColliderInfo> &colliders, int32 parentOriginX, int32 parentOriginY, bool isRoot);
	static bool sortColliderPredicate(const ColliderInfo &a, const ColliderInfo &b);

	Common::Array<VolumeState> _volumes;
	Common::SharedPtr<ProjectDescription> _queuedProjectDesc;
	Common::SharedPtr<Project> _project;
	Common::ScopedPtr<VThread> _vthread;
	Common::Array<Common::SharedPtr<MessageDispatch> > _messageQueue;
	Common::Array<Common::SharedPtr<OSEvent> > _osEventQueue;
	ObjectLinkingScope _rootLinkingScope;

	Common::Array<Teardown> _pendingTeardowns;
	Common::Array<LowLevelSceneStateTransitionAction> _pendingLowLevelTransitions;
	Common::Array<HighLevelSceneTransition> _pendingSceneTransitions;
	Common::Array<SceneStackEntry> _sceneStack;
	Common::SharedPtr<Structural> _activeMainScene;
	Common::SharedPtr<Structural> _activeSharedScene;
	Common::Array<SceneReturnListEntry> _sceneReturnList;

	SceneTransitionState _sceneTransitionState;
	SceneTransitionEffect _sourceSceneTransitionEffect;
	SceneTransitionEffect _destinationSceneTransitionEffect;
	SceneTransitionEffect *_activeSceneTransitionEffect;
	Common::SharedPtr<Graphics::ManagedSurface> _sceneTransitionOldFrame;
	Common::SharedPtr<Graphics::ManagedSurface> _sceneTransitionNewFrame;
	uint32 _sceneTransitionStartTime;
	uint32 _sceneTransitionEndTime;

	Common::WeakPtr<Window> _mainWindow;
	Common::Array<Common::SharedPtr<Window> > _windows;

	Common::SharedPtr<Graphics::MacFontManager> _macFontMan;

	Common::SharedPtr<Common::RandomSource> _random;

	uint32 _nextRuntimeGUID;

	bool _displayModeSupported[kColorDepthModeCount];
	Graphics::PixelFormat _displayModePixelFormats[kColorDepthModeCount];
	ColorDepthMode _realDisplayMode;
	ColorDepthMode _fakeDisplayMode;
	uint16 _displayWidth;
	uint16 _displayHeight;

	uint64 _realTimeBase;
	uint64 _playTimeBase;

	uint32 _realTime;
	uint32 _playTime;

	Scheduler _scheduler;
	OSystem *_system;
	Audio::Mixer *_mixer;

	ISaveUIProvider *_saveProvider;
	ILoadUIProvider *_loadProvider;
	Common::SharedPtr<Graphics::Surface> _saveScreenshotOverride;

	Common::SharedPtr<CursorGraphic> _lastFrameCursor;
	Common::SharedPtr<CursorGraphic> _defaultCursor;

	Common::WeakPtr<Window> _mouseFocusWindow;
	bool _mouseFocusFlags[Actions::kMouseButtonCount];

	Common::WeakPtr<Window> _keyFocusWindow;

	ProjectPlatform _platform;

	Common::SharedPtr<SystemInterface> _systemInterface;
	Common::SharedPtr<WorldManagerInterface> _worldManagerInterface;
	Common::SharedPtr<AssetManagerInterface> _assetManagerInterface;

	// The cached mouse position is updated at frame end
	Common::Point _cachedMousePosition;

	// The real mouse position is updated all the time (even when suspended)
	Common::Point _realMousePosition;

	// Mouse control is tracked in two ways: Mouse over is detected with mouse movement AND when
	// "refreshCursor" is set on the world manager, it indicates the frontmost object that
	// responds to any mouse event.  The mouse tracking object is the object that was clicked.
	// These can differ if the user holds down the mouse and moves it to a spot where the tracked
	// object is either not clickable, or is behind another object with mouse collision.
	// Note that mouseOverObject is also NOT necessarily what will receive mouse down events.
	Common::WeakPtr<Structural> _mouseOverObject;
	Common::WeakPtr<Structural> _mouseTrackingObject;
	Common::Point _mouseTrackingDragStart;
	Common::Point _mouseTrackingObjectInitialOrigin;
	bool _trackedMouseOutside;
	bool _forceCursorRefreshOnce;
	bool _autoResetCursor;

	uint32 _modifierOverrideCursorID;
	bool _haveModifierOverrideCursor;

	bool _defaultVolumeState;

	// True if any elements were added to the scene, removed from the scene, or reparented since last draw
	bool _sceneGraphChanged;

	bool _isQuitting;

	Common::Array<Common::SharedPtr<CollisionCheckState> > _colliders;
	Common::Array<BoundaryCheckState> _boundaryChecks;
	uint32 _collisionCheckTime;

	Common::Array<IPostEffect *> _postEffects;

	Common::SharedPtr<SubtitleRenderer> _subtitleRenderer;

	Hacks _hacks;

	Common::HashMap<uint32, Common::String> _getSetAttribIDsToAttribName;

#ifdef MTROPOLIS_DEBUG_ENABLE
	Common::SharedPtr<Debugger> _debugger;
#endif
};

struct IModifierContainer : public IInterfaceBase {
	virtual const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const = 0;
	virtual void appendModifier(const Common::SharedPtr<Modifier> &modifier) = 0;
};

class SimpleModifierContainer : public IModifierContainer {
public:
	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

private:
	Common::Array<Common::SharedPtr<Modifier> > _modifiers;
};

class RuntimeObject {
	template<typename TElement, typename TElementData>
	friend class ElementFactory;

public:
	RuntimeObject();
	virtual ~RuntimeObject();

	uint32 getStaticGUID() const;
	uint32 getRuntimeGUID() const;

	void setRuntimeGUID(uint32 runtimeGUID);

	void setSelfReference(const Common::WeakPtr<RuntimeObject> &selfReference);
	const Common::WeakPtr<RuntimeObject> &getSelfReference() const;

	virtual bool isStructural() const;
	virtual bool isProject() const;
	virtual bool isSection() const;
	virtual bool isSubsection() const;
	virtual bool isModifier() const;
	virtual bool isElement() const;

	virtual bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib);
	virtual bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index);
	virtual MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib);
	virtual MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib, const DynamicValue &index);

protected:
	// This is the static GUID stored in the data, it is not guaranteed
	// to be globally unique at runtime.  In particular, cloning an object
	// and using aliased modifiers will cause multiple objects with the same
	// static GUID to exist with separate runtime GUIDs.
	uint32 _guid;
	uint32 _runtimeGUID;
	Common::WeakPtr<RuntimeObject> _selfReference;
};

struct MessageProperties {
	MessageProperties(const Event &evt, const DynamicValue &value, const Common::WeakPtr<RuntimeObject> &source);

	const Event &getEvent() const;
	const DynamicValue &getValue() const;
	const Common::WeakPtr<RuntimeObject> &getSource() const;

	void setValue(const DynamicValue &value);

private:
	Event _evt;
	DynamicValue _value;
	Common::WeakPtr<RuntimeObject> _source;
};

struct IStructuralReferenceVisitor : public IInterfaceBase {
	virtual void visitChildStructuralRef(Common::SharedPtr<Structural> &structural) = 0;
	virtual void visitChildModifierRef(Common::SharedPtr<Modifier> &modifier) = 0;
	virtual void visitWeakStructuralRef(Common::WeakPtr<Structural> &structural) = 0;
	virtual void visitWeakModifierRef(Common::WeakPtr<Modifier> &modifier) = 0;
};

struct IMessageConsumer : public IInterfaceBase {
	// These should only be implemented as direct responses - child traversal is handled by the message propagation process
	virtual bool respondsToEvent(const Event &evt) const = 0;
	virtual VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) = 0;
};

class WorldManagerInterface : public RuntimeObject {
public:
	WorldManagerInterface();

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

private:
	MiniscriptInstructionOutcome setCurrentScene(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setRefreshCursor(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setAutoResetCursor(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setWinSndBufferSize(MiniscriptThread *thread, const DynamicValue &value);
};

class AssetManagerInterface : public RuntimeObject {
};

class SystemInterface : public RuntimeObject {
public:
	const int kFullVolume = 7;

	SystemInterface();

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

private:
	static ColorDepthMode bitDepthToDisplayMode(int32 bitDepth);
	static int32 displayModeToBitDepth(ColorDepthMode displayMode);

	MiniscriptInstructionOutcome setEjectCD(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setGameMode(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setMasterVolume(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setMonitorBitDepth(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome setVolumeName(MiniscriptThread *thread, const DynamicValue &value);

	Common::String _volumeName;
	int _masterVolume;
};

class StructuralHooks {
public:
	virtual ~StructuralHooks();

	virtual void onCreate(Structural *structural);
	virtual void onSetPosition(Structural *structural, Common::Point &pt);
};

class Structural : public RuntimeObject, public IModifierContainer, public IMessageConsumer, public Debuggable {
public:
	Structural();
	virtual ~Structural();

	bool isStructural() const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	const Common::Array<Common::SharedPtr<Structural> > &getChildren() const;
	void addChild(const Common::SharedPtr<Structural> &child);
	void removeAllChildren();
	void removeAllModifiers();
	void removeChild(Structural *child);
	void removeAllAssets();

	void holdAssets(const Common::Array<Common::SharedPtr<Asset> > &assets);

	Structural *getParent() const;
	Structural *findNextSibling() const;
	Structural *findPrevSibling() const;
	void setParent(Structural *parent);

	// Helper that finds the scene containing the structural object, or itself if it is the scene
	VisualElement *findScene();

	const Common::String &getName() const;

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void materializeSelfAndDescendents(Runtime *runtime, ObjectLinkingScope *outerScope);
	void materializeDescendents(Runtime *runtime, ObjectLinkingScope *outerScope);

	virtual VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg);

	virtual void activate();
	virtual void deactivate();

	void recursiveCollectObjectsMatchingCriteria(Common::Array<Common::WeakPtr<RuntimeObject> > &results, bool (*evalFunc)(void *userData, RuntimeObject *object), void *userData, bool onlyEnabled);

	void setHooks(const Common::SharedPtr<StructuralHooks> &hooks);
	const Common::SharedPtr<StructuralHooks> &getHooks() const;

#ifdef MTROPOLIS_DEBUG_ENABLE
	SupportStatus debugGetSupportStatus() const override;
	const Common::String &debugGetName() const override;
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

protected:
	virtual ObjectLinkingScope *getPersistentStructuralScope();
	virtual ObjectLinkingScope *getPersistentModifierScope();

	MiniscriptInstructionOutcome scriptSetPaused(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetDebug(MiniscriptThread *thread, const DynamicValue &value);

	// If you override this, you must override visitInternalReferences too.
	virtual void linkInternalReferences(ObjectLinkingScope *outerScope);

	virtual void onPauseStateChanged();

	Structural *_parent;
	Common::Array<Common::SharedPtr<Structural> > _children;
	Common::Array<Common::SharedPtr<Modifier> > _modifiers;
	Common::String _name;

	Common::Array<Common::SharedPtr<Asset> > _assets;

	// "paused" attrib is available for ALL structural types, even when it doesn't do anything.
	// Changing it does not affect modifiers on the object that play media, but does fire
	// "Paused"/"Unpaused" events.
	bool _paused;

	// "loop" appears to have been made available on everything in 1.2.  Obsidian depends on it
	// being available for sound indexes to be properly set up.
	bool _loop;

	int32 _flushPriority;

	Common::SharedPtr<StructuralHooks> _hooks;
};

struct ProjectPresentationSettings {
	ProjectPresentationSettings();

	uint16 width;
	uint16 height;
	uint32 bitsPerPixel;
};

struct AssetDefLoaderContext {
	Common::Array<Common::SharedPtr<Asset> > assets;
};

struct ChildLoaderContext {
	enum Type {
		kTypeUnknown,
		kTypeCountedModifierList,
		kTypeFlagTerminatedModifierList,
		kTypeProject,
		kTypeSection,
		kTypeFilteredElements,
	};

	struct FilteredElements {
		Structural *structural;
		bool (*filterFunc)(Data::DataObjectTypes::DataObjectType dataObjectType);
	};

	union ContainerUnion {
		IModifierContainer *modifierContainer;
		Structural *structural;
		FilteredElements filteredElements;
	};

	ChildLoaderContext();

	uint remainingCount;
	Type type;
	ContainerUnion containerUnion;
};

struct ChildLoaderStack {
	Common::Array<ChildLoaderContext> contexts;
};

class ProjectPlugInRegistry : public IPlugInModifierRegistrar  {
public:
	ProjectPlugInRegistry();

	void registerPlugInModifier(const char *name, const Data::IPlugInModifierDataFactory *dataFactory, const IPlugInModifierFactory *factory) override;

	const Data::PlugInModifierRegistry &getDataLoaderRegistry() const;
	const IPlugInModifierFactory *findPlugInModifierFactory(const char *name) const;

private:
	Data::PlugInModifierRegistry _dataLoaderRegistry;
	Common::HashMap<Common::String, const IPlugInModifierFactory *> _factoryRegistry;
};

struct IPlayMediaSignalReceiver : public IInterfaceBase {
	virtual void playMedia(Runtime *runtime, Project *project) = 0;
};

class PlayMediaSignaller {
public:
	PlayMediaSignaller();
	~PlayMediaSignaller();

	void playMedia(Runtime *runtime, Project *project);
	void addReceiver(IPlayMediaSignalReceiver *receiver);
	void removeReceiver(IPlayMediaSignalReceiver *receiver);

private:
	Common::Array<IPlayMediaSignalReceiver *> _receivers;
};

struct ISegmentUnloadSignalReceiver : public IInterfaceBase {
	virtual void onSegmentUnloaded(int segmentIndex) = 0;
};

class SegmentUnloadSignaller {
public:
	explicit SegmentUnloadSignaller(Project *project, int segmentIndex);
	~SegmentUnloadSignaller();

	void onSegmentUnloaded();
	void addReceiver(ISegmentUnloadSignalReceiver *receiver);
	void removeReceiver(ISegmentUnloadSignalReceiver *receiver);

private:
	Project *_project;
	int _segmentIndex;
	Common::Array<ISegmentUnloadSignalReceiver *> _receivers;
};

struct IKeyboardEventReceiver : public IInterfaceBase {
	virtual void onKeyboardEvent(Runtime *runtime, Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt) = 0;
};

class KeyboardEventSignaller {
public:
	KeyboardEventSignaller();
	~KeyboardEventSignaller();

	void onKeyboardEvent(Runtime *runtime, Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt);
	void addReceiver(IKeyboardEventReceiver *receiver);
	void removeReceiver(IKeyboardEventReceiver *receiver);

private:
	Common::Array<IKeyboardEventReceiver *> _receivers;
	Common::SharedPtr<KeyboardEventSignaller> _signaller;
};

struct ICollider : public IInterfaceBase {
	virtual void getCollisionProperties(Modifier *&modifier, bool &collideInFront, bool &collideBehind, bool &excludeParents) const = 0;
	virtual void triggerCollision(Runtime *runtime, Structural *collidingElement, bool wasInContact, bool isInContact, bool &outShouldStop) = 0;
};

struct IBoundaryDetector : public IInterfaceBase {
	enum EdgeFlags {
		kEdgeTop = 0x1,
		kEdgeBottom = 0x2,
		kEdgeLeft = 0x4,
		kEdgeRight = 0x8,
	};

	virtual void getCollisionProperties(Modifier *&modifier, uint &edgeFlags, bool &mustBeCompletelyOutside, bool &continuous) const = 0;
	virtual void triggerCollision(Runtime *runtime) = 0;
};

struct IPostEffect : public IInterfaceBase {
	virtual void renderPostEffect(Graphics::ManagedSurface &surface) const = 0;
};

struct MediaCueState {
	enum TriggerTiming {
		kTriggerTimingStart = 0,
		kTriggerTimingDuring = 1,
		kTriggerTimingEnd = 2,
	};

	int32 minTime;
	int32 maxTime;

	Modifier *sourceModifier;
	TriggerTiming triggerTiming;
	MessengerSendSpec send;
	DynamicValue incomingData;

	MediaCueState();
	void checkTimestampChange(Runtime *runtime, uint32 oldTS, uint32 newTS, bool continuousTimestamps, bool canTriggerDuring);
};

class Project : public Structural {
public:
	explicit Project(Runtime *runtime);
	~Project();

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void loadFromDescription(const ProjectDescription &desc, const Hacks &hacks);
	void loadSceneFromStream(const Common::SharedPtr<Structural> &structural, uint32 streamID, const Hacks &hacks);

	Common::SharedPtr<Modifier> resolveAlias(uint32 aliasID) const;
	void materializeGlobalVariables(Runtime *runtime, ObjectLinkingScope *scope);

	const ProjectPresentationSettings &getPresentationSettings() const;

	bool isProject() const override;

	Common::String getAssetNameByID(uint32 assetID) const;
	Common::WeakPtr<Asset> getAssetByID(uint32 assetID) const;
	size_t getSegmentForStreamIndex(size_t streamIndex) const;
	void openSegmentStream(int segmentIndex);
	void closeSegmentStream(int segmentIndex);
	Common::SeekableReadStream *getStreamForSegment(int segmentIndex);

	void onPostRender();
	void onKeyboardEvent(Runtime *runtime, const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt);

	Common::SharedPtr<SegmentUnloadSignaller> notifyOnSegmentUnload(int segmentIndex, ISegmentUnloadSignalReceiver *receiver);
	Common::SharedPtr<KeyboardEventSignaller> notifyOnKeyboardEvent(IKeyboardEventReceiver *receiver);
	Common::SharedPtr<PlayMediaSignaller> notifyOnPlayMedia(IPlayMediaSignalReceiver *receiver);

	const char *findAuthorMessageName(uint32 id) const;

	const Common::SharedPtr<CursorGraphicCollection> &getCursorGraphics() const;

	const SubtitleTables &getSubtitles() const;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Project"; }
#endif

private:
	struct LabelSuperGroup {
		LabelSuperGroup();

		size_t firstRootNodeIndex;
		size_t numRootNodes;
		size_t numTotalNodes;

		uint32 superGroupID;
		Common::String name;
	};

	struct LabelTree {
		LabelTree();

		size_t firstChildIndex;
		size_t numChildren;

		uint32 id;
		Common::String name;
	};

	struct Segment {
		Segment();

		SegmentDescription desc;
		Common::SharedPtr<Common::SeekableReadStream> rcStream;
		Common::SeekableReadStream *weakStream;
		Common::SharedPtr<SegmentUnloadSignaller> unloadSignaller;
	};

	enum StreamType {
		kStreamTypeUnknown,

		kStreamTypeAsset,
		kStreamTypeBoot,
		kStreamTypeScene,
	};

	struct StreamDesc {
		StreamDesc();

		StreamType streamType;
		uint16 segmentIndex;
		uint32 size;
		uint32 pos;
	};

	struct AssetDesc {
		AssetDesc();

		uint32 typeCode;
		size_t id;
		Common::String name;

		// If the asset is live, this will be its asset info
		Common::WeakPtr<Asset> asset;
	};

	void loadBootStream(size_t streamIndex, const Hacks &hacks);

	void loadPresentationSettings(const Data::PresentationSettings &presentationSettings);
	void loadAssetCatalog(const Data::AssetCatalog &assetCatalog);
	void loadGlobalObjectInfo(ChildLoaderStack &loaderStack, const Data::GlobalObjectInfo &globalObjectInfo);
	void loadAssetDef(size_t streamIndex, AssetDefLoaderContext &context, const Data::DataObject &dataObject);
	void loadContextualObject(size_t streamIndex, ChildLoaderStack &stack, const Data::DataObject &dataObject);
	Common::SharedPtr<Modifier> loadModifierObject(ModifierLoaderContext &loaderContext, const Data::DataObject &dataObject);
	void loadLabelMap(const Data::ProjectLabelMap &projectLabelMap);
	static size_t recursiveCountLabels(const Data::ProjectLabelMap::LabelTree &tree);

	ObjectLinkingScope *getPersistentStructuralScope() override;
	ObjectLinkingScope *getPersistentModifierScope() override;

	void assignAssets(const Common::Array<Common::SharedPtr<Asset> > &assets, const Hacks &hacks);

	Common::Array<Segment> _segments;
	Common::Array<StreamDesc> _streams;
	Common::Array<LabelTree> _labelTree;
	Common::Array<LabelSuperGroup> _labelSuperGroups;
	Data::ProjectFormat _projectFormat;
	bool _isBigEndian;

	Common::Array<AssetDesc *> _assetsByID;
	Common::Array<AssetDesc> _realAssets;

	Common::HashMap<Common::String, size_t> _assetNameToID;

	ProjectPresentationSettings _presentationSettings;

	bool _haveGlobalObjectInfo;
	bool _haveProjectStructuralDef;
	SimpleModifierContainer _globalModifiers;

	ProjectPlugInRegistry _plugInRegistry;

	Common::Array<Common::SharedPtr<PlugIn> > _plugIns;
	Common::SharedPtr<ProjectResources> _resources;
	Common::SharedPtr<CursorGraphicCollection> _cursorGraphics;

	ObjectLinkingScope _structuralScope;
	ObjectLinkingScope _modifierScope;

	Common::SharedPtr<PlayMediaSignaller> _playMediaSignaller;
	Common::SharedPtr<KeyboardEventSignaller> _keyboardEventSignaller;

	SubtitleTables _subtitles;

	Runtime *_runtime;
};

class Section : public Structural {
public:
	bool load(const Data::SectionStructuralDef &data);

	bool isSection() const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Section"; }
#endif

private:
	ObjectLinkingScope *getPersistentStructuralScope() override;
	ObjectLinkingScope *getPersistentModifierScope() override;

	ObjectLinkingScope _structuralScope;
	ObjectLinkingScope _modifierScope;
};

class Subsection : public Structural {
public:
	bool load(const Data::SubsectionStructuralDef &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

	ObjectLinkingScope *getSceneLoadMaterializeScope();

	bool isSubsection() const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Subsection"; }
#endif

private:
	ObjectLinkingScope *getPersistentStructuralScope() override;
	ObjectLinkingScope *getPersistentModifierScope() override;

	ObjectLinkingScope _structuralScope;
	ObjectLinkingScope _modifierScope;
};

class Element : public Structural {
public:
	Element();

	virtual bool isVisual() const = 0;
	virtual bool canAutoPlay() const;
	virtual void queueAutoPlayEvents(Runtime *runtime, bool isAutoPlaying);

	bool isElement() const override;

	uint32 getStreamLocator() const;

	void addMediaCue(MediaCueState *mediaCue);
	void removeMediaCue(const MediaCueState *mediaCue);

	void triggerAutoPlay(Runtime *runtime);

	virtual bool resolveMediaMarkerLabel(const Label &label, int32 &outResolution) const;

protected:
	uint32 _streamLocator;
	uint16 _sectionID;

	Common::Array<MediaCueState *> _mediaCues;

	bool _haveCheckedAutoPlay;
};

class VisualElementTransitionProperties {
public:
	VisualElementTransitionProperties();

	uint8 getAlpha() const;
	void setAlpha(uint8 alpha);

	bool isDirty() const;
	void clearDirty();

private:
	uint8 _alpha;
	bool _isDirty;
};

class VisualElementRenderProperties {
public:
	VisualElementRenderProperties();

	enum InkMode {
		kInkModeCopy = 0x0,
		kInkModeTransparent = 0x1,				// src*dest
		kInkModeGhost = 0x3,					// (1-src)+dest
		kInkModeReverseCopy = 0x4,				// 1-src
		kInkModeReverseGhost = 0x7,				// src+dest
		kInkModeReverseTransparent = 0x9,		// (1-src)*dest
		kInkModeBlend = 0x20,					// (src*bgcolor)+(dest*(1-bgcolor)
		kInkModeBackgroundTransparent = 0x24,	// BG color is transparent
		kInkModeChameleonDark = 0x25,			// src+dest
		kInkModeChameleonLight = 0x27,			// src*dest
		kInkModeBackgroundMatte = 0x224,		// BG color is transparent and non-interactive
		kInkModeInvisible = 0xffff,				// Not drawn, but interactive

		kInkModeXor = 0x7ffffff0,				// Fake ink mode for Obsidian canvas puzzle, not a valid value from data
		kInkModeDefault = 0x7fffffff,			// Not a valid value from data
	};

	enum Shape {
		kShapeRect = 0x1,
		kShapeRoundedRect = 0x2,
		kShapeOval = 0x3,
		kShapePolygon = 0x9,
		kShapeStar = 0xb, // 5-point star, horizontal arms are at (top+bottom*2)/3

		// Fake shapes for Obsidian canvas puzzle, not a valid from data
		kShapeObsidianCanvasPuzzleTri1 = 0x7ffffff1,
		kShapeObsidianCanvasPuzzleTri2 = 0x7ffffff2,
		kShapeObsidianCanvasPuzzleTri3 = 0x7ffffff3,
		kShapeObsidianCanvasPuzzleTri4 = 0x7ffffff4,
	};

	InkMode getInkMode() const;
	void setInkMode(InkMode inkMode);

	Shape getShape() const;
	void setShape(Shape shape);

	const ColorRGB8 &getForeColor() const;
	void setForeColor(const ColorRGB8 &color);

	const ColorRGB8 &getBackColor() const;
	void setBackColor(const ColorRGB8 &color);

	const ColorRGB8 &getBorderColor() const;
	void setBorderColor(const ColorRGB8 &color);

	const ColorRGB8 &getShadowColor() const;
	void setShadowColor(const ColorRGB8 &color);

	uint16 getBorderSize() const;
	void setBorderSize(uint16 size);

	uint16 getShadowSize() const;
	void setShadowSize(uint16 size);

	const Common::Array<Common::Point> &getPolyPoints() const;
	Common::Array<Common::Point> &modifyPolyPoints();

	bool isDirty() const;
	void clearDirty();

	//VisualElementRenderProperties &operator=(const VisualElementRenderProperties &other);

private:
	InkMode _inkMode;
	Shape _shape;
	ColorRGB8 _foreColor;
	ColorRGB8 _backColor;
	uint16 _borderSize;
	ColorRGB8 _borderColor;
	uint16 _shadowSize;
	ColorRGB8 _shadowColor;

	Common::Array<Common::Point> _polyPoints;

	bool _isDirty;
};

class VisualElement : public Element {
public:
	VisualElement();

	bool isVisual() const override;
	virtual bool isTextLabel() const;

	VThreadState consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	bool isVisible() const;
	void setVisible(bool visible);

	bool isDirectToScreen() const;
	void setDirectToScreen(bool directToScreen);

	uint16 getLayer() const;
	void setLayer(uint16 layer);

	bool isMouseInsideDrawableArea(int32 relativeX, int32 relativeY) const;

	// Returns true if there is mouse collision at a specified point, assuming it has already passed isMouseInsideBox
	virtual bool isMouseCollisionAtPoint(int32 relativeX, int32 relativeY) const;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;

	Common::Point getParentOrigin() const;
	Common::Point getGlobalPosition() const;
	const Common::Rect &getRelativeRect() const;
	virtual Common::Rect getRelativeCollisionRect() const;

	void setRelativeRect(const Common::Rect &rect);

	// The cached absolute origin is from the last time the element was rendered.
	// Do not rely on it mid-frame.
	const Common::Point &getCachedAbsoluteOrigin() const;
	void setCachedAbsoluteOrigin(const Common::Point &absOrigin);

	void setDragMotionProperties(const Common::SharedPtr<DragMotionProperties> &dragProps);
	const Common::SharedPtr<DragMotionProperties> &getDragMotionProperties() const;

	void handleDragMotion(Runtime *runtime, const Common::Point &initialOrigin, const Common::Point &targetOrigin);

	struct OffsetTranslateTaskData {
		OffsetTranslateTaskData() : dx(0), dy(0) {}

		int32 dx;
		int32 dy;
	};

	VThreadState offsetTranslateTask(const OffsetTranslateTaskData &data);

	void setRenderProperties(const VisualElementRenderProperties &props, const Common::WeakPtr<GraphicModifier> &primaryGraphicModifier);
	const VisualElementRenderProperties &getRenderProperties() const;
	const Common::WeakPtr<GraphicModifier> &getPrimaryGraphicModifier() const;

	void setTransitionProperties(const VisualElementTransitionProperties &props);
	const VisualElementTransitionProperties &getTransitionProperties() const;

	bool needsRender() const;
	virtual void render(Window *window) = 0;
	void finalizeRender();

#ifdef MTROPOLIS_DEBUG_ENABLE
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

protected:
	bool loadCommon(const Common::String &name, uint32 guid, const Data::Rect &rect, uint32 elementFlags, uint16 layer, uint32 streamLocator, uint16 sectionID);

	MiniscriptInstructionOutcome scriptSetDirect(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetPosition(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetPositionX(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetPositionY(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetCenterPosition(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetCenterPositionX(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetCenterPositionY(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetVisibility(MiniscriptThread *thread, const DynamicValue &result);
	MiniscriptInstructionOutcome scriptSetWidth(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetHeight(MiniscriptThread *thread, const DynamicValue &dest);
	MiniscriptInstructionOutcome scriptSetLayer(MiniscriptThread *thread, const DynamicValue &dest);

	MiniscriptInstructionOutcome scriptWriteRefPositionAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib);
	MiniscriptInstructionOutcome scriptWriteRefCenterPositionAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib);

	void offsetTranslate(int32 xDelta, int32 yDelta, bool cachedOriginOnly);

	Common::Point getCenterPosition() const;

	struct ChangeFlagTaskData {
		ChangeFlagTaskData() : desiredFlag(false), runtime(nullptr) {}

		bool desiredFlag;
		Runtime *runtime;
	};

	VThreadState changeVisibilityTask(const ChangeFlagTaskData &taskData);

	static VisualElement *recursiveFindItemWithLayer(VisualElement *element, int32 layer);

	bool _directToScreen;
	bool _visible;
	Common::Rect _rect;
	Common::Point _cachedAbsoluteOrigin;
	uint16 _layer;

	Common::SharedPtr<DragMotionProperties> _dragProps;

	// Quirk: When a graphic modifier is applied, it becomes the primary graphic modifier, and disabling it
	// will only take effect if it's the primary graphic modifier.
	VisualElementRenderProperties _renderProps;
	Common::WeakPtr<GraphicModifier> _primaryGraphicModifier;

	VisualElementTransitionProperties _transitionProps;

	Common::Rect _prevRect;
	bool _contentsDirty;
};

class NonVisualElement : public Element {
public:
	bool isVisual() const override;

	bool loadCommon(const Common::String &name, uint32 guid, uint32 elementFlags);
};

struct ModifierFlags {
	ModifierFlags();
	bool load(const uint32 dataModifierFlags);

	bool isLastModifier : 1;
	bool flagsWereLoaded : 1;
};

class ModifierSaveLoad {
public:
	virtual ~ModifierSaveLoad();

	void save(Modifier *modifier, Common::WriteStream *stream);
	bool load(Modifier *modifier, Common::ReadStream *stream, uint32 saveFileVersion);
	virtual void commitLoad() const = 0;

protected:
	// Saves the modifier state to a stream
	virtual void saveInternal(Common::WriteStream *stream) const = 0;

	// Loads the modifier state from a stream into the save/load state and returns true
	// if successful.  This will not trigger any actual changes until "commit" is called.
	virtual bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) = 0;
};

class ModifierHooks {
public:
	virtual ~ModifierHooks();

	virtual void onCreate(Modifier *modifier);
};

class Modifier : public RuntimeObject, public IMessageConsumer, public Debuggable {
public:
	Modifier();
	virtual ~Modifier();

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;

	void materialize(Runtime *runtime, ObjectLinkingScope *outerScope);

	virtual bool isAlias() const;
	virtual bool isVariable() const;
	virtual bool isBehavior() const;
	virtual bool isCompoundVariable() const;
	virtual bool isKeyboardMessenger() const;
	virtual Common::SharedPtr<ModifierSaveLoad> getSaveLoad();

	bool isModifier() const override;

	// This should only return a propagation container if messages should actually be propagated (i.e. NOT switched-off behaviors!)
	virtual IModifierContainer *getMessagePropagationContainer();
	virtual IModifierContainer *getChildContainer();

	const Common::WeakPtr<RuntimeObject> &getParent() const;
	void setParent(const Common::WeakPtr<RuntimeObject> &parent);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void setName(const Common::String &name);
	const Common::String &getName() const;

	const ModifierFlags &getModifierFlags() const;

	// Shallow clones only need to copy the object.  Descendent copies are done using visitInternalReferences.
	virtual Common::SharedPtr<Modifier> shallowClone() const = 0;

	// Returns the default name of the modifier.  This isn't optional: It can cause behavioral changes, e.g.
	// Obsidian depends on this working properly to resolve the TextWork modifier in the Piazza.
	virtual const char *getDefaultName() const = 0;

	// Visits any internal references in the object.
	// Any references to other elements owned by the object MUST be SharedPtr, any references to non-owned objects
	// MUST be WeakPtr, in order for the cloning and materialization logic to work correctly.
	virtual void visitInternalReferences(IStructuralReferenceVisitor *visitor);

	bool loadPlugInHeader(const PlugInModifierLoaderContext &plugInContext);

	void recursiveCollectObjectsMatchingCriteria(Common::Array<Common::WeakPtr<RuntimeObject> > &results, bool (*evalFunc)(void *userData, RuntimeObject *object), void *userData, bool onlyEnabled);

	Structural *findStructuralOwner() const;

	void setHooks(const Common::SharedPtr<ModifierHooks> &hooks);
	const Common::SharedPtr<ModifierHooks> &getHooks() const;

	// Recursively disable due to containing behavior being disabled
	virtual void disable(Runtime *runtime) = 0;

#ifdef MTROPOLIS_DEBUG_ENABLE
	SupportStatus debugGetSupportStatus() const override;
	const Common::String &debugGetName() const override;
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

protected:
	bool loadTypicalHeader(const Data::TypicalModifierHeader &typicalHeader);

	// Links any references contained in the object, resolving static GUIDs to runtime object references.
	// If you override this, you should override visitInternalReferences too
	virtual void linkInternalReferences(ObjectLinkingScope *scope);

	Common::String _name;
	ModifierFlags _modifierFlags;
	Common::WeakPtr<RuntimeObject> _parent;

	Common::SharedPtr<ModifierHooks> _hooks;
};

class VariableModifier : public Modifier {
public:
	virtual bool isVariable() const override;
	virtual bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) = 0;
	virtual void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const = 0;
	virtual Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override = 0;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

	void disable(Runtime *runtime) override;

	virtual DynamicValueWriteProxy createWriteProxy();

private:
	struct WriteProxyInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};
};

enum AssetType {
	kAssetTypeNone,

	kAssetTypeMovie,
	kAssetTypeAudio,
	kAssetTypeColorTable,
	kAssetTypeImage,
	kAssetTypeText,
	kAssetTypeMToon,
};

class AssetHooks {
public:
	virtual ~AssetHooks();

	virtual void onLoaded(Asset *asset, const Common::String &name);
};

class Asset {
public:
	Asset();
	virtual ~Asset();

	uint32 getAssetID() const;

	virtual AssetType getAssetType() const = 0;

protected:
	uint32 _assetID;
};


} // End of namespace MTropolis

#endif
