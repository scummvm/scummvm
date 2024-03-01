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

#ifndef BAGEL_BOFLIB_GFX_DISPLAY_OBJECT_H
#define BAGEL_BOFLIB_GFX_DISPLAY_OBJECT_H

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/gui/window.h"

namespace Bagel {

#define DISPLAYOBJ_TOPMOST		0
#define DISPLAYOBJ_FOREGROUND	64
#define DISPLAYOBJ_MIDDLE		128
#define DISPLAYOBJ_BACKGROUND	192
#define DISPLAYOBJ_HINDMOST		255

#define DISPTYPE_OBJECT 0x0000
#define DISPTYPE_WINDOW (0x0001 | DISPTYPE_OBJECT)
#define DISPTYPE_DIALOG (0x0002 | DISPTYPE_WINDOW)
#define DISPTYPE_BUTTON (0x0004 | DISPTYPE_WINDOW)
#define DISPTYPE_SCROLL (0x0008 | DISPTYPE_WINDOW)

class CBofDisplayObject: public CLList, public CBofError, public CBofObject {

    public:
        CBofDisplayObject();
        virtual ~CBofDisplayObject();

        ERROR_CODE LoadImage(const CHAR *pszFileName);
        ERROR_CODE CreateImage(INT dx, INT dy);
        ERROR_CODE ReleaseImage(VOID);

        ERROR_CODE Paint(const INT x, const INT y);
        ERROR_CODE Paint(CBofPoint &cPoint)         { return(Paint(cPoint.x, cPoint.y)); }

        ERROR_CODE BatchPaint(const INT x, const INT y);
        ERROR_CODE BatchPaint(CBofPoint &cPoint)    { return(BatchPaint(cPoint.x, cPoint.y)); }

        ERROR_CODE Erase(VOID);
        ERROR_CODE BatchErase(VOID);

        ERROR_CODE SetPosition(INT x, INT y);
        ERROR_CODE SetPosition(const CBofPoint &cPoint) { return(SetPosition(cPoint.x, cPoint.y)); }
        CBofPoint  GetPosition(VOID)                { return(m_cPosition); }

        ERROR_CODE SetZOrder(INT nValue);
        INT        GetZOrder(VOID)                  { return(m_nZOrder); }

        VOID       SetSize(const CBofSize &cSize)   { m_cSize = cSize; }
        CBofSize   GetSize(VOID)                    { return(m_cSize); }

        INT        Width(VOID)                      { return(m_cSize.cx); }
        INT        Height(VOID)                     { return(m_cSize.cy); }

        CBofRect   GetRect(VOID)                    { return(CBofRect(m_cPosition, m_cSize)); }

        ERROR_CODE LinkChild(CBofDisplayObject *pChild);
        ERROR_CODE UnlinkChild(CBofDisplayObject *pChild);

        BOOL       IsMasked(VOID)           { return(m_nMaskColor != NOT_TRANSPARENT); }
        VOID       SetMaskColor(INT nColor) { m_nMaskColor = nColor; }
        INT        GetMaskColor(VOID)       { return(m_nMaskColor); }

        BOOL       IsType(ULONG lType)      { return((m_lType & lType) != 0); }

        CBofDisplayObject *GetNext(VOID)    { return((CBofDisplayObject *)m_pNext); }
        CBofDisplayObject *GetPrev(VOID)    { return((CBofDisplayObject *)m_pPrev); }

        ERROR_CODE AddChild(CBofDisplayObject *pChildObj);

        CBofPoint  LocalToGlobal(const CBofPoint &cPoint);
        CBofPoint  GlobalToLocal(const CBofPoint &cPoint);

        CBofRect   LocalToGlobal(const CBofRect &cRect);
        CBofRect   GlobalToLocal(const CBofRect &cRect);

        CBofDisplayObject *GetChildFromPoint(const CBofPoint &cPoint);

        virtual ERROR_CODE OnPaint(CBofBitmap *pDestBmp, CBofRect *pDirtyRect);

        static ERROR_CODE Initialize(VOID);
        static ERROR_CODE CleanUp(VOID);

        static ERROR_CODE ReMapPalette(CBofPalette *pPalette);

        static CBofDisplayObject *GetMain(VOID) { return(m_pMainDisplay); }

        static ERROR_CODE AddToDirtyRect(CBofRect *pRect);
        static ERROR_CODE UpdateDirtyRect(VOID);
        static VOID ClearDirtyRect(VOID)    { m_cDirtyRect.SetRectEmpty(); }

       	virtual VOID OnBofButton(CBofObject *pButton, INT nExtraInfo) {}


    protected:
        static CBofRect     m_cDirtyRect;
        static CBofDisplayObject *m_pMainDisplay;

        CBofSize            m_cSize;            // Object's Image size
        CBofPoint           m_cPosition;        // position (Local coordinates)

        CBofDisplayObject  *m_pParent;          // one parent

        CBofDisplayObject  *m_pDisplayList;     // many children
        CBofBitmap         *m_pImage;

        INT                 m_nZOrder;
        INT                 m_nMaskColor;

        ULONG               m_lType;

        BOOL                m_bPositioned;
        BOOL                m_bCreated;
};

} // namespace Bagel

#endif
