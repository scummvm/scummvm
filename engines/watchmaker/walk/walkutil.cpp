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

#include "watchmaker/walk/walkutil.h"
#include "watchmaker/types.h"
#include "watchmaker/t3d.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/globvar.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

/* -----------------05/06/00 12.49-------------------
 *                  PointIn2DRectangle
 * --------------------------------------------------*/
bool PointInside2DRectangle(double pgon[4][2], double x, double z) {
	bool inside_flag;

//	Crossing-Multiply algorithm
	{
		int j, yflag0, yflag1, xflag0;
		double  *vtx0, *vtx1 ;

		vtx0 = pgon[3];
		// get test bit for above/below X axis
		yflag0 = (vtx0[1] >= z);
		vtx1 = pgon[0];

		inside_flag = 0;
		for (j = 5; --j ;) {
			yflag1 = (vtx1[1] >= z);
			if (yflag0 != yflag1) {
				xflag0 = (vtx0[0] >= x);
				if ((xflag0 == (vtx1[0] >= x)) && (xflag0))
					inside_flag += (yflag0 ? -1 : 1);
				else if ((vtx1[0] - (vtx1[1] - z) * (vtx0[0] - vtx1[0]) / (vtx0[1] - vtx1[1])) >= x)
					inside_flag += (yflag0 ? -1 : 1);
			}
			// Move to the next pair of vertices, retaining info as possible.
			yflag0 = yflag1 ;
			vtx0 = vtx1 ;
			vtx1 += 2 ;
		}
	}

	return (inside_flag) ;
}

/* 04/02/98 16.01 ----------------------------------
    Guarda se un pto e' all'interno di un pannello
--------------------------------------------------*/
int PointInside(int32 oc, int32 pan, const PointXZ &point) {
	return PointInside(oc, pan, (double)point.x, (double)point.z);
}
int PointInside(int32 oc, int32 pan, double x, double z) {
	t3dWALK *w = &Character[oc]->Walk;
	double pgon[4][2], ox, oz, s;

	if (pan < 0)
		return FALSE;

	pgon[0][0] = (double)w->Panel[pan].a.x;
	pgon[0][1] = (double)w->Panel[pan].a.z;
	pgon[3][0] = (double)w->Panel[pan].b.x;
	pgon[3][1] = (double)w->Panel[pan].b.z;

	pgon[1][0] = (double)w->Panel[pan].backA.x;
	pgon[1][1] = (double)w->Panel[pan].backA.z;

	pgon[2][0] = (double)w->Panel[pan].backB.x;
	pgon[2][1] = (double)w->Panel[pan].backB.z;

	ox = pgon[3][0] - pgon[0][0];
	oz = pgon[3][1] - pgon[0][1];
	s = sqrt(ox * ox + oz * oz);
	ox /= s;
	oz /= s;
	pgon[0][0] -= EPSILON * ox;
	pgon[0][1] -= EPSILON * oz;
	pgon[3][0] += EPSILON * ox;
	pgon[3][1] += EPSILON * oz;

	ox = pgon[2][0] - pgon[1][0];
	oz = pgon[2][1] - pgon[1][1];
	s = sqrt(ox * ox + oz * oz);
	ox /= s;
	oz /= s;
	pgon[1][0] -= EPSILON * ox;
	pgon[1][1] -= EPSILON * oz;
	pgon[2][0] += EPSILON * ox;
	pgon[2][1] += EPSILON * oz;

	return (PointInside2DRectangle(pgon, x, z)) ;
}

/*-----------------07/10/96 11.14-------------------
            Distanza falsa tra 2 punti 2D
--------------------------------------------------*/
float DistF(PointXZ a, PointXZ b) {
	return DistF(a.x, a.z, b.x, b.z);
}

