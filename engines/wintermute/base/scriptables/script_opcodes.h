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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SCOPCODES_H
#define WINTERMUTE_SCOPCODES_H

namespace Wintermute {

const uint32 foxtail_1_2_896_mapping[] = {
	II_CMP_LE,
	II_JMP,
	II_POP_REG1,
	II_PUSH_BOOL,
	II_MODULO,
	II_POP_EMPTY,
	II_CALL_BY_EXP,
	II_CMP_L,
	II_PUSH_FLOAT,
	II_NOT,
	II_PUSH_THIS,
	II_PUSH_BY_EXP,
	II_PUSH_THIS_FROM_STACK,
	II_CMP_G,
	II_DEF_GLOB_VAR,
	II_PUSH_STRING,
	II_PUSH_REG1,
	II_DEF_VAR,
	II_PUSH_VAR_THIS,
	II_RET_EVENT,
	II_PUSH_VAR_REF,
	II_CMP_NE,
	II_DBG_LINE,
	II_OR,
	II_POP_VAR,
	II_AND,
	II_EXTERNAL_CALL,
	II_CORRECT_STACK,
	II_RET,
	II_DIV,
	II_PUSH_VAR,
	II_SUB,
	II_CALL,
	II_CREATE_OBJECT,
	II_MUL,
	II_POP_BY_EXP,
	II_DEF_CONST_VAR,
	II_PUSH_NULL,
	II_JMP_FALSE,
	II_ADD,
	II_CMP_GE,
	II_CMP_STRICT_EQ,
	II_CMP_STRICT_NE,
	II_PUSH_INT,
	II_CMP_EQ,
	II_POP_THIS,
	II_SCOPE
};

const uint32 foxtail_1_2_902_mapping[] = {
	II_CMP_L,
	II_CALL,
	II_DEF_GLOB_VAR,
	II_DBG_LINE,
	II_JMP_FALSE,
	II_CMP_STRICT_EQ,
	II_PUSH_FLOAT,
	II_CALL_BY_EXP,
	II_MODULO,
	II_PUSH_THIS,
	II_CMP_GE,
	II_PUSH_BOOL,
	II_PUSH_VAR,
	II_PUSH_VAR_REF,
	II_POP_BY_EXP,
	II_CMP_STRICT_NE,
	II_RET_EVENT,
	II_PUSH_BY_EXP,
	II_CORRECT_STACK,
	II_POP_VAR,
	II_CMP_G,
	II_PUSH_THIS_FROM_STACK,
	II_JMP,
	II_AND,
	II_CREATE_OBJECT,
	II_POP_REG1,
	II_PUSH_STRING,
	II_POP_EMPTY,
	II_DIV,
	II_ADD,
	II_RET,
	II_EXTERNAL_CALL,
	II_NOT,
	II_OR,
	II_SUB,
	II_PUSH_INT,
	II_DEF_VAR,
	II_SCOPE,
	II_CMP_EQ,
	II_MUL,
	II_POP_THIS,
	II_CMP_LE,
	II_PUSH_REG1,
	II_DEF_CONST_VAR,
	II_PUSH_NULL,
	II_CMP_NE,
	II_PUSH_VAR_THIS
};

} // End of namespace Wintermute

#endif
