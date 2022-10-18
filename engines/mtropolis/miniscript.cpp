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

#include "common/random.h"
#include "common/memstream.h"

#include "mtropolis/miniscript.h"

namespace MTropolis {

bool miniscriptEvaluateTruth(const DynamicValue &value) {
	// NOTE: Comparing equal to "true" only passes for 1 exactly, but for conditions,
	// any non-zero value is true.
	switch (value.getType()) {
	case DynamicValueTypes::kBoolean:
		return value.getBool();
	case DynamicValueTypes::kInteger:
		return (value.getInt() != 0);
	case DynamicValueTypes::kFloat:
		return !(value.getFloat() == 0.0);
	default:
		return false;
	}
}

MiniscriptInstruction::~MiniscriptInstruction() {
}

MiniscriptReferences::LocalRef::LocalRef() : guid(0) {
}

MiniscriptReferences::MiniscriptReferences(const Common::Array<LocalRef> &localRefs) : _localRefs(localRefs) {
}

void MiniscriptReferences::linkInternalReferences(ObjectLinkingScope *scope) {
	// Resolve using name lookups since there are some known cases where the GUID is broken
	// e.g. "bArriveFromCutScene" in "Set bArriveFromCutScene on PE" in Obsidian
	for (Common::Array<LocalRef>::iterator it = _localRefs.begin(), itEnd = _localRefs.end(); it != itEnd; ++it) {
		it->resolution = scope->resolve(it->guid, it->name, false);
	}
}

void MiniscriptReferences::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	for (LocalRef &ref : _localRefs) {
		Common::SharedPtr<RuntimeObject> obj = ref.resolution.lock();
		if (obj) {
			if (obj->isModifier()) {
				Common::WeakPtr<Modifier> mod = obj.staticCast<Modifier>();
				visitor->visitWeakModifierRef(mod);
				ref.resolution = mod;
			} else if (obj->isStructural()) {
				Common::WeakPtr<Structural> struc = obj.staticCast<Structural>();
				visitor->visitWeakStructuralRef(struc);
				ref.resolution = struc;
			}
		}
	}
}

Common::WeakPtr<RuntimeObject> MiniscriptReferences::getRefByIndex(uint index) const {
	if (index >= _localRefs.size())
		return Common::WeakPtr<RuntimeObject>();
	return _localRefs[index].resolution;
}

MiniscriptProgram::MiniscriptProgram(const Common::SharedPtr<Common::Array<uint8> > &programData, const Common::Array<MiniscriptInstruction *> &instructions, const Common::Array<Attribute> &attributes)
	: _programData(programData), _instructions(instructions), _attributes(attributes) {
}

MiniscriptProgram::~MiniscriptProgram() {
	// Destruct all instructions
	for (Common::Array<MiniscriptInstruction *>::const_iterator it = _instructions.begin(), itEnd = _instructions.end(); it != itEnd; ++it)
		(*it)->~MiniscriptInstruction();
}

const Common::Array<MiniscriptInstruction *> &MiniscriptProgram::getInstructions() const {
	return _instructions;
}

const Common::Array<MiniscriptProgram::Attribute> &MiniscriptProgram::getAttributes() const {
	return _attributes;
}

template<class T>
struct MiniscriptInstructionLoader {
	static bool loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader);
};

template<class T>
bool MiniscriptInstructionLoader<T>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	// Default loader for simple instructions with no private data
	new (dest) T();
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::Send>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	Data::Event dataEvent;
	if (!dataEvent.load(instrDataReader))
		return false;

	Event evt;
	if (!evt.load(dataEvent))
		return false;

	MessageFlags msgFlags;
	msgFlags.immediate = ((instrFlags & 0x04) == 0);
	msgFlags.cascade = ((instrFlags & 0x08) == 0);
	msgFlags.relay = ((instrFlags & 0x10) == 0);

	new (dest) MiniscriptInstructions::Send(evt, msgFlags);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::BuiltinFunc>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 functionID;
	if (!instrDataReader.readU32(functionID))
		return false;

	if (functionID < 1 || functionID > 20)
		return false; // Unknown function

	new (dest) MiniscriptInstructions::BuiltinFunc(static_cast<MiniscriptInstructions::BuiltinFunc::BuiltinFunctionID>(functionID));
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::GetChild>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 childAttribute;
	if (!instrDataReader.readU32(childAttribute))
		return false;

	new (dest) MiniscriptInstructions::GetChild(childAttribute, (instrFlags & 1) != 0, (instrFlags & 32) != 0);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::PushGlobal>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 globalID;
	if (!instrDataReader.readU32(globalID))
		return false;

	new (dest) MiniscriptInstructions::PushGlobal(globalID, (instrFlags & 1) != 0);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::Jump>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 jumpFlags, unknown, instrOffset;
	if (!instrDataReader.readU32(jumpFlags) || !instrDataReader.readU32(unknown) || !instrDataReader.readU32(instrOffset))
		return false;

	bool isConditional = (jumpFlags == 2);
	if (jumpFlags != 1 && jumpFlags != 2)
		return false; // Don't recognize this flag combination

	if (instrOffset == 0)
		return false; // Not valid

	new (dest) MiniscriptInstructions::Jump(instrOffset, isConditional);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::PushValue>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint16 dataType;
	if (!instrDataReader.readU16(dataType))
		return false;

	if (dataType == 0)
		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeNull, nullptr, false);
	else if (dataType == 0x15) {
		Common::XPFloat f;
		if (!instrDataReader.readPlatformFloat(f))
			return false;

		double d = f.toDouble();
		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeDouble, &d, false);
	} else if (dataType == 0x1a) {
		uint8 boolValue;
		if (!instrDataReader.readU8(boolValue))
			return false;

		bool b = (boolValue != 0);
		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeBool, &b, false);
	} else if (dataType == 0x1f9) {
		uint32 refValue;
		if (!instrDataReader.readU32(refValue))
			return false;

		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeLocalRef, &refValue, (instrFlags & 1) != 0);
	} else if (dataType == 0x1fa) {
		uint32 refValue;
		if (!instrDataReader.readU32(refValue))
			return false;

		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeGlobalRef, &refValue, (instrFlags & 1) != 0);
	} else if (dataType == 0x1d) {
		MiniscriptInstructions::PushValue::Label label;
		if (!instrDataReader.readU32(label.superGroup) || !instrDataReader.readU32(label.id))
			return false;

		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeLabel, &label, false);
	} else
		return false;

	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::PushString>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint16 strLength;
	if (!instrDataReader.readU16(strLength))
		return false;

	// Unlike most cases, in this case the string is null-terminated but the str length doesn't include the terminator
	Common::String str;
	if (!instrDataReader.readTerminatedStr(str, strLength + 1))
		return false;

	new (dest) MiniscriptInstructions::PushString(str);

	return true;
}

struct SIMiniscriptInstructionFactory {
	bool (*create)(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader, MiniscriptInstruction *&outMiniscriptInstructionPtr);
	void (*getSizeAndAlignment)(size_t &outSize, size_t &outAlignment);
};

template<class T>
class MiniscriptInstructionFactory {
public:
	static bool create(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader, MiniscriptInstruction *&outMiniscriptInstructionPtr);
	static void getSizeAndAlignment(size_t &outSize, size_t &outAlignment);

	static SIMiniscriptInstructionFactory *getInstance();

private:
	static SIMiniscriptInstructionFactory _instance;
};

template<class T>
bool MiniscriptInstructionFactory<T>::create(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader, MiniscriptInstruction *&outMiniscriptInstructionPtr) {
	if (!MiniscriptInstructionLoader<T>::loadInstruction(dest, instrFlags, instrDataReader))
		return false;

	outMiniscriptInstructionPtr = static_cast<MiniscriptInstruction *>(static_cast<T *>(dest));
	return true;
}

