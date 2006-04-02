/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef MATRIX_OPS_H
#define MATRIX_OPS_H

#include <ronin/matrix.h>

#define MAX_MATRIX_STACK_DEPTH 16

class MatrixOps {
 private:
	int _matrix_depth;
	float _matrix_stack[MAX_MATRIX_STACK_DEPTH][4][4];

 public:
	void clearMatrixStack() { _matrix_depth = 0; }
	void loadMatrix(float (*m)[4][4]) { load_matrix(m); }
	void applyMatrix(float (*m)[4][4]) { apply_matrix(m); }
	void pushMatrix();
	void popMatrix();
	void translate(float x, float y, float z);
	void rotateX(float deg);
	void rotateY(float deg);
	void rotateZ(float deg);
};

#endif
