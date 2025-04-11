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

#include "bagel/hodjnpodj/metagame/boardgame/gtl_data.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

int CGtlData::compile(const char *xpszPathName) {
	int iError = 0;		// error code
	bool bDone;			// loop termination variable
	char szOut[100];

	Common::strcpy_s(m_szGtlFile, xpszPathName);

	if (!m_xpGtlFile.open(m_szGtlFile)) {
		iError = 100;
		goto cleanup;
	}

	bDone = false;		// not done yet

	while (!bDone) {
		if (!CGtlData::ReadLine())  // read line, check for error
			(void)CGtlData::parseLine();

		if (m_bEof)
			// end of file
			bDone = true;
	}

cleanup:

	m_xpGtlFile.close();

	if (iError) {
		Common::sprintf_s(szOut, "CGtlData::Compile -- Error code %i", iError);
		CGtlData::ErrorMsg(NULL, szOut);
	}

	return iError != 0;
}

bool CGtlData::parseLine() {
	char szBuf[MAX_LABEL_LENGTH];
	int iError = 0;		// error code
	CLexElement *xpLxel;		// lexical element block
	CNode *lpNode, *lpLinkNode;  // ptrs to node objects
	CMap *lpMap;	// bitmap pointer
	CSectorTable *pSectorEntry;
	char *xpStr;
	int iLink;		// index of link node
	int iTmp;

	m_xpLexLabel = NULL;		// no label yet

	xpLxel = &m_cLexElts[0];	// point to first lex element

	// save label pointer, move past colon
	if (xpLxel->m_iType == LXT_IDENT && (xpLxel + 1)->m_iType == LXT_COLON)
		m_xpLexLabel = xpLxel, xpLxel += 2;

	// if not a recognized identifer
	if (xpLxel->m_iType != LXT_IDENT || xpLxel->m_iVal == 0) {

		iError = 101;
		CGtlData::ErrorMsg(xpLxel, "Unrecognized statement type.");
		goto cleanup;
	}

	switch (xpLxel->m_iVal) {

	case KT_DIRECTORY:      // specify bitmap director

		// if there's a label
		if (m_xpLexLabel)
			ErrorMsg(xpLxel, "Label ignored.");

		xpLxel = ParseString(xpLxel, LXT_IDENT, m_szBmpDirectory, NULL);

		// if there's a directory
		//
		if (m_szBmpDirectory[0]) {

			// point to last char
			xpStr = m_szBmpDirectory + (strlen(m_szBmpDirectory) - 1);

			// if it's not a backslash
			if (*xpStr != '\\') {

				// insert backslash
				*++xpStr = '\\';
				*++xpStr = 0;
			}
		}
		break;

	case KT_BMP:
		if (m_iMaps >= MAX_BITMAPS - 1) {
			ErrorMsg(xpLxel, "Too many bitmaps -- increase MAX_BITMAPS");
			iError = 130;
			goto cleanup;
		}

		// point to first/next bitmap
		lpMap = m_lpMaps + m_iMaps++;

		if (!lpMap->m_lpcBgbObject && ((lpMap->m_lpcBgbObject = new CBgbObject) == NULL)) {
			ErrorMsg(xpLxel, "Can't allocate BGB");
			iError = 131;  // can't allocate
			goto cleanup;
		}

		// if there's a label
		if (m_xpLexLabel) {

			// copy label
			Common::strcpy_s(lpMap->m_szLabel, MAX_LABEL_LENGTH - 1, &m_szStringList[m_xpLexLabel->m_iStringListPos]);
		}

		xpLxel = ParseString(xpLxel, LXT_IDENT, lpMap->m_szFilename, NULL);

		lpMap->m_bRelocatable = true;  // default
		while (xpLxel->m_iType == LXT_IDENT) {
			switch (xpLxel->m_iVal) {
			case KT_LEFT:
			case KT_RIGHT:
			case KT_ABOVE:
			case KT_BELOW:
				if (lpMap->m_iRelationType) {
					iError = 100;
					ErrorMsg(xpLxel, "Duplicate relative position");
					goto cleanup;
				}
				lpMap->m_iRelationType = xpLxel->m_iVal;
				if ((++xpLxel)->m_iType == LXT_IDENT && xpLxel->m_iVal == KT_PREVIOUS) {
					lpMap->m_iRelation = m_iMaps - 2;

					// search for label, and test
				} else if (GetLabel(xpLxel, false, lpMap->m_iRelation)) {
					goto cleanup;
				}

				++xpLxel;
				break;

			case KT_NODE:
				if (lpMap->m_iRelationType) {
					iError = 101;
					ErrorMsg(xpLxel, "Duplicate relative position");
					goto cleanup;
				}
				lpMap->m_iRelationType = xpLxel->m_iVal;

				// search for label, and test
				if (GetLabel(++xpLxel, true, lpMap->m_iRelation))
					goto cleanup;
				++xpLxel;
				break;

			case KT_PALETTE:
				lpMap->m_bPalette = true;
				++xpLxel;
				break;

			case KT_BORDER:
				lpMap->m_bRelocatable = false;
				++xpLxel;
				break;

			case KT_OVERLAY:
				lpMap->m_bOverlay = true;
				++xpLxel;
				break;

			case KT_MASKED:
				lpMap->m_bMasked = lpMap->m_lpcBgbObject->m_bMasked = true;
				++xpLxel;
				break;

			case KT_FOREGROUND:
				lpMap->m_lpcBgbObject->m_nZOrder = SPRITE_FOREGROUND;
				++xpLxel;
				break;

			case KT_BACKGROUND:
				lpMap->m_lpcBgbObject->m_nZOrder = SPRITE_BACKGROUND;
				++xpLxel;
				break;

			case KT_MIDDLE:
				lpMap->m_lpcBgbObject->m_nZOrder = SPRITE_MIDDLE;
				++xpLxel;
				break;

			case KT_ANIMATED:
				lpMap->m_lpcBgbObject->m_bAnimated = true;
				xpLxel = ParseInteger(++xpLxel, LXT_LPR, lpMap->m_lpcBgbObject->m_nCels);
				if (xpLxel->m_iType == LXT_RPR)
					++xpLxel;
				break;

			case KT_FREQUENCY:
				xpLxel = ParseInteger(++xpLxel, LXT_LPR, (int &)lpMap->m_lpcBgbObject->m_nFreq);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, (int &)lpMap->m_lpcBgbObject->m_nRepeat);
				if (xpLxel->m_iType == LXT_RPR)
					++xpLxel;
				break;

			case KT_SIZE:
				xpLxel = ParseInteger(++xpLxel, LXT_LPR, lpMap->m_lpcBgbObject->m_cSize.cx);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, lpMap->m_lpcBgbObject->m_cSize.cy);
				if (xpLxel->m_iType == LXT_RPR)
					++xpLxel;
				break;

			case KT_METAGAME:
				lpMap->m_bMetaGame = true;
				++xpLxel;
				break;

			case KT_SPRITE: // sprite implies overlay
				lpMap->m_bSprite = lpMap->m_bOverlay = true;
				++xpLxel;
				break;

			case KT_POSITION:
				lpMap->m_bPositionDetermined = true;
				lpMap->m_bPositionSpecified = true;
				lpMap->m_lpcBgbObject->m_bSpecial = true;
				xpLxel = ParseInteger(++xpLxel, LXT_LPR, lpMap->m_lpcBgbObject->m_crPosition.cx);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, lpMap->m_lpcBgbObject->m_crPosition.cy);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, lpMap->m_lpcBgbObject->m_cSize.cx);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, lpMap->m_lpcBgbObject->m_cSize.cy);
				if (xpLxel->m_iType == LXT_RPR)
					++xpLxel;
				break;

			default:
				iError = 102;
				ErrorMsg(xpLxel, "Unrecognized keyword.");
				goto cleanup;
				// break ;
			}
		}
		break;

	case KT_NODE:
		if (m_iNodes >= MAX_NODES - 1) {
			ErrorMsg(xpLxel, "Too many nodes -- increase MAX_NODES");
			iError = 700;
			goto cleanup;
		}

		// point to first/next node
		lpNode = m_lpNodes + m_iNodes++;

		// if there's a label
		if (m_xpLexLabel) {

			// copy label
			Common::strcpy_s(lpNode->m_szLabel, MAX_LABEL_LENGTH - 1, &m_szStringList[m_xpLexLabel->m_iStringListPos]);
		}

		++xpLxel;
		lpNode->m_bRelocatable = true; // default

		while (xpLxel->m_iType == LXT_IDENT) {

			switch (xpLxel->m_iVal) {

			case KT_BMP:
				if (lpNode->m_bRelative) {
					iError = 701;  // ****
					ErrorMsg(xpLxel, "Duplicate relative position");
					goto cleanup;
				}
				lpNode->m_bRelative = true;
				if (GetLabel(++xpLxel, false, lpNode->m_iBitmap)) {
					// search for label, and test
					goto cleanup;
				}

				xpLxel = ParseInteger(++xpLxel, LXT_LPR, lpNode->m_iX);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, lpNode->m_iY);
				if (xpLxel->m_iType == LXT_RPR)
					++xpLxel;
				break;

			case KT_POSITION:
				xpLxel = ParseInteger(++xpLxel, LXT_LPR, lpNode->m_iX);
				xpLxel = ParseInteger(xpLxel, LXT_COMMA, lpNode->m_iY);
				if (xpLxel->m_iType == LXT_RPR)
					++xpLxel;
				break;

			case KT_LINKPREV:
				if (!m_lpLastNode) {
					iError = 702;
					ErrorMsg(xpLxel, "No previous node.");
					goto cleanup;
				}

				AddLink(lpNode, m_lpLastNode);
				++xpLxel;
				break;

			case KT_BORDER:
				lpNode->m_bRelocatable = false;
				++xpLxel;
				break;

			case KT_PASSTHRU:
				lpNode->m_bWgtSpec = true;
				lpNode->m_iWeight = 0;
				++xpLxel;
				break;

			case KT_WEIGHT:
				lpNode->m_bWgtSpec = true;
				xpLxel = ParseInteger(xpLxel, LXT_IDENT, iTmp);
				lpNode->m_iWeight = (byte)iTmp;
				break;

			case KT_SENSITIVITY:
				lpNode->m_bSenSpec = true;
				xpLxel = ParseInteger(xpLxel, LXT_IDENT, iTmp);
				lpNode->m_iSensitivity = (byte)iTmp;
				break;

			case KT_SECTOR:

				xpLxel = ParseString(xpLxel, LXT_IDENT, szBuf, NULL);

				// find this node's sector
				//
				lpNode->m_iSector = MG_SECTOR_ANY;
				pSectorEntry = CMgStatic::cSectorTable;
				while (pSectorEntry->m_iSectorCode != 0) {
					if (scumm_stricmp(pSectorEntry->m_lpszLabel, szBuf) == 0) {
						lpNode->m_iSector = pSectorEntry->m_iSectorCode;
						break;
					}
					pSectorEntry++;
				}
				break;

			default:
				iError = 703;
				ErrorMsg(xpLxel, "Unrecognized identifier.");
				goto cleanup;
				// break ;
			}
		}
		if (!lpNode->m_bWgtSpec)
			lpNode->m_iWeight = 1; // default weight is 1
		m_lpLastNode = lpNode; // that's new last node
		break;

	case KT_LINK:

		// get last node
		if ((lpNode = m_lpLastNode) == NULL) {
			iError = 101;
			goto cleanup;
		}

		// if there's a label
		if (m_xpLexLabel)
			ErrorMsg(xpLxel, "Label ignored.");

		// search for label, and test
		if (GetLabel(++xpLxel, true, iLink))
			goto cleanup;

		++xpLxel;
		lpLinkNode = m_lpNodes + iLink;     // set last node
		AddLink(lpNode, lpLinkNode);
		break;

	case KT_EOF:
		m_bEof = true;
		++xpLxel;
		break;

	default:
		iError = 102;
		CGtlData::ErrorMsg(xpLxel, "Unrecognized statement type.");
		goto cleanup;
		// break ;
	}

	if (xpLxel->m_iType != LXT_EOX) {
		iError = 110;
		CGtlData::ErrorMsg(xpLxel, "Error - end of line expected.");
		goto cleanup;
	}

