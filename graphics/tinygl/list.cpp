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
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
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

GLContext *gl_get_context() {
	return gl_ctx;
}

static GLList *find_list(GLContext *c, uint list) {
	return c->shared_state.lists[list];
}

static void delete_list(GLContext *c, int list) {
	GLParamBuffer *pb, *pb1;
	GLList *l;

	l = find_list(c, list);
	assert(l);

	// free param buffer
	pb = l->first_op_buffer;
	while (pb) {
		pb1 = pb->next;
		gl_free(pb);
		pb = pb1;
	}

	gl_free(l);
	c->shared_state.lists[list] = NULL;
}

static GLList *alloc_list(GLContext *c, int list) {
	GLList *l;
	GLParamBuffer *ob;

	l = (GLList *)gl_zalloc(sizeof(GLList));
	ob = (GLParamBuffer *)gl_zalloc(sizeof(GLParamBuffer));

	ob->next = nullptr;
	l->first_op_buffer = ob;

	ob->ops[0].op = OP_EndList;

	c->shared_state.lists[list] = l;
	return l;
}

static void gl_print_op(GLParam *p) {
	int op;
	const char *s;
	Common::StreamDebug debug = streamDbg();

	op = p[0].op;
	p++;
	s = op_table_str[op];
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
	int op, op_size;
	GLParamBuffer *ob, *ob1;
	int index;

	op = p[0].op;
	op_size = op_table_size[op];
	index = current_op_buffer_index;
	ob = current_op_buffer;

	// we should be able to add a NextBuffer opcode
	if ((index + op_size) > (OP_BUFFER_MAX_SIZE - 2)) {

		ob1 = (GLParamBuffer *)gl_zalloc(sizeof(GLParamBuffer));
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
	int op;

	op = p[0].op;
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
	GLList *l;
	int list, op;

	list = p[1].ui;
	l = find_list(this, list);
	if (!l)
		error("list %d not defined", list);
	p = l->first_op_buffer->ops;

	while (1) {
		op = p[0].op;
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

void tglNewList(TGLuint list, TGLenum mode) {
	GLList *l;
	GLContext *c = gl_get_context();

	assert(mode == TGL_COMPILE || mode == TGL_COMPILE_AND_EXECUTE);
	assert(c->compile_flag == 0);

	l = find_list(c, list);
	if (l)
		delete_list(c, list);
	l = alloc_list(c, list);

	c->current_op_buffer = l->first_op_buffer;
	c->current_op_buffer_index = 0;

	c->compile_flag = 1;
	c->exec_flag = (mode == TGL_COMPILE_AND_EXECUTE);
}

void tglEndList() {
	GLContext *c = gl_get_context();
	GLParam p[1];

	assert(c->compile_flag == 1);

	// end of list
	p[0].op = OP_EndList;
	c->gl_compile_op(p);

	c->compile_flag = 0;
	c->exec_flag = 1;
}

TGLboolean tglIsList(TGLuint list) {
	GLContext *c = gl_get_context();
	GLList *l = find_list(c, list);

	return (l != nullptr);
}

TGLuint tglGenLists(TGLsizei range) {
	GLContext *c = gl_get_context();
	int count, list;
	GLList **lists;

	lists = c->shared_state.lists;
	count = 0;
	for (int i = 0; i < MAX_DISPLAY_LISTS; i++) {
		if (!lists[i]) {
			count++;
			if (count == range) {
				list = i - range + 1;
				for (int j = 0; j < range; j++) {
					alloc_list(c, list + j);
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
