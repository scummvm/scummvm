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

#include "watchmaker/walk/walk.h"
#include "watchmaker/types.h"
#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/walk/walkutil.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/walk/act.h"

namespace Watchmaker {

// locals
/*
WALK60      0   225 0
WALKBACK    4   162 226
RUN80       5   92  385
GIRODX      4   63  473
GIROSX      4   63  533

aNULL           0   x3  0   0
aSTAND          1   x3  3   3
aWALK_START     1+  x3  4   4
aWALK_LOOP      14  x3  42  42
aWALK_END       75  x3  225 225
aBACK_START     1+  x3  4   226
aBACK_LOOP      12  x3  36  258
aBACK_END       54  x3  162 384
aRUN_START      1+  x4  5   385
aRUN_LOOP       7   x4  28  408
aRUN_END        23  x4  92  472
aROT_DX         21  x3  63  473
aROT_SX         21  x3  63  533
                            592
*/
int32 ActionStart[] = {0, 3, 4, 42, 225, 226, 258, 384, 385, 408, 472, 473, 533, 592 };
//                     N   S  WS  WL  WE  BS  BL  BE  RS, RL  RE  RD  RS
int32 ActionLen[] = {1, 1, 38, 183, 1, 32, 126, 1, 23, 64, 1, 63, 63, 0};

/*-----------------16/10/96 11.07-------------------
    Guarda e 2 pannelli sono nello stesso blocco
--------------------------------------------------*/
int FindAttachedPanel(int32 oc, int srcp, int destp) {
	t3dCHARACTER *Act = Character[oc];
	t3dWALK *w = &Act->Walk;
	int curp;
	int nearp;
	int b;

	// se almeno uno e' sul pavimento sul pavimento esci
	if ((srcp < 0) || (destp < 0))
		return (0);

	// se sono uguali torna 1
	if (srcp == destp)
		return (1);

	curp  = srcp;
	nearp = w->Panel[srcp].near1;

	for (b = 0;; b++) {
		// se sono attaccati torna 1
		if (curp == destp)
			return (1);

		// se e' tornato al pannello di partenza torna 0
		if ((srcp == curp) && (b))
			return (0);

		if (b > w->PanelNum)
			return (0);

		// se sono attaccati al vertice 1 prende il 2
		if (w->Panel[nearp].near1 == curp) {
			curp  = nearp;
			nearp = w->Panel[curp].near2;
		} else {
			curp  = nearp;
			nearp = w->Panel[curp].near1;
		}
	}
}


/*-----------------19/10/96 10.38-------------------
        Fa uscire puto da pannello interno
--------------------------------------------------*/
void PointOut(int32 oc, t3dCAMERA *Camera) {
	t3dCHARACTER *Act = Character[oc];
	t3dWALK *w = &Act->Walk;
	PointXZ p;
	float inters;
	float temp;
	float nx, nz;
	int b;
#define LARGEVAL    60.0    // 30 cm = 15 enlarge * 2

	inters = 32000.0;

	// se ho beccato il pavimento devo contare quante volte interseca
	// i box larghi la retta omino pto
	if (w->CurPanel < 0)
		return;

	nx = w->Panel[w->CurPanel].a.z - w->Panel[w->CurPanel].b.z;
	nz = w->Panel[w->CurPanel].b.x - w->Panel[w->CurPanel].a.x;
	temp = (t3dF32)sqrt(nx * nx + nz * nz);
	nx /= temp;
	nz /= temp;

	// sposta il punto trovato sui pannelli larghi
	for (b = 0; b < w->PanelNum; b++) {
		// controlla solo pannelli esterni con stessi flags
//		if( ( Panel[b].flags & 0x80000000 ) &&
//			( Panel[b].flags & (Panel[CurPanel].flags & 0x7FFFFFFF) ) )
		{
			// controlla pto 1
			temp = DistF(w->Cur, w->Panel[b].a);

			if (temp < inters) {
				inters = temp;
				w->CurPanel = b;
				p = w->Panel[b].a;
			}

			// controlla pto 2
			temp = DistF(w->Cur, w->Panel[b].b);

			if (temp < inters) {
				inters = temp;
				w->CurPanel = b;
				p = w->Panel[b].b;
			}

			// controlla intersezione con camera
			PointResult res = IntersLineLine(w->Panel[b].a, w->Panel[b].b,
			                                 Camera->Source.x, Camera->Source.z,
			                                 w->Cur.x, w->Cur.z);
			if (res.isValid) {
				temp = DistF(w->Cur, res.result);

				if (temp < inters) {
					inters = temp;
					w->CurPanel = b;
					p = res.result;
				}
			}

			// controlla intersezione con omino
			res = IntersLineLine(w->Panel[b].a, w->Panel[b].b,
			                     Act->Pos.x, Act->Pos.z,
			                     w->Cur.x, w->Cur.z);
			if (res.isValid) {
				temp = DistF(w->Cur, res.result);

				if (temp < inters) {
					inters = temp;
					w->CurPanel = b;
					p = res.result;
				}
			}

			// controlla intersezione con normale pannello
			/*          PointResult res = IntersLineLine( w->Panel[b].x1, w->Panel[b].z1,
			                                              w->Panel[b].x2, w->Panel[b].z2,
			                                              w->CurX+nx*LARGEVAL, w->CurZ+nz*LARGEVAL,
			                                              w->CurX-nx*LARGEVAL, w->CurZ-nz*LARGEVAL );
			            if (res.isValid)
			            {
			                temp = DistF( w->Cur, res.result );

			                if( temp < inters )
			                {
			                    inters = temp;
			                    w->CurPanel = b;
			                    x = res.result.x;
			                    z = res.result.z;
			                }
			            }
			*/
		}
	}

	w->Cur = p;

#undef LARGEVAL
}

/*-----------------15/10/96 14.23-------------------
            Valuta lunghezza percorso
--------------------------------------------------*/
float EvalPath(int32 oc, int a, PointXZ dest, int nearp) {
	t3dCHARACTER *Act = Character[oc];
	t3dWALK *w = &Act->Walk;
	float len;
	PointXZ cur;
	int b;
	int curp;

	b = 0;

	len = 0.0;

	curp = w->PathNode[a].curp;
	cur = w->PathNode[a].pos;

	for (;;) {
		// se raggiunge il pto esce
		if (curp == w->PathNode[a + 1].curp) {
			len += DistF(cur, w->PathNode[a + 1].pos);
			break;
		}

		// se e' tornato al pannello di partenza non trova starda
		if ((curp == w->PathNode[a].curp) && (b)) {
			len += 320000.0;        // lunghezza assurda
			break;
		}

		// altrimenti va fino al prossimo pannello

		// se nearp e' attacato a curp col vertice 1
		if (w->Panel[nearp].near1 == curp) {
			// vai al vertice 2 la prossima volta
			len += DistF(cur, dest);

			cur = dest;

			dest = w->Panel[nearp].b;

			curp  = nearp;
			nearp = w->Panel[curp].near2;
		} else {
			// vai al vertice 1 la prossima volta
			len += DistF(cur, dest);

			cur = dest;

			dest = w->Panel[nearp].a;

			curp  = nearp;
			nearp = w->Panel[curp].near1;
		}

		b++;
	}

	return (len);
}


/*-----------------15/10/96 10.18-------------------
    cerca percorso piu' corto aggirando ostacoli
--------------------------------------------------*/
void FindShortPath(int32 oc) {
	t3dCHARACTER *Act = Character[oc];
	t3dWALK *w = &Act->Walk;
	t3dPATHNODE TempPath[T3D_MAX_PATHNODES];
	int    count = 0, inters;
	float  len1, len2;
	PointXZ cur;
	int    curp, nearp, oldp;
	PointXZ dest;
	signed int    a, b, c, fail = 0;

	count = 0;
	// aggiunge partenza
	TempPath[count].pos.x = Act->Pos.x;
	TempPath[count].pos.z = Act->Pos.z;
	TempPath[count].dist = 0.0;
	TempPath[count].oldp = w->OldPanel;
	TempPath[count].curp = w->OldPanel;
	count ++;

	// per ogni ostacolo cerca di aggirarlo a destra e a sinistra
	// poi guarda quello che e' piu' corto
	for (a = 0; a < w->NumPathNodes - 1; a++) {
		memcpy(&TempPath[count], &w->PathNode[a], sizeof(t3dPATHNODE));
		count ++;

		curp = w->PathNode[a].curp;

		// se pannello sorgente e destinazione non sono sullo stesso blocco
		if (!(FindAttachedPanel(oc, curp, w->PathNode[a + 1].curp)))
			continue;

		// aggira l'ostacolo partendo da near1
		len1 = EvalPath(oc, a, w->Panel[curp].a, w->Panel[curp].near1) + DistF(w->PathNode[a].pos, w->Panel[curp].a);

		// aggira l'ostacolo partendo da near2
		len2 = EvalPath(oc, a, w->Panel[curp].b, w->Panel[curp].near2) + DistF(w->PathNode[a].pos,w->Panel[curp].b);

		// guarda quale starda era piu' breve e se esiste una strada
		if ((len1 < 320000.0) && (len2 < 320000.0)) {
			if (len1 < len2) {
				dest = w->Panel[curp].a;
				nearp = w->Panel[curp].near1;
			} else {
				dest = w->Panel[curp].b;
				nearp = w->Panel[curp].near2;
			}

			cur = w->PathNode[a].pos;
			oldp = curp;

			b = 0;

			// salva percorso piu corto
			for (;;) {

				TempPath[count].pos = cur;
				TempPath[count].oldp = oldp;
				TempPath[count].curp = curp;
				count ++;

				// se raggiunge il pto esce
				if (curp == w->PathNode[a + 1].curp) {
					// attenzione: prima era
					//memcpy( &TempPath[count], &PathNode[++a], sizeof( struct SPathNode ) );
					memcpy(&TempPath[count], &w->PathNode[a + 1], sizeof(t3dPATHNODE));
					count ++;

					break;
				}

				// se e' tornato al pannello di partenza non trova starda
				if ((curp == w->PathNode[a].curp) && (b)) {
					fail = 1;   // si ferma al vertice prima
					break;      // e non contiuma a camminare
				}

				// altrimenti va fino al prossimo pannello

				// se nearp e' attacato a curp col vertice 1
				if (w->Panel[nearp].near1 == curp) {
					// vai al vertice 2 la prossima volta
					cur = dest;

					dest = w->Panel[nearp].b;

					oldp  = curp;
					curp  = nearp;
					nearp = w->Panel[curp].near2;
				} else {
					// vai al vertice 1 la prossima volta
					cur = dest;

					dest = w->Panel[nearp].a;

					oldp  = curp;
					curp  = nearp;
					nearp = w->Panel[curp].near1;
				}

				b++;
			}

		} else {
			//DebugText("Fail: FindShortPath");
			fail = 1;
		}

		if (fail)        // se non riesce ad aggirare ostacolo si ferma
			break;
	}

	// aggiunge arrivo
	TempPath[count].pos = w->Cur;
	TempPath[count].dist = 0.0;
	TempPath[count].oldp = w->CurPanel;
	TempPath[count].curp = w->CurPanel;
	count ++;

	for (b = 0; b < count; b++)
		DebugLogFile("FSP %d: %f %f | %d %d", b, TempPath[b].pos.x, TempPath[b].pos.z, TempPath[b].oldp, TempPath[b].curp);
	// dopo che ha aggirato tutti gli ostacoli ottimizza

//	memcpy( w->PathNode, TempPath, sizeof( struct SPathNode )*count );
//	w->NumPathNodes = count;
//	return ;

	w->NumPathNodes = 0;
	for (a = 0; a < count; a++) {
		//printf("N:%d | ",a);
		if (w->NumPathNodes > (T3D_MAX_PATHNODES - 2))
			w->NumPathNodes = (T3D_MAX_PATHNODES - 2);

		// prima leva tutti i nodi attaccati
		for (b = (count - 1); b >= a; b--)
			if (DistF(TempPath[b].pos, TempPath[a].pos) < EPSILON)
				break;
		DebugLogFile("Da %d passo a %d\n", a, b);
		a = b;

		memcpy(&w->PathNode[w->NumPathNodes], &TempPath[a], sizeof(t3dPATHNODE));
		w->NumPathNodes ++;

		for (b = (count - 1); b > a + 1; b--) {
			inters = 0;
			for (c = 0; c < w->PanelNum; c++) {
				// non deve intersecare pannello stretto mai
//				if( !( w->Panel[c].flags & 0x80000000 ) )
				{
					PointResult res = IntersLineLine(w->Panel[c].backA, w->Panel[c].backB,
					                                TempPath[a].pos, TempPath[b].pos);
					if (res.isValid)
						inters ++;

					res = IntersLineLine(w->Panel[c].a, w->Panel[c].backA,
					                     TempPath[a].pos, TempPath[b].pos);
					if (res.isValid) {
						len2 = DistF(res.result, TempPath[a].pos);
						len1 = DistF(res.result, TempPath[b].pos);

						// interseca in un pto distante da partenza e arrivo
						if ((len1 > EPSILON) && (len2 > EPSILON))
							inters ++;
					}

					res = IntersLineLine(w->Panel[c].b, w->Panel[c].backB,
					                     TempPath[a].pos, TempPath[b].pos);
					if (res.isValid) {
						len2 = DistF(res.result, TempPath[a].pos);
						len1 = DistF(res.result, TempPath[b].pos);

						// interseca in un pto distante da partenza e arrivo
						if ((len1 > EPSILON) && (len2 > EPSILON))
							inters ++;
					}

					if (inters)
						break;
				}
			}

			// se da a posso raggiungere b direttamente
			if (!inters) {
				curp = w->PathNode[w->NumPathNodes - 1].curp;
				oldp = TempPath[b].oldp;

				for (c = a; c <= b; c++)
					if ((TempPath[c].oldp == curp) && (TempPath[c].curp == oldp))
						break;

				// se non erano collegati signica che e' passato per il pavimento
				if (c > b) {
					DebugLogFile("Arr %d %d Part %d %d | %d", b, TempPath[b].oldp, a, w->PathNode[w->NumPathNodes - 1].curp, w->NumPathNodes - 1);
					w->PathNode[w->NumPathNodes - 1].curp = -1; // partenza
					TempPath[b].oldp = -1;               // arrivo
				}
				a = b - 1;
				break;
			}
		}
	}
	for (b = 0; b < w->NumPathNodes; b++)
		DebugLogFile("SSP %d: %f %f | %d %d", b, w->PathNode[b].pos.x, w->PathNode[b].pos.z, w->PathNode[b].oldp, w->PathNode[b].curp);
}

/* -----------------04/02/98 15.48-------------------
 *                  Crea percorso
 * --------------------------------------------------*/
void FindPath(int32 oc, t3dCAMERA *Camera) {
	t3dCHARACTER *Act = Character[oc];
	t3dWALK *w = &Act->Walk;
	int inters, check;
	float dist;
	int b;

	w->NumPathNodes = 0;
	w->CurrentStep = 0;
	w->NumSteps = 0;

	// FIXME: condition never happen!
	//if( !w->PathNode ) return;

	check = 0;
	inters = 0;
	w->NumPathNodes = 0;

	w->OldPanel = w->CurPanel;
	if (FloorHit)
		w->CurPanel = -1;
	/*  else
	        w->CurPanel = 1;
	*/
	PointOut(oc, Camera);
	w->Look = w->Cur;

	// se hai cliccato dietro il pannello di partenza o dell'angolo non puo' camminare
	if ((w->CurPanel < 0) && (w->OldPanel >= 0) &&
	        // dietro il pannello di partenza
	        ((PointInside(oc, b = w->OldPanel, w->Cur)) ||
	         // dietro il pannello angolo1
	         ((DistF(w->Panel[w->OldPanel].a.x, w->Panel[w->OldPanel].a.z, Act->Pos.x, Act->Pos.z) < EPSILON) &&
	          (PointInside(oc, b = w->Panel[w->OldPanel].near1, w->Cur))) ||
	         // dietro il pannello angolo2
	         ((DistF(w->Panel[w->OldPanel].b.x, w->Panel[w->OldPanel].b.z, Act->Pos.x, Act->Pos.z) < EPSILON) &&
	          (PointInside(oc, b = w->Panel[w->OldPanel].near2, w->Cur))))) {
		w->Cur.x = Act->Pos.x;
		w->Cur.z = Act->Pos.z;
		w->CurPanel = b;
		w->NumPathNodes = 0;
		check |= CLICKINTO;
		w->Check = check;
		return ;
	}
	DebugLogFile("W: CP %d OP %d | %f %f | %f %f", w->CurPanel, w->OldPanel, Act->Pos.x, Act->Pos.z, w->Cur.x, w->Cur.z);

	dist = DistF(Act->Pos.x, Act->Pos.z, w->Cur.x, w->Cur.z);
//	if( dist < EPSILON )
//		return ;

	for (b = 0; b < w->PanelNum; b++) {
		PointResult res = IntersLineLine(w->Panel[b].a, w->Panel[b].b,
		                                 Act->Pos.x, Act->Pos.z,
		                                 w->Cur.x, w->Cur.z);
		if (res.isValid) {
			inters ++;
			DebugLogFile("Inters %d: %f %f %f %f", b, w->Panel[b].a.x, w->Panel[b].a.z, w->Panel[b].b.x, w->Panel[b].b.z);

			w->PathNode[w->NumPathNodes].pos = res.result;
			w->PathNode[w->NumPathNodes].dist = DistF(Act->Pos.x, Act->Pos.z, res.result.x, res.result.z);
			w->PathNode[w->NumPathNodes].oldp = b;
			w->PathNode[w->NumPathNodes].curp = b;
			w->NumPathNodes ++;

			// ANGOLI - leva intersezioni in angoli
			if ((w->OldPanel >= 0) && ((b == w->Panel[w->OldPanel].near1) || (b == w->Panel[w->OldPanel].near2))) {
				// altrimenti se e' vicino al pannello di partenza
				if ((w->PathNode[w->NumPathNodes - 1].dist < EPSILON) &&
				        (b != w->OldPanel) && (b != w->CurPanel)) {
					// e la distanza e' molto piccola leva l'intersezione
					inters --;
					w->NumPathNodes --;
					check |= OLDANGLESKIP;

					// se ho cliccato dentro il pannello vicino
					if ((w->CurPanel < 0) && (PointInside(oc, b, w->Cur))) {
						w->Cur.x = Act->Pos.x;
						w->Cur.z = Act->Pos.z;
						w->CurPanel = b;
						w->NumPathNodes = 0;
						check |= CLICKINTO;
						w->Check = check;
						return ;
					}
				}
			} else if ((w->CurPanel >= 0) && ((b == w->Panel[w->CurPanel].near1) || (b == w->Panel[w->CurPanel].near2))) {
				// altrimenti se e' vicino al pannello di arrivo
				if ((fabs(w->PathNode[w->NumPathNodes - 1].dist - dist) < EPSILON) &&
				        (b != w->OldPanel) && (b != w->CurPanel)) {
					// e la distanza e' molto piccola leva l'intersezione
					inters --;
					w->NumPathNodes --;
					check |= CURANGLESKIP;
				}
			}

		}
		// aggiunge sempre nodo partenza e arrivo solo se su un pannelli
		else if (b == w->OldPanel) {
			inters ++;

			w->PathNode[w->NumPathNodes].pos.x = Act->Pos.x;
			w->PathNode[w->NumPathNodes].pos.z = Act->Pos.z;
			w->PathNode[w->NumPathNodes].dist = 0.0;
			w->PathNode[w->NumPathNodes].oldp = w->OldPanel;
			w->PathNode[w->NumPathNodes].curp = w->OldPanel;
			w->NumPathNodes ++;

			check |= NOOLDINTERS;
		} else if (b == w->CurPanel) {
			inters ++;

			w->PathNode[w->NumPathNodes].pos = w->Cur;
			w->PathNode[w->NumPathNodes].dist = dist;
			w->PathNode[w->NumPathNodes].oldp = w->CurPanel;
			w->PathNode[w->NumPathNodes].curp = w->CurPanel;
			w->NumPathNodes ++;

			check |= NOCURINTERS;
		}
	}

	// il percorso e' definito da:
	// start        Act.px, Act.pz
	// NumPathNodes PathNode
	// end          CurX, CurZ

	// se scontra contro qualche pannello
	if (inters) {
		SortPath(oc);

		// se dispari e vado nel pavimento ma non sono partito da pannello
		// se vado nel pavimento e ultimi due nodi non sono su stesso blocco
		// se all'esterno dell'ultimo pannello sposta ultimo nodo
		if (w->NumPathNodes > 1)
			DebugLogFile("I %d | CP %d | OP %d | FA %d (%d %d) | PI %d (%d)", inters, w->CurPanel, w->OldPanel,
			             FindAttachedPanel(oc, w->PathNode[w->NumPathNodes - 2].curp, w->PathNode[w->NumPathNodes - 1].curp), w->PathNode[w->NumPathNodes - 2].curp, w->PathNode[w->NumPathNodes - 1].curp,
			             PointInside(oc, w->PathNode[w->NumPathNodes - 1].curp, w->Cur), w->PathNode[w->NumPathNodes - 1].curp);

		if (((inters   & 1) && (w->CurPanel < 0) && (w->OldPanel < 0)) ||
		        ((w->CurPanel < 0) && (w->NumPathNodes >= 1) && (PointInside(oc, w->PathNode[w->NumPathNodes - 1].curp, w->Cur))) ||
		        (((inters - 1) & 1) && (w->CurPanel < 0) && (w->NumPathNodes >= 2) &&
		         (!(FindAttachedPanel(oc, w->PathNode[w->NumPathNodes - 2].curp, w->PathNode[w->NumPathNodes - 1].curp)) ||
		          (PointInside(oc, w->PathNode[w->NumPathNodes - 1].curp, w->Cur))))) {
			w->CurPanel = w->PathNode[w->NumPathNodes - 1].curp;

			PointOut(oc, Camera);        // tira fuori il pto trovato

			w->PathNode[w->NumPathNodes].pos = w->Cur;
			w->PathNode[w->NumPathNodes].oldp = w->CurPanel;
			w->PathNode[w->NumPathNodes].curp = w->CurPanel;

			UpdateLook(oc);

			w->NumPathNodes ++;

			check |= POINTOUT1;

			DebugLogFile("PO %d %d %f", w->NumPathNodes, w->CurPanel, DistF(Act->Pos.x, Act->Pos.z, w->Cur.x, w->Cur.z));
//			if ( DistF( Act->Pos.x, Act->Pos.z, w->CurX, w->CurZ ) < EPSILON )
//				check |= CLICKINTO;
		}

		// se arrivo su pavimento
		if (w->CurPanel < 0) {
			inters = 0;

			// conto intersezioni con pannelli stretti
			// e con unione pannelli larghi con pannelli stretti
			for (b = 0; b < w->PanelNum; b++) {
				PointResult res;
				res = IntersLineLine(w->Panel[b].a, w->Panel[b].b,
				                     w->PathNode[w->NumPathNodes - 1].pos, w->Cur);
				if (res.isValid)
					if ((DistF(res.result, w->PathNode[w->NumPathNodes - 1].pos) > EPSILON) &&
					        (DistF(res.result, w->Cur) > EPSILON))
						inters ++;

				res = IntersLineLine(w->Panel[b].a, w->Panel[b].backA,
				                     w->PathNode[w->NumPathNodes - 1].pos, w->Cur);
				if (res.isValid)
					if ((DistF(res.result, w->PathNode[w->NumPathNodes - 1].pos) > EPSILON) &&
					        (DistF(res.result, w->Cur) > EPSILON))
						inters ++;

				res = IntersLineLine(w->Panel[b].b, w->Panel[b].backB,
				                     w->PathNode[w->NumPathNodes - 1].pos, w->Cur);
				if (res.isValid)
					if ((DistF(res.result, w->PathNode[w->NumPathNodes - 1].pos) > EPSILON) &&
					        (DistF(res.result, w->Cur) > EPSILON))
						inters ++;

				if (inters)
					break;
			}

			// se nell'ultimo tratto c'e un ostacolo leva primo nodo
			if (inters) {
				w->CurPanel = w->PathNode[w->NumPathNodes - 1].curp;

				PointOut(oc, Camera);        // tira fuori il pto trovato
				w->PathNode[w->NumPathNodes].pos = w->Cur;
				w->PathNode[w->NumPathNodes].oldp = w->CurPanel;
				w->PathNode[w->NumPathNodes].curp = w->CurPanel;

				UpdateLook(oc);

				w->NumPathNodes ++;

				check |= POINTOUT2;
			}
		}

		DebugLogFile("CP: %d || OP: %d || %f %f || I: %d || C: %d || NPN: %d", w->CurPanel, w->OldPanel, w->Cur.x, w->Cur.z, inters, check, w->NumPathNodes);
//DebugText("CP: %d || OP: %d || I: %d || C: %d || PI: %d || NPN: %d", CurPanel, OldPanel, inters, check, PointInside( OldPanel, CurX, CurZ ), NumPathNodes );

		w->PathNode[w->NumPathNodes].pos = w->Cur;
		w->PathNode[w->NumPathNodes].dist = DistF(Act->Pos.x, Act->Pos.z, w->Cur.x, w->Cur.z);
		w->PathNode[w->NumPathNodes].oldp = w->CurPanel;
		w->PathNode[w->NumPathNodes].curp = w->CurPanel;
		w->NumPathNodes ++;

		for (b = 0; b < w->NumPathNodes; b++)
			DebugLogFile("FP %d: %f %f | %d %d", b, w->PathNode[b].pos.x, w->PathNode[b].pos.z, w->PathNode[b].oldp, w->PathNode[b].curp);

		FindShortPath(oc);

//		DisplayPath();
	} else {        // altrimenti starda diretta
		DebugLogFile("NOI CP: %d || OP: %d || I: %d || C: %d || NPN: %d", w->CurPanel, w->OldPanel, inters, check, w->NumPathNodes);
//DebugText("NOI CP: %d || OP: %d || I: %d || C: %d || PI: %d || NPN: %d", CurPanel, OldPanel, inters, check, PointInside( OldPanel, CurX, CurZ ), NumPathNodes );
		w->PathNode[w->NumPathNodes].pos.x = Act->Pos.x;
		w->PathNode[w->NumPathNodes].pos.z = Act->Pos.z;
		w->PathNode[w->NumPathNodes].dist = 0.0;
		w->PathNode[w->NumPathNodes].oldp = w->OldPanel;
		w->PathNode[w->NumPathNodes].curp = w->OldPanel;
		w->NumPathNodes ++;

		w->PathNode[w->NumPathNodes].pos = w->Cur;
		w->PathNode[w->NumPathNodes].dist = DistF(Act->Pos.x, Act->Pos.z, w->Cur.x, w->Cur.z);
		w->PathNode[w->NumPathNodes].oldp = w->CurPanel;
		w->PathNode[w->NumPathNodes].curp = w->CurPanel;
		w->NumPathNodes ++;

	}
	w->Check = check;
	DebugLogFile("End Walk %f %f  %d | %f %f %d", Act->Pos.x, Act->Pos.z, w->OldPanel, w->Cur.x, w->Cur.z, w->CurPanel);
}

/* -----------------12/02/99 11.07-------------------
 *                  ForceAnimInBounds
 * --------------------------------------------------*/
void ForceAnimInBounds(int32 oc) {
	t3dCHARACTER *Act = Character[oc];
	t3dWALK *w = &Act->Walk;
	t3dV3F *Trasl;
	int inters, nf;
	float dist;
	int a, b;

	if (!Act || !w) return ;

	nf = Act->Mesh->Anim.NumFrames;
	Trasl = Act->Mesh->Anim.BoneTable[0].Trasl;
	if (!Trasl) return ;

	inters = 0;
	dist = DistF(Trasl[0].x, Trasl[0].z, Trasl[nf - 1].x, Trasl[nf - 1].z);
	if (dist < EPSILON) return ;                                                                 // Se non si muove esce

	for (a = 1; a < nf; a++) {
		for (b = 0; b < w->PanelNum; b++) {
//			Se all'ultimo frame finirei dentro un pannello aggiorna CurPanel
			if ((a == (nf - 1)) && (PointInside(oc, b, Trasl[a].x, Trasl[a].z))) {
				w->OldPanel = w->CurPanel;
				w->CurPanel = b;
				DebugLogFile("Aggiorno CurPanel %d", b);
			}
//			Se un punto interseca pannello slida
			PointResult res = IntersLineLine(w->Panel[b].a, w->Panel[b].b,
			                                 Trasl[0].x, Trasl[0].z,
			                                 Trasl[a].x, Trasl[a].z);
			if (res.isValid) {
				inters ++;

				Trasl[a].x = res.result.x;
				Trasl[a].z = res.result.z;
				DebugLogFile("%d: entrerebbe in %d", a, b);
			}
		}
	}
	(void)inters;
}

} // End of namespace Watchmaker
