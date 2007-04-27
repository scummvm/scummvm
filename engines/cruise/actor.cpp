/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "cruise/cruise.h"

namespace Cruise {

int16 mainProc13(int overlayIdx, int param1, actorStruct* pStartEntry, int param2)
{
  actorStruct* pCurrentEntry = pStartEntry->next;

  while(pCurrentEntry)
  {
    if( (pCurrentEntry->overlayNumber == overlayIdx || overlayIdx == -1) &&
        (pCurrentEntry->var4 == param1 || param1 == -1) &&
        (pCurrentEntry->type == param2 || param2 == -1) &&
        (pCurrentEntry->pathId != -2))
    {
      return 0;
    }
    
    pCurrentEntry = pCurrentEntry->next;
  }

  return 1;
}

actorStruct* findActor(int overlayIdx, int param1, actorStruct* pStartEntry, int param2)
{
  actorStruct* pCurrentEntry = pStartEntry->next;

  while(pCurrentEntry)
  {
    if( (pCurrentEntry->overlayNumber == overlayIdx || overlayIdx == -1) &&
        (pCurrentEntry->var4 == param1 || param1 == -1) &&
        (pCurrentEntry->type == param2 || param2 == -1) )
    {
      return pCurrentEntry;
    }
    
    pCurrentEntry = pCurrentEntry->next;
  }

  return NULL;
}

int nclick_noeud;
int flag_aff_chemin;

void getPixel(int x, int y)
{
  int	x_min, x_max, y_min, y_max;

  ctpVar19Struct *polygone;
  ctpVar19SubStruct *tableau;

  polygone = ctpVar19;	/* adr structure polygone */

  while ( polygone->field_0 != (ctpVar19Struct*)-1)
  {
    tableau = &polygone->subStruct;

    x_min = tableau->minX;
    x_max = tableau->maxX;
    y_min = tableau->minY;
    y_max = tableau->maxY;

    computedVar14 = tableau->boxIdx;	/* numero polygone	*/

    if ( walkboxChange[computedVar14]==0 && ((x>=x_min && x<=x_max) && (y>=y_min && y<=y_max)) )
    {
      // click was in given box
    /*  u = y-y_min;

      //tableau+=u;
      tableau = &polygone[u].subStruct;

      x_min = tableau->minX;
      x_max = tableau->maxX;

      if ( (x>=x_min && x<=x_max) ) */
      {		
        flag_obstacle=walkboxType[computedVar14];	/* sa couleur */

        return;
      }
    }
    polygone = polygone->field_0;
  }

  flag_obstacle=0;
}

int x_mouse;
int y_mouse;

int point_select;

int table_ptselect[2][2];

int X;
int Y;

int modelVar9;
int modelVar10;

void polydroite(int x1, int y1, int x2, int y2)
{
  int dx;
  int dy;

  int mD0;
  int mD1;

  int mA0;
  int mA1;

  int bp;
  int cx;
  int si;

  int ax;
  int bx;

  modelVar9 = x1;
  modelVar10 = y1;

  dx = x2-x1;
  dy = y2-y1;

  mD0 = mD1 = 1;

  if(dx<0)
  {
    dx = -dx;
    mD0 = -1;
  }

  if(dy<0)
  {
    dy = -dy;
    mD1 = -1;
  }

  if(dx<dy)
  {
    mA0 = 0;
    bp = dx;
    cx = dy;

    mA1 = mD1;
  }
  else
  {
    mA1 = 0;
    bp = dy;
    cx = dx;

    mA0 = mD0;
  }

  bp=bp*2;
  dx=bp-cx;
  si=dx-cx;

  ax = modelVar9;
  bx = modelVar10;

  getPixel(modelVar9, modelVar10);

  X = modelVar9;
  Y = modelVar10;

  if(flag_obstacle==0)
  {
    flag_obstacle = 1;
    return;
  }

  while(--cx)
  {
    if(dx>0)
    {
      ax+=mD0;
      bx+=mD1;
      dx+=si;
    }
    else
    {
      ax+=mA0;
      bx+=mA1;
      dx+=bp;
    }

    getPixel(ax, bx);

    X = ax;
    Y = bx;

    if(flag_obstacle==0)
    {
      flag_obstacle = 1;
      return;
    }
  }

  flag_obstacle = 0;
}

void poly2(int x1, int y1, int x2, int y2)
{
  int dx;
  int dy;

  int mD0;
  int mD1;

  int mA0;
  int mA1;

  int bp;
  int cx;
  int si;

  int ax;
  int bx;

  modelVar9 = x1;
  modelVar10 = y1;

  dx = x2-x1;
  dy = y2-y1;

  mD0 = mD1 = 1;

  if(dx<0)
  {
    dx = -dx;
    mD0 = -1;
  }

  if(dy<0)
  {
    dy = -dy;
    mD1 = -1;
  }

  if(dx<dy)
  {
    mA0 = 0;
    bp = dx;
    cx = dy;

    mA1 = mD1;
  }
  else
  {
    mA1 = 0;
    bp = dy;
    cx = dx;

    mA0 = mD0;
  }

  bp=bp*2;
  dx=bp-cx;
  si=dx-cx;

  ax = modelVar9;
  bx = modelVar10;

  getPixel(modelVar9, modelVar10);

  X = modelVar9;
  Y = modelVar10;

  if(flag_obstacle!=0)
  {
    flag_obstacle = 1;
    return;
  }

  while(--cx)
  {
    if(dx>0)
    {
      ax+=mD0;
      bx+=mD1;
      dx+=si;
    }
    else
    {
      ax+=mA0;
      bx+=mA1;
      dx+=bp;
    }

    getPixel(ax, bx);

    X = ax;
    Y = bx;

    if(flag_obstacle!=0)
    {
      flag_obstacle = 1;
      return;
    }
  }

  flag_obstacle = 0;
}

int	point_proche( int16 table[][2] )
{
  int	x1, y1, i, x, y, p;
  int	d1=1000;

  ctpVar19 = ctpVar11;

  if ( nclick_noeud == 1 )
  {
    x  = x_mouse;
    y  = y_mouse;
    x1 = table_ptselect[0][0];
    y1 = table_ptselect[0][1];

    ctpVar19 = ctpVar15;

    getPixel( x, y );

    if ( !flag_obstacle )
    {	
      ctpVar19 = ctpVar11;

      getPixel( x, y );

      if ( flag_obstacle )
      {
        polydroite( x1, y1, x, y );
      }
      ctpVar19 = ctpVar15;
    }
    if ( !flag_obstacle )	/* dans flag_obstacle --> couleur du point */
    {
      x1 = table_ptselect[0][0];
      y1 = table_ptselect[0][1];

      poly2( x, y, x1, y1 );

      x_mouse = X;
      y_mouse = Y;
    } 
  }
  ctpVar19 = ctpVar11;
  
  p = -1;
  for ( i=0; i < ctp_routeCoordCount; i++ )
  {
    x = table[i][0];
    y = table[i][1];

    ctpProc2( x_mouse, y_mouse, x, y );
    if ( ctpVar14 < d1 )
    {
      polydroite( x_mouse, y_mouse, x, y );

      if ( !flag_obstacle && ctp_routes[i][0] > 0 )
      {
        d1 = ctpVar14;
        p  = i;
      }
    }
  }

  return ( p );
}

#define NBNOEUD 20

int16 select_noeud[3];
char	solution[20+1];

int prem;
int prem2;
int dist_chemin;
int idsol;
int solmax;

char 	fl[NBNOEUD+1];
char	sol[NBNOEUD+1];
char	Fsol[NBNOEUD+1];

int D;

void	explore( int depart, int arrivee )
{
	int	id1, id2, i;

	id1 = depart;

	fl[id1]++;
	sol[idsol++] = (char)id1;

	if ( idsol > solmax )
	{
		fl[id1]=-1;
		idsol--;

		return;
	}

	while ( (i=fl[id1]) < 20 )
	{
		id2 = ctp_routes[id1][i+1];

		if ( id2 == arrivee )
		{
			if ( idsol < solmax )
			{
				sol[idsol] = (char)arrivee;
				D=0;
				for ( i=0; i < idsol; i++ )
				{
					D = D+ctp_routeCoords[sol[i]][sol[i+1]];
					Fsol[i]=sol[i];
				}
				prem2=0;
				if (!prem) 
				{
					dist_chemin = D;
					prem = 1;
					for ( i=0; i <= idsol; i++ )
					{
						solution[i] = sol[i];
					}
					solution[i++]=-1;
					solution[i]=-1;
				}
				else if ( D < dist_chemin )
				{
					dist_chemin = D;
					for ( i=0; i <= idsol; i++ )
					{
						solution[i] = sol[i];
					}
					solution[i++]=-1;
					solution[i]=-1;
				}
			}
			fl[id1]=-1;
			idsol--;

			return;
		}
		else	if ((id2!=-1) && ((int)fl[id2]==-1) )
				explore(id2,arrivee);
		else	if (id2==-1)
		{
			fl[id1]=-1;
			idsol--;

			return;
		}
		fl[id1]++;
	}

	fl[id1]=-1;
	idsol--;
}

void	chemin0( int depart, int arrivee )
{
	int	i;
	//int	y=30;
	
	prem  = 0;
	prem2 = 0;
	dist_chemin=0;
	idsol = 0;
	solmax=999;

	for ( i=0; i < 20+1; i++ ) fl[i] = -1;

	X=0,Y=30;

	explore( depart, arrivee );
}

void valide_noeud( int16 table[], int16 p, int *nclick, int16 solution0[20+3][2] )
{
	int	a, b, d, i, p1, x1, x2, y1, y2;
	//int	y=30;

	table[*nclick]=p;
	table[(*nclick)+1]=-1;	
	table_ptselect[*nclick][0]=x_mouse;	
	table_ptselect[*nclick][1]=y_mouse;	
	(*nclick)++;
	ctpVar19=ctpVar11;

	if (*nclick==2) // second point
	{
		x1=table_ptselect[0][0];
		y1=table_ptselect[0][1];
		x2=table_ptselect[1][0];
		y2=table_ptselect[1][1];
		if ( (x1==x2) && (y1==y2))
		{
			return;
		}
		flag_aff_chemin=1;
		ctpVar19=ctpVar15;

    // can we go there directly ?
		polydroite(x1,y1,x2,y2);
    ////////////////
    flag_obstacle = 0;
    ////////////////
		if (!flag_obstacle)
		{
			solution0[0][0]=x1;
			solution0[0][1]=y1;
			ctpVar19=ctpVar15;

			poly2(x2,y2,ctp_routeCoords[select_noeud[1]][0],ctp_routeCoords[select_noeud[1]][1]);

			solution0[1][0]=table_ptselect[1][0]=X;
			solution0[1][1]=table_ptselect[1][1]=Y;
			solution0[2][0]=-1;

			if ( (x1==X) && (y1==Y))
			{
				flag_aff_chemin=0;
				return;
			}
		}
		else	
		{
      // no, we take the fastest way
			solution[0]=-1;
			if (ctp_routes[select_noeud[0]][0]>0) 
				chemin0(table[0],table[1]);

			if (solution[0]==-1)
			{
				x1=table_ptselect[0][0];
				y1=table_ptselect[0][1];
				polydroite(x1,y1,x_mouse,y_mouse);
				solution0[0][0]=x1;
				solution0[0][1]=y1;
				solution0[1][0]=X;
				solution0[1][1]=Y;

				solution0[2][0]=-1;
				if ( (x1==X) && (y1==Y))
				{
					flag_aff_chemin=0;
					return;
				}
			}	
			else
			{
				solution0[0][0]=x1;
				solution0[0][1]=y1;
				i=0;
				while (solution[i]!=-1)
				{
					p1=solution[i];
					solution0[i+1][0]=ctp_routeCoords[p1][0];
					solution0[++i][1]=ctp_routeCoords[p1][1];
				}
				ctpVar19=ctpVar15;
				poly2(x2,y2,ctp_routeCoords[select_noeud[1]][0],ctp_routeCoords[select_noeud[1]][1]);
				solution0[i+1][0]=table_ptselect[1][0]=X;
				solution0[i+1][1]=table_ptselect[1][1]=Y;
				solution0[i+2][0]=-1;
				if ( (x1==X) && (y1==Y))
				{
					flag_aff_chemin=0;
					return;
				}

	/****** COUPE LE CHEMIN ******/			

				i++;
				d=0;
				a=i;
				flag_obstacle=1;
				while (d!=a)
				{
					x1=solution0[d][0];
					y1=solution0[d][1];
					
					while (flag_obstacle && i!=d)
					{
						x2=solution0[i][0];
						y2=solution0[i][1];
						ctpVar19=ctpVar15;
						polydroite(x1,y1,x2,y2);
						i--;
					}
					flag_obstacle=1;
					if (d!=i)
					{
						i++;
						for (b=d+1;b<i;b++)
						{
							solution0[b][0]=-2;
						}
					}
					else i++;
					d=i;
					i=a;
				}	
				flag_obstacle=0;
			}
		}
	}
}

//computePathfinding(returnVar2, params.X, params.Y, var34, var35, currentActor->stepX, currentActor->stepY);
int16 computePathfinding(int16* pSolution, int16 X, int16 Y, int16 destX, int16 destY, int16 stepX, int16 stepY, int16 oldPathId)
{
  persoStruct* perso;
  int num;

  if(!polyStruct)
  {
    pSolution[0] = -1;
    pSolution[1] = -1;

    return -1;
  }

  if(oldPathId >= 0)
  {
    if(persoTable[oldPathId])
    {
      freePerso(oldPathId);
    }
  }

  if(!flagCt)
  {
    int i;
    int16* ptr;

    for(i=0;i<10;i++) // 10 = num perso
    {
      if(!persoTable[i])
      {
        break;
      }
    }

    if(i == 10)
    {
      pSolution[0] = -1;
      pSolution[1] = -1;

      return -1;
    }

    perso = persoTable[i] = (persoStruct*)malloc(sizeof(persoStruct));

    ptr = perso->solution[0];

    perso->inc_jo1 = stepX;
    perso->inc_jo2 = stepY;

    *(ptr++) = X;
    *(ptr++) = Y;
    *(ptr++) = pSolution[0] = destX;
    *(ptr++) = pSolution[1] = destY;
    *(ptr++) = -1;

    pSolution[4] = computedVar14;

    perso->inc_droite = 0;
    perso->inc_chemin = 0;

    return i;
  }

  nclick_noeud=0;
	ctpVar19=ctpVar11;
	flag_aff_chemin=0;

	if (X==destX && Y==destY)
	{
		pSolution[0]=-1;
		pSolution[1]=-1;

		return(-1);
	}

/******* cherche le premier noeud ******/

  getPixel(X,Y);

  pSolution[4] = computedVar14;

  x_mouse = X;
  y_mouse = Y;

	if (!flag_obstacle || (point_select=point_proche(ctp_routeCoords))==-1)
	{
		pSolution[0]=-1;
		pSolution[1]=-1;

		return(-1);
	}

	valide_noeud(select_noeud,point_select,&nclick_noeud,NULL);

	flag_aff_chemin=0;

/******* cherche le deuxieme noeud ******/

	num=0;
	while ( num < 10 && persoTable[num] != NULL )	num++;

	if (num==10)
	{
		pSolution[0]=-1;
		pSolution[1]=-1;
		return(-1);
	}
	
  perso = persoTable[num] = (persoStruct*)malloc(sizeof(persoStruct));

  perso->inc_jo1 = stepX;
  perso->inc_jo2 = stepY;

  x_mouse = destX;
  y_mouse = destY;

  if((point_select=point_proche(ctp_routeCoords))!=-1)
		valide_noeud(select_noeud,point_select,&nclick_noeud,perso->solution);

	if ( (!flag_aff_chemin) || ((table_ptselect[0][0]==table_ptselect[1][0]) && (table_ptselect[0][1]==table_ptselect[1][1]) ))
	{
		pSolution[0]=-1;
		pSolution[1]=-1;
		freePerso(num);

		return(-1);
	}
		
	pSolution[0]=table_ptselect[1][0];
	pSolution[1]=table_ptselect[1][1];
	pSolution[4]=computedVar14;
  perso->inc_chemin=0;
	perso->inc_droite=0;

	return (num);
}

void	set_anim( int ovl, int obj, int start, int x, int y, int mat, int state )
{
	int	newf, zoom;

	newf = abs(mat)-1;

	zoom = subOp22(y);
	if (mat<0) zoom=-zoom;

	setObjectPosition(ovl,obj,0, x);
	setObjectPosition(ovl,obj,1, y);
	setObjectPosition(ovl,obj,2, y);
	setObjectPosition(ovl,obj,4, zoom);
	setObjectPosition(ovl,obj,3, newf+start);
	setObjectPosition(ovl,obj,5, state);
}

int raoul_move[][13] =
{
	{  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 0 },  /* dos         */
	{ 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 0 },  /* droite      */
	{ 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0 },  /* face        */
	{-13,-14,-15,-16,-17,-18,-19,-20,-21,-22,-23,-24, 0 }   /* gauche      */
};

int raoul_end[][13] =
{
	{ 37,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0 },  /* stat dos    */     	
	{ 38,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0 },  /* stat droite */     	
	{ 39,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0 },  /* stat face   */     	
	{-38,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0 }   /* stat gauche */     	
};

int raoul_stat[][13]=
{
	{ 53, 54, 55, 56, 57,  0,  0,  0,  0,  0,  0,  0, 0 },  /* ret dos-dr  */
	{ 59, 60, 62, 63, 78,  0,  0,  0,  0,  0,  0,  0, 0 },  /* ret dr-face */ 	
	{-78,-63,-62,-60,-59,  0,  0,  0,  0,  0,  0,  0, 0 },  /* ret face-ga */ 
	{-57,-56,-55,-54,-53,  0,  0,  0,  0,  0,  0,  0, 0 }  /* ret ga-dos  */  
};

int raoul_invstat[][13]=
{
	{-53,-54,-55,-56,-57,  0,  0,  0,  0,  0,  0,  0, 0 },  /* ret dos-dr  */
	{ 57, 56, 55, 54, 53,  0,  0,  0,  0,  0,  0,  0, 0 },  /* ret ga-dos  */  
	{ 78, 63, 62, 60, 59,  0,  0,  0,  0,  0,  0,  0, 0 },  /* ret face-ga */ 
	{-59,-60,-62,-63,-78,  0,  0,  0,  0,  0,  0,  0, 0 }  /* ret dr-face */ 	

};

void processActors(void)
{
  objectParamsQuery params;
  int16 returnVar2[5];
  actorStruct* currentActor = &actorHead;

  while(currentActor->next)
  {
    currentActor = currentActor->next;

    if(!currentActor->freeze && ((currentActor->type == 0) || (currentActor->type == 1)))
    {
      getMultipleObjectParam(currentActor->overlayNumber,currentActor->var4,&params);

      if(((animationStart && !currentActor->flag) || (!animationStart && currentActor->x_dest != -1 && currentActor->y_dest != -1)) && (currentActor->type == 0))
      {
        // mouse animation
        if(!animationStart)
        {
          var34 = currentActor->x_dest;
          var35 = currentActor->y_dest;

          currentActor->x_dest = -1;
          currentActor->y_dest = -1;

          currentActor->flag = 1;
        }

        currentActor->pathId = computePathfinding(returnVar2, params.X, params.Y, var34, var35, currentActor->stepX, currentActor->stepY, currentActor->pathId);

        if(currentActor->pathId == -1)
        {
          if((currentActor->endDirection != -1) && (currentActor->endDirection != currentActor->startDirection))
          {
            currentActor->phase = ANIM_PHASE_STATIC_END;
            currentActor->nextDirection = currentActor->endDirection;
            currentActor->endDirection = -1;
            currentActor->counter = 0;
          }
          else
          {
            currentActor->pathId = -2;
            currentActor->flag = 0;
            currentActor->endDirection = -1;
            currentActor->phase = ANIM_PHASE_WAIT;
          }
        }
        else
        {
          currentActor->phase = ANIM_PHASE_STATIC;
          currentActor->counter = -1;
        }
      }
      else
      if((currentActor->type == 1) && (currentActor->x_dest != -1) && (currentActor->y_dest != -1))
      {
        // track animation
        currentActor->pathId = computePathfinding(returnVar2, params.X, params.Y, currentActor->x_dest, currentActor->y_dest, currentActor->stepX, currentActor->stepY, currentActor->pathId);

        currentActor->x_dest = -1;
        currentActor->y_dest = -1;

        if(currentActor->pathId == -1)
        {
          if((currentActor->endDirection != -1) && (currentActor->endDirection != currentActor->startDirection))
          {
            currentActor->phase = ANIM_PHASE_STATIC_END;
            currentActor->nextDirection = currentActor->endDirection;
            currentActor->endDirection = -1;
            currentActor->counter = 0;
          }
          else
          {
            currentActor->pathId = -2;
            currentActor->flag = 0;
            currentActor->endDirection = -1;
            currentActor->phase = ANIM_PHASE_WAIT;
          }
        }
        else
        {
          currentActor->phase = ANIM_PHASE_STATIC;
          currentActor->counter = -1;
        }
      }

      animationStart = 0;

      if(currentActor->pathId >= 0 || currentActor->phase == ANIM_PHASE_STATIC_END)
      {
        switch(currentActor->phase)
        {
        case ANIM_PHASE_STATIC_END:
        case ANIM_PHASE_STATIC:
          {
            if(currentActor->counter == -1 && currentActor->phase == ANIM_PHASE_STATIC)
            {
              affiche_chemin(currentActor->pathId, returnVar2);

              if(returnVar2[0] == -1)
              {
                currentActor->pathId = -2;
                currentActor->flag = 0;
                currentActor->endDirection = -1;
                currentActor->phase = ANIM_PHASE_WAIT;
                break;
              }

							currentActor->x = returnVar2[0];
							currentActor->y = returnVar2[1];
							currentActor->nextDirection = returnVar2[2];	
							currentActor->poly = returnVar2[4];
              currentActor->counter = 0;

              if (currentActor->startDirection == currentActor->nextDirection)
                currentActor->phase = ANIM_PHASE_MOVE;
            }

            if ((currentActor->counter>=0) && ((currentActor->phase==ANIM_PHASE_STATIC_END) || (currentActor->phase==ANIM_PHASE_STATIC)))
            {
              int newA;
							int inc = 1;
              int t_inc = currentActor->startDirection-1;

							if (t_inc<0)
                t_inc=3;

              if (currentActor->nextDirection==t_inc)
                inc=-1;

							if (inc>0)
                newA = raoul_stat[currentActor->startDirection][currentActor->counter++];
							else
                newA = raoul_invstat[currentActor->startDirection][currentActor->counter++];

							if (newA==0)	
							{
								currentActor->startDirection = currentActor->startDirection+inc;

								if (currentActor->startDirection>3)
                  currentActor->startDirection=0;

								if (currentActor->startDirection<0)
                  currentActor->startDirection=3;

								currentActor->counter=0;

								if (currentActor->startDirection==currentActor->nextDirection)	
								{
									if (currentActor->phase == ANIM_PHASE_STATIC)
                    currentActor->phase = ANIM_PHASE_MOVE;
									else
                    currentActor->phase = ANIM_PHASE_END;
								}
								else
								{	
									newA = raoul_stat[currentActor->startDirection][currentActor->counter++];

									if (inc==-1)
                    newA=-newA;

									set_anim(currentActor->overlayNumber,currentActor->var4,currentActor->start,params.X,params.Y,newA,currentActor->poly);
									break;
								}
							}
							else 
							{
								set_anim(currentActor->overlayNumber,currentActor->var4,currentActor->start,params.X,params.Y,newA,currentActor->poly);
								break;
							}
            }
            break;
          }
        case ANIM_PHASE_MOVE:
          {
            if (currentActor->counter>=1)
            {
								affiche_chemin(currentActor->pathId,returnVar2);
								if (returnVar2[0]==-1) 
								{ 
									if ((currentActor->endDirection==-1) || (currentActor->endDirection==currentActor->nextDirection))
									{
									currentActor->phase=ANIM_PHASE_END;
									}
									else
									{
										currentActor->phase = ANIM_PHASE_STATIC_END; 
										currentActor->nextDirection=currentActor->endDirection;
									}
									currentActor->counter=0; 
									break;
								}							
								else
								{	
									currentActor->x		= returnVar2[0];
									currentActor->y		= returnVar2[1];
									currentActor->nextDirection	= returnVar2[2];	
									currentActor->poly	= returnVar2[4];

									/*
									if (pl->next_dir!=pl->start_dir)
									{
										pl->phase=PHASE_STATIC; 
										pl->cnt=0;	
										break;
									}
									*/
								}
            }

            if (currentActor->phase==ANIM_PHASE_MOVE)
            {	
              int newA;

              currentActor->startDirection = currentActor->nextDirection;

              newA = raoul_move[currentActor->startDirection][currentActor->counter++];
              if (!newA) 
              {
                currentActor->counter=0;
                newA = raoul_move[currentActor->startDirection][currentActor->counter++];
              }
              set_anim(currentActor->overlayNumber,currentActor->var4,currentActor->start,currentActor->x,currentActor->y,newA,currentActor->poly);
              break;
            }

            break;
          }
        case ANIM_PHASE_END:
          {
            int newA = raoul_end[currentActor->startDirection][0];

			set_anim(currentActor->overlayNumber,currentActor->var4,currentActor->start,currentActor->x,currentActor->y,newA,currentActor->poly);

			currentActor->pathId = -2; 
			currentActor->phase=ANIM_PHASE_WAIT;
			currentActor->flag=0;	
			currentActor->endDirection=-1;	
            break;
          }
        default:
          {
            printf("Unimplemented currentActor->phase=%d in processActors()\n", currentActor->phase);
           // exit(1);
          }
        }
      }
    }
  }
}
} // End of namespace Cruise
