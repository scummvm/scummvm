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

#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/utils.h"

namespace Watchmaker {

int32 t3dFloatToInt(t3dF32 nfloat) {
	int32  sint = nfloat;
	//  warning("STUBBED: t3dFloatToInt\n");
	/*
	    __asm
	    {
	        fld     nfloat
	        fistp   sint
	    }*/
	return sint;
}

void t3dMatIdentity(t3dM3X3F *d) {
	d->M[1] = d->M[2] = d->M[3] = d->M[5] = d->M[6] = d->M[7] = 0.0;
	d->M[0] = d->M[4] = d->M[8] = 1.0;

	d->Flags |= T3D_MATRIX_IDENTITY;
}

void t3dMatMul(t3dM3X3F *Dest, t3dM3X3F *a, t3dM3X3F *b) {
	t3dM3X3F    Tmp, *d = &Tmp;
	/*  t3dM3X3F Tmp,*d=&Tmp;
	    int r,c;

	    if (Dest == a || Dest == b)
	        d=&Tmp;
	    else
	        d=Dest;

	    for (r=0; r<3; r++)
	        for (c=0; c<3; c++)
	            d->M[r][c]=a->M[r][0]*b->M[0][c]+a->M[r][1]*b->M[1][c]+a->M[r][2]*b->M[2][c];*/

	d->M[0] = (a->M[0] * b->M[0] + a->M[1] * b->M[3] + a->M[2] * b->M[6]);
	d->M[1] = (a->M[0] * b->M[1] + a->M[1] * b->M[4] + a->M[2] * b->M[7]);
	d->M[2] = (a->M[0] * b->M[2] + a->M[1] * b->M[5] + a->M[2] * b->M[8]);

	d->M[3] = (a->M[3] * b->M[0] + a->M[4] * b->M[3] + a->M[5] * b->M[6]);
	d->M[4] = (a->M[3] * b->M[1] + a->M[4] * b->M[4] + a->M[5] * b->M[7]);
	d->M[5] = (a->M[3] * b->M[2] + a->M[4] * b->M[5] + a->M[5] * b->M[8]);

	d->M[6] = (a->M[6] * b->M[0] + a->M[7] * b->M[3] + a->M[8] * b->M[6]);
	d->M[7] = (a->M[6] * b->M[1] + a->M[7] * b->M[4] + a->M[8] * b->M[7]);
	d->M[8] = (a->M[6] * b->M[2] + a->M[7] * b->M[5] + a->M[8] * b->M[8]);

	if (d != Dest)
		t3dMatCopy(Dest, &Tmp);

}

void t3dMatCopy(t3dM3X3F *d, t3dM3X3F *s) {
	d->M[0] = s->M[0];
	d->M[1] = s->M[1];
	d->M[2] = s->M[2];
	d->M[3] = s->M[3];
	d->M[4] = s->M[4];
	d->M[5] = s->M[5];
	d->M[6] = s->M[6];
	d->M[7] = s->M[7];
	d->M[8] = s->M[8];
	d->Flags = 0;
}

void t3dMatView(t3dM3X3F *dest, t3dV3F *eye, t3dV3F *center) {
	t3dM3X3F *fm = dest;
	t3dM3X3F fmat;

//	D3DVECTOR from,at,up;

	if ((!eye) || (!dest)) return;

	// F
	t3dF32 dx = (t3dF32)(center->x - eye->x);
	t3dF32 dy = (t3dF32)(center->y - eye->y);
	t3dF32 dz = (t3dF32)(center->z - eye->z);

	t3dF32 dd = (dx * dx + dy * dy + dz * dz);
	if (dd == 0.0)dd = 1.0;
	dd = (float)sqrt((t3dF64)dd); // dd = ||F||

	t3dF32 d2 = (dz * dz + dx * dx);
	if (d2 == 0.0)d2 = 1.0;
	d2 = (float)sqrt((t3dF64)d2); // |s|

	t3dF32 fcosa = (dz / d2);
	t3dF32 fsina = (dx / d2);
	fm->M[0] = fcosa;
	fm->M[1] = 0;
	fm->M[2] = -fsina;
	fm->M[3] = 0;
	fm->M[4] = 1;
	fm->M[5] = 0;
	fm->M[6] = fsina;
	fm->M[7] = 0;
	fm->M[8] = fcosa;

	fcosa = (d2 / dd);
	fsina = (dy / dd);
	fmat.M[0] = 1;
	fmat.M[1] = 0;
	fmat.M[2] = 0;
	fmat.M[3] = 0;
	fmat.M[4] = fcosa;
	fmat.M[5] = -fsina;
	fmat.M[6] = 0;
	fmat.M[7] = fsina;
	fmat.M[8] = fcosa;
	t3dMatMul(dest, &fmat, dest);

	const float roll = 0.0f;
	fcosa = (t3dF32)cos((-roll) / 180.0f * T3D_PI);
	fsina = (t3dF32)sin((-roll) / 180.0f * T3D_PI);
	fmat.M[0] = fcosa;
	fmat.M[1] = fsina;
	fmat.M[2] = 0;
	fmat.M[3] = -fsina;
	fmat.M[4] = fcosa;
	fmat.M[5] = 0;
	fmat.M[6] = 0;
	fmat.M[7] = 0;
	fmat.M[8] = 1;
	t3dMatMul(dest, &fmat, dest);

	dest->Flags &= ~T3D_MATRIX_IDENTITY;
}

void t3dVectTransform(t3dV3F *d, t3dV3F *s, t3dM3X3F *mat) {
	t3dV3F Tmp;
	Tmp.x = (s->x * mat->M[0]) + (s->y * mat->M[1]) + (s->z * mat->M[2]);
	Tmp.y = (s->x * mat->M[3]) + (s->y * mat->M[4]) + (s->z * mat->M[5]);
	Tmp.z = (s->x * mat->M[6]) + (s->y * mat->M[7]) + (s->z * mat->M[8]);
	d->x = Tmp.x;
	d->y = Tmp.y;
	d->z = Tmp.z;
}

void t3dVectTransformInv(t3dV3F *d, t3dV3F *s, t3dM3X3F *mat) {
	t3dV3F Tmp;
	Tmp.x = (s->x * mat->M[0]) + (s->y * mat->M[3]) + (s->z * mat->M[6]);
	Tmp.y = (s->x * mat->M[1]) + (s->y * mat->M[4]) + (s->z * mat->M[7]);
	Tmp.z = (s->x * mat->M[2]) + (s->y * mat->M[5]) + (s->z * mat->M[8]);
	d->x = Tmp.x;
	d->y = Tmp.y;
	d->z = Tmp.z;
}

void t3dVectCross(t3dV3F *d, t3dV3F *v2, t3dV3F *v3) {
	t3dV3F Tmp;
	Tmp.x  = (v2->y * v3->z) - (v2->z * v3->y);
	Tmp.y  = (v2->z * v3->x) - (v2->x * v3->z);
	Tmp.z  = (v2->x * v3->y) - (v2->y * v3->x);
	d->x = Tmp.x;
	d->y = Tmp.y;
	d->z = Tmp.z;
}

void t3dVectSub(t3dV3F *d, t3dV3F *a, t3dV3F *b) {          // d = a - b
	d->x = a->x - b->x;
	d->y = a->y - b->y;
	d->z = a->z - b->z;
}

void t3dVectAdd(t3dV3F *d, t3dV3F *a, t3dV3F *b) {          // d = a - b
	d->x = a->x + b->x;
	d->y = a->y + b->y;
	d->z = a->z + b->z;
}

void t3dVectFill(t3dV3F *d, t3dF32 a) {
	d->x = a;
	d->y = a;
	d->z = a;
}

void t3dVectInit(t3dV3F *d, t3dF32 a1, t3dF32 a2, t3dF32 a3) {
	d->x = a1;
	d->y = a2;
	d->z = a3;
}

void t3dVectCopy(t3dV3F *d, t3dV3F *s) {
	memcpy(d, s, sizeof(t3dV3F));
}


t3dF32 t3dVectMod(t3dV3F *c) {
	t3dF32 mod = (c->x * c->x + c->y * c->y + c->z * c->z);
	if (mod)
		return (float)sqrt((double)mod);
	else
		return 0.0f;
}

t3dF32 t3dVectDistance(t3dV3F *a, t3dV3F *b) {
	t3dF32 mod;
	t3dV3F c;
	c.x = b->x - a->x;
	c.y = b->y - a->y;
	c.z = b->z - a->z;
	mod = (c.x * c.x + c.y * c.y + c.z * c.z);

	if (mod)
		return ((float)sqrt((double)mod));
	else
		return 0.0f;
}

t3dF32 t3dVectDot(t3dV3F *a, t3dV3F *b) {
	return (t3dF32)(a->x * b->x + a->y * b->y + a->z * b->z);
}

void t3dMatMulInv(t3dM3X3F *Dest, t3dM3X3F *a, t3dM3X3F *b) {
	t3dM3X3F    Tmp, *d = &Tmp;

	d->M[0] = (a->M[0] * b->M[0] + a->M[1] * b->M[1] + a->M[2] * b->M[2]);
	d->M[1] = (a->M[0] * b->M[3] + a->M[1] * b->M[4] + a->M[2] * b->M[5]);
	d->M[2] = (a->M[0] * b->M[6] + a->M[1] * b->M[7] + a->M[2] * b->M[8]);

	d->M[3] = (a->M[3] * b->M[0] + a->M[4] * b->M[1] + a->M[5] * b->M[2]);
	d->M[4] = (a->M[3] * b->M[3] + a->M[4] * b->M[4] + a->M[5] * b->M[5]);
	d->M[5] = (a->M[3] * b->M[6] + a->M[4] * b->M[7] + a->M[5] * b->M[8]);

	d->M[6] = (a->M[6] * b->M[0] + a->M[7] * b->M[1] + a->M[8] * b->M[2]);
	d->M[7] = (a->M[6] * b->M[3] + a->M[7] * b->M[4] + a->M[8] * b->M[5]);
	d->M[8] = (a->M[6] * b->M[6] + a->M[7] * b->M[7] + a->M[8] * b->M[8]);

	if (d != Dest)
		t3dMatCopy(Dest, &Tmp);
}

void t3dPlaneNormal(t3dNORMAL *n, t3dV3F *p0, t3dV3F *p1, t3dV3F *p2) {
	t3dV3F a, b;

	t3dVectSub(&a, p1, p0);
	t3dVectSub(&b, p2, p0);
	t3dVectCross(&n->n, &a, &b);
	t3dVectNormalize(&n->n);

	n->dist = t3dVectDot(&n->n, p0);
}

void t3dVectNormalize(t3dV3F *c) {
	t3dF32 mod = (float)sqrt((double)(c->x * c->x + c->y * c->y + c->z * c->z));
	if (!mod)return;
	mod = 1.0f / mod;
	c->x = c->x * mod;
	c->y = c->y * mod;
	c->z = c->z * mod;
}

void t3dMatRot(t3dM3X3F *matrix, t3dF32 x, t3dF32 y, t3dF32 z) {
	/*  t3dF32 cx, cy, cz, sx, sy, sz,sxsy,szsy,szcy,czcy;

	    sx = sin(x);cx = cos(x);                                   // qui ci vorrebbe una FSINCOS in asm
	    sy = sin(y);cy = cos(y);
	    sz = sin(z);cz = cos(z);
	    sxsy=sx*sy;szsy=sz*sy;szcy=sz*cy;czcy=cz*cy;
	    matrix->M[0] =  czcy + sx*szsy ;
	    matrix->M[1] =  -szcy + sxsy*cz ;
	    matrix->M[2] = cx*sy;
	    matrix->M[3] = cx*sz;
	    matrix->M[4] = cx*cz;
	    matrix->M[5] = -sx;
	    matrix->M[6] = -cz*sy + sx*szcy;
	    matrix->M[7] = szsy + sx*czcy;
	    matrix->M[8] = cx*cy ;*/


	t3dM3X3F    matrix_x, matrix_y, matrix_z;

	t3dMatIdentity(&matrix_x);
	t3dMatIdentity(&matrix_y);
	t3dMatIdentity(&matrix_z);

	/*  |  1      0      0     0 |            x' = x
	    |  0    cos �  -sin �  0 |            y' = (cos �) * y - (sin �) * z
	    |  0    sin �   cos �  0 |            z' = (sin �) * y + (cos �) * z
	    |  0      0      0     1 |*/

	matrix_x.M[4] = (float)cos(x);
	matrix_x.M[5] = -(float)sin(x);
	matrix_x.M[7] = (float)sin(x);
	matrix_x.M[8] = (float)cos(x);

	/*  |  cos �   0    sin �   0 |            x' = (cos �) * x + (sin �) * z
	    |    0     1     0      0 |            y' = y
	    |  -sin �  0    cos �   0 |            z' = -(sin �) * x + (cos �) * z
	    |    0      0    0      1 |*/

	matrix_y.M[0] = (float)cos(y);
	matrix_y.M[2] = (float)sin(y);
	matrix_y.M[6] = -(float)sin(y);
	matrix_y.M[8] = (float)cos(y);

	/*  |  cos �   -sin �   0    0 |            x' = (cos �) * x - (sin �) * y
	    |  sin �    cos �   0    0 |            y' = (sin �) * x + (cos �) * y
	    |    0       0      1    0 |            z' = z
	    |    0       0      0    1 |*/

	matrix_z.M[0] = (float)cos(z);
	matrix_z.M[1] = -(float)sin(z);
	matrix_z.M[3] = (float)sin(z);
	matrix_z.M[4] = (float)cos(z);

	t3dMatMul(matrix, &matrix_x, &matrix_y);
	t3dMatMul(matrix, matrix, &matrix_z);
}

t3dF32 t3dVectPlaneDistance(t3dV3F start, t3dNORMAL n) {
	return t3dVectDot(&start, &n.n) - n.dist;
}

uint8 t3dVectPlaneIntersection(t3dV3F *inter, t3dV3F start, t3dV3F end, t3dNORMAL n) {
	t3dF32 divi;
	t3dF32 d1 = t3dVectPlaneDistance(start, n);
	t3dF32 d2 = t3dVectPlaneDistance(end, n);

	if ((d1 < 0) && (d2 < 0))
		return 0;
	else if ((d1 >= 0) && (d2 >= 0))
		return 1;


	if ((d1 < 0) && (d2 >= 0)) {
		d2 = d2 - d1;
		divi = -d1 / d2;
		inter->x = start.x + divi * (end.x - start.x);
		inter->y = start.y + divi * (end.y - start.y);
		inter->z = start.z + divi * (end.z - start.z);
		return 2;
	} else {
		d1 = d1 - d2;
		divi = -d2 / d1;
		inter->x = end.x + divi * (start.x - end.x);
		inter->y = end.y + divi * (start.y - end.y);
		inter->z = end.z + divi * (start.z - end.z);
		return 3;
	}
}

uint8 t3dVectTriangleIntersection(t3dV3F *inter, t3dV3F start, t3dV3F end,
								  t3dV3F v1, t3dV3F v2, t3dV3F v3, t3dNORMAL n) {
	t3dV3F appo;
	t3dNORMAL normal;

	if (t3dVectPlaneIntersection(inter, start, end, n) <= 1)
		return 0;

	t3dVectAdd(&appo, &n.n, &v1);
	t3dPlaneNormal(&normal, &appo, &v1, &v2);

	if (t3dVectPlaneDistance(*inter, normal) >= 0.0f) {
		t3dVectAdd(&appo, &n.n, &v2);
		t3dPlaneNormal(&normal, &appo, &v2, &v3);
		if (t3dVectPlaneDistance(*inter, normal) >= 0.0f) {
			t3dVectAdd(&appo, &n.n, &v3);
			t3dPlaneNormal(&normal, &appo, &v3, &v1);
			if (t3dVectPlaneDistance(*inter, normal) >= 0.0f)
				return 1;
		}
	}

	return 0;
}

t3dF32 t3dVectSquaredDistance(t3dV3F *a, t3dV3F *b) {
	t3dV3F c;
	c.x = b->x - a->x;
	c.y = b->y - a->y;
	c.z = b->z - a->z;

	return (c.x * c.x + c.y * c.y + c.z * c.z);
}

t3dF32 t3dPointSquaredDistance(t3dV3F *c) {
	return (c->x * c->x + c->y * c->y + c->z * c->z);
}

void t3dMatReflect(t3dM3X3F *Matrix, t3dV3F *mirrorpos, t3dNORMAL *n) {
	//                  | 1-2*nx*nx    -2*nx*ny     -2*nx*nz     -2*nx*k |
	//                  |  -2*ny*nx   1-2*ny*ny     -2*ny*nz     -2*ny*k |
	//                  |  -2*nz*nx    -2*nz*ny    1-2*nz*nz     -2*nz*k |
	//                  |      0           0            0            1   |

	Matrix->M[0] = 1 - 2 * n->n.x * n->n.x;
	Matrix->M[1] = -2 * n->n.x * n->n.y;
	Matrix->M[2] = -2 * n->n.x * n->n.z;
	Matrix->M[3] = -2 * n->n.y * n->n.x;
	Matrix->M[4] = 1 - 2 * n->n.y * n->n.y;
	Matrix->M[5] = -2 * n->n.y * n->n.z;
	Matrix->M[6] = -2 * n->n.z * n->n.x;
	Matrix->M[7] = -2 * n->n.z * n->n.y;
	Matrix->M[8] = 1 - 2 * n->n.z * n->n.z;

	mirrorpos->x = -2 * n->n.x * n->dist;
	mirrorpos->y = -2 * n->n.y * n->dist;
	mirrorpos->z = -2 * n->n.z * n->dist;

}

} // End of namespace Watchmaker