template<class T>
void MiniscriptInstructionFactory<T>::getSizeAndAlignment(size_t &outSize, size_t &outAlignment) {
	outSize = sizeof(T);
	outAlignment = alignof(T);
}

template<class T>
inline SIMiniscriptInstructionFactory *MiniscriptInstructionFactory<T>::getInstance() {
	return &_instance;
}

template<class T>
SIMiniscriptInstructionFactory MiniscriptInstructionFactory<T>::_instance = {
	MiniscriptInstructionFactory<T>::create,
	MiniscriptInstructionFactory<T>::getSizeAndAlignment
};

MiniscriptParser::InstructionData::InstructionData()
	: opcode(0), flags(0), pdPosition(0), instrFactory(nullptr) {
}

bool MiniscriptParser::parse(const Data::MiniscriptProgram &program, Common::SharedPtr<MiniscriptProgram> &outProgram, Common::SharedPtr<MiniscriptReferences> &outReferences) {
	Common::Array<MiniscriptReferences::LocalRef> localRefs;
	Common::Array<MiniscriptProgram::Attribute> attributes;
	Common::SharedPtr<Common::Array<uint8> > programDataPtr;
	Common::Array<MiniscriptInstruction *> miniscriptInstructions;

	// If the program is empty then just return an empty program
	if (program.bytecode.size() == 0 || program.numOfInstructions == 0) {
		outProgram = Common::SharedPtr<MiniscriptProgram>(new MiniscriptProgram(programDataPtr, miniscriptInstructions, attributes));
		outReferences = Common::SharedPtr<MiniscriptReferences>(new MiniscriptReferences(localRefs));
		return true;
	}

	localRefs.resize(program.localRefs.size());
	for (size_t i = 0; i < program.localRefs.size(); i++) {
		localRefs[i].guid = program.localRefs[i].guid;
		localRefs[i].name = program.localRefs[i].name;
	}

	attributes.resize(program.attributes.size());
	for (size_t i = 0; i < program.attributes.size(); i++) {
		attributes[i].name = program.attributes[i].name;
	}

	Common::MemoryReadStreamEndian stream(&program.bytecode[0], program.bytecode.size(), program.isBigEndian);
	Data::DataReader reader(0, stream, program.projectFormat);

	Common::Array<InstructionData> rawInstructions;
	rawInstructions.resize(program.numOfInstructions);

	for (size_t i = 0; i < program.numOfInstructions; i++) {
		InstructionData &rawInstruction = rawInstructions[i];
		uint16 instrSize;
		if (!reader.readU16(rawInstruction.opcode) || !reader.readU16(rawInstruction.flags) || !reader.readU16(instrSize))
			return false;

		if (instrSize < 6)
			return false;

		if (instrSize > 6) {
			rawInstruction.contents.resize(instrSize - 6);
			if (!reader.read(&rawInstruction.contents[0], instrSize - 6))
				return false;
		}
	}

	programDataPtr.reset(new Common::Array<uint8>());
	Common::Array<uint8> &programData = *programDataPtr.get();

	// Find out how much space we need and place instructions
	size_t maxAlignment = 1;
	size_t programDataSize = 0;
	for (size_t i = 0; i < program.numOfInstructions; i++) {
		InstructionData &rawInstruction = rawInstructions[i];

		SIMiniscriptInstructionFactory *factory = resolveOpcode(rawInstruction.opcode);
		rawInstruction.instrFactory = factory;

		if (!factory)
			return false;

		size_t compiledSize = 0;
		size_t compiledAlignment = 0;
		factory->getSizeAndAlignment(compiledSize, compiledAlignment);

		if (programDataSize % compiledAlignment != 0)
			programDataSize += (compiledAlignment - (programDataSize % compiledAlignment));

		rawInstruction.pdPosition = programDataSize;
		programDataSize += compiledSize;

		if (maxAlignment < compiledAlignment)
			maxAlignment = compiledAlignment;
	}

	programData.resize(programDataSize + maxAlignment - 1);
	uintptr programDataAddress = reinterpret_cast<uintptr>(&programData[0]);

	size_t baseOffset = 0;
	if (programDataAddress % maxAlignment != 0)
		baseOffset = (maxAlignment - (programDataSize % maxAlignment));

	miniscriptInstructions.resize(program.numOfInstructions);

	// Create instructions
	for (size_t i = 0; i < program.numOfInstructions; i++) {
		const InstructionData &rawInstruction = rawInstructions[i];

		const void *dataLoc = nullptr;
		if (rawInstruction.contents.size() != 0)
			dataLoc = &rawInstruction.contents[0];

		Common::MemoryReadStreamEndian instrContentsStream(static_cast<const byte *>(dataLoc), rawInstruction.contents.size(), reader.isBigEndian());
		Data::DataReader instrContentsReader(0, instrContentsStream, reader.getProjectFormat());

		if (!rawInstruction.instrFactory->create(&programData[baseOffset + rawInstruction.pdPosition], rawInstruction.flags, instrContentsReader, miniscriptInstructions[i])) {
			// Destroy any already-created instructions
			for (size_t di = 0; di < i; di++) {
				miniscriptInstructions[i - 1 - di]->~MiniscriptInstruction();
			}

			return false;
		}
	}

	// Done
	outProgram = Common::SharedPtr<MiniscriptProgram>(new MiniscriptProgram(programDataPtr, miniscriptInstructions, attributes));
	outReferences = Common::SharedPtr<MiniscriptReferences>(new MiniscriptReferences(localRefs));

	return true;
}

SIMiniscriptInstructionFactory *MiniscriptParser::resolveOpcode(uint16 opcode) {
	switch (opcode) {
	case 0x834:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Set>::getInstance();
	case 0x898:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Send>::getInstance();
	case 0xc9:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Add>::getInstance();
	case 0xca:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Sub>::getInstance();
	case 0xcb:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Mul>::getInstance();
	case 0xcc:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Div>::getInstance();
	case 0xcd:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Pow>::getInstance();
	case 0xce:
		return MiniscriptInstructionFactory<MiniscriptInstructions::And>::getInstance();
	case 0xcf:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Or>::getInstance();
	case 0xd0:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Neg>::getInstance();
	case 0xd1:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Not>::getInstance();
	case 0xd2:
		return MiniscriptInstructionFactory<MiniscriptInstructions::CmpEqual>::getInstance();
	case 0xd3:
		return MiniscriptInstructionFactory<MiniscriptInstructions::CmpNotEqual>::getInstance();
	case 0xd4:
		return MiniscriptInstructionFactory<MiniscriptInstructions::CmpLessOrEqual>::getInstance();
	case 0xd5:
		return MiniscriptInstructionFactory<MiniscriptInstructions::CmpLess>::getInstance();
	case 0xd6:
		return MiniscriptInstructionFactory<MiniscriptInstructions::CmpGreaterOrEqual>::getInstance();
	case 0xd7:
		return MiniscriptInstructionFactory<MiniscriptInstructions::CmpGreater>::getInstance();
	case 0xd8:
		return MiniscriptInstructionFactory<MiniscriptInstructions::BuiltinFunc>::getInstance();
	case 0xd9:
		return MiniscriptInstructionFactory<MiniscriptInstructions::DivInt>::getInstance();
	case 0xda:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Modulo>::getInstance();
	case 0xdb:
		return MiniscriptInstructionFactory<MiniscriptInstructions::StrConcat>::getInstance();
	case 0x12f:
		return MiniscriptInstructionFactory<MiniscriptInstructions::PointCreate>::getInstance();
	case 0x130:
		return MiniscriptInstructionFactory<MiniscriptInstructions::RangeCreate>::getInstance();
	case 0x131:
		return MiniscriptInstructionFactory<MiniscriptInstructions::VectorCreate>::getInstance();
	case 0x135:
		return MiniscriptInstructionFactory<MiniscriptInstructions::GetChild>::getInstance();
	case 0x136:
		return MiniscriptInstructionFactory<MiniscriptInstructions::ListAppend>::getInstance();
	case 0x137:
		return MiniscriptInstructionFactory<MiniscriptInstructions::ListCreate>::getInstance();
	case 0x191:
		return MiniscriptInstructionFactory<MiniscriptInstructions::PushValue>::getInstance();
	case 0x192:
		return MiniscriptInstructionFactory<MiniscriptInstructions::PushGlobal>::getInstance();
	case 0x193:
		return MiniscriptInstructionFactory<MiniscriptInstructions::PushString>::getInstance();
	case 0x7d3:
		return MiniscriptInstructionFactory<MiniscriptInstructions::Jump>::getInstance();
	default:
		return nullptr;
	}
}

