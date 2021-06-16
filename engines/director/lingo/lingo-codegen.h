/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

	ScriptContext *compileAnonymous(const char *code);
	ScriptContext *compileLingo(const char *code, LingoArchive *archive, ScriptType type, uint16 id, const Common::String &scriptName, bool anonyomous = false);
	ScriptContext *compileLingoV4(Common::SeekableReadStreamEndian &stream, LingoArchive *archive, const Common::String &archName, uint16 version);

	int code1(inst code) { _currentAssembly->push_back(code); return _currentAssembly->size() - 1; }
	int code2(inst code_1, inst code_2) { int o = code1(code_1); code1(code_2); return o; }
	int code3(inst code_1, inst code_2, inst code_3) { int o = code1(code_1); code1(code_2); code1(code_3); return o; }
	int code4(inst code_1, inst code_2, inst code_3, inst code_4) { int o = code1(code_1); code1(code_2); code1(code_3); code1(code_4); return o; }
	int codeCmd(Common::String *s, int numpar);
	int codeFloat(double f);
	int codeFunc(Common::String *s, int numpar);
	int codeInt(int val);
	int codeString(const char *s);
	void registerFactory(Common::String &s);
	void registerMethodVar(const Common::String &name, VarType type);

	LingoArchive *_assemblyArchive;
	ScriptContext *_assemblyContext;
	Node *_assemblyAST;
	ScriptData *_currentAssembly;
	bool _indef;
	uint _linenumber;
	uint _colnumber;
	uint _bytenumber;
	const char *_lines[3];
	bool _inFactory;

	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVars;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVarsStash;

	bool _hadError;

public:
	void compile(Node *node);
	void compileList(NodeList *nodes);

	virtual void visitScriptNode(ScriptNode *node);
	virtual void visitFactoryNode(FactoryNode *node);
	virtual void visitHandlerNode(HandlerNode *node);
	virtual void visitCmdNode(CmdNode *node);
	virtual void visitGlobalNode(GlobalNode *node);
	virtual void visitPropertyNode(PropertyNode *node);
	virtual void visitInstanceNode(InstanceNode *node);
	virtual void visitIfStmtNode(IfStmtNode *node);
	virtual void visitIfElseStmtNode(IfElseStmtNode *node);
	virtual void visitIntNode(IntNode *node);
	virtual void visitFloatNode(FloatNode *node);
	virtual void visitSymbolNode(SymbolNode *node);
	virtual void visitStringNode(StringNode *node);
	virtual void visitFuncNode(FuncNode *node);
	virtual void visitVarNode(VarNode *node);
	virtual void visitParensNode(ParensNode *node);
	virtual void visitUnaryOpNode(UnaryOpNode *node);
	virtual void visitBinaryOpNode(BinaryOpNode *node);

private:
	int parse(const char *code);
	void parseMenu(const char *code);

public:
	// lingo-preprocessor.cpp
	Common::String codePreprocessor(const char *s, LingoArchive *archive, ScriptType type, uint16 id, bool simple = false);

	// lingo-patcher.cpp
	Common::String patchLingoCode(Common::String &line, LingoArchive *archive, ScriptType type, uint16 id, int linenumber);
};

} // End of namespace Director

#endif
