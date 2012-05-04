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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                        ...                  Spyral Software snc        *
 *        .             x#""*$Nu       -= We create much MORE than ALL =- *
 *      d*#R$.          R     ^#$o     様様様様様様様様様様様様様様様様様 *
 *    .F    ^$k         $        "$b                                      *
 *   ."       $b      u "$         #$L                                    *
 *   P         $c    :*$L"$L        '$k  Project: MPAL................... *
 *  d    @$N.   $.   d ^$b^$k         $c                                  *
 *  F   4  "$c  '$   $   #$u#$u       '$ Module:  Expression gestor...... *
 * 4    4k   *N  #b .>    '$N'*$u      *                                  *
 * M     $L   #$  $ 8       "$c'#$b.. .@ Author:  Giovanni Bajo.......... *
 * M     '$u   "$u :"         *$. "#*#"                                   *
 * M      '$N.  "  F           ^$k       Desc:    Gestisce le espressioni *
 * 4>       ^R$oue#             d                 matematiche............ *
 * '$          ""              @                  ....................... *
 *  #b                       u#                                           *
 *   $b                    .@"           OS: [ ] DOS  [X] WIN95  [ ] OS/2 *
 *    #$u                .d"                                              *
 *     '*$e.          .zR".@           様様様様様様様様様様様様様様様様様 *
 *        "*$$beooee$*"  @"M                  This source code is         *
 *             """      '$.?              Copyright (C) Spyral Software   *
 *                       '$d>                 ALL RIGHTS RESERVED         *
 *                        '$>          様様様様様様様様様様様様様様様様様 *
 *                                                                        *
 **************************************************************************/

#include "mpal.h"
#include "memory.h"
#include "mpaldll.h"
#include "stubs.h"
#include "tony/tony.h"

/*
#include "lzo1x.h"
*/

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Operazioni matematiche gestite
\****************************************************************************/

#define OP_MUL     ((1<<4)|0)
#define OP_DIV     ((1<<4)|1)
#define OP_MODULE  ((1<<4)|2)
#define OP_ADD     ((2<<4)|0)
#define OP_SUB     ((2<<4)|1)
#define OP_SHL     ((3<<4)|0)
#define OP_SHR     ((3<<4)|1)
#define OP_MINOR   ((4<<4)|0)
#define OP_MAJOR   ((4<<4)|1)
#define OP_MINEQ   ((4<<4)|2)
#define OP_MAJEQ   ((4<<4)|3)
#define OP_EQUAL   ((5<<4)|0)
#define OP_NOEQUAL ((5<<4)|1)
#define OP_BITAND  ((6<<4)|0)
#define OP_BITXOR  ((7<<4)|0)
#define OP_BITOR   ((8<<4)|0)
#define OP_AND     ((9<<4)|0)
#define OP_OR      ((10<<4)|0)


/****************************************************************************\
*       enum ExprListTypes
*       ------------------
* Description: Tipi di oggetto che possono essere contenuti in una struttura
*   EXPRESSION.
\****************************************************************************/

enum ExprListTypes
{
  ELT_NUMBER=1,
  ELT_VAR=2,
  ELT_PARENTH=3,
  ELT_PARENTH2=4
};


/****************************************************************************\
*       Structures
\****************************************************************************/

/****************************************************************************\
*       typedef EXPRESSION
*       ------------------
* Description: Struttura per gestire le operazioni matematiche
\****************************************************************************/

typedef struct {
	byte type;						// Tipo di oggetto (vedi enum ExprListTypes)
	byte unary;						// Unary operatore (NON SUPPORTATO)

	union {
		int num;                    // Numero (se type==ELT_NUMBER)
		char *name;                 // Nome variabile (se type==ELT_VAR)
		HGLOBAL son;                // Handle a espressione (type==ELT_PARENTH)
		byte *pson;					// Handle lockato (type==ELT_PARENTH2)
	} val;

	byte symbol;					// Simbolo matematico (vedi #define OP_*)

} EXPRESSION;
typedef EXPRESSION*     LPEXPRESSION;


/****************************************************************************\
*
* Function:     LPEXPRESSION DuplicateExpression(HGLOBAL h);
*
* Description:  Duplica un'espressione matematica. L'espressione duplicata
*               sara' formata da memoria non swappabile.
*
* Input:        HGLOBAL h               Handle dell'espressione originale
*
* Return:       Pointer all'espressione clone della prima
*
\****************************************************************************/

static byte *DuplicateExpression(HGLOBAL h) {
	int i, num;
	byte *orig, *clone;
	LPEXPRESSION one, two;

	orig=(byte *)GlobalLock(h);

	num=*(byte *)orig;
	one=(LPEXPRESSION)(orig+1);

	clone = (byte *)GlobalAlloc(GMEM_FIXED, sizeof(EXPRESSION)*num+1);
	two=(LPEXPRESSION)(clone+1);

	CopyMemory(clone,orig,sizeof(EXPRESSION)*num+1);

	for (i=0;i<num;i++) {
		if (one->type==ELT_PARENTH) {
			two->type=ELT_PARENTH2;
			two->val.pson=DuplicateExpression(two->val.son);
		}

		one++;
		two++;
	}

	GlobalUnlock(h);
	return clone;
}

static int Compute(int a, int b, byte symbol) {
	switch (symbol) {
	case OP_MUL:
		return a*b;
    case OP_DIV:
		return a/b;
    case OP_MODULE:
		return a%b;
    case OP_ADD:
		return a+b;
    case OP_SUB:
		return a-b;
    case OP_SHL:
		return a<<b;
    case OP_SHR:
		return a>>b;
    case OP_MINOR:
		return a<b;
    case OP_MAJOR:
		return a>b;
    case OP_MINEQ:
		return a<=b;
    case OP_MAJEQ:
		return a>=b;
    case OP_EQUAL:
		return a==b;
    case OP_NOEQUAL:
		return a!=b;
    case OP_BITAND:
		return a&b;
    case OP_BITXOR:
		return a^b;
    case OP_BITOR:
		return a|b;
    case OP_AND:
		return a&&b;
    case OP_OR:
		return a||b;
    default:
		mpalError=1;
		break;
	}
 
	return 0;
}

static void Solve(LPEXPRESSION one, int num) {
	LPEXPRESSION two, three;
	int j;

	while (num>1) {
		two=one+1;
		if ((two->symbol==0) || (one->symbol&0xF0) <= (two->symbol&0xF0)) {
			two->val.num=Compute(one->val.num,two->val.num,one->symbol);
			CopyMemory(one,two,(num-1)*sizeof(EXPRESSION));
			num--;
		} else {
			j=1;
			three=two+1;
			while ((three->symbol!=0) && (two->symbol&0xF0)>(three->symbol&0xF0)) {
				two++;
				three++;
				j++;
			}

			three->val.num=Compute(two->val.num,three->val.num,two->symbol);
			CopyMemory(two,three,(num-j-1)*sizeof(EXPRESSION));
			num--;
		}
	}
}


/****************************************************************************\
*
* Function:     int EvaluateAndFreeExpression(byte *expr);
*
* Description:  Calcola il risultato di una espressione matematica, sosti-
*               tuendo ad eventuali variabili il valore corrente.
*
* Input:        byte *expr             Pointer all'espressione duplicata
*                                       tramite DuplicateExpression
*
* Return:       Valore dell'espressione
*
\****************************************************************************/

static int EvaluateAndFreeExpression(byte *expr) {
	int i,num,val;
	LPEXPRESSION one,cur;

	num=*expr;
	one=(LPEXPRESSION)(expr+1);

	// 1) Sostituzioni delle variabili
	for (i=0,cur=one;i<num;i++,cur++) {
		if (cur->type==ELT_VAR) {
			cur->type=ELT_NUMBER;
			cur->val.num=varGetValue(cur->val.name);
		}
	}

	// 2) Sostituzioni delle parentesi (tramite ricorsione)
	for (i=0,cur=one;i<num;i++,cur++) {
		if (cur->type==ELT_PARENTH2) {
			cur->type=ELT_NUMBER;
			cur->val.num=EvaluateAndFreeExpression(cur->val.pson);
		}
	}

	// 3) Risoluzione algebrica
	Solve(one,num);
	val=one->val.num;
	GlobalFree(expr);

	return val;
}

/****************************************************************************\
*
* Function:     byte *ParseExpression(byte *buf, HGLOBAL *h);
*
* Description:  Esegue il parsing da file .MPC di un'espressione matematica.
*
* Input:        byte *buf              Buffer contenente l'espressione
*                                       compilata.
*               HGLOBAL *h              Pointer a un handle che, alla fine
*                                       dell'esecuzione, puntera' alla
*                                       zona di memoria contenete l'espres-
*                                       sione parsata
*
* Return:       Puntatore al buffer subito dopo l'espressione, o NULL in caso
*               di errore.
*
\****************************************************************************/

const byte *ParseExpression(const byte *lpBuf, HGLOBAL *h) {
	LPEXPRESSION cur;
	byte *start;
	uint32 num, i;

	num = *lpBuf;
	lpBuf++;

	if (num == 0)
		return NULL;

	*h = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, num * sizeof(EXPRESSION) + 1);
	if (*h==NULL)
		return NULL;

	start=(byte *)GlobalLock(*h);
	*start=(byte)num;

	cur=(LPEXPRESSION)(start+1);

	for (i=0;i<num;i++) {
		cur->type=*(lpBuf);
		cur->unary=*(lpBuf+1);
		lpBuf+=2;
		switch (cur->type) {
		case ELT_NUMBER:
			cur->val.num=*(int *)lpBuf;
			lpBuf+=4;
			break;

		case ELT_VAR:
			cur->val.name=(char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,(*lpBuf)+1);
			if (cur->val.name==NULL)
				return NULL;
			CopyMemory(cur->val.name,lpBuf+1,*lpBuf);
			lpBuf+=*lpBuf+1;
			break;

		case ELT_PARENTH:
			lpBuf=ParseExpression(lpBuf,&cur->val.son);
			if (lpBuf==NULL)
				return NULL;
			break;

		default:
			return NULL;
		}

		cur->symbol=*lpBuf;
		lpBuf++;

		cur++;
	}

	if (*lpBuf!=0)
		return NULL;

	lpBuf++;

	return lpBuf;
}


/****************************************************************************\
*
* Function:     int EvaluateExpression(HGLOBAL h);
*
* Description:  Calcola il valore di un'espressione matematica
*
* Input:        HGLOBAL h               Handle all'espressione
*
* Return:       Valore numerico
*
\****************************************************************************/

int EvaluateExpression(HGLOBAL h) {
	int ret;

	LockVar();
	ret=EvaluateAndFreeExpression(DuplicateExpression(h));
	UnlockVar();

	return ret;
}


/****************************************************************************\
*
* Function:     bool CompareExpressions(HGLOBAL h1, HGLOBAL h2);
*
* Description:  Confronta due espressioni matematiche tra loro
*
* Input:        HGLOBAL h1,h2            Espressioni da confrontare
*
* Return:       true se sono uguali, false se sono diverse
*
\****************************************************************************/

bool CompareExpressions(HGLOBAL h1, HGLOBAL h2) {
	int i,num1,num2;
	byte *e1, *e2;
	LPEXPRESSION one, two;

	e1=(byte *)GlobalLock(h1);
	e2=(byte *)GlobalLock(h2);

	num1=*(byte *)e1;
	num2=*(byte *)e2;

	if (num1 != num2) {
		GlobalUnlock(h1);
		GlobalUnlock(h2);
		return false;
	}

	one=(LPEXPRESSION)(e1+1);
	two=(LPEXPRESSION)(e2+1);

	for (i=0;i<num1;i++) {
		if (one->type!=two->type || (i!=num1-1 && one->symbol!=two->symbol)) {
			GlobalUnlock(h1);
			GlobalUnlock(h2);
			return false;
		}

		switch (one->type) {
		case ELT_NUMBER:
			if (one->val.num != two->val.num) {
				GlobalUnlock(h1);
				GlobalUnlock(h2);
				return false;
			}
			break;
		 
		case ELT_VAR:
			if (strcmp(one->val.name, two->val.name) != 0) {
				GlobalUnlock(h1);
				GlobalUnlock(h2);
				return false;
			}
			break;
	
		case ELT_PARENTH:
			if (!CompareExpressions(one->val.son,two->val.son)) {
				GlobalUnlock(h1);
				GlobalUnlock(h2);
				return false;
			}
			break;
		}

		one++; 
		two++;
	}

	GlobalUnlock(h1);
	GlobalUnlock(h2);
 
	return true;
}


} // end of namespace MPAL

} // end of namespace Tony
