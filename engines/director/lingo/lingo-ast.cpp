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

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-object.h"

namespace Director {

/* ScriptNode */

void ScriptNode::compile() {
	for (uint i = 0; i < nodes->size(); i++) {
		(*nodes)[i]->compile();
	}
}

/* FactoryNode */

void FactoryNode::compile() {
	g_lingo->_compiler->_inFactory = true;
	ScriptContext *mainContext = g_lingo->_compiler->_assemblyContext;
	g_lingo->_compiler->_assemblyContext = new ScriptContext(mainContext->getName(), mainContext->_archive, mainContext->_scriptType, mainContext->_id);

	g_lingo->_compiler->codeFactory(*name);
	for (uint i = 0; i < methods->size(); i++) {
		(*methods)[i]->compile();
	}

	g_lingo->_compiler->_inFactory = false;
	g_lingo->_compiler->_assemblyContext = mainContext;
}

/* HandlerNode */

void HandlerNode::compile() {
	g_lingo->_compiler->_indef = true;
	VarTypeHash *mainMethodVars = g_lingo->_compiler->_methodVars;
	g_lingo->_compiler->_methodVars = new VarTypeHash;

	for (VarTypeHash::iterator i = mainMethodVars->begin(); i != mainMethodVars->end(); ++i) {
		if (i->_value == kVarGlobal || i->_value == kVarProperty)
			(*g_lingo->_compiler->_methodVars)[i->_key] = i->_value;
	}
	if (g_lingo->_compiler->_inFactory) {
		for (DatumHash::iterator i = g_lingo->_compiler->_assemblyContext->_properties.begin(); i != g_lingo->_compiler->_assemblyContext->_properties.end(); ++i) {
			(*g_lingo->_compiler->_methodVars)[i->_key] = kVarInstance;
		}
	}

	for (uint i = 0; i < args->size(); i++) { // TODO: eliminate argstack
		g_lingo->_compiler->codeArg((*args)[i]);
	}

	uint start = g_lingo->_compiler->_currentAssembly->size(); // TODO: should always be zero
	for (uint i = 0; i < stmts->size(); i++) {
		(*stmts)[i]->compile();
	}

	g_lingo->_compiler->code1(LC::c_procret);
	g_lingo->_compiler->codeDefine(*name, start, args->size());

	g_lingo->_compiler->clearArgStack();
	g_lingo->_compiler->_indef = false;
	delete g_lingo->_compiler->_methodVars;
	g_lingo->_compiler->_methodVars = mainMethodVars;
}

/* CmdNode */

void CmdNode::compile() {
	for (uint i = 0; i < args->size(); i++) {
		(*args)[i]->compile();
	}
	g_lingo->_compiler->codeCmd(name, args->size());
}

/* GlobalNode */

void GlobalNode::compile() {
	for (uint i = 0; i < names->size(); i++) {
		g_lingo->_compiler->registerMethodVar(*(*names)[i], kVarGlobal);
	}
}

/* PropertyNode */

void PropertyNode::compile() {
	for (uint i = 0; i < names->size(); i++) {
		g_lingo->_compiler->registerMethodVar(*(*names)[i], kVarProperty);
	}
}

/* InstanceNode */

void InstanceNode::compile() {
	for (uint i = 0; i < names->size(); i++) {
		g_lingo->_compiler->registerMethodVar(*(*names)[i], kVarInstance);
	}
}

/* IntNode */

void IntNode::compile() {
	g_lingo->_compiler->code1(LC::c_intpush);
	g_lingo->_compiler->codeInt(val);
}

/* FloatNode */

void FloatNode::compile() {
	g_lingo->_compiler->code1(LC::c_floatpush);
	g_lingo->_compiler->codeFloat(val);
}

/* SymbolNode */

void SymbolNode::compile() {
	g_lingo->_compiler->code1(LC::c_symbolpush);
	g_lingo->_compiler->codeString(val->c_str());
}

/* StringNode */

void StringNode::compile() {
	g_lingo->_compiler->code1(LC::c_stringpush);
	g_lingo->_compiler->codeString(val->c_str());
}

/* FuncNode */

void FuncNode::compile() {
	for (uint i = 0; i < args->size(); i++) {
		(*args)[i]->compile();
	}
	g_lingo->_compiler->codeFunc(name, args->size());
}

/* VarNode */

void VarNode::compile() {
	if (g_lingo->_builtinConsts.contains(*name)) {
		g_lingo->_compiler->code1(LC::c_constpush);
	} else {
		g_lingo->_compiler->code1(LC::c_eval);
	}
	g_lingo->_compiler->codeString(name->c_str());
}

/* ParensNode */

void ParensNode::compile() {
	expr->compile();
}

/* UnaryOpNode */

void UnaryOpNode::compile() {
	arg->compile();
	g_lingo->_compiler->code1(op);
}

/* BinaryOpNode */

void BinaryOpNode::compile() {
	a->compile();
	b->compile();
	g_lingo->_compiler->code1(op);
}

} // End of namespace Director