cleanup:

	if (iError) {
		char szMsg[100];   // error message text
		Common::sprintf_s(szMsg, "CGtlData::parseLine -- Error code %i.", iError);
		CGtlData::ErrorMsg(xpLxel, szMsg);
	}

	return iError != 0;
}

CLexElement *CGtlData::ParseInteger(CLexElement *xpLxel, int iPrevType, int &iValue) {
	int iSign = +1;		// plus (+1) or minus (-1) sign

	if (xpLxel->m_iType == iPrevType) {
		if ((++xpLxel)->m_iType == LXT_PLUS)
			iSign = +1, ++xpLxel;

		else if (xpLxel->m_iType == LXT_MINUS)
			iSign = -1, ++xpLxel;

		if (xpLxel->m_iType == LXT_INTEGER)
			iValue = iSign * xpLxel++->m_iVal;
	}

	return xpLxel ;
}

CLexElement *CGtlData::ParseString(CLexElement *xpLxel, int iPrevType, char *lpszValue, int *xpiValue) {
	if (xpiValue)
		*xpiValue = 0;		// default -- zero keyword value

	if (xpLxel->m_iType == iPrevType && ((++xpLxel)->m_iType == LXT_IDENT || xpLxel->m_iType == LXT_STRING)) {

		// copy over the text string
		if (lpszValue)
			Common::strcpy_s(lpszValue, 255, &m_szStringList[xpLxel->m_iStringListPos]);

		if (xpiValue)
			*xpiValue = xpLxel->m_iVal;

		++xpLxel;  // and move to lexeme beyond
	}

	return xpLxel;
}

