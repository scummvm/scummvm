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

#ifndef DIRECTOR_LINGO_LINGO_CODEGEN_H
#define DIRECTOR_LINGO_LINGO_CODEGEN_H

#include "director/types.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"

namespace Director {

class LingoCompiler : NodeVisitor {
public:
	LingoCompiler();
	virtual ~LingoCompiler() {}

	ScriptContext *compileAnonymous(const Common::U32String &code);
	ScriptContext *compileLingo(const Common::U32String &code, LingoArchive *archive, ScriptType type, CastMemberID id, const Common::String &scriptName, bool anonyomous = false, uint32 preprocFlags = kLPPNone);
	ScriptContext *compileLingoV4(Common::SeekableReadStreamEndian &stream, uint16 lctxIndex, LingoArchive *archive, const Common::String &archName, uint16 version);

	int code1(inst code) { _currentAssembly->push_back(code); return _currentAssembly->size() - 1; }
	int code2(inst code_1, inst code_2) { int o = code1(code_1); code1(code_2); return o; }
	int code3(inst code_1, inst code_2, inst code_3) { int o = code1(code_1); code1(code_2); code1(code_3); return o; }
	int code4(inst code_1, inst code_2, inst code_3, inst code_4) { int o = code1(code_1); code1(code_2); code1(code_3); code1(code_4); return o; }
	int codeCmd(const Common::String &s, int numpar);
	int codeFloat(double f);
	int codeFunc(const Common::String &s, int numpar);
	int codeInt(int val);
	int codeString(const char *s);
	void codeVarSet(const Common::String &name);
	void codeVarRef(const Common::String &name);
	void codeVarGet(const Common::String &name);
	int getTheFieldID(int entity, const Common::String &field, bool silent = false);
	void registerFactory(Common::String &s);
	void registerMethodVar(const Common::String &name, VarType type = kVarGeneric);
	void updateLoopJumps(uint nextTargetPos, uint exitTargetPos);

	LingoArchive *_assemblyArchive;
	ScriptContext *_assemblyContext;
	Node *_assemblyAST;
	int32 _assemblyId;
	ScriptData *_currentAssembly;
	bool _indef;
	uint _linenumber;
	uint _colnumber;
	uint _bytenumber;
	const char *_lines[3];
	bool _inFactory;
	LoopNode *_currentLoop;
	bool _refMode;

	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVars;

	bool _hadError;

public:
	virtual bool visitScriptNode(ScriptNode *node);
	virtual bool visitFactoryNode(FactoryNode *node);
	virtual bool visitHandlerNode(HandlerNode *node);
	virtual bool visitCmdNode(CmdNode *node);
	virtual bool visitPutIntoNode(PutIntoNode *node);
	virtual bool visitPutAfterNode(PutAfterNode *node);
	virtual bool visitPutBeforeNode(PutBeforeNode *node);
	virtual bool visitSetNode(SetNode *node);
	virtual bool visitGlobalNode(GlobalNode *node);
	virtual bool visitPropertyNode(PropertyNode *node);
	virtual bool visitInstanceNode(InstanceNode *node);
	virtual bool visitIfStmtNode(IfStmtNode *node);
	virtual bool visitIfElseStmtNode(IfElseStmtNode *node);
	virtual bool visitRepeatWhileNode(RepeatWhileNode *node);
	virtual bool visitRepeatWithToNode(RepeatWithToNode *node);
	virtual bool visitRepeatWithInNode(RepeatWithInNode *node);
	virtual bool visitNextRepeatNode(NextRepeatNode *node);
	virtual bool visitExitRepeatNode(ExitRepeatNode *node);
	virtual bool visitExitNode(ExitNode *node);
	virtual bool visitTellNode(TellNode *node);
	virtual bool visitWhenNode(WhenNode *node);
	virtual bool visitDeleteNode(DeleteNode *node);
	virtual bool visitHiliteNode(HiliteNode *node);
	virtual bool visitAssertErrorNode(AssertErrorNode *node);
	virtual bool visitIntNode(IntNode *node);
	virtual bool visitFloatNode(FloatNode *node);
	virtual bool visitSymbolNode(SymbolNode *node);
	virtual bool visitStringNode(StringNode *node);
	virtual bool visitListNode(ListNode *node);
	virtual bool visitPropListNode(PropListNode *node);
	virtual bool visitPropPairNode(PropPairNode *node);
	virtual bool visitFuncNode(FuncNode *node);
	virtual bool visitVarNode(VarNode *node);
	virtual bool visitParensNode(ParensNode *node);
	virtual bool visitUnaryOpNode(UnaryOpNode *node);
	virtual bool visitBinaryOpNode(BinaryOpNode *node);
	virtual bool visitFrameNode(FrameNode *node);
	virtual bool visitMovieNode(MovieNode *node);
	virtual bool visitIntersectsNode(IntersectsNode *node);
	virtual bool visitWithinNode(WithinNode *node);
	virtual bool visitTheNode(TheNode *node);
	virtual bool visitTheOfNode(TheOfNode *node);
	virtual bool visitTheNumberOfNode(TheNumberOfNode *node);
	virtual bool visitTheLastNode(TheLastNode *node);
	virtual bool visitTheDateTimeNode(TheDateTimeNode *node);
	virtual bool visitMenuNode(MenuNode *node);
	virtual bool visitMenuItemNode(MenuItemNode *node);
	virtual bool visitSoundNode(SoundNode *node);
	virtual bool visitSpriteNode(SpriteNode *node);
	virtual bool visitChunkExprNode(ChunkExprNode *node);

private:
	int parse(const char *code);

public:
	// lingo-preprocessor.cpp
	Common::U32String codePreprocessor(const Common::U32String &code, LingoArchive *archive, ScriptType type, CastMemberID id, uint32 flags);

	// lingo-patcher.cpp
	Common::U32String patchLingoCode(const Common::U32String &line, LingoArchive *archive, ScriptType type, CastMemberID id, int linenumber);
};

} // End of namespace Director

#endif
