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

#include "mtropolis/miniscript.h"
#include "common/config-manager.h"

#include "common/memstream.h"

namespace MTropolis {

static bool miniscriptEvaluateTruth(const DynamicValue& value) {
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

MiniscriptReferences::MiniscriptReferences(const Common::Array<LocalRef> &localRefs) : _localRefs(localRefs) {
}

void MiniscriptReferences::linkInternalReferences(ObjectLinkingScope *scope) {
	// Resolve using name lookups since there are some known cases where the GUID is broken
	// e.g. "bArriveFromCutScene" in "Set bArriveFromCutScene on PE" in Obsidian
	for (Common::Array<LocalRef>::iterator it = _localRefs.begin(), itEnd = _localRefs.end(); it != itEnd; ++it) {
		it->resolution = scope->resolve(it->guid, it->name, false);
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
		Data::XPFloat f;
		if (!f.load(instrDataReader))
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

struct IMiniscriptInstructionFactory {
	virtual bool create(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader, MiniscriptInstruction *&outMiniscriptInstructionPtr) const = 0;
	virtual void getSizeAndAlignment(size_t &outSize, size_t &outAlignment) const = 0;
};

template<class T>
class MiniscriptInstructionFactory : public IMiniscriptInstructionFactory {
public:
	bool create(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader, MiniscriptInstruction *&outMiniscriptInstructionPtr) const override;
	void getSizeAndAlignment(size_t &outSize, size_t &outAlignment) const override;

	static IMiniscriptInstructionFactory *getInstance();

private:
	static MiniscriptInstructionFactory<T> _instance;
};

template<class T>
bool MiniscriptInstructionFactory<T>::create(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader, MiniscriptInstruction *&outMiniscriptInstructionPtr) const {
	if (!MiniscriptInstructionLoader<T>::loadInstruction(dest, instrFlags, instrDataReader))
		return false;

	outMiniscriptInstructionPtr = static_cast<MiniscriptInstruction *>(static_cast<T *>(dest));
	return true;
}

template<class T>
void MiniscriptInstructionFactory<T>::getSizeAndAlignment(size_t &outSize, size_t &outAlignment) const {
	outSize = sizeof(T);
	outAlignment = alignof(T);
}

template<class T>
inline IMiniscriptInstructionFactory *MiniscriptInstructionFactory<T>::getInstance() {
	return &_instance;
}

template<class T>
MiniscriptInstructionFactory<T> MiniscriptInstructionFactory<T>::_instance;

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

	struct InstructionData {
		uint16 opcode;
		uint16 flags;
		size_t pdPosition;
		IMiniscriptInstructionFactory *instrFactory;
		Common::Array<uint8> contents;
	};

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

		IMiniscriptInstructionFactory *factory = resolveOpcode(rawInstruction.opcode);
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
	uintptr_t programDataAddress = reinterpret_cast<uintptr_t>(&programData[0]);

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

IMiniscriptInstructionFactory *MiniscriptParser::resolveOpcode(uint16 opcode) {
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
	MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
	if (outcome != kMiniscriptInstructionOutcomeContinue)
		return outcome;

	const MiniscriptStackValue &srcValue = thread->getStackValueFromTop(0);
	MiniscriptStackValue &target = thread->getStackValueFromTop(1);

	if (target.value.getType() == DynamicValueTypes::kWriteProxy) {
		const DynamicValueWriteProxyPOD &proxy = target.value.getWriteProxyPOD();
		if (!proxy.ifc->write(thread, srcValue.value, proxy.objectRef, proxy.ptrOrOffset)) {
			thread->error("Failed to assign value");
			return kMiniscriptInstructionOutcomeFailed;
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

	return kMiniscriptInstructionOutcomeContinue;
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
		thread->error("Invalid message destination (target isn't an object reference)");
		return kMiniscriptInstructionOutcomeFailed;
	}

	Common::SharedPtr<RuntimeObject> obj = targetValue.getObject().object.lock();

	if (!obj) {
		thread->error("Invalid message destination (object reference is invalid)");
		return kMiniscriptInstructionOutcomeFailed;
	}

	Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(_evt, payloadValue, thread->getModifier()->getSelfReference()));
	Common::SharedPtr<MessageDispatch> dispatch;
	if (obj->isModifier())
		dispatch.reset(new MessageDispatch(msgProps, static_cast<Modifier *>(obj.get()), _messageFlags.cascade, _messageFlags.relay, true));
	else if (obj->isStructural())
		dispatch.reset(new MessageDispatch(msgProps, static_cast<Structural *>(obj.get()), _messageFlags.cascade, _messageFlags.relay, true));
	else {
		thread->error("Message destination is not a structural object or modifier");
		return kMiniscriptInstructionOutcomeFailed;
	}

	thread->popValues(2);

	if (_messageFlags.immediate) {
		thread->getRuntime()->sendMessageOnVThread(dispatch);
		return kMiniscriptInstructionOutcomeYieldToVThread;
	} else {
		thread->getRuntime()->queueMessage(dispatch);
		return kMiniscriptInstructionOutcomeContinue;
	}
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
			}
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

	thread->popValues(1);

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

	xValDest.setPoint(Point16::create(coords[0], coords[1]));

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

	if (_isIndexed) {
		if (thread->getStackSize() < 2) {
			thread->error("Stack underflow");
			return kMiniscriptInstructionOutcomeFailed;
		}

		// Convert index
		MiniscriptInstructionOutcome outcome = thread->dereferenceRValue(0, false);
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
				if (!obj->writeRefAttributeIndexed(thread, proxy, attrib, indexSlot.value)) {
					thread->error("Failed to get a writeable reference to attribute '" + attrib + "'");
					return kMiniscriptInstructionOutcomeFailed;
				}
			} else if (indexableValueSlot.value.getType() == DynamicValueTypes::kWriteProxy) {
				DynamicValueWriteProxy proxy = indexableValueSlot.value.getWriteProxyTEMP();

				if (!proxy.pod.ifc->refAttribIndexed(thread, proxy, proxy.pod.objectRef, proxy.pod.ptrOrOffset, attrib, indexSlot.value)) {
					thread->error("Can't write to indexed attribute '" + attrib + "'");
					return kMiniscriptInstructionOutcomeFailed;
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
					thread->error("Tried to read '" + attrib + "' to an invalid object reference");
					return kMiniscriptInstructionOutcomeFailed;
				}

				DynamicValueWriteProxy writeProxy;
				if (!obj->writeRefAttribute(thread, writeProxy, attrib)) {
					thread->error("Failed to read attribute '" + attrib + "'");
					return kMiniscriptInstructionOutcomeFailed;
				}

				indexableValueSlot.value.setWriteProxy(writeProxy);
			} else if (indexableValueSlot.value.getType() == DynamicValueTypes::kWriteProxy) {
				DynamicValueWriteProxy proxy = indexableValueSlot.value.getWriteProxyTEMP();
				if (!proxy.pod.ifc->refAttrib(thread, proxy, proxy.pod.objectRef, proxy.pod.ptrOrOffset, attrib)) {
					thread->error("Can't write to attribute '" + attrib + "'");
					return kMiniscriptInstructionOutcomeFailed;
				}
				indexableValueSlot.value.setWriteProxy(proxy);
			} else {
				thread->error("Tried to l-value index something that was not writeable");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} else {
			MiniscriptInstructionOutcome outcome = readRValueAttrib(thread, indexableValueSlot.value, attrib);
			if (outcome != kMiniscriptInstructionOutcomeContinue)
				return outcome;
		}
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome GetChild::readRValueAttrib(MiniscriptThread *thread, DynamicValue &valueSrcDest, const Common::String &attrib) const {
	switch (valueSrcDest.getType()) {
	case DynamicValueTypes::kIntegerRange:
		if (attrib == "start")
			valueSrcDest.setInt(valueSrcDest.getIntRange().min);
		else if (attrib == "end")
			valueSrcDest.setInt(valueSrcDest.getIntRange().max);
		else {
			thread->error(Common::String("Integer range has no attribute '") + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;

	case DynamicValueTypes::kVector:
		if (attrib == "angle")
			valueSrcDest.setInt(valueSrcDest.getVector().angleRadians * (180.0 / M_PI));
		else if (attrib == "magnitude")
			valueSrcDest.setInt(valueSrcDest.getVector().magnitude);
		else {
			thread->error(Common::String("Vector has no attribute '") + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;
	case DynamicValueTypes::kObject: {
			Common::SharedPtr<RuntimeObject> obj = valueSrcDest.getObject().object.lock();
			if (!obj) {
				thread->error("Unable to read attribute '" + attrib + "' from invalid object");
				return kMiniscriptInstructionOutcomeFailed;
			} else if (!obj->readAttribute(thread, valueSrcDest, attrib)) {
				thread->error("Unable to read attribute '" + attrib + "'");
				return kMiniscriptInstructionOutcomeFailed;
			}
		} break;
	default:
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
		} else {
			thread->error("Unable to read list attribute '" + attrib + "'");
			return kMiniscriptInstructionOutcomeFailed;
		}
		break;
	default:
		return kMiniscriptInstructionOutcomeFailed;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

PushValue::PushValue(DataType dataType, const void *value, bool isLValue)
	: _dataType(dataType), _isLValue(isLValue) {
	switch (dataType) {
	case DataType::kDataTypeBool:
		_value.b = *static_cast<const bool *>(value);
		break;
	case DataType::kDataTypeDouble:
		_value.f = *static_cast<const double *>(value);
		break;
	case DataType::kDataTypeLocalRef:
	case DataType::kDataTypeGlobalRef:
		_value.ref = *static_cast<const uint32 *>(value);
		break;
	case DataType::kDataTypeLabel:
		_value.lbl = *static_cast<const Label *>(value);
		break;
	default:
		break;
	}
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
	DynamicValue value;
	switch (_globalID) {
	case kGlobalRefElement:
	case kGlobalRefSection:
	case kGlobalRefSubsection:
	case kGlobalRefScene:
	case kGlobalRefProject:
		return executeFindFilteredParent(thread);
	case kGlobalRefIncomingData:
		value = thread->getMessageProperties()->getValue();
		break;
	case kGlobalRefSource:
		value.setObject(ObjectReference(thread->getMessageProperties()->getSource()));
		break;
	case kGlobalRefMouse:
		thread->error("'mouse' global ref not yet implemented");
		return kMiniscriptInstructionOutcomeFailed;
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

	thread->pushValue(value);

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome PushGlobal::executeFindFilteredParent(MiniscriptThread *thread) const {
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
		case kGlobalRefSubsection:
			isMatch = obj->isSubsection();
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

	DynamicValue value;
	value.setObject(ref);

	thread->pushValue(value);

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
	warning("Miniscript error: %s", message.c_str());

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
	case DynamicValueTypes::kReadProxy: {
			const DynamicValueReadProxyPOD &readProxy = stackValue.value.getReadProxyPOD();
			if (!readProxy.ifc->read(this, stackValue.value, readProxy.objectRef, readProxy.ptrOrOffset)) {
				this->error("Failed to access a proxy value");
				return kMiniscriptInstructionOutcomeFailed;
			}
		}
		break;
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
		MiniscriptInstruction *instr = instrs[_currentInstruction++];

		MiniscriptInstructionOutcome outcome = instr->execute(this);
		if (outcome == kMiniscriptInstructionOutcomeFailed) {
			// Should this also interrupt the message dispatch?
			_failed = true;
			return kVThreadReturn;
		}

		if (outcome == kMiniscriptInstructionOutcomeYieldToVThread)
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
