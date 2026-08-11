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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/crypt/globals.h"
#include "bagel/hodjnpodj/crypt/crypt.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

/*****************************************************************
 *
 * CCryptogram
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Initializes members.
 *
 * FORMAL PARAMETERS:
 *
 *  None
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  None
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  None
 *
 * RETURN VALUE:
 *
 *  None
 *
 ****************************************************************/
CCryptogram::CCryptogram(CDC *pDC) {
	m_cRecordGram   = new CCryptRecord();
	m_cPaintGram    = new CPaintGram(pDC);
	m_cStats        = new CStats();

	/********************************************************
	* Solved switch is used to prevent further user updates *
	* after cryptogram is solved.                           *
	********************************************************/
	bIsGameOver         = false;        // Initialize solved switch

	BagelMetaEngine::setKeybindingMode(KBMODE_MINIMAL);
}

/*****************************************************************
 *
 * ~Cryptogram
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Destructor
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
CCryptogram::~CCryptogram() {
	if (m_cStats != nullptr)
		delete m_cStats;

	if (m_cPaintGram != nullptr)
		delete m_cPaintGram;

	if (m_cRecordGram != nullptr)
		delete m_cRecordGram;

	BagelMetaEngine::setKeybindingMode(KBMODE_NORMAL);
}

/*****************************************************************
 *
 * DrawGram
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      [Description of function]
 *
 * FORMAL PARAMETERS:
 *
 *      [Show arguments]
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      [External data read]
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
 *
 * RETURN VALUE:
 *
 *      [Discuss return value]
 *
 ****************************************************************/
bool CCryptogram::DrawGram(CDC *pDC) {
	bIsGameOver = false;

	if (m_cRecordGram->GetRecord(m_cStats->ResetGame()) == false)        // Attempt to get the record
		return false;

	m_cPaintGram->ClearGram(pDC);
	m_cPaintGram->PaintAlphabet(pDC);

	CreateCryptMap(m_cStats->m_nLettersSolved);
	MarkSolvedLetters(pDC);

	m_cPaintGram->InitGramPosition(m_cRecordGram);
	m_cPaintGram->PaintGram(pDC, m_chEncryptGram);

	return true;
}

void CCryptogram::DrawSource(CDC *pDC) {
	m_cPaintGram->HiLiteOff(pDC);
	m_cPaintGram->PaintGram(pDC, m_cRecordGram->GetSource());
}

void CCryptogram::MarkSolvedLetters(CDC *pDC) {
	int i;

	for (i = 0; i < ALPHABET;  i++) {                // flip thru Crypt Map
		if (m_nCryptMap[DECRYPT_MAP][i] == i)        // Does letter rep itself?
			m_cPaintGram->RevealOn(     // yes - Mark char as solved
			    pDC,
			    m_cPaintGram->IndexToChar(i)
			);
	} // end for
}

void CCryptogram::SolveCryptogram(CDC *pDC) {
	int     nReplaceCode;
	int     nAlphaCode;
	int     nGramCode;
	int     i;

	for (i = 0; i < ALPHABET;  i++) {                                // flip thru Crypt Map
		if ((m_nCryptMap[DECRYPT_MAP][i] != i) &&                    // Any chars rep another char?
		        (m_nCryptMap[DECRYPT_MAP][i] != NOT_USED)) {

			nAlphaCode  = m_cPaintGram->IndexToChar(i);             // Replace this char
			nGramCode   = m_cPaintGram->IndexToChar(m_nCryptMap[DECRYPT_MAP][i]);

			nReplaceCode = UpdateCryptMap(nGramCode, nAlphaCode);   // Update internal rep
			if (nReplaceCode != NOT_USED) {                          // New char used in gram?
				m_cPaintGram->ReplaceLetter(pDC, m_cPaintGram->GetCharType(nAlphaCode),             // Yes - swap w/temp char
				                            m_cPaintGram->SetLimboTypeOn(nReplaceCode));    // ...Temporarily set it's code to limbo

				m_cPaintGram->ReplaceLetter(pDC, nGramCode, m_cPaintGram->GetCharType(nAlphaCode)); // swap old char with new char
				m_cPaintGram->ReplaceLetter(pDC, m_cPaintGram->SetLimboTypeOn(nReplaceCode),        // Turn all limbo types off
				                            m_cPaintGram->SetLimboTypeOff(nReplaceCode));
			} else {                                                                                // New char was not used in gram...
				m_cPaintGram->ReplaceLetter(pDC, nGramCode, m_cPaintGram->GetCharType(nAlphaCode)); // ...simply replace old with new.
			}

			m_cPaintGram->UsedOff(pDC, nGramCode);                      // Turn used code off on old char since it's not being used anymore
			m_cPaintGram->UsedOn(pDC, nAlphaCode);                      // Turn used code on new char now appearing in cryptogram
		} // end if
	} // end for
}

