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

class MiniscriptThread;
struct MiniscriptStackValue;
struct SIMiniscriptInstructionFactory;

bool miniscriptEvaluateTruth(const DynamicValue &value);

class MiniscriptInstruction {
public:
	virtual ~MiniscriptInstruction();

	virtual MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const = 0;
};

class IMiniscriptInstructionParserFeedback {
public:
	virtual ~IMiniscriptInstructionParserFeedback();

	virtual uint registerGlobalGUIDIndex(uint32 guid) = 0;
};

class MiniscriptReferences {
public:
	struct LocalRef {
		LocalRef();

		uint32 guid;
		Common::String name;
		Common::WeakPtr<RuntimeObject> resolution;
	};

	struct GlobalRef {
		GlobalRef();

		uint32 guid;
		Common::WeakPtr<RuntimeObject> resolution;
	};

	explicit MiniscriptReferences(const Common::Array<LocalRef> &localRefs, const Common::Array<GlobalRef> &globalRefs);

	void linkInternalReferences(ObjectLinkingScope *scope);
	void visitInternalReferences(IStructuralReferenceVisitor *visitor);

	Common::WeakPtr<RuntimeObject> getRefByIndex(uint index) const;
	Common::WeakPtr<RuntimeObject> getGlobalRefByIndex(uint index) const;

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

	const Common::Array<MiniscriptInstruction *> &getInstructions() const;
	const Common::Array<Attribute> &getAttributes() const;

private:
	Common::SharedPtr<Common::Array<uint8> > _programData;
	Common::Array<MiniscriptInstruction *> _instructions;
	Common::Array<Attribute> _attributes;
};

class MiniscriptParser {
public:
	static bool parse(const Data::MiniscriptProgram &programData, Common::SharedPtr<MiniscriptProgram> &outProgram, Common::SharedPtr<MiniscriptReferences> &outReferences);

	static SIMiniscriptInstructionFactory *resolveOpcode(uint16 opcode);

private:
	struct InstructionData {
		InstructionData();

		uint16 opcode;
		uint16 flags;
		size_t pdPosition;
		SIMiniscriptInstructionFactory *instrFactory;
		Common::Array<uint8> contents;
	};
};

namespace MiniscriptInstructions {
	class UnimplementedInstruction : public MiniscriptInstruction {
	private:
		virtual MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class Set : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class Send : public MiniscriptInstruction {
	public:
		explicit Send(const Event &evt, const MessageFlags &messageFlags);

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

		Event _evt;
		MessageFlags _messageFlags;
	};

	class BinaryArithInstruction : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

	protected:
		virtual MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const = 0;
	};

	class Add : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class Sub : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class Mul : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class Div : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class Pow : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class DivInt : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class Modulo : public BinaryArithInstruction {
	private:
		MiniscriptInstructionOutcome arithExecute(MiniscriptThread *thread, double &result, double left, double right) const override;
	};