namespace MiniscriptInstructions {

MiniscriptInstructionOutcome UnimplementedInstruction::execute(MiniscriptThread *thread) const {
	thread->error("Unimplemented instruction");
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome Set::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() != 2) {
		// Sets are only allowed when they would empty the stack
		thread->error("Invalid stack state for set instruction");
		return kMiniscriptInstructionOutcomeFailed;
	}

	// Convert value
	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, true);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	const MiniscriptStackValue &srcValue = thread->getStackValueFromTop(0);
	MiniscriptStackValue &target = thread->getStackValueFromTop(1);

	if (target.value.getType() == DynamicValueTypes::kWriteProxy) {
		const DynamicValueWriteProxy &proxy = target.value.getWriteProxy();
		outcome = proxy.pod.ifc->write(thread, srcValue.value, proxy.pod.objectRef, proxy.pod.ptrOrOffset);
		if (outcome == kMiniscriptInstructionOutcomeFailed) {
			thread->error("Failed to assign value to proxy");
			return outcome;
		}
	} else {
		VariableModifier *var = nullptr;
		if (target.value.getType() == DynamicValueTypes::kObject) {
			Common::SharedPtr<RuntimeObject> obj = target.value.getObject().object.lock();
			if (obj && obj->isModifier() && static_cast<const Modifier *>(obj.get())->isVariable())
				var = static_cast<VariableModifier *>(obj.get());
		}

		if (var != nullptr) {
			if (!var->varSetValue(thread, srcValue.value)) {
				thread->error("Couldn't assign value to variable, probably wrong type");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} else {
			thread->error("Can't assign to rvalue");
			return kMiniscriptInstructionOutcomeFailed;
		}
	}
	thread->popValues(2);

	return outcome;
}

Send::Send(const Event &evt, const MessageFlags &messageFlags) : _evt(evt), _messageFlags(messageFlags) {
}

MiniscriptInstructionOutcome Send::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() != 2) {
		// Send instructions are only allowed if they empty the stack
		thread->error("Invalid stack state for send instruction");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, true);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &targetValue = thread->getStackValueFromTop(0).value;
	DynamicValue &payloadValue = thread->getStackValueFromTop(1).value;

	if (targetValue.getType() != DynamicValueTypes::kObject) {
		// Failed sends are non-fatal (Obsidian requires this in the aircraft propulsion room to enter the propulsion puzzle)
		warning("Invalid message destination (target isn't an object reference)");
		thread->popValues(2);
		return kMiniscriptInstructionOutcomeContinue;
	}

	Common::SharedPtr<RuntimeObject> obj = targetValue.getObject().object.lock();

	if (!obj) {
		// Obsidian also triggers NAV_Restart on Project Started, which triggers "<init globals> on NAV_Restart"
		// which sends PRG_Toggle_Status_Display to sharedScene, even though at that point there is no shared scene.
		warning("Invalid message destination (target object is invalid)");
		thread->popValues(2);
		return kMiniscriptInstructionOutcomeContinue;
	}

	Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(_evt, payloadValue, thread->getModifier()->getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch;
	if (obj->isModifier())
		dispatch.reset(new MessageDispatch(msgProps, static_cast<Modifier *>(obj.get()), _messageFlags.cascade, _messageFlags.relay, true));
	else if (obj->isStructural())
		dispatch.reset(new MessageDispatch(msgProps, static_cast<Structural *>(obj.get()), _messageFlags.cascade, _messageFlags.relay, true));
	else {
		warning("Invalid message destination (target object is not a modifier or structural object)");
		return kMiniscriptInstructionOutcomeContinue;
	}

	thread->popValues(2);

	if (_messageFlags.immediate) {
		thread->getRuntime()->sendMessageOnVThread(dispatch);
		return kMiniscriptInstructionOutcomeYieldToVThreadNoRetry;
	} else {
		thread->getRuntime()->queueMessage(dispatch);
		return kMiniscriptInstructionOutcomeContinue;
	}
}