/*****************************************************************
 *
 * HandleUserUpdate
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Processes user interaction with the displayed cryptogram.
 *  Will modify both internal cryptogram representation by calling
 *  appropriate CCryptogram members, and visual crytogram rep by
 *  calling CPaintGram members.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - used for visual updating
 *  cpointClicked - place where user clicked
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_cPaintGram member
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  modifies m_cPaintGram
 *
 * RETURN VALUE:
 *
 *  void
 *
 ****************************************************************/
bool CCryptogram::HandleUserUpdate(CDC *pDC, CPoint cpointClicked) {
	CSprite *pSprite;
	int     nClickedCode;
	int     nHiLiteCode;
	int     nReplaceCode;
	int     nAlphaCode;
	int     nGramCode;

	/*****************************
	* Cryptogram already solved? *
	*****************************/
	if (bIsGameOver == true) {
		//MessageBeep(-1);                          // No - exit
		return false;
	}

	pSprite = m_cPaintGram->m_cDisplayLetters->Touched(cpointClicked);

	/********************************
	* Clicked on letter anywhere?   *
	********************************/
	if (pSprite == nullptr) {
		//MessageBeep(-1);                          // No - exit
		return false;
	}

	/********************
	* Symbol hilited?   *
	********************/
	nClickedCode = (*pSprite).GetTypeCode();
	if (m_cPaintGram->IsSymbolChar(nClickedCode) == true) {
		return false;                                   // Yes - do not hilite symbols
	}

	/********************
	* Anything hilited? *
	********************/
	if (m_cPaintGram->IsHiLiteOn() == false) {
		m_cPaintGram->HiLiteOn(pDC, nClickedCode);  // No - hilite letter
		return false;
	}

	/****************************************************************
	* Was the letter clicked same as the letter currenly hilited?   *
	****************************************************************/
	if (m_cPaintGram->IsHiLiteType(nClickedCode) == true) {
		m_cPaintGram->HiLiteOff(pDC);                // Yes - toggle hilite to off state
		return false;
	}

	/************************************************************
	* Was the area clicked same as the area currenly hilited?   *
	************************************************************/
	nHiLiteCode = m_cPaintGram->GetHiLiteType(pDC);
	ASSERT(nHiLiteCode);
	if (
	    (m_cPaintGram->IsGramType(nClickedCode) &&
	     m_cPaintGram->IsGramType(nHiLiteCode))  ||
	    (m_cPaintGram->IsAlphabetType(nClickedCode) &&
	     m_cPaintGram->IsAlphabetType(nHiLiteCode))
	) {
		m_cPaintGram->HiLiteOff(pDC);               // Yes - turn hilite off
		m_cPaintGram->HiLiteOn(pDC, nClickedCode);  // ...hilite new char
		return false;                               // out of here.
	}

	/************************************************************
	* User wants to switch letters.                             *
	* Need to update internal cryptogram and visual reps.       *
	************************************************************/
	if (m_cPaintGram->IsAlphabetType(nHiLiteCode) == true) {     // Is the hilited char in the alphabet region?
		nAlphaCode  = nHiLiteCode;                              // Yes - assign it the alpha code
		nGramCode   = nClickedCode;
	} else {
		nAlphaCode  = nClickedCode;                             // No - swap around clicked code
		nGramCode   = nHiLiteCode;
	}

	nReplaceCode = UpdateCryptMap(nGramCode, nAlphaCode);       // Update internal rep
	if (nReplaceCode != NOT_USED) {                              // New char used in gram?
		/****************************************************************
		* These next lines introduces the "LimboType."  From the fact   *
		* that the new letter, "nAlphaCode," is already somewhere in    *
		* the displayed cryptogram, two additional letters need         *
		* replacing -- not just the old letter, "nGramCode," with the   *
		* new.  The following algorithm was used to resolve this:       *
		* - The new char currently displayed needs to be replaced       *
		*   with another temp char, "nReplaced," which is the           *
		*   old char + LimboType.                                       *
		* - Next all old chars are replaced with the new char.  Note    *
		*   that the "old char + LimboType" will not be replaced with   *
		*   the new char.                                               *
		* - Finally, set back temp char, "old + limbo," with old        *
		*   char.                                                       *
		*                                                               *
		* That's what the next three lines do respectively.             *
		****************************************************************/
		m_cPaintGram->ReplaceLetter(pDC, m_cPaintGram->GetCharType(nAlphaCode),             // Yes - swap w/temp char
		                            m_cPaintGram->SetLimboTypeOn(nReplaceCode));    // ...Temporarily set it's code to limbo

		m_cPaintGram->ReplaceLetter(pDC, nGramCode, m_cPaintGram->GetCharType(nAlphaCode)); // swap old char with new char
		m_cPaintGram->ReplaceLetter(pDC, m_cPaintGram->SetLimboTypeOn(nReplaceCode),        // Turn all limbo types off
		                            m_cPaintGram->SetLimboTypeOff(nReplaceCode));
	} else {                                                                                // New char was not used in gram...
		m_cPaintGram->ReplaceLetter(pDC, nGramCode, m_cPaintGram->GetCharType(nAlphaCode)); // ...simply replace old with new.
	}

	m_cPaintGram->HiLiteOff(pDC);                               // Turn hilite off
	m_cPaintGram->UsedOff(pDC, nGramCode);                      // Turn used code off on old char since it's not being used anymore
	m_cPaintGram->UsedOn(pDC, nAlphaCode);                      // Turn used code on new char now appearing in cryptogram

	return IsSolved();
}

