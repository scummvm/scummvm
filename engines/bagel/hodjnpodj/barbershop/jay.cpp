/*****************************************************************
 *
 *  MyFocusRect( CDC *pDC, CRect rect, int nDrawMode )
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Draws a rectangle which inverts the current pixels,
 *			thereby delineating the current area of focus.
 *
 *  FORMAL PARAMETERS:
 *
 *      CDC *pDC	The Device context in which the FocusRect is to be drawn
 *		CRect rect	The CRect object holding the location of the FocusRect
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      pMyPen and pMyBrush, global pointers to the Pen and Brush used
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void MyFocusRect( CDC *pDC, CRect rect, int nDrawMode )
{
	CBrush		*pMyBrush = NULL;					// New Brush
	CBrush 		*pOldBrush = NULL;                  // Pointer to old brush
	CPen		*pMyPen = NULL;						// New Pen
	CPen 		*pOldPen = NULL;                    // Pointer to old pen
	CPalette	*pPalOld = NULL;					// Pointer to old palette
	int			OldDrawMode;						// Holder for old draw mode

	pMyBrush = new CBrush();						// Construct new brush
	pMyPen = new CPen();							// Construct new pen

	LOGBRUSH lb;									// log brush type
	lb.lbStyle = BS_HOLLOW;							// Don't fill in area
	pMyBrush->CreateBrushIndirect( &lb );			// Create a new brush
	pMyPen->CreatePen( PS_INSIDEFRAME, HILITE_BORDER, RGBCOLOR_DARKRED );	// Create a new pen

	pPalOld = (*pDC).SelectPalette( pGamePalette, FALSE );	// Select in game palette
	(*pDC).RealizePalette();								// Use it
	pOldPen = pDC->SelectObject( pMyPen );      	// Select the new pen & save old
	pOldBrush = pDC->SelectObject( pMyBrush );  	// Select the new brush & save old
	OldDrawMode = pDC->SetROP2( nDrawMode );		// Set pen mode, saving old state
	pDC->Rectangle( rect );                     	// Draw the Rectangle to the DC
	pDC->SelectObject( pOldPen );               	// Select the old pen
	pDC->SelectObject( pOldBrush );             	// Select the old brush
	pDC->SetROP2( OldDrawMode );					// Set pen mode back to old state
	(*pDC).SelectPalette( pPalOld, FALSE );         // Select back the old palette

	if (pMyBrush != NULL ) {						// If the brush was constructed, delete it
		pMyBrush->DeleteObject();
		delete pMyBrush;
	}

	if (pMyPen != NULL ) {							// If the pen was constructed, delete it
		pMyPen->DeleteObject();
		delete pMyPen;
	}

} // End MyFocusRect()