MiniscriptInstructionOutcome BinaryArithInstruction::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &rs = thread->getStackValueFromTop(0).value;
	DynamicValue &lsDest = thread->getStackValueFromTop(1).value;

	if (lsDest.getType() == DynamicValueTypes::kPoint && rs.getType() == DynamicValueTypes::kPoint) {
		Common::Point lsPoint = lsDest.getPoint();
		Common::Point rsPoint = rs.getPoint();

		double resultX = 0.0;
		double resultY = 0.0;
		outcome = arithExecute(thread, resultX, lsPoint.x, rsPoint.x);
		if (outcome != kMiniscriptInstructionOutcomeContinue)
			return outcome;

		outcome = arithExecute(thread, resultY, lsPoint.y, rsPoint.y);
		if (outcome != kMiniscriptInstructionOutcomeContinue)
			return outcome;

		lsDest.setPoint(Common::Point(static_cast<int16>(round(resultX)), static_cast<int16>(round(resultY))));
	} else {
		double leftVal = 0.0;
		switch (lsDest.getType()) {
		case DynamicValueTypes::kInteger:
			leftVal = lsDest.getInt();
			break;
		case DynamicValueTypes::kFloat:
			leftVal = lsDest.getFloat();
			break;
		case DynamicValueTypes::kBoolean:
			leftVal = lsDest.getBool() ? 1.0 : 0.0;
			break;
		default:
			thread->error("Invalid left-side type for binary arithmetic operator");
			return kMiniscriptInstructionOutcomeFailed;
		}

		double rightVal = 0.0;
		switch (rs.getType()) {
		case DynamicValueTypes::kInteger:
			rightVal = rs.getInt();
			break;
		case DynamicValueTypes::kFloat:
			rightVal = rs.getFloat();
			break;
		case DynamicValueTypes::kBoolean:
			rightVal = rs.getBool() ? 1.0 : 0.0;
			break;
		default:
			thread->error("Invalid right-side type for binary arithmetic operator");
			return kMiniscriptInstructionOutcomeFailed;
		}

		double result = 0.0;
		outcome = arithExecute(thread, result, leftVal, rightVal);
		if (outcome != kMiniscriptInstructionOutcomeContinue)
			return outcome;

		lsDest.setFloat(result);
	}

	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Add::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	result = left + right;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Sub::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	result = left - right;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Mul::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	result = left * right;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Div::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	if (right == 0.0) {
		thread->error("Arithmetic error: Division by zero");
		return kMiniscriptInstructionOutcomeFailed;
	}
	result = left / right;
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Pow::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	if (left < 0.0 && right != floor(right)) {
		thread->error("Arithmetic error: Left side is negative but right side is not an integer");
		return kMiniscriptInstructionOutcomeFailed;
	}
	result = pow(left, right);
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome DivInt::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	if (right == 0.0) {
		thread->error("Arithmetic error: Integer division by zero");
		return kMiniscriptInstructionOutcomeFailed;
	}
	result = floor(left / right);
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Modulo::arithExecute(MiniscriptThread *thread, double &result, double left, double right) const {
	if (right == 0.0) {
		thread->error("Arithmetic error: Modulo division by zero");
		return kMiniscriptInstructionOutcomeFailed;
	}
	result = fmod(left, right);
	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome UnorderedCompareInstruction::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &rs = thread->getStackValueFromTop(0).value;
	DynamicValue &lsDest = thread->getStackValueFromTop(1).value;

	bool isEqual = false;
	bool isUndefined = false;
	switch (lsDest.getType()) {
	case DynamicValueTypes::kString:
		if (rs.getType() == DynamicValueTypes::kString)
			isEqual = caseInsensitiveEqual(lsDest.getString(), rs.getString());
		break;
	case DynamicValueTypes::kBoolean: {
			switch (rs.getType()) {
			case DynamicValueTypes::kInteger:
				isEqual = (rs.getInt() == (lsDest.getBool() ? 1 : 0));
				break;
			case DynamicValueTypes::kFloat:
				isEqual = (rs.getFloat() == (lsDest.getBool() ? 1.0 : 0.0));
				break;
			case DynamicValueTypes::kBoolean:
				isEqual = (rs.getBool() == lsDest.getBool());
				break;
			default:
				isEqual = (lsDest.getBool() == false);
				break;
			}
		} break;
	case DynamicValueTypes::kFloat: {
			if (isnan(lsDest.getFloat()))
				isUndefined = true;
			else {
				switch (rs.getType()) {
				case DynamicValueTypes::kInteger:
					isEqual = (rs.getInt() == lsDest.getFloat());
					break;
				case DynamicValueTypes::kFloat:
					if (isnan(rs.getFloat()))
						isUndefined = true;
					else
						isEqual = (rs.getFloat() == lsDest.getFloat());
					break;
				case DynamicValueTypes::kBoolean:
					isEqual = ((rs.getBool() ? 1.0 : 0.0) == lsDest.getFloat());
					break;
				default:
					isEqual = false;
					break;
				}
			}
		} break;
	case DynamicValueTypes::kInteger: {
			switch (rs.getType()) {
			case DynamicValueTypes::kInteger:
				isEqual = (rs.getInt() == lsDest.getInt());
				break;
			case DynamicValueTypes::kFloat:
				if (isnan(rs.getFloat()))
					isUndefined = true;
				else
					isEqual = (rs.getFloat() == lsDest.getInt());
				break;
			case DynamicValueTypes::kBoolean:
				isEqual = ((rs.getBool() ? 1 : 0) == lsDest.getInt());
				break;
			default:
				isEqual = false;
				break;
			}
		} break;
	case DynamicValueTypes::kLabel: {
			// Really not sure how this works but there are buggy scripts in Obsidian which
			// were probably written as "if loop = false then ..." except Miniscript resolved
			// "loop" as a sound marker (!) because a sound marker with that name exists instead
			// of resolving it as equivalent to element.loop as it usually would.
			// Strict equality checks prevent the "GEN_Streaming_Update on ALC" script from
			// working, which prevents the bqtstreaming and bstreaming flags from being cleared,
			// causing, among other things, the player to get stuck after the Forest->Bureau
			// transition because the stuck streaming flags are blocking the VO.
			if (rs.getType() == DynamicValueTypes::kBoolean)
				isEqual = !rs.getBool();
			else
				isEqual = (lsDest == rs);
		} break;
	default:
		isEqual = (lsDest == rs);
		break;
	}

	lsDest.setBool(isUndefined == false && this->resolve(isEqual));
	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}