/*****************************************************************
 *
 * HandleUserUpdate
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Processes user interaction with the displayed cryptogram.
 *  Will modify both internal cryptogram representation by calling
 *  appropriate CCryptogram members, and visual crytogram rep by
 *  calling CPaintGram members.
 *
 * FORMAL PARAMETERS:
 *
 *  pDC - used for visual updating
 *  cpointClicked - place where user clicked
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_cPaintGram member
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  modifies m_cPaintGram
 *
 * RETURN VALUE:
 *
 *  void
 *
 ****************************************************************/
bool CCryptogram::HandleUserUpdate(CDC *pDC, unsigned int nChar) {
	char    nNewChar = toupper(nChar);
	int     nHiLiteCode;
	int     nReplaceCode;
	int     nAlphaCode;
	int     nGramCode;

	/*****************************
	* Cryptogram already solved? *
	*****************************/
	if (bIsGameOver == true) {
		//MessageBeep(-1);                          // No - exit
		return false;
	}

	/****************************************
	* Is this a valid alphabetical letter?  *
	****************************************/
	if (Common::isAlpha(nNewChar) == false) {
		MessageBeep(-1);                            // No - exit
		return false;
	}

	/********************
	* Anything hilited? *
	********************/
	if (m_cPaintGram->IsHiLiteOn() == false) {
		m_cPaintGram->HiLiteOn(pDC, nNewChar);      // Turn hilite on that spec char
		return false;
	}

	/*******************************
	* Hilite in cryptogram region? *
	*******************************/
	nHiLiteCode = m_cPaintGram->GetHiLiteType(pDC);
	ASSERT(nHiLiteCode);
	if (m_cPaintGram->IsGramType(nHiLiteCode) == false) {
		MessageBeep(-1);                            // No - exit
		return false;
	}

	/*************************************
	* Hilite same char as user typed in? *
	*************************************/
	if (nHiLiteCode == nNewChar) {
		m_cPaintGram->HiLiteOff(pDC);               // Turn hilite off
		return false;
	}

	nAlphaCode  = nNewChar;
	nGramCode   = nHiLiteCode;

	/************************************************************
	* User wants to switch letters.                             *
	* Need to update internal cryptogram and visual reps.       *
	************************************************************/

	nReplaceCode = UpdateCryptMap(nGramCode, nAlphaCode);       // Update internal rep
	if (nReplaceCode != NOT_USED) {                              // New char used in gram?
		/****************************************************************
		* These next lines introduces the "LimboType."  From the fact   *
		* that the new letter, "nAlphaCode," is already somewhere in    *
		* the displayed cryptogram, two additional letters need         *
		* replacing -- not just the old letter, "nGramCode," with the   *
		* new.  The following algorithm was used to resolve this:       *
		* - The new char currently displayed needs to be replaced       *
		*   with another temp char, "nReplaced," which is the           *
		*   old char + LimboType.                                       *
		* - Next all old chars are replaced with the new char.  Note    *
		*   that the "old char + LimboType" will not be replaced with   *
		*   the new char.                                               *
		* - Finally, set back temp char, "old + limbo," with old        *
		*   char.                                                       *
		*                                                               *
		* That's what the next three lines do respectively.             *
		****************************************************************/
		m_cPaintGram->ReplaceLetter(pDC, m_cPaintGram->GetCharType(nAlphaCode),             // Yes - swap w/temp char
		                            m_cPaintGram->SetLimboTypeOn(nReplaceCode));    // ...Temporarily set it's code to limbo

		m_cPaintGram->ReplaceLetter(pDC, nGramCode, m_cPaintGram->GetCharType(nAlphaCode)); // swap old char with new char
		m_cPaintGram->ReplaceLetter(pDC, m_cPaintGram->SetLimboTypeOn(nReplaceCode),        // Turn all limbo types off
		                            m_cPaintGram->SetLimboTypeOff(nReplaceCode));
	} else {                                                                                // New char was not used in gram...
		m_cPaintGram->ReplaceLetter(pDC, nGramCode, m_cPaintGram->GetCharType(nAlphaCode)); // ...simply replace old with new.
	}

	m_cPaintGram->HiLiteOff(pDC);                               // Turn hilite off
	m_cPaintGram->UsedOff(pDC, nGramCode);                      // Turn used code off on old char since it's not being used anymore
	m_cPaintGram->UsedOn(pDC, nAlphaCode);                      // Turn used code on new char now appearing in cryptogram

	return IsSolved();
}
/*****************************************************************
 *
 * Encrypt
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Uses the Crypt Map to encode the phrase, as it originally
 *  appears.
 *
 * FORMAL PARAMETERS:
 *
 *  None
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_cRecordGram - to obtain the original phrase.
 *  m_nCryptMap - to encrypt phrase
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  m_chEncrptGram - updated according to structure of
 *      m_nCryptMap.
 *
 * RETURN VALUE:
 *
 *  void
 *
 ****************************************************************/