float DistF(float x1, float y1, float x2, float y2) {
	float d1 = (float)fabs(x1 - x2);
	float d2 = (float)fabs(y1 - y2);
#if 0
	float minimum;

	if (d1 >= d2)
		minimum = d2;
	else
		minimum = d1;

	return d1+d2 - ( minimum / 2.0 );
#endif
	return (float)(sqrt(d1 * d1 + d2 * d2));
}
/*-----------------07/10/96 11.21-------------------
        Interseca linea 2D con linea 2D
--------------------------------------------------*/
PointResult IntersLineLine(const PointXZ &a, const PointXZ &b, const PointXZ &c, const PointXZ &d) {
	return IntersLineLine(a.x, a.z, b.x, b.z, c.x, c.z, d.x, d.z);
}
PointResult IntersLineLine(const PointXZ &a, const PointXZ &b, float xc, float yc, float xd, float yd) {
	return IntersLineLine(a.x, a.z, b.x, b.z, xc, yc, xd, yd);
}
PointResult IntersLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd) {
	float divisor = (float)((xb - xa) * (yd - yc) - (yb - ya) * (xd - xc));
	if (!divisor) divisor = 0.000001f;
	float r = (float)((ya - yc) * (xd - xc) - (xa - xc) * (yd - yc)) / divisor;
	float s = (float)((ya - yc) * (xb - xa) - (xa - xc) * (yb - ya)) / divisor;

	PointResult result;
	if ((r < -EPSILON) || (r > (1.0f + EPSILON)) || (s < -EPSILON) || (s > (1.0f + EPSILON))) {
		result.isValid = false;
	} else {
		if (r < 0.0f)    r = 0.0f;
		else if (r > 1.0f)   r = 1.0f;

		result.result.x = xa + r * (xb - xa);
		result.result.z = ya + r * (yb - ya);
	}

	return result;
}

/*-----------------15/10/96 10.33-------------------
        Compara distanza percorso (qsort)
--------------------------------------------------*/
int PathCompare(const void *arg1, const void *arg2) {
	const t3dPATHNODE *p1, *p2;

	p1 = (const t3dPATHNODE *)arg1;
	p2 = (const t3dPATHNODE *)arg2;

	if (p1->dist < p2->dist)
		return -1;
	else if (p1->dist > p2->dist)
		return 1;
	else
		return 0;
}

/*-----------------15/10/96 10.34-------------------
        Sorta i nodi del percorso trovato
--------------------------------------------------*/
void SortPath(int32 oc) {
	t3dWALK *w = &Character[oc]->Walk;
	qsort(&w->PathNode[0], w->NumPathNodes, sizeof(t3dPATHNODE), PathCompare);
}

/*-----------------06/11/95 15.42-------------------
        Torna l'angolo in rad dati seno e coseno
--------------------------------------------------*/
t3dF32 SinCosAngle(t3dF32 sinus, t3dF32 cosinus) {
	t3dF32 t;
	t = (t3dF32)sqrt((t3dF64)(sinus * sinus) + (t3dF64)(cosinus * cosinus));
	cosinus /= t;
	sinus /= t;

	if ((sinus == cosinus) && (cosinus == 0)) {
		return 0;
#if 0
	// FIXME: Is this duplicate code correct as the 2e4 is identical...
	} else if (sinus * cosinus >= 0) {
		// 1e3 quad
		if (sinus >= 0)
			// 1 quad
			return (t3dF32)acos(cosinus);
		else
			// 3 quad
			return (t3dF32)T3D_PI * 2.0f - (t3dF32)acos(cosinus);
#endif
	} else {
		// 2e4 quad
		if (sinus >= 0)
			// 2 quad
			return (t3dF32)acos(cosinus);
		else
			// 3 quad
			return (t3dF32)T3D_PI * 2.0f - (t3dF32)acos(cosinus);
	}
}

/* -----------------09/11/98 10.44-------------------
 *                  t3dVectAngle
 * torna angolo in gradi (da -180 a +180) tra due vettori
 * --------------------------------------------------*/
t3dF32 t3dVectAngle(t3dV3F *n, t3dV3F *o) {
	t3dF32 a = ((SinCosAngle(n->z, n->x) - SinCosAngle(o->z, o->x)) * 180.0f) / T3D_PI;

	while (a > 360.0f) a -= 360.0f;
	while (a < 0.0f) a += 360.0f;

	if (a > 180.0f) a -= 360.0f;
	else if (a < -180.0f) a += 360.0f;

	return a;
}

} // End of namespace Watchmaker
