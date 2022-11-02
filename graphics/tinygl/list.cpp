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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "common/streamdebug.h"

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

#define ADD_OP(aa, bb, ff) \
static void glop ## aa (GLContext *c, GLParam *p) { \
        c->glop ## aa (p);                          \
}
#include "graphics/tinygl/opinfo.h"

static const char *op_table_str[] = {
#define ADD_OP(a, b, c) "gl" #a " " #c,
#include "graphics/tinygl/opinfo.h"
};

static void (*op_table_func[])(GLContext *, GLParam *) = {
#define ADD_OP(a, b, c) glop ## a ,
#include "graphics/tinygl/opinfo.h"
};

static int op_table_size[] = {
#define ADD_OP(a, b, c) b + 1 ,
#include "graphics/tinygl/opinfo.h"
};

GLList *GLContext::find_list(uint list) {
	return shared_state.lists[list];
}

void GLContext::delete_list(int list) {
	GLList *l = find_list(list);
	assert(l);

	// free param buffer
	GLParamBuffer *pb = l->first_op_buffer;
	while (pb) {
		GLParamBuffer *pb1 = pb->next;
		gl_free(pb);
		pb = pb1;
	}

	gl_free(l);
	shared_state.lists[list] = nullptr;
}

GLList *GLContext::alloc_list(int list) {
	GLList *l = (GLList *)gl_zalloc(sizeof(GLList));
	GLParamBuffer *ob = (GLParamBuffer *)gl_zalloc(sizeof(GLParamBuffer));

	ob->next = nullptr;
	l->first_op_buffer = ob;

	ob->ops[0].op = OP_EndList;

	shared_state.lists[list] = l;
	return l;
}

static void gl_print_op(GLParam *p) {
	Common::StreamDebug debug = streamDbg();
	int op = p[0].op;

	p++;
	const char *s = op_table_str[op];
	while (*s != 0) {
		if (*s == '%') {
			s++;
			switch (*s++) {
			case 'f':
				debug << p[0].f;
				break;
			default:
				debug << p[0].i;
				break;
			}
			p++;
		} else {
			debug << *s;
			s++;
		}
	}
	debug << "\n";
}

void GLContext::gl_compile_op(GLParam *p) {
	int op = p[0].op;
	int op_size = op_table_size[op];
	int index = current_op_buffer_index;
	GLParamBuffer *ob = current_op_buffer;

	// we should be able to add a NextBuffer opcode
	if ((index + op_size) > (OP_BUFFER_MAX_SIZE - 2)) {

		GLParamBuffer *ob1 = (GLParamBuffer *)gl_zalloc(sizeof(GLParamBuffer));
		ob1->next = nullptr;

		ob->next = ob1;
		ob->ops[index].op = OP_NextBuffer;
		ob->ops[index + 1].p = (void *)ob1;

		current_op_buffer = ob1;
		ob = ob1;
		index = 0;
	}

	for (int i = 0; i < op_size; i++) {
		ob->ops[index] = p[i];
		index++;
	}
	current_op_buffer_index = index;
}

void GLContext::gl_add_op(GLParam *p) {
	GLContext *c = gl_get_context();

	int op = p[0].op;
	if (exec_flag) {
		op_table_func[op](c, p);
	}
	if (compile_flag) {
		gl_compile_op(p);
	}
	if (print_flag) {
		gl_print_op(p);
	}
}

// this opcode is never called directly
void GLContext::glopEndList(GLParam *) {
	assert(0);
}

// this opcode is never called directly
void GLContext::glopNextBuffer(GLParam *) {
	assert(0);
}

void GLContext::glopCallList(GLParam *p) {
	uint list = p[1].ui;
	GLList *l = find_list(list);

	if (!l)
		error("list %d not defined", list);
	p = l->first_op_buffer->ops;

	while (1) {
		int op = p[0].op;
		if (op == OP_EndList)
			break;
		if (op == OP_NextBuffer) {
			p = (GLParam *)p[1].p;
		} else {
			op_table_func[op](this, p);
			p += op_table_size[op];
		}
	}
}

void GLContext::gl_NewList(TGLuint list, TGLenum mode) {
	assert(mode == TGL_COMPILE || mode == TGL_COMPILE_AND_EXECUTE);
	assert(compile_flag == 0);

	GLList *l = find_list(list);
	if (l)
		delete_list(list);
	l = alloc_list(list);

	current_op_buffer = l->first_op_buffer;
	current_op_buffer_index = 0;

	compile_flag = 1;
	exec_flag = (mode == TGL_COMPILE_AND_EXECUTE);
}

void GLContext::gl_EndList() {
	GLParam p[1];

	assert(compile_flag == 1);

	// end of list
	p[0].op = OP_EndList;
	gl_compile_op(p);

	compile_flag = 0;
	exec_flag = 1;
}

TGLboolean GLContext::gl_IsList(TGLuint list) {
	GLList *l = find_list(list);

	return (l != nullptr);
}

TGLuint GLContext::gl_GenLists(TGLsizei range) {
	GLList **lists = shared_state.lists;
	int count = 0;

	for (int i = 0; i < MAX_DISPLAY_LISTS; i++) {
		if (!lists[i]) {
			count++;
			if (count == range) {
				uint list = i - range + 1;
				for (int j = 0; j < range; j++) {
					alloc_list(list + j);
				}
				return list;
			}
		} else {
			count = 0;
		}
	}
	return 0;
}

} // end of namespace TinyGL