	class And : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class Or : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class Neg : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class Not : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class OrderedCompareInstruction : public MiniscriptInstruction{
	protected:
		virtual bool compareFloat(double a, double b) const = 0;

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class UnorderedCompareInstruction : public MiniscriptInstruction {
	protected:
		virtual bool resolve(bool isEqual) const = 0;

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class CmpEqual : public UnorderedCompareInstruction {
	private:
		bool resolve(bool isEqual) const override { return isEqual; };
	};

	class CmpNotEqual : public UnorderedCompareInstruction {
	private:
		bool resolve(bool isEqual) const override { return !isEqual; };
	};

	class CmpLessOrEqual : public OrderedCompareInstruction {
	private:
		bool compareFloat(double a, double b) const override { return a <= b; }
	};

	class CmpLess : public OrderedCompareInstruction {
	private:
		bool compareFloat(double a, double b) const override { return a < b; }
	};

	class CmpGreaterOrEqual : public OrderedCompareInstruction {
	private:
		bool compareFloat(double a, double b) const override { return a >= b; }
	};

	class CmpGreater : public OrderedCompareInstruction {
	private:
		bool compareFloat(double a, double b) const override { return a > b; }
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
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

		MiniscriptInstructionOutcome executeFunction(MiniscriptThread *thread, DynamicValue *returnValue) const;
		MiniscriptInstructionOutcome executeSimpleNumericInstruction(MiniscriptThread *thread, DynamicValue *returnValue) const;
		MiniscriptInstructionOutcome executeRectToPolar(MiniscriptThread *thread, DynamicValue *returnValue) const;
		MiniscriptInstructionOutcome executePolarToRect(MiniscriptThread *thread, DynamicValue *returnValue) const;
		MiniscriptInstructionOutcome executeNum2Str(MiniscriptThread *thread, DynamicValue *returnValue) const;
		MiniscriptInstructionOutcome executeStr2Num(MiniscriptThread *thread, DynamicValue *returnValue) const;

		BuiltinFunctionID _funcID;
	};

	class StrConcat : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class PointCreate : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class RangeCreate : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class VectorCreate : public UnimplementedInstruction {
	};

	class GetChild : public MiniscriptInstruction {
	public:
		GetChild(uint32 attribute, bool isLValue, bool isIndexed);

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
		MiniscriptInstructionOutcome readRValueAttrib(MiniscriptThread *thread, DynamicValue &valueSrcDest, const Common::String &attrib) const;
		MiniscriptInstructionOutcome readRValueAttribIndexed(MiniscriptThread *thread, DynamicValue &valueSrcDest, const Common::String &attrib, const DynamicValue &index) const;

		uint32 _attribute;
		bool _isLValue;
		bool _isIndexed;
	};

	class ListAppend : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class ListCreate : public MiniscriptInstruction {
	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;
	};

	class PushValue : public MiniscriptInstruction {
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
		~PushValue();

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

		union ValueUnion {
			ValueUnion();

			bool b;
			double f;
			uint32 ref;
			Label lbl;
		};

		DataType _dataType;
		ValueUnion _value;
		//bool _isLValue;
	};

	class PushGlobal : public MiniscriptInstruction {
	public:
		explicit PushGlobal(uint32 globalID, bool isLValue);

		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

	private:
		enum {
			kGlobalRefElement = 1,
			kGlobalRefModifier = 2,
			kGlobalRefSource = 3,
			kGlobalRefIncomingData = 4,
			kGlobalRefMouse = 5,
			kGlobalRefTicks = 6,
			kGlobalRefScene = 7,
			kGlobalRefSharedScene = 8,
			kGlobalRefSection = 9,
			kGlobalRefProject = 10,
			kGlobalRefActiveScene = 11,
		};

		MiniscriptInstructionOutcome executeFindFilteredParent(MiniscriptThread *thread, DynamicValue &result) const;

		uint32 _globalID;
		bool _isLValue;
	};

	class PushString : public UnimplementedInstruction {
	public:
		explicit PushString(const Common::String &str);

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

		Common::String _str;
	};

	class Jump : public MiniscriptInstruction {
	public:
		Jump(uint32 instrOffset, bool isConditional);

	private:
		MiniscriptInstructionOutcome execute(MiniscriptThread *thread) const override;

		uint32 _instrOffset;
		bool _isConditional;
	};
} // End of namespace MiniscriptInstructions


struct MiniscriptStackValue {
	DynamicValue value;
};

class MiniscriptThread {
public:
	MiniscriptThread(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msgProps, const Common::SharedPtr<MiniscriptProgram> &program, const Common::SharedPtr<MiniscriptReferences> &refs, Modifier *modifier);

	void error(const Common::String &message);

	const Common::SharedPtr<MiniscriptProgram> &getProgram() const;
	const Common::SharedPtr<MiniscriptReferences> &getRefs() const;
	Modifier *getModifier() const;
	const Common::SharedPtr<MessageProperties> &getMessageProperties() const;
	Runtime *getRuntime() const;

	void pushValue(const DynamicValue &value);
	void popValues(size_t count);
	size_t getStackSize() const;
	MiniscriptStackValue &getStackValueFromTop(size_t offset);
	MiniscriptInstructionOutcome dereferenceRValue(size_t offset);

	void jumpOffset(size_t offset);

	bool evaluateTruthOfResult(bool &isTrue);

	void createWriteIncomingDataProxy(DynamicValueWriteProxy &proxy);

	void retryInstruction();

	struct ResumeThreadCoroutine {
		CORO_DEFINE_RETURN_TYPE(void);
		CORO_DEFINE_PARAMS_1(Common::SharedPtr<MiniscriptThread>, thread);
	};

private:
	struct IncomingDataWriteInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};

	MiniscriptInstructionOutcome runNextInstruction();

	VThreadState resume(MiniscriptThread *thread);

	MiniscriptInstructionOutcome tryLoadVariable(MiniscriptStackValue &stackValue);

	Common::SharedPtr<MiniscriptProgram> _program;
	Common::SharedPtr<MiniscriptReferences> _refs;
	Common::SharedPtr<MessageProperties> _msgProps;

	Modifier *_modifier;
	Runtime *_runtime;
	Common::Array<MiniscriptStackValue> _stack;

	size_t _currentInstruction;
	bool _failed;
};

MiniscriptInstructionOutcome miniscriptIgnoreFailure(MiniscriptInstructionOutcome outcome);

} // End of namespace MTropolis

#endif
