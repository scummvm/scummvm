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
 *  F   4  "$c  '$   $   #$u#$u       '$ Module:  Expression gestor heade *
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

#ifndef MPAL_EXPR_H
#define MPAL_EXPR_H

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Prototipi di funzione
\****************************************************************************/

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

const byte *ParseExpression(const byte *lpBuf, HGLOBAL *h);


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

int EvaluateExpression(HGLOBAL h);


/****************************************************************************\
*
* Function:     bool CompareExpressions(HGLOBAL h1, HGLOBAL h2);
*
* Description:  Confronta due espressioni matematiche tra loro
*
* Input:        HGLOBAL h1,h2            Espressioni da confrontare
*
* Return:       TRUE se sono uguali, FALSE se sono diverse
*
\****************************************************************************/

bool CompareExpressions(HGLOBAL h1, HGLOBAL h2);


} // end of namespace MPAL

} // end of namespace Tony

#endif