//* CGtlData::GetLabel -- get bitmap or node label
bool CGtlData::GetLabel(CLexElement *xpLxel, bool bNode, int &iIndex)
// xpLxel -- pointer to lexeme being tested
// bNode -- false for bitmaps, true for nodes
// iIndex -- output: index of bitmap or node for label
// returns: true if error (label not found), false otherwise
{
	int iError = 0;		// error code

	if (xpLxel->m_iType != LXT_IDENT) {
		ErrorMsg(xpLxel, "Invalid label.");
		iError = 100;
		goto cleanup;
	}

	if ((iError = GetLabel(&m_szStringList[xpLxel->m_iStringListPos], bNode, iIndex)) != 0) {
		if (bNode)
			ErrorMsg(xpLxel, "Can't find node label.");
		else
			ErrorMsg(xpLxel, "Can't find bitmap label.");
		goto cleanup;
	}

cleanup:
	return iError != 0;
}


//* CGtlData::GetLabel -- get bitmap or node label
bool CGtlData::GetLabel(char *lpszLabel, bool bNode, int &iIndex)
// lpszLabel -- pointer to label string for bitmap/node being sought
// bNode -- false for bitmaps, true for nodes
// iIndex -- output: index of bitmap or node for label
// returns: true if error (label not found), false otherwise
{
	int iError = 0;		// error code

	// if looking for node label
	if (bNode) {
		// loop through nodes, searching for label
		for (iIndex = 0; iIndex < m_iNodes &&
			(strcmp(m_lpNodes[iIndex].m_szLabel, lpszLabel) || m_lpNodes[iIndex].m_bDeleted); ++iIndex) {
		}

		if (iIndex >= m_iNodes) {
			iError = 101;
			goto cleanup;
		}

		// looking for a bitmap label
		//
	} else {

		// loop through bitmaps, searching for label
		for (iIndex = 0; iIndex < m_iMaps && strcmp(m_lpMaps[iIndex].m_szLabel, lpszLabel); ++iIndex)
			;   // null loop body

		if (iIndex >= m_iMaps) {
			iError = 102;
			goto cleanup;
		}
	}

cleanup:
	return iError != 0;
}


