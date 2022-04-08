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
#include "mtropolis/alignment_helper.h"

#include "common/memstream.h"

#include <new>

namespace MTropolis {

MiniscriptInstruction::~MiniscriptInstruction() {
}

MiniscriptProgram::MiniscriptProgram(const Common::SharedPtr<Common::Array<uint8> > &programData, const Common::Array<MiniscriptInstruction *> &instructions,
									 const Common::Array<LocalRef> &localRefs, const Common::Array<Attribute> &attributes)
	: _programData(programData), _instructions(instructions), _localRefs(localRefs), _attributes(attributes) {
}

MiniscriptProgram::~MiniscriptProgram() {
	// Destruct all instructions
	for (Common::Array<MiniscriptInstruction *>::const_iterator it = _instructions.begin(), itEnd = _instructions.end(); it != itEnd; ++it)
		(*it)->~MiniscriptInstruction();
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

	new (dest) MiniscriptInstructions::Send(evt);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::BuiltinFunc>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 functionID;
	if (!instrDataReader.readU32(functionID))
		return false;

	if (functionID < 1 || functionID > 20)
		return false;	// Unknown function

	new (dest) MiniscriptInstructions::BuiltinFunc(static_cast<MiniscriptInstructions::BuiltinFunc::BuiltinFunctionID>(functionID));
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::GetChild>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 childAttribute;
	if (!instrDataReader.readU32(childAttribute))
		return false;

	new (dest) MiniscriptInstructions::GetChild(childAttribute);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::PushGlobal>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 globalID;
	if (!instrDataReader.readU32(globalID))
		return false;

	new (dest) MiniscriptInstructions::PushGlobal(globalID);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::Jump>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint32 jumpFlags, unknown, instrOffset;
	if (!instrDataReader.readU32(jumpFlags) || !instrDataReader.readU32(unknown) || !instrDataReader.readU32(instrOffset))
		return false;

	bool isConditional = (jumpFlags == 2);
	if (jumpFlags != 1 && jumpFlags != 2)
		return false;	// Don't recognize this flag combination

	if (instrOffset == 0)
		return false;	// Not valid

	new (dest) MiniscriptInstructions::Jump(instrOffset, isConditional);
	return true;
}

template<>
bool MiniscriptInstructionLoader<MiniscriptInstructions::PushValue>::loadInstruction(void *dest, uint32 instrFlags, Data::DataReader &instrDataReader) {
	uint16 dataType;
	if (!instrDataReader.readU16(dataType))
		return false;

	if (dataType == 0)
		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeNull, nullptr);
	else if (dataType == 0x15) {
		double d;
		if (!instrDataReader.readF64(d))
			return false;

		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeDouble, &d);
	} else if (dataType == 0x1a) {
		uint8 boolValue;
		if (!instrDataReader.readU8(boolValue))
			return false;

		bool b = (boolValue != 0);
		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeBool, &b);
	} else if (dataType == 0x1f9) {
		uint32 refValue;
		if (!instrDataReader.readU32(refValue))
			return false;

		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeLocalRef, &refValue);

	} else if (dataType == 0x1fa) {
		uint32 refValue;
		if (!instrDataReader.readU32(refValue))
			return false;

		new (dest) MiniscriptInstructions::PushValue(MiniscriptInstructions::PushValue::kDataTypeGlobalRef, &refValue);
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
void MiniscriptInstructionFactory<T>::getSizeAndAlignment(size_t& outSize, size_t& outAlignment) const {
	outSize = sizeof(T);
	outAlignment = AlignmentHelper<T>::getAlignment();
}

template<class T>
inline IMiniscriptInstructionFactory* MiniscriptInstructionFactory<T>::getInstance() {
	return &_instance;
}

template<class T>
MiniscriptInstructionFactory<T> MiniscriptInstructionFactory<T>::_instance;


Common::SharedPtr<MiniscriptProgram> MiniscriptParser::parse(const Data::MiniscriptProgram &program) {
	Common::Array<MiniscriptProgram::LocalRef> localRefs;
	Common::Array<MiniscriptProgram::Attribute> attributes;
	Common::SharedPtr<Common::Array<uint8> > programDataPtr;
	Common::Array<MiniscriptInstruction *> miniscriptInstructions;

	// If the program is empty then just return an empty program
	if (program.bytecode.size() == 0 || program.numOfInstructions == 0)
		return Common::SharedPtr<MiniscriptProgram>(new MiniscriptProgram(programDataPtr, miniscriptInstructions, localRefs, attributes));

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
	Data::DataReader reader(stream, program.projectFormat);

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
			return nullptr;

		if (instrSize < 6)
			return nullptr;

		if (instrSize > 6) {
			rawInstruction.contents.resize(instrSize - 6);
			if (!reader.read(&rawInstruction.contents[0], instrSize - 6))
				return nullptr;
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
			return nullptr;

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
		Data::DataReader instrContentsReader(instrContentsStream, reader.getProjectFormat());

		if (!rawInstruction.instrFactory->create(&programData[baseOffset + rawInstruction.pdPosition], rawInstruction.flags, instrContentsReader, miniscriptInstructions[i])) {
			// Destroy any already-created instructions
			for (size_t di = 0; di < i; di++) {
				miniscriptInstructions[i - 1 - di]->~MiniscriptInstruction();
			}

			return nullptr;
		}
	}

	// Done
	return Common::SharedPtr<MiniscriptProgram>(new MiniscriptProgram(programDataPtr, miniscriptInstructions, localRefs, attributes));
}

IMiniscriptInstructionFactory* MiniscriptParser::resolveOpcode(uint16 opcode) {
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

Send::Send(const Event &evt) : _evt(evt) {
}

BuiltinFunc::BuiltinFunc(BuiltinFunctionID bfid) : _funcID(bfid) {
}

GetChild::GetChild(uint32 attribute) : _attribute(attribute) {
}

PushValue::PushValue(DataType dataType, const void *value) {
	switch (dataType) {
	case DataType::kDataTypeBool:
		_value.b = *static_cast<const bool *>(value);
		break;
	case DataType::kDataTypeDouble:
		_value.d = *static_cast<const double *>(value);
		break;
	case DataType::kDataTypeLocalRef:
	case DataType::kDataTypeGlobalRef:
		_value.ref = *static_cast<const uint32 *>(value);
		break;
	default:
		break;
	}
}

PushGlobal::PushGlobal(uint32 globalID) : _globalID(globalID) {
}

PushString::PushString(const Common::String &str) : _str(str) {
}

Jump::Jump(uint32 instrOffset, bool isConditional) : _instrOffset(instrOffset), _isConditional(isConditional) {
}

} // End of namespace MiniscriptInstructions

} // End of namespace MTropolis