void CCryptogram::Encrypt() {
	int i;

	/*******************
	* Reset workbench. *
	*******************/
	Common::strcpy_s(m_chEncryptGram, m_cRecordGram->GetGram());    // Acquire copy of original

	/*****************************************
	* Encrypt entire string using crypt map. *
	*****************************************/
	for (i = 0; m_chEncryptGram[i] != 0; i++) {
		if ((m_cPaintGram->IsAlphaChar(m_chEncryptGram[i]) == true) &&                   // Is this a char?
		        (m_nCryptMap[DECRYPT_MAP][m_cPaintGram->CharToIndex(m_chEncryptGram[i])] != NOT_USED)   // and should this char be encrypted?
		   ) {
			m_chEncryptGram[i] = m_cPaintGram->IndexToChar(
			                         m_nCryptMap[DECRYPT_MAP][
			                             m_cPaintGram->CharToIndex(m_chEncryptGram[i])
			                         ]
			                     );
		}
	}
}

/*****************************************************************
 *
 * CreateEncryptMap
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Creates an En-Cryption Map Key by randomally selecting
 *  unique representations for each alphabetical letter.
 *
 * FORMAL PARAMETERS:
 *
 *  nLettersSolved - number of characters not encrypted
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_nCryptMap - Resets En-Cryption Map Key
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  m_nCryptMap - Complete En-Cryption Map Key with exactly
 *      nLettersSolved chars mapped to themselves.
 *
 * RETURN VALUE:
 *
 *  void
 *
 ****************************************************************/