MiniscriptInstructionOutcome And::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &rs = thread->getStackValueFromTop(0).value;
	DynamicValue &lsDest = thread->getStackValueFromTop(1).value;
	lsDest.setBool(miniscriptEvaluateTruth(lsDest) && miniscriptEvaluateTruth(rs));

	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Or::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &rs = thread->getStackValueFromTop(0).value;
	DynamicValue &lsDest = thread->getStackValueFromTop(1).value;
	lsDest.setBool(miniscriptEvaluateTruth(lsDest) || miniscriptEvaluateTruth(rs));

	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Neg::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 1) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &value = thread->getStackValueFromTop(0).value;
	switch (value.getType()) {
	case DynamicValueTypes::kFloat:
		value.setFloat(-value.getFloat());
		break;
	case DynamicValueTypes::kInteger: {
		int32 i = value.getInt();
		if (i == (0 - 1 -0x7fffffff))
			value.setFloat(-static_cast<double>(i));
		else
			value.setInt(-i);
		} break;
	default:
		thread->error("Couldn't negate a value of a non-numeric type");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome Not::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 1) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &value = thread->getStackValueFromTop(0).value;
	value.setBool(!miniscriptEvaluateTruth(value));

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome OrderedCompareInstruction::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &rs = thread->getStackValueFromTop(0).value;
	DynamicValue &lsDest = thread->getStackValueFromTop(1).value;

	double leftValue = 0.0;
	double rightValue = 0.0;
	if (lsDest.getType() == DynamicValueTypes::kFloat)
		leftValue = lsDest.getFloat();
	else if (lsDest.getType() == DynamicValueTypes::kInteger)
		leftValue = lsDest.getInt();
	else {
		thread->error("Left-side value is invalid for comparison");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (rs.getType() == DynamicValueTypes::kFloat)
		rightValue = rs.getFloat();
	else if (rs.getType() == DynamicValueTypes::kInteger)
		rightValue = rs.getInt();
	else {
		thread->error("Right-side value is invalid for comparison");
		return kMiniscriptInstructionOutcomeFailed;
	}

	lsDest.setBool(this->compareFloat(leftValue, rightValue));
	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

BuiltinFunc::BuiltinFunc(BuiltinFunctionID bfid) : _funcID(bfid) {
}

MiniscriptInstructionOutcome BuiltinFunc::execute(MiniscriptThread *thread) const {
	size_t stackArgsNeeded = 1;
	bool returnsValue = true;

	if (thread->getStackSize() < stackArgsNeeded) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	for (size_t i = 0; i < stackArgsNeeded; i++) {
		MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(i, false);
		if (outcome != kMiniscriptInstructionOutcomeContinue)
			return outcome;
	}

	DynamicValue staticDest;
	DynamicValue *dest = nullptr;

	if (returnsValue) {
		if (stackArgsNeeded > 0)
			dest = &thread->getStackValueFromTop(stackArgsNeeded - 1).value;
		else
			dest = &staticDest;
	}

	MiniscriptInstructionOutcome outcome = executeFunction(thread, dest);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	if (stackArgsNeeded > 0) {
		size_t valuesToPop = stackArgsNeeded;
		if (returnsValue)
			valuesToPop--;

		if (valuesToPop > 0) {
			// coverity[dead_error_line]
			thread->popValues(valuesToPop);
		}
	} else {
		// coverity[dead_error_line]
		if (returnsValue)
			thread->pushValue(staticDest);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome BuiltinFunc::executeFunction(MiniscriptThread *thread, DynamicValue *returnValue) const {
	switch (_funcID) {
	case kSin:
	case kCos:
	case kRandom:
	case kSqrt:
	case kTan:
	case kAbs:
	case kSign:
	case kArctangent:
	case kExp:
	case kLn:
	case kLog:
	case kCosH:
	case kSinH:
	case kTanH:
	case kTrunc:
	case kRound:
		return executeSimpleNumericInstruction(thread, returnValue);
	case kRect2Polar:
		return executeRectToPolar(thread, returnValue);
	case kPolar2Rect:
		return executePolarToRect(thread, returnValue);
	case kNum2Str:
		return executeNum2Str(thread, returnValue);
	case kStr2Num:
		return executeStr2Num(thread, returnValue);
	default:
		thread->error("Unimplemented built-in function");
		return kMiniscriptInstructionOutcomeFailed;
	}
}

MiniscriptInstructionOutcome BuiltinFunc::executeSimpleNumericInstruction(MiniscriptThread *thread, DynamicValue *returnValue) const {
	double result = 0.0;

	double input = 0.0;
	const DynamicValue &inputDynamicValue = thread->getStackValueFromTop(0).value;

	switch (inputDynamicValue.getType()) {
	case DynamicValueTypes::kInteger:
		input = inputDynamicValue.getInt();
		break;
	case DynamicValueTypes::kFloat:
		input = inputDynamicValue.getFloat();
		break;
	default:
		thread->error("Invalid numeric function input type");
		return kMiniscriptInstructionOutcomeFailed;
	}

	switch (_funcID) {
	case kSin:
		result = sin(input * (M_PI / 180.0));
		break;
	case kCos:
		result = cos(input * (M_PI / 180.0));
		break;
	case kRandom:
		if (input < 1.5)
			result = 0.0;
		else {
			uint rngMax = static_cast<uint>(floor(input + 0.5)) - 1;
			result = thread->getRuntime()->getRandom()->getRandomNumber(rngMax);
		}
		break;
	case kSqrt:
		result = sqrt(input);
		break;
	case kTan:
		result = tan(input * (M_PI / 180.0));
		break;
	case kAbs:
		result = fabs(input);
		break;
	case kSign:
		if (input < 0.0)
			result = -1;
		else if (input > 0.0)
			result = 1;
		else
			result = 0;
		break;
	case kArctangent:
		result = atan(input) * (180.0 / M_PI);
		break;
	case kExp:
		result = exp(input);
		break;
	case kLn:
		result = log(input);
		break;
	case kLog:
		result = log10(input);
		break;
	case kCosH:
		result = cosh(input * (M_PI / 180.0));
		break;
	case kSinH:
		result = sinh(input * (M_PI / 180.0));
		break;
	case kTanH:
		result = tanh(input * (M_PI / 180.0));
		break;
	case kTrunc:
		result = trunc(input);
		break;
	case kRound:
		result = round(input);
		break;
	default:
		thread->error("Unimplemented numeric function");
		return kMiniscriptInstructionOutcomeFailed;
	}

	returnValue->setFloat(result);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome BuiltinFunc::executeRectToPolar(MiniscriptThread *thread, DynamicValue *returnValue) const {
	const DynamicValue &inputDynamicValue = thread->getStackValueFromTop(0).value;

	if (inputDynamicValue.getType() != DynamicValueTypes::kPoint) {
		thread->error("Polar to rect input must be a vector");
		return kMiniscriptInstructionOutcomeFailed;
	}

	Common::Point pt = inputDynamicValue.getPoint();

	double angle = atan2(pt.y, pt.x);
	double magnitude = sqrt(pt.x * pt.x + pt.y * pt.y);

	returnValue->setVector(AngleMagVector::createRadians(angle, magnitude));

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome BuiltinFunc::executePolarToRect(MiniscriptThread *thread, DynamicValue *returnValue) const {
	const DynamicValue &inputDynamicValue = thread->getStackValueFromTop(0).value;

	if (inputDynamicValue.getType() != DynamicValueTypes::kVector) {
		thread->error("Polar to rect input must be a vector");
		return kMiniscriptInstructionOutcomeFailed;
	}

	const AngleMagVector &vec = inputDynamicValue.getVector();

	double x = cos(vec.angleDegrees * (M_PI / 180.0)) * vec.magnitude;
	double y = sin(vec.angleDegrees * (M_PI / 180.0)) * vec.magnitude;

	returnValue->setPoint(Common::Point(static_cast<int16>(round(x)), static_cast<int16>(round(y))));

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome BuiltinFunc::executeNum2Str(MiniscriptThread *thread, DynamicValue *returnValue) const {
	Common::String result;

	const DynamicValue &inputDynamicValue = thread->getStackValueFromTop(0).value;
	switch (inputDynamicValue.getType()) {
	case DynamicValueTypes::kInteger:
		result = Common::String::format("%i", static_cast<int>(inputDynamicValue.getInt()));
		break;
	case DynamicValueTypes::kFloat:
		result = Common::String::format("%g", static_cast<double>(inputDynamicValue.getFloat()));
		break;
	default:
		thread->error("Invalid input value to num2str");
		return kMiniscriptInstructionOutcomeFailed;
	}

	returnValue->setString(result);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome BuiltinFunc::executeStr2Num(MiniscriptThread *thread, DynamicValue *returnValue) const {
	double result = 0.0;

	const DynamicValue &inputDynamicValue = thread->getStackValueFromTop(0).value;
	if (inputDynamicValue.getType() != DynamicValueTypes::kString) {
		thread->error("Invalid input value to str2num");
		return kMiniscriptInstructionOutcomeFailed;
	}

	const Common::String &str = inputDynamicValue.getString();
	if (str.empty())
		result = 0.0;
	else if (str.size() == 0 || !sscanf(str.c_str(), "%lf", &result)) {
		thread->error("Couldn't parse number");
		return kMiniscriptInstructionOutcomeFailed;
	}

	returnValue->setFloat(result);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome StrConcat::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &rVal = thread->getStackValueFromTop(0).value;
	DynamicValue &lValDest = thread->getStackValueFromTop(1).value;

	if (rVal.getType() != DynamicValueTypes::kString) {
		thread->error("String concat right side was not a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (lValDest.getType() != DynamicValueTypes::kString) {
		thread->error("String concat left side was not a string");
		return kMiniscriptInstructionOutcomeFailed;
	}

	lValDest.setString(lValDest.getString() + rVal.getString());
	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome PointCreate::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &yVal = thread->getStackValueFromTop(0).value;
	DynamicValue &xValDest = thread->getStackValueFromTop(1).value;

	int16 coords[2];
	DynamicValue *coordInputs[2] = {&xValDest, &yVal};

	for (int i = 0; i < 2; i++) {
		DynamicValue *v = coordInputs[i];
		DynamicValue listContents;

		if (v->getType() == DynamicValueTypes::kList) {
			// Yes this is actually allowed
			const Common::SharedPtr<DynamicList> &list = v->getList();
			if (list->getSize() != 1 || !list->getAtIndex(0, listContents)) {
				thread->error("Can't convert list to integer");
				return kMiniscriptInstructionOutcomeFailed;
			}

			v = &listContents;
		}

		switch (v->getType()) {
		case DynamicValueTypes::kFloat:
			coords[i] = static_cast<int16>(floor(v->getFloat() + 0.5)) & 0xffff;
			break;
		case DynamicValueTypes::kInteger:
			coords[i] = static_cast<int16>(v->getInt());
			break;
		case DynamicValueTypes::kBoolean:
			coords[i] = (v->getBool()) ? 1 : 0;
			break;
		default:
			thread->error("Invalid input for point creation");
			return kMiniscriptInstructionOutcomeFailed;
		}
	}

	xValDest.setPoint(Common::Point(coords[0], coords[1]));

	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome RangeCreate::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	DynamicValue &yVal = thread->getStackValueFromTop(0).value;
	DynamicValue &xValDest = thread->getStackValueFromTop(1).value;

	int32 coords[2];
	DynamicValue *coordInputs[2] = {&xValDest, &yVal};

	for (int i = 0; i < 2; i++) {
		DynamicValue *v = coordInputs[i];
		DynamicValue listContents;

		if (v->getType() == DynamicValueTypes::kList) {
			// Yes this is actually allowed
			const Common::SharedPtr<DynamicList> &list = v->getList();
			if (list->getSize() != 1 || !list->getAtIndex(0, listContents)) {
				thread->error("Can't convert list to integer");
				return kMiniscriptInstructionOutcomeFailed;
			}

			v = &listContents;
		}

		switch (v->getType()) {
		case DynamicValueTypes::kFloat:
			coords[i] = static_cast<int32>(floor(v->getFloat() + 0.5));
			break;
		case DynamicValueTypes::kInteger:
			coords[i] = v->getInt();
			break;
		case DynamicValueTypes::kBoolean:
			coords[i] = (v->getBool()) ? 1 : 0;
			break;
		default:
			thread->error("Invalid input for point creation");
			return kMiniscriptInstructionOutcomeFailed;
		}
	}

	xValDest.setIntRange(IntRange(coords[0], coords[1]));

	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

GetChild::GetChild(uint32 attribute, bool isLValue, bool isIndexed)
	: _attribute(attribute), _isLValue(isLValue), _isIndexed(isIndexed) {
}

MiniscriptInstructionOutcome GetChild::execute(MiniscriptThread *thread) const {
	const Common::Array<MiniscriptProgram::Attribute> &attribs = thread->getProgram()->getAttributes();
	if (_attribute >= attribs.size()) {
		thread->error("Invalid attribute index");
		return kMiniscriptInstructionOutcomeFailed;
	}

	const Common::String &attrib = attribs[_attribute].name;

	MiniscriptInstructionOutcome outcome = kMiniscriptInstructionOutcomeFailed;

	if (_isIndexed) {
		if (thread->getStackSize() < 2) {
			thread->error("Stack underflow");
			return kMiniscriptInstructionOutcomeFailed;
		}

		// Convert index
		outcome = thread->dereferenceRValue(0, false);
		if (outcome != kMiniscriptInstructionOutcomeContinue)
			return outcome;

		const MiniscriptStackValue &indexSlot = thread->getStackValueFromTop(0);
		MiniscriptStackValue &indexableValueSlot = thread->getStackValueFromTop(1);

		if (_isLValue) {
			if (indexableValueSlot.value.getType() == DynamicValueTypes::kObject) {
				Common::SharedPtr<RuntimeObject> obj = indexableValueSlot.value.getObject().object.lock();
				if (!obj) {
					thread->error("Tried to write '" + attrib + "' to an invalid object reference");
					return kMiniscriptInstructionOutcomeFailed;
				}

				DynamicValueWriteProxy proxy;
				outcome = obj->writeRefAttributeIndexed(thread, proxy, attrib, indexSlot.value);
				if (outcome == kMiniscriptInstructionOutcomeFailed) {
					thread->error("Failed to get a writeable reference to indexed attribute '" + attrib + "'");
					return outcome;
				}

				indexableValueSlot.value.setWriteProxy(proxy);
			} else if (indexableValueSlot.value.getType() == DynamicValueTypes::kWriteProxy) {
				DynamicValueWriteProxy proxy = indexableValueSlot.value.getWriteProxy();

				outcome = proxy.pod.ifc->refAttribIndexed(thread, proxy, proxy.pod.objectRef, proxy.pod.ptrOrOffset, attrib, indexSlot.value);
				if (outcome == kMiniscriptInstructionOutcomeFailed) {
					thread->error("Can't write to indexed attribute '" + attrib + "'");
					return outcome;
				}

				indexableValueSlot.value.setWriteProxy(proxy);
			} else {
				thread->error("Tried to l-value index something that was not writeable");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} else {
			outcome = readRValueAttribIndexed(thread, indexableValueSlot.value, attrib, indexSlot.value);
			if (outcome != kMiniscriptInstructionOutcomeContinue)
				return outcome;
		}

		thread->popValues(1);
	} else {
		if (thread->getStackSize() < 1) {
			thread->error("Stack underflow");
			return kMiniscriptInstructionOutcomeFailed;
		}

		MiniscriptStackValue &indexableValueSlot = thread->getStackValueFromTop(0);

		if (_isLValue) {
			if (indexableValueSlot.value.getType() == DynamicValueTypes::kObject) {
				Common::SharedPtr<RuntimeObject> obj = indexableValueSlot.value.getObject().object.lock();
				if (!obj) {
					thread->error("Tried to indirect '" + attrib + "' using an invalid object reference");
					return kMiniscriptInstructionOutcomeFailed;
				}

				DynamicValueWriteProxy writeProxy;
				outcome = obj->writeRefAttribute(thread, writeProxy, attrib);
				if (outcome == kMiniscriptInstructionOutcomeFailed) {
					thread->error("Failed to get a writeable reference to attribute '" + attrib + "'");
					return outcome;
				}

				indexableValueSlot.value.setWriteProxy(writeProxy);
			} else if (indexableValueSlot.value.getType() == DynamicValueTypes::kWriteProxy) {
				DynamicValueWriteProxy proxy = indexableValueSlot.value.getWriteProxy();
				outcome = proxy.pod.ifc->refAttrib(thread, proxy, proxy.pod.objectRef, proxy.pod.ptrOrOffset, attrib);
				if (outcome == kMiniscriptInstructionOutcomeFailed) {
					thread->error("Can't write to attribute '" + attrib + "'");
					return outcome;
				}
				indexableValueSlot.value.setWriteProxy(proxy);
			} else {
				thread->error("Tried to l-value index something that was not writeable");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} else {
			outcome = readRValueAttrib(thread, indexableValueSlot.value, attrib);
		}
	}

	return outcome;
}

MiniscriptInstructionOutcome GetChild::readRValueAttrib(MiniscriptThread *thread, DynamicValue &valueSrcDest, const Common::String &attrib) const {
	switch (valueSrcDest.getType()) {
	case DynamicValueTypes::kPoint:
		if (attrib == "x")
			valueSrcDest.setInt(valueSrcDest.getPoint().x);
		else if (attrib == "y")
			valueSrcDest.setInt(valueSrcDest.getPoint().y);
		else {
			thread->error("Point has no attribute '" + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;
	case DynamicValueTypes::kIntegerRange:
		if (attrib == "start")
			valueSrcDest.setInt(valueSrcDest.getIntRange().min);
		else if (attrib == "end")
			valueSrcDest.setInt(valueSrcDest.getIntRange().max);
		else {
			thread->error("Integer range has no attribute '" + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;

	case DynamicValueTypes::kVector:
		if (attrib == "angle")
			valueSrcDest.setFloat(valueSrcDest.getVector().angleDegrees);
		else if (attrib == "magnitude")
			valueSrcDest.setFloat(valueSrcDest.getVector().magnitude);
		else {
			thread->error("Vector has no attribute '" + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;
	case DynamicValueTypes::kObject: {
			Common::SharedPtr<RuntimeObject> obj = valueSrcDest.getObject().object.lock();
			if (!obj) {
				thread->error("Unable to read object attribute '" + attrib + "' from invalid object");
				return kMiniscriptInstructionOutcomeFailed;
			} else if (!obj->readAttribute(thread, valueSrcDest, attrib)) {
				thread->error("Unable to read object attribute '" + attrib + "'");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} break;
	case DynamicValueTypes::kList: {
			Common::SharedPtr<DynamicList> list = valueSrcDest.getList();
			if (attrib == "count") {
				valueSrcDest.setInt(list->getSize());
			} else {
				thread->error("Unable to read list attribute '" + attrib + "'");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} break;
	default:
		thread->error("Unable to read attribute '" + attrib + "' from rvalue");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome GetChild::readRValueAttribIndexed(MiniscriptThread *thread, DynamicValue &valueSrcDest, const Common::String &attrib, const DynamicValue &index) const {
	switch (valueSrcDest.getType()) {
	case DynamicValueTypes::kList:
		if (attrib == "value") {
			// Hold list ref since it may get released by the read operation
			Common::SharedPtr<DynamicList> list = valueSrcDest.getList();
			size_t realIndex = 0;
			if (!DynamicList::dynamicValueToIndex(realIndex, index)) {
				thread->error("Unable to list value at specified index");
				return kMiniscriptInstructionOutcomeFailed;
			}

			if (!list->getAtIndex(realIndex, valueSrcDest)) {
				thread->error("List read index out of bounds");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} else {
			thread->error("Unable to read list attribute '" + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;
	case DynamicValueTypes::kObject: {
			Common::SharedPtr<RuntimeObject> obj = valueSrcDest.getObject().object.lock();
			if (!obj) {
				thread->error("Unable to read object indexed attribute '" + attrib + "' from invalid object");
				return kMiniscriptInstructionOutcomeFailed;
			} else if (!obj->readAttributeIndexed(thread, valueSrcDest, attrib, index)) {
				thread->error("Unable to read object indexed attribute '" + attrib + "'");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} break;
	default:
		thread->error("Unable to read indexed rvalue attribute '" + attrib + "'");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

PushValue::PushValue(DataType dataType, const void *value, bool isLValue)
	: _dataType(dataType)/*, _isLValue(isLValue) */ {

	switch (dataType) {
	case DataType::kDataTypeBool:
		new (static_cast<bool *>(&_value.b)) bool(*static_cast<const bool *>(value));
		break;
	case DataType::kDataTypeDouble:
		new (static_cast<double *>(&_value.f)) double(*static_cast<const double *>(value));
		break;
	case DataType::kDataTypeLocalRef:
	case DataType::kDataTypeGlobalRef:
		new (static_cast<uint32 *>(&_value.ref)) uint32(*static_cast<const uint32 *>(value));
		break;
	case DataType::kDataTypeLabel:
		new (static_cast<Label *>(&_value.lbl)) Label(*static_cast<const Label *>(value));
		break;
	case DataType::kDataTypeNull:
		break;
	default:
		warning("PushValue instruction has an unknown type of value, this will probably malfunction!");
		break;
	}
}

PushValue::ValueUnion::ValueUnion() {
}

PushValue::~PushValue() {
	switch (_dataType) {
	case DataType::kDataTypeLabel:
		_value.lbl.~Label();
		break;
	default:
		break;
	}
}

MiniscriptInstructionOutcome ListCreate::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	MiniscriptStackValue &rs = thread->getStackValueFromTop(0);
	MiniscriptStackValue &lsDest = thread->getStackValueFromTop(1);

	Common::SharedPtr<DynamicList> list(new DynamicList());
	if (!list->setAtIndex(1, rs.value)) {
		thread->error("Failed to set value 2 of list");
		return kMiniscriptInstructionOutcomeFailed;
	}
	if (!list->setAtIndex(0, lsDest.value)) {
		thread->error("Failed to set value 1 of list");
		return kMiniscriptInstructionOutcomeFailed;
	}

	lsDest.value.setList(list);
	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome ListAppend::execute(MiniscriptThread *thread) const {
	if (thread->getStackSize() < 2) {
		thread->error("Stack underflow");
		return kMiniscriptInstructionOutcomeFailed;
	}

	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	outcome = thread->dereferenceRValue(1, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	MiniscriptStackValue &rs = thread->getStackValueFromTop(0);
	MiniscriptStackValue &lsDest = thread->getStackValueFromTop(1);

	if (lsDest.value.getType() != DynamicValueTypes::kList) {
		thread->error("Expected list on left side of list_append");
		return kMiniscriptInstructionOutcomeFailed;
	}

	Common::SharedPtr<DynamicList> listRef = lsDest.value.getList();
	if (listRef.refCount() != 2) {
		listRef = listRef->clone();
		lsDest.value.setList(listRef);
	}

	if (!listRef->setAtIndex(listRef->getSize(), rs.value)) {
		thread->error("Failed to expand list");
		return kMiniscriptInstructionOutcomeFailed;
	}

	thread->popValues(1);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome PushValue::execute(MiniscriptThread *thread) const {
	DynamicValue value;

	switch (_dataType) {
	case DataType::kDataTypeNull:
		value.clear();
		break;
	case DataType::kDataTypeDouble:
		value.setFloat(_value.f);
		break;
	case DataType::kDataTypeBool:
		value.setBool(_value.b);
		break;
	case DataType::kDataTypeLocalRef:
		value.setObject(ObjectReference(thread->getRefs()->getRefByIndex(_value.ref)));
		break;
	case DataType::kDataTypeGlobalRef:
		thread->error("Global references are not implemented");
		return kMiniscriptInstructionOutcomeFailed;
	case DataType::kDataTypeLabel: {
		MTropolis::Label label;
		label.id = _value.lbl.id;
		label.superGroupID = _value.lbl.superGroup;
		value.setLabel(label);
	} break;
	default:
		assert(false);
		break;
	}

	thread->pushValue(value);

	return kMiniscriptInstructionOutcomeContinue;
}

PushGlobal::PushGlobal(uint32 globalID, bool isLValue) : _globalID(globalID), _isLValue(isLValue) {
}

MiniscriptInstructionOutcome PushGlobal::execute(MiniscriptThread *thread) const {
	thread->pushValue(DynamicValue());

	DynamicValue &value = thread->getStackValueFromTop(0).value;

	switch (_globalID) {
	case kGlobalRefElement:
	case kGlobalRefSection:
	case kGlobalRefScene:
	case kGlobalRefProject:
		return executeFindFilteredParent(thread, value);
	case kGlobalRefModifier:
		value.setObject(thread->getModifier()->getSelfReference());
		break;
	case kGlobalRefIncomingData:
		if (_isLValue) {
			DynamicValueWriteProxy proxy;
			thread->createWriteIncomingDataProxy(proxy);
			value.setWriteProxy(proxy);
		} else
			value = thread->getMessageProperties()->getValue();
		break;
	case kGlobalRefSource:
		value.setObject(ObjectReference(thread->getMessageProperties()->getSource()));
		break;
	case kGlobalRefMouse:
		value.setPoint(thread->getRuntime()->getCachedMousePosition());
		break;
	case kGlobalRefTicks:
		value.setInt(thread->getRuntime()->getPlayTime() * 60 / 1000);
		break;
	case kGlobalRefSharedScene:
		value.setObject(ObjectReference(thread->getRuntime()->getActiveSharedScene()));
		break;
	case kGlobalRefActiveScene:
		value.setObject(ObjectReference(thread->getRuntime()->getActiveMainScene()));
		break;
	default:
		assert(false);
		thread->error("Unknown global ref type");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome PushGlobal::executeFindFilteredParent(MiniscriptThread *thread, DynamicValue &result) const {
	Common::WeakPtr<RuntimeObject> ref = thread->getModifier()->getSelfReference();
	for (;;) {
		Common::SharedPtr<RuntimeObject> obj = ref.lock();
		if (!obj)
			break;

		bool isMatch = false;
		switch (_globalID) {
		case kGlobalRefElement:
			isMatch = obj->isStructural();	// We don't classify the project, sections, and subsections as elements, but mTropolis does
			break;
		case kGlobalRefSection:
			isMatch = obj->isSection();
			break;
		case kGlobalRefScene:
			// FIXME: Need better detection of scenes
			isMatch = obj->isElement() && static_cast<Element *>(obj.get())->getParent()->isSubsection();
			break;
		case kGlobalRefProject:
			isMatch = obj->isProject();
			break;
		default:
			break;
		};

		if (isMatch)
			break;
		else if (obj->isStructural()) {
			Structural *parent = static_cast<Structural *>(obj.get())->getParent();
			if (parent)
				ref = parent->getSelfReference();
			else {
				ref.reset();
				break;
			}
		} else if (obj->isModifier()) {
			ref = static_cast<Modifier *>(obj.get())->getParent();
		} else {
			ref.reset();
			break;
		}
	}

	result.setObject(ref);

	return kMiniscriptInstructionOutcomeContinue;
}

PushString::PushString(const Common::String &str) : _str(str) {
}

MiniscriptInstructionOutcome PushString::execute(MiniscriptThread *thread) const {
	DynamicValue str;
	str.setString(_str);
	thread->pushValue(str);

	return kMiniscriptInstructionOutcomeContinue;
}

Jump::Jump(uint32 instrOffset, bool isConditional) : _instrOffset(instrOffset), _isConditional(isConditional) {
}

MiniscriptInstructionOutcome Jump::execute(MiniscriptThread *thread) const {
	if (_isConditional) {
		if (thread->getStackSize() < 1) {
			thread->error("Stack underflow");
			return kMiniscriptInstructionOutcomeFailed;
		}

		MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
		if (outcome != kMiniscriptInstructionOutcomeContinue)
			return outcome;

		bool isTrue = miniscriptEvaluateTruth(thread->getStackValueFromTop(0).value);

		thread->popValues(1);

		if (!isTrue)
			thread->jumpOffset(this->_instrOffset);
	} else {
		thread->jumpOffset(this->_instrOffset);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

} // End of namespace MiniscriptInstructions

MiniscriptThread::MiniscriptThread(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msgProps, const Common::SharedPtr<MiniscriptProgram> &program, const Common::SharedPtr<MiniscriptReferences> &refs, Modifier *modifier)
	: _runtime(runtime), _msgProps(msgProps), _program(program), _refs(refs), _modifier(modifier), _currentInstruction(0), _failed(false) {
}

void MiniscriptThread::runOnVThread(VThread &vthread, const Common::SharedPtr<MiniscriptThread> &thread) {
	ResumeTaskData *taskData = vthread.pushTask("MiniscriptThread::resumeTask", resumeTask);
	taskData->thread = thread;
}

void MiniscriptThread::error(const Common::String &message) {
#ifdef MTROPOLIS_DEBUG_ENABLE
	if (_runtime->debugGetDebugger())
		_runtime->debugGetDebugger()->notify(kDebugSeverityError, Common::String("Miniscript error: " + message));
#endif
	warning("Miniscript error in (%x '%s'): %s", _modifier->getStaticGUID(), _modifier->getName().c_str(), message.c_str());

	// This should be redundant
	_failed = true;
}

const Common::SharedPtr<MiniscriptProgram> &MiniscriptThread::getProgram() const {
	return _program;
}

const Common::SharedPtr<MiniscriptReferences> &MiniscriptThread::getRefs() const {
	return _refs;
}

Modifier *MiniscriptThread::getModifier() const {
	return _modifier;
}

const Common::SharedPtr<MessageProperties> &MiniscriptThread::getMessageProperties() const {
	return _msgProps;
}

Runtime *MiniscriptThread::getRuntime() const {
	return _runtime;
}

void MiniscriptThread::pushValue(const DynamicValue &value) {
	_stack.push_back(MiniscriptStackValue());

	MiniscriptStackValue &stackValue = _stack.back();
	stackValue.value = value;
}

void MiniscriptThread::popValues(size_t count) {
	while (count--)
		_stack.pop_back();
}

size_t MiniscriptThread::getStackSize() const {
	return _stack.size();
}

MiniscriptStackValue &MiniscriptThread::getStackValueFromTop(size_t offset) {
	assert(offset < _stack.size());
	return _stack[_stack.size() - 1 - offset];
}

MiniscriptInstructionOutcome MiniscriptThread::dereferenceRValue(size_t offset, bool cloneLists) {
	assert(offset < _stack.size());
	MiniscriptStackValue &stackValue = _stack[_stack.size() - 1 - offset];

	switch (stackValue.value.getType()) {
	case DynamicValueTypes::kObject: {
			Common::SharedPtr<RuntimeObject> obj = stackValue.value.getObject().object.lock();
			if (obj && obj->isModifier()) {
				const Modifier *modifier = static_cast<const Modifier *>(obj.get());
				if (modifier->isVariable()) {
					static_cast<const VariableModifier *>(modifier)->varGetValue(this, stackValue.value);
				}
			}
		} break;
	case DynamicValueTypes::kWriteProxy:
		this->error("Attempted to dereference an lvalue proxy");
		return kMiniscriptInstructionOutcomeFailed;
	case DynamicValueTypes::kList:
			if (cloneLists)
				stackValue.value.setList(stackValue.value.getList()->clone());
			break;
	default:
		break;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

void MiniscriptThread::jumpOffset(size_t offset) {
	if (offset == 0) {
		this->error("Invalid jump offset");
		_failed = true;
		return;
	}

	_currentInstruction += offset - 1;
}

bool MiniscriptThread::evaluateTruthOfResult(bool &isTrue) {
	if (_stack.size() != 1) {
		this->error("Miniscript program didn't return a result");
		return false;
	}

	MiniscriptInstructionOutcome outcome = dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue) {
		this->error("Miniscript program result couldn't be dereferenced");
		return false;
	}

	isTrue = miniscriptEvaluateTruth(_stack[0].value);
	return true;
}

void MiniscriptThread::createWriteIncomingDataProxy(DynamicValueWriteProxy &proxy) {
	proxy.pod.ifc = DynamicValueWriteInterfaceGlue<IncomingDataWriteInterface>::getInstance();
	proxy.pod.objectRef = this;
	proxy.pod.ptrOrOffset = 0;
}

MiniscriptInstructionOutcome MiniscriptThread::IncomingDataWriteInterface::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	thread->_msgProps->setValue(value);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MiniscriptThread::IncomingDataWriteInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	// TODO: Generic refAttrib for dynamic values
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome MiniscriptThread::IncomingDataWriteInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	// TODO: Generic refAttribIndexed for dynamic values
	return kMiniscriptInstructionOutcomeFailed;
}


VThreadState MiniscriptThread::resumeTask(const ResumeTaskData &data) {
	return data.thread->resume(data);
}

VThreadState MiniscriptThread::resume(const ResumeTaskData &taskData) {
	const Common::Array<MiniscriptInstruction *> &instrsArray = _program->getInstructions();

	if (instrsArray.size() == 0)
		return kVThreadReturn;

	MiniscriptInstruction *const *instrs = &instrsArray[0];
	size_t numInstrs = instrsArray.size();

	if (_currentInstruction >= numInstrs || _failed)
		return kVThreadReturn;

	// Requeue now so that any VThread tasks queued by instructions run in front of the resume
	{
		ResumeTaskData *requeueData = _runtime->getVThread().pushTask("MiniscriptThread::resumeTask", resumeTask);
		requeueData->thread = taskData.thread;
	}

	while (_currentInstruction < numInstrs && !_failed) {
		size_t instrNum = _currentInstruction++;
		MiniscriptInstruction *instr = instrs[instrNum];

		MiniscriptInstructionOutcome outcome = instr->execute(this);
		if (outcome == kMiniscriptInstructionOutcomeFailed) {
			// Should this also interrupt the message dispatch?
			_failed = true;
			return kVThreadReturn;
		}

		if (outcome == kMiniscriptInstructionOutcomeYieldToVThreadAndRetry) {
			_currentInstruction = instrNum;
			return kVThreadReturn;
		}

		if (outcome == kMiniscriptInstructionOutcomeYieldToVThreadNoRetry)
			return kVThreadReturn;
	}

	return kVThreadReturn;
}

MiniscriptInstructionOutcome MiniscriptThread::tryLoadVariable(MiniscriptStackValue &stackValue) {
	if (stackValue.value.getType() == DynamicValueTypes::kObject) {
		Common::SharedPtr<RuntimeObject> obj = stackValue.value.getObject().object.lock();
		if (obj && obj->isModifier() && static_cast<Modifier *>(obj.get())->isVariable()) {
			VariableModifier *varMod = static_cast<VariableModifier *>(obj.get());
			varMod->varGetValue(this, stackValue.value);
		}
	}

	return kMiniscriptInstructionOutcomeContinue;
}

} // End of namespace MTropolis