//* CGtlData::AddLink -- link together a pair of nodes
bool CGtlData::AddLink(CNode *lpNode1, CNode *lpNode2)
// lpNode1, lpNode2 -- nodes to be linked
// returns: true if error, false otherwise
{
	int iError = 0;		// error code

	AddLink(lpNode1, lpNode2 - m_lpNodes);
	AddLink(lpNode2, lpNode1 - m_lpNodes);

	return iError != 0;
}


//* CGtlData::AddLink -- add link to one node
bool CGtlData::AddLink(CNode *lpNode, int iLink)
// lpNode -- node to add link to
// int iLink -- index of linked node
// returns: true if error, false otherwise
{
	int iError = 0;		// error code
	bool bFound = false;	// if true, link already exists
	int iK;		// loop variable

	assert(iLink >= 0 && iLink < MAX_NODES);

	for (iK = 0; iK < lpNode->m_iNumLinks && !bFound; ++iK)
		bFound = lpNode->m_iLinks[iK] == iLink;

	if (!bFound) {
		if (lpNode->m_iNumLinks >= DIMENSION(lpNode->m_iLinks)) {
			iError = 100;
			ErrorMsg(NULL, "Too many links for node.");
			goto cleanup;
		}
		lpNode->m_iLinks[lpNode->m_iNumLinks++] = iLink;
	}

cleanup:

	return iError != 0;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
