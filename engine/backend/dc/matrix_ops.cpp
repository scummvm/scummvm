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

#include "matrix_ops.h"

#include <ronin/ronin.h>
#include <ronin/matrix.h>
#include <ronin/sincos_rroot.h>
#include <assert.h>

#define DEG(X)   ((int)(X*(65536.0/360.0)))

void MatrixOps::pushMatrix()
{
	assert(_matrix_depth < MAX_MATRIX_STACK_DEPTH);
	save_matrix(&_matrix_stack[_matrix_depth++]);
}

void MatrixOps::popMatrix()
{
	assert(_matrix_depth > 0);
	load_matrix(&_matrix_stack[--_matrix_depth]);
}

void MatrixOps::translate(float x, float y, float z)
{
	static float matrix[4][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	};
	matrix[3][0] = x;
	matrix[3][1] = y;
	matrix[3][2] = z;
	apply_matrix(&matrix);
}

void MatrixOps::rotateX(float deg)
{
	static float matrix[4][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	};
	float s, c;
	SINCOS(DEG(deg), s, c);
	matrix[1][1] = matrix[2][2] = c;
	matrix[1][2] = -(matrix[2][1] = s);
	apply_matrix(&matrix);
}

void MatrixOps::rotateY(float deg)
{
	static float matrix[4][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	};
	float s, c;
	SINCOS(DEG(deg), s, c);
	matrix[0][0] = matrix[2][2] = c;
	matrix[2][0] = -(matrix[0][2] = s);
	apply_matrix(&matrix);
}

void MatrixOps::rotateZ(float deg)
{
	static float matrix[4][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
	};
	float s, c;
	SINCOS(DEG(deg), s, c);
	matrix[0][0] = matrix[1][1] = c;
	matrix[0][1] = -(matrix[1][0] = s);
	apply_matrix(&matrix);
}

