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

#include "bagel/boflib/gui/display_object.h"
#include "bagel/boflib/app.h"

namespace Bagel {

// static members
//
CBofRect CBofDisplayObject::m_cDirtyRect;
CBofDisplayObject *CBofDisplayObject::m_pMainDisplay = NULL;


CBofDisplayObject::CBofDisplayObject() {
	m_bPositioned = FALSE;
	m_nMaskColor = NOT_TRANSPARENT;
	m_pImage = NULL;
	m_pParent = NULL;
	m_pDisplayList = NULL;
	m_nZOrder = DISPLAYOBJ_TOPMOST;

	m_lType = DISPTYPE_OBJECT;
}


CBofDisplayObject::~CBofDisplayObject() {
	Assert(IsValidObject(this));

	ReleaseImage();
}


ERROR_CODE CBofDisplayObject::CreateImage(INT dx, INT dy) {
	Assert(IsValidObject(this));

	// Release any previous bitmap
	ReleaseImage();

	// Create the bitmap
	//
	if ((m_pImage = new CBofBitmap(dx, dy, CBofApp::GetApp()->GetPalette())) != NULL) {

		m_cSize.cx = dx;
		m_cSize.cy = dy;

	} else {
		ReportError(ERR_MEMORY);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::LoadImage(const CHAR *pszFileName) {
	Assert(IsValidObject(this));

	// Release any previous bitmap
	ReleaseImage();

	// Load new bitmap
	//
	if ((m_pImage = new CBofBitmap(pszFileName)) != NULL) {

		m_cSize.cx = m_pImage->Width();
		m_cSize.cy = m_pImage->Height();

	} else {
		ReportError(ERR_MEMORY);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::ReleaseImage(VOID) {
	Assert(IsValidObject(this));

	if (m_pImage != NULL) {
		delete m_pImage;
		m_pImage = NULL;
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::SetPosition(INT x, INT y) {
	Assert(IsValidObject(this));

	m_bPositioned = TRUE;
	m_cPosition.x = x;
	m_cPosition.y = y;

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::Paint(const INT x, const INT y) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		BatchPaint(x, y);

		UpdateDirtyRect();
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::Erase(VOID) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		BatchErase();

		UpdateDirtyRect();
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::BatchErase(VOID) {
	Assert(IsValidObject(this));

	// If there is something to erase...
	//
	if (m_bPositioned)  {

		m_bPositioned = FALSE;

		// invalidate it's current area
		//
		CBofPoint cScreenPos;
		CBofRect cScreenArea;

		cScreenPos = m_cPosition;
		if (m_pParent != NULL)
			cScreenPos = m_pParent->LocalToGlobal(m_cPosition);

		cScreenArea.SetRect(cScreenPos.x, cScreenPos.y, cScreenPos.x + m_cSize.cx - 1, cScreenPos.y + m_cSize.cy - 1);

		AddToDirtyRect(&cScreenArea);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::BatchPaint(const INT x, const INT y) {
	Assert(IsValidObject(this));

	CBofRect cDstRect;
	CBofPoint cScreenPos, cLocalPos(x, y);

	cScreenPos = cLocalPos;
	if (m_pParent != NULL) {
		// Target position is relative to my parent
		cScreenPos = m_pParent->LocalToGlobal(cLocalPos);
	}

	// Calculate destination rectangle (In global coordinates)
	//
	cDstRect.SetRect(cScreenPos.x, cScreenPos.y, cScreenPos.x + m_cSize.cx - 1, cScreenPos.y + m_cSize.cy - 1);

	// add the destination position to the dirty rectangle list

	AddToDirtyRect(&cDstRect);

	// If the sprite is already on screen, then we must also add it's old
	// current location to the dirty rect list so that it is erase properly
	//
	if (m_bPositioned)  {

		cScreenPos = m_cPosition;
		if (m_pParent != NULL) {
			// Previous position is relative to my parent
			cScreenPos = m_pParent->LocalToGlobal(m_cPosition);
		}
		cDstRect.SetRect(cScreenPos.x, cScreenPos.y, cScreenPos.x + m_cSize.cx - 1, cScreenPos.y + m_cSize.cy - 1);

		AddToDirtyRect(&cDstRect);
	}

	// now establish this object's new position (Given local coordinates)
	SetPosition(x, y);

	return (m_errCode);
}


CBofPoint CBofDisplayObject::LocalToGlobal(const CBofPoint &cPoint) {
	Assert(IsValidObject(this));

	CBofPoint cGlobalPos;
	CBofDisplayObject *pObj;

	cGlobalPos = cPoint;

	pObj = this;
	while (pObj != NULL) {

		cGlobalPos += pObj->m_cPosition;

		pObj = pObj->m_pParent;
	}

	return (cGlobalPos);
}


CBofPoint CBofDisplayObject::GlobalToLocal(const CBofPoint &cPoint) {
	Assert(IsValidObject(this));

	CBofPoint cLocalPos;
	CBofDisplayObject *pObj;

	cLocalPos = cPoint;

	pObj = this;
	while (pObj != NULL) {

		cLocalPos -= pObj->m_cPosition;

		pObj = pObj->m_pParent;
	}

	return (cLocalPos);
}

CBofRect CBofDisplayObject::LocalToGlobal(const CBofRect &cRect) {
	Assert(IsValidObject(this));
	CBofRect cGlobalRect(LocalToGlobal(cRect.TopLeft()), cRect.Size());

	return (cGlobalRect);
}


CBofRect CBofDisplayObject::GlobalToLocal(const CBofRect &cRect) {
	Assert(IsValidObject(this));

	CBofRect cLocalRect(GlobalToLocal(cRect.TopLeft()), cRect.Size());

	return (cLocalRect);
}


ERROR_CODE CBofDisplayObject::AddToDirtyRect(CBofRect *pRect) {
	Assert(pRect != NULL);

	CBofRect cRect;

	if (m_cDirtyRect.IsRectEmpty()) {
		cRect = *pRect;
	} else {
		cRect.UnionRect(&m_cDirtyRect, pRect);
	}
	m_cDirtyRect = cRect;

	return (ERR_NONE);
}

ERROR_CODE CBofDisplayObject::UpdateDirtyRect(VOID) {
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	Assert(m_pMainDisplay != NULL);

	// We won't do anything unless we actually have an area to update
	//
	if (!m_cDirtyRect.IsRectEmpty()) {

		CBofRect cTempRect;
		CBofDisplayObject *pDisplayObject;
		CBofBitmap *pBmp;

		pBmp = m_pMainDisplay->m_pImage;

		pBmp->FillRect(&m_cDirtyRect, COLOR_BLACK);

		// Walk thru my list of children and tell them to update
		// any of their areas that intersect my dirty area.
		//
		pDisplayObject = m_pMainDisplay->m_pDisplayList;
		while (pDisplayObject != NULL) {

			// If this child intersects the dirty rectangle
			//
			if (cTempRect.IntersectRect(pDisplayObject->GetRect(), m_cDirtyRect)) {
				//cTempRect -= cTempRect.TopLeft();

				pDisplayObject->OnPaint(pBmp, &cTempRect);
			}

			pDisplayObject = pDisplayObject->GetNext();
		}

		//
		// Now, update the actual screen with my final image + children
		//
		errCode = pBmp->Paint(CBofApp::GetApp()->GetActualWindow(), &m_cDirtyRect, &m_cDirtyRect);

		ClearDirtyRect();
	}

	return (errCode);
}

ERROR_CODE CBofDisplayObject::OnPaint(CBofBitmap *pDestBmp, CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));
	Assert(pDestBmp != NULL);

	// If no previous error
	//
	if (!ErrorOccurred()) {

		// We won't do anything unless we actually have an area to update
		//
		if (pDirtyRect != NULL && !pDirtyRect->IsRectEmpty()) {

			// As long as this object is visible...
			//
			if (m_bPositioned) {

				CBofRect cMyDirtyRect;

				CBofDisplayObject *pDisplayObject;
				//CBofPoint cGlobalPos;

				// Convert to global coordinate system
				//
				//cGlobalPos = LocalToGlobal(pDirtyRect->TopLeft());
				CBofRect cGlobalRect;
				cGlobalRect = LocalToGlobal(*pDirtyRect);

				//CBofRect cMyRect(cGlobalPos, pDirtyRect->Size());

				// Show my image as the background behind any of my children
				//
				if (m_pImage != NULL) {
					m_pImage->Paint(pDestBmp, &cGlobalRect, pDirtyRect, m_nMaskColor);
				}

				// Walk thru my list of children and tell them to update
				// any of their areas that intersect my dirty area.
				//
				pDisplayObject = m_pDisplayList;
				while (pDisplayObject != NULL) {

					if (cMyDirtyRect.IntersectRect(*pDirtyRect, pDisplayObject->GetRect())) {
						cMyDirtyRect -= pDisplayObject->GetRect().TopLeft();

						pDisplayObject->OnPaint(pDestBmp, &cMyDirtyRect);
					}

					pDisplayObject = pDisplayObject->GetNext();
				}
			}
		}
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::SetZOrder(INT nValue) {
	Assert(IsValidObject(this));
	Assert(nValue >= DISPLAYOBJ_TOPMOST && nValue <= DISPLAYOBJ_HINDMOST);

	// Cannot change the Z-Order of THE top level DisplayObject
	Assert(m_pParent != NULL);

	m_nZOrder = nValue;

	if (m_pParent != NULL) {
		CBofDisplayObject *pParent;

		pParent = m_pParent;

		// Relinking this object after setting it's new Z-Order will
		// add it to the correct Z-Order sorted location (Insertion Sort)
		//
		pParent->UnlinkChild(this);
		pParent->LinkChild(this);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::LinkChild(CBofDisplayObject *pChild) {
	Assert(IsValidObject(this));
	Assert(pChild != NULL);

	if (pChild != NULL) {

		if (m_pDisplayList != NULL) {

			CBofDisplayObject *pCurObj, *pLastObj;

			switch (m_nZOrder) {

			case DISPLAYOBJ_TOPMOST:
				m_pDisplayList->AddToTail(pChild);
				break;

			case DISPLAYOBJ_HINDMOST:
				m_pDisplayList->AddToHead(pChild);
				m_pDisplayList = pChild;
				break;

			default:

				pLastObj = pCurObj = m_pDisplayList;

				while (pCurObj != NULL && pCurObj->m_nZOrder > m_nZOrder) {
					pLastObj = pCurObj;
					pCurObj = pCurObj->GetNext();
				}
				pLastObj->Insert(pChild);
				break;
			}

		} else {
			m_pDisplayList = pChild;
		}

		// m_pDisplayList must always point to the head of the linked list
		Assert(m_pDisplayList == (CBofDisplayObject *)m_pDisplayList->GetHead());

		pChild->m_pParent = this;
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::UnlinkChild(CBofDisplayObject *pChild) {
	Assert(IsValidObject(this));
	Assert(pChild != NULL);

	if (pChild != NULL) {

		// I had better be my child's parent
		Assert(pChild->m_pParent == this);

		// Give child up for adoption
		pChild->m_pParent = NULL;

		// Keep track of my other children
		//
		if (m_pDisplayList == pChild)
			m_pDisplayList = pChild->GetNext();

		// C ya
		pChild->Delete();
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayObject::Initialize(VOID) {
	CBofApp *pApp;
	INT dx, dy;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pApp = CBofApp::GetApp()) != NULL) {
		dx = pApp->GetActualWindow()->Width();
		dy = pApp->GetActualWindow()->Height();

		// Allocate the offscreen buffer we will be using for all
		// display objects
		//
		if ((m_pMainDisplay = new CBofDisplayObject) != NULL) {
			m_pMainDisplay->CreateImage(dx, dy);
			m_pMainDisplay->SetPosition(0, 0);

		} else {
			errCode = ERR_MEMORY;
		}
	}

	return (errCode);
}


ERROR_CODE CBofDisplayObject::CleanUp(VOID) {
	// Destroy top-level object container
	//
	if (m_pMainDisplay != NULL) {
		delete m_pMainDisplay;
		m_pMainDisplay = NULL;
	}

	return (ERR_NONE);
}


ERROR_CODE CBofDisplayObject::ReMapPalette(CBofPalette *pPalette) {
	Assert(pPalette != NULL);

	if (m_pMainDisplay != NULL && m_pMainDisplay->m_pImage != NULL) {

		m_pMainDisplay->m_pImage->ReMapPalette(pPalette);
	}

	return (ERR_NONE);
}


CBofDisplayObject *CBofDisplayObject::GetChildFromPoint(const CBofPoint &cPoint) {
	Assert(IsValidObject(this));

	CBofRect cRect;
	CBofDisplayObject *pWindow, *pNewWin;
	CBofDisplayObject *pObj;

	// Assume no window found
	pWindow = NULL;

	cRect = LocalToGlobal(GetRect());

	if (cRect.PtInRect(cPoint)) {

		if (IsType(DISPTYPE_WINDOW)) {
			pWindow = this;
		}

		pObj = m_pDisplayList;
		while (pObj != NULL) {

			if (pObj->IsType(DISPTYPE_WINDOW)) {

				if ((pNewWin = pObj->GetChildFromPoint(cPoint)) != NULL) {
					pWindow = pNewWin;
				}
			}
			pObj = pObj->GetNext();
		}
	}

	return (pWindow);
}

} // namespace Bagel