void CCryptogram::CreateCryptMap(int nLettersSolved) {
	int     nEncryptCode;       // encrypted value
	int     nDecryptCode;       // normal/decrypted value
	bool    bIsUsed;            // tells if encrypt-decrypt map was used
	int     i, j;               // index

	/*******************
	* Reset workbench. *
	*******************/
	Common::strcpy_s(m_chEncryptGram, m_cRecordGram->GetGram());    // Acquire copy of original
	for (i = 0; i < ALPHABET;  i++) {                    // Reset cryptmap
		m_nCryptMap[DECRYPT_MAP][i] = NOT_USED;
		m_nCryptMap[ENCRYPT_MAP][i] = NOT_USED;
	}

	/****************************************************
	* Create encryption map based on letters in phrase. *
	****************************************************/
	for (i = 0; m_chEncryptGram[i] != 0; i++) {
		if (m_cPaintGram->IsAlphaChar(m_chEncryptGram[i]) == true) {     // Is this a char?

			nDecryptCode = m_cPaintGram->CharToIndex(m_chEncryptGram[i]);
			bIsUsed = (m_nCryptMap[DECRYPT_MAP][nDecryptCode] != NOT_USED);

			if (bIsUsed == true)     // Char already encrypted?
				continue;           // Yes - loop to next char in text

			/******************************
			* Find an unused encrypt map. *
			******************************/
			do {
				nEncryptCode = brand() % ALPHABET;
				bIsUsed = (m_nCryptMap[ENCRYPT_MAP][nEncryptCode] != NOT_USED);
			} while (bIsUsed == true || nEncryptCode == nDecryptCode);   // find unused map

			/**************************************
			* Record new encrypt/decrypt mapping. *
			**************************************/
			m_nCryptMap[DECRYPT_MAP][nDecryptCode] = nEncryptCode;
			m_nCryptMap[ENCRYPT_MAP][nEncryptCode] = nDecryptCode;
		}
	}

	/************************************************************************
	* Decrypt letters solved given as function arg.                         *
	* To keep letters solved random (as opposed to decrypting A, B, and C   *
	* in order) it seemed easier to change the map after it was fully       *
	* encrypted, provided from above.                                       *
	************************************************************************/
	for (i = 0; i < nLettersSolved ; i++) {

		for (j = 0; j < ALPHABET; j++) {    // Are there any letters left to decrypt?
			if (
			    m_nCryptMap[DECRYPT_MAP][j] != NOT_USED &&      // in the quote and
			    m_nCryptMap[DECRYPT_MAP][j] != j                // not already solved
			) {
				bIsUsed = true;             // Yes - so break
				break;
			} else
				bIsUsed = false;            // No - not this letter, keep looking
		}

		if (bIsUsed == false)                // Any letters left to decrypt?
			break;                          // No - by pass loop.

		do {
			nDecryptCode = brand() % ALPHABET;                  // find used char
			bIsUsed = (
			              m_nCryptMap[DECRYPT_MAP][nDecryptCode] != NOT_USED &&   // in quote and
			              m_nCryptMap[DECRYPT_MAP][nDecryptCode] != nDecryptCode  // not already solved
			          );
		} while (bIsUsed == false);

		nEncryptCode = m_nCryptMap[DECRYPT_MAP][nDecryptCode];  // gets corres decoder

		/********************************************************************
		* Need to know if the decrypted letter was used in the encryption   *
		* map before.  If it was, then another encryption letter needs to   *
		* be used in it's place.                                            *
		********************************************************************/
		bIsUsed = (m_nCryptMap[ENCRYPT_MAP][nDecryptCode] != NOT_USED);
		if (bIsUsed == true) {                                                               // Decrypted letter used before?
			m_nCryptMap[DECRYPT_MAP][m_nCryptMap[ENCRYPT_MAP][nDecryptCode]] = nEncryptCode;    // Yes - Swap around encrypted chars
			m_nCryptMap[ENCRYPT_MAP][nEncryptCode] = m_nCryptMap[ENCRYPT_MAP][nDecryptCode];
		} else {
			m_nCryptMap[ENCRYPT_MAP][nEncryptCode] = NOT_USED;                                  // No - Mark encryption map as unused
		}

		/********************************************************************
		* ENCRYPT_MAP is a reversed mirror of DECRYPT_MAP.  I.e. if         *
		* "A points to X" in the DECRYPT_MAP, then "X points to A" in the   *
		* ENCRYPT_MAP.  Specific reasons for assigning both sides of map    *
		* to "nDecryptCode" are tricky...8-].  Think about it.              *
		********************************************************************/
		m_nCryptMap[DECRYPT_MAP][nDecryptCode] = nDecryptCode;  // Match Decrypt map
		m_nCryptMap[ENCRYPT_MAP][nDecryptCode] = nDecryptCode;  // ...with Encrypt map
	}

	Encrypt();  // Create the real thing...update internal rep.

}

