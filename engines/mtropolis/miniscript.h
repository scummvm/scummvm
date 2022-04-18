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

#ifndef MTROPOLIS_MINISCRIPT_H
#define MTROPOLIS_MINISCRIPT_H

#include "mtropolis/data.h"
#include "mtropolis/runtime.h"

namespace MTropolis {

struct MiniscriptVM;
struct IMiniscriptInstructionFactory;

class MiniscriptInstruction {
public:
	virtual ~MiniscriptInstruction();
};

class MiniscriptReferences {
public:
	struct LocalRef {
		uint32 guid;
		Common::String name;
	};

	struct GlobalRef {
		uint32 guid;
	};

	MiniscriptReferences(const Common::Array<LocalRef> &localRefs, const Common::Array<GlobalRef> &globalRefs);

private:
	Common::Array<LocalRef> _localRefs;
	Common::Array<GlobalRef> _globalRefs;
};

class MiniscriptProgram {
public:

	struct Attribute {
		Common::String name;
	};

	MiniscriptProgram(const Common::SharedPtr<Common::Array<uint8> > &programData, const Common::Array<MiniscriptInstruction *> &instructions, const Common::Array<Attribute> &attributes);
	~MiniscriptProgram();

private:
	Common::SharedPtr<Common::Array<uint8> > _programData;
	Common::Array<MiniscriptInstruction *> _instructions;
	Common::Array<Attribute> _attributes;
};

class MiniscriptParser {
public:
	static bool parse(const Data::MiniscriptProgram &programData, Common::SharedPtr<MiniscriptProgram> &outProgram, Common::SharedPtr<MiniscriptReferences> &outReferences);

	static IMiniscriptInstructionFactory *resolveOpcode(uint16 opcode);
};

namespace MiniscriptInstructions {
	class UnimplementedInstruction : public MiniscriptInstruction {
	};

	class Set : public UnimplementedInstruction {
	};

	class Send : public UnimplementedInstruction {
	public:
		explicit Send(const Event &evt);

	private:
		Event _evt;
	};

	class Add : public UnimplementedInstruction {
	};

	class Sub : public UnimplementedInstruction {
	};

	class Mul : public UnimplementedInstruction {
	};

	class Div : public UnimplementedInstruction {
	};

	class Pow : public UnimplementedInstruction {
	};

	class And : public UnimplementedInstruction {
	};

	class Or : public UnimplementedInstruction {
	};

	class Neg : public UnimplementedInstruction {
	};

	class Not : public UnimplementedInstruction {
	};

	class CmpEqual : public UnimplementedInstruction {
	};

	class CmpNotEqual : public UnimplementedInstruction {
	};

	class CmpLessOrEqual : public UnimplementedInstruction {
	};

	class CmpLess : public UnimplementedInstruction {
	};

	class CmpGreaterOrEqual : public UnimplementedInstruction {
	};

	class CmpGreater : public UnimplementedInstruction {
	};

	class BuiltinFunc : public UnimplementedInstruction {
	public:
		enum BuiltinFunctionID {
			kSin = 1,
			kCos = 2,
			kRandom = 3,
			kSqrt = 4,
			kTan = 5,
			kAbs = 6,
			kSign = 7,
			kArctangent = 8,
			kExp = 9,
			kLn = 10,
			kLog = 11,
			kCosH = 12,
			kSinH = 13,
			kTanH = 14,
			kRect2Polar = 15,
			kPolar2Rect = 16,
			kTrunc = 17,
			kRound = 18,
			kNum2Str = 19,
			kStr2Num = 20,
		};

		explicit BuiltinFunc(BuiltinFunctionID bfid);

	private:
		BuiltinFunctionID _funcID;
	};

	class DivInt : public UnimplementedInstruction {
	};

	class Modulo : public UnimplementedInstruction {
	};

	class StrConcat : public UnimplementedInstruction {
	};

	class PointCreate : public UnimplementedInstruction {
	};

	class RangeCreate : public UnimplementedInstruction {
	};

	class VectorCreate : public UnimplementedInstruction {
	};

	class GetChild : public UnimplementedInstruction {
	public:
		GetChild(uint32 attribute, bool isLValue, bool isIndexed);

	private:
		uint32 _attribute;
		bool _isLValue;
		bool _isIndexed;
	};

	class ListAppend : public UnimplementedInstruction {
	};

	class ListCreate : public UnimplementedInstruction {
	};

	class PushValue : public UnimplementedInstruction {
	public:
		enum DataType {
			kDataTypeNull,
			kDataTypeDouble,
			kDataTypeBool,
			kDataTypeLocalRef,
			kDataTypeGlobalRef,
			kDataTypeLabel,
		};

		struct Label {
			uint32 superGroup;
			uint32 id;
		};

		PushValue(DataType dataType, const void *value, bool isLValue);

	private:
		union ValueUnion {
			bool b;
			double f;
			uint32 ref;
			Label lbl;
		};

		DataType _dataType;
		ValueUnion _value;
		bool _isLValue;
	};

	class PushGlobal : public UnimplementedInstruction {
	public:
		explicit PushGlobal(uint32 guid, bool isLValue);

		uint32 getStaticGUID() const;
		void setReferenceSetIndex(size_t refSetIndex);
		size_t getReferenceSetIndex() const;

	private:
		uint32 _guid;
		size_t _refSetIndex;
		bool _isLValue;
	};

	class PushString : public UnimplementedInstruction {
	public:
		explicit PushString(const Common::String &str);

	private:
		Common::String _str;
	};

	class Jump : public UnimplementedInstruction {
	public:
		Jump(uint32 instrOffset, bool isConditional);

	private:
		uint32 _instrOffset;
		bool _isConditional;
	};
} // End of namespace MiniscriptInstructions

} // End of namespace MTropolis

#endif