/*****************************************************************
 *
 * UpdateGramChar
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Replaces old char, "nOldType," with new char, "nNewType," in
 *  Crypt Map.  Handles specific case when new char appears in
 *  Crypt Map prior to replacement.
 *
 * FORMAL PARAMETERS:
 *
 *  nOldType - type code of old char
 *  nNewType - type code of new char
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_nCryptMap - En-Cryption Map Key
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  m_nCryptMap - nOldType is remapped to nNewType
 *
 * RETURN VALUE:
 *
 *  nReplaceCode - value of any additional character that may
 *      need replacing.
 *
 ****************************************************************/
int CCryptogram::UpdateCryptMap(int nOldType, int nNewType) {
	int     nOldIndex;      // Crypt Map index equivalent of nOldType
	int     nNewIndex;      // index equivalent of nNewType
	int     nEncryptCode;   // old type
	int     nDecryptCode;   // decryption map corresponding to old type
	int     nReplaceCode;   // temporary old type

	nOldIndex = m_cPaintGram->CharToIndex(m_cPaintGram->GetCharType(nOldType));
	nNewIndex = m_cPaintGram->CharToIndex(m_cPaintGram->GetCharType(nNewType));

	nEncryptCode = nOldIndex;
	nDecryptCode = m_nCryptMap[ENCRYPT_MAP][nEncryptCode];
	nReplaceCode = NOT_USED;

	/************************************************************************
	* Need to know if nNewType letter was used in the cryptogram            *
	* before.  This can occur in two different instances: another letter    *
	* was encrypted into nNewType, or when nNewType just appears as a       *
	* straight representation of itself. If either was case, then another   *
	* letter needs to be used in it's place.                                *
	************************************************************************/
	if (m_nCryptMap[ENCRYPT_MAP][nNewIndex] != NOT_USED) {                           // New type used in encryption ma?
		m_nCryptMap[DECRYPT_MAP][m_nCryptMap[ENCRYPT_MAP][nNewIndex]] = nEncryptCode;   // Swap around encrypted chars
		m_nCryptMap[ENCRYPT_MAP][nEncryptCode] = m_nCryptMap[ENCRYPT_MAP][nNewIndex];
		nReplaceCode = m_cPaintGram->IndexToChar(nEncryptCode);
	} else {
		m_nCryptMap[ENCRYPT_MAP][nEncryptCode]  = NOT_USED;
	}

	m_nCryptMap[DECRYPT_MAP][nDecryptCode] = nNewIndex; // Match Decrypt map
	m_nCryptMap[ENCRYPT_MAP][nNewIndex] = nDecryptCode; // ...with Encrypt map

	Encrypt();                                          // Update internal rep
	return nReplaceCode;                                // return temporary value....
}

/*****************************************************************
 *
 * IsSolved
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Figures out if the cryptogram is solved, based on
 *  state of "m_nCryptMap" -- the En-Cryption Map Key.
 *
 * FORMAL PARAMETERS:
 *
 *  void
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_nCryptMap - En-Cryption Map Key
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  None
 *
 * RETURN VALUE:
 *
 *  nTotalSolved - number of correctly decrypted letters
 *
 ****************************************************************/
bool CCryptogram::IsSolved() {
	int i;

	for (i = 0; i < ALPHABET;  i++) {                    // flip thru Crypt Map
		if ((m_nCryptMap[DECRYPT_MAP][i] != i) &&        // Any chars rep another char?
		        (m_nCryptMap[DECRYPT_MAP][i] != NOT_USED))
			return false;                               // Yes - cryptogram not solved
	}

	bIsGameOver = true;                                 // Mark cryptogram as solved

	return true;
}

/*****************************************************************
 *
 * LettersSolved
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Figures out how many letters are properly decrypted, based on
 *  state of "m_nCryptMap" -- the En-Cryption Map Key.
 *
 * FORMAL PARAMETERS:
 *
 *  void
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_nCryptMap - En-Cryption Map Key
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  None
 *
 * RETURN VALUE:
 *
 *  nTotalSolved - number of correctly decrypted letters
 *
 ****************************************************************/
int CCryptogram::LettersSolved() {
	int i;
	int nTotalSolved = 0;

	for (i = 0; i < ALPHABET;  i++) {                // flip thru Crypt Map
		if (m_nCryptMap[DECRYPT_MAP][i] == i)        // Does letter rep itself?
			nTotalSolved++;                         // Yes - it is a solved letter
	}

	return nTotalSolved;
}

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel
