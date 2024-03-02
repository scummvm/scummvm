
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

#ifndef BAGEL_BAGLIB_LOG_MSG_H
#define BAGEL_BAGLIB_LOG_MSG_H

#include "bagel/baglib/text_object.h"
#include "bagel/baglib/storage_dev_bmp.h"

namespace Bagel {

#define INCLUDE_RP_AND_VP TRUE

class CBagLogResidue : public CBagTextObject {
protected:
	int m_nSdevWidth;

public:
	CBagLogResidue(int nSdevWidth);
	virtual ~CBagLogResidue() {}

	VOID SetSize(const CBofSize &xSize);

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = NULL, INT /*nMaskColor*/ = -1);
};

#define mMsgTimeMask 0x3FFF
#define mMsgPlayedMask 0x4000

class CBagLogMsg : public CBagTextObject {
protected:
	CBofString m_sMsgSendee;
	CBofString m_sMsgTimeStr;
	int m_nSdevWidth;

public:
	CBagLogMsg(int nSdevWidth);
	virtual ~CBagLogMsg() {}

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = NULL, INT /*nMaskColor*/ = -1);

	PARSE_CODES SetInfo(bof_ifstream &istr);
	// PARSE_CODES	        SetInfo(CBofFile&);
	VOID SetSize(const CBofSize &xSize);

	VOID SetProperty(const CBofString &sProp, int nVal);
	INT GetProperty(const CBofString &sProp);

	VOID SetMsgSendee(const CBofString &sProp) { m_sMsgSendee = sProp; }
	CBofString GetMsgSendee() { return m_sMsgSendee; }

	VOID SetMsgTime(INT &nVal) {
		INT nState = GetState();
		SetState((nState & mMsgPlayedMask) | (nVal & mMsgTimeMask));
	}
	INT GetMsgTime() { return (GetState() & mMsgTimeMask); }

	VOID SetMsgPlayed(BOOL bPlayed) {
		INT nState = GetState();
		SetState((nState & mMsgTimeMask) | (bPlayed == TRUE ? mMsgPlayedMask : 0));
	}
	BOOL GetMsgPlayed() { return ((GetState() & mMsgPlayedMask) != 0); }
};

#define mSusChecked 0x0001
#define mSusVoicePrinted 0x0002
#define mSusResiduePrinted 0x0004

class CBagLogSuspect : public CBagTextObject {
protected:
	int m_nSdevWidth;

	CBofString m_sSusName;
	CBofString m_sSusSpecies;
	CBofString m_sSusRoom;

public:
	CBagLogSuspect(int nSdevWidth);
	virtual ~CBagLogSuspect() {}

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = NULL, INT /*nMaskColor*/ = -1);

	VOID SetSize(const CBofSize &xSize);

	PARSE_CODES SetInfo(bof_ifstream &istr);
	// PARSE_CODES	        SetInfo(CBofFile&);

	VOID SetProperty(const CBofString &sProp, int nVal);
	INT GetProperty(const CBofString &sProp);

	VOID SetSusName(const CBofString &sProp) { m_sSusName = sProp; }
	VOID SetSusSpecies(const CBofString &sProp) { m_sSusSpecies = sProp; }
	VOID SetSusRoom(const CBofString &sProp) { m_sSusRoom = sProp; }

#if INCLUDE_RP_AND_VP
	VOID SetSusVP(BOOL bVal) { bVal == FALSE ? SetState(GetState() & ~mSusVoicePrinted) : SetState(GetState() | mSusVoicePrinted); }
	BOOL GetSusVP() { return ((GetState() & mSusVoicePrinted) != 0); }

	VOID SetSusRP(BOOL bVal) { bVal == FALSE ? SetState(GetState() & ~mSusResiduePrinted) : SetState(GetState() | mSusResiduePrinted); }
	BOOL GetSusRP() { return ((GetState() & mSusResiduePrinted) != 0); }
#endif

	VOID SetSusChecked(BOOL bVal) { bVal == FALSE ? SetState(GetState() & ~mSusChecked) : SetState(GetState() | mSusChecked); }
	BOOL GetSusChecked() { return ((GetState() & mSusChecked) != 0); }
};

class CBagLog : public CBagStorageDevBmp {
protected:
	// int m_nCurFltPage; 						// mdm 8/5

	CBofList<CBagObject *>
		*m_pQueued_Msgs; // Queued messages waited to be
						 // played and inserted into SDEV
	static CBagLog *m_bLastFloatPage;

public:
	CBagLog();
	virtual ~CBagLog();

	virtual CBagObject *OnNewUserObject(const CBofString &sInit);
	virtual ERROR_CODE ActivateLocalObject(CBagObject *pObj);
	ERROR_CODE ReleaseMsg();
	ERROR_CODE PlayMsgQue();
	BOOL RemoveFromMsgQueue(CBagObject *pObj);

	CBofPoint ArrangeFloater(CBofPoint nPos, CBagObject *pObj);
	INT GetCurFltPage();
	VOID SetCurFltPage(int nFltPage);

	static VOID ArrangePages(VOID);
	static VOID InitArrangePages(VOID) { m_bLastFloatPage = NULL; }
};

class CBagEnergyDetectorObject : public CBagTextObject {
public:
	CBagEnergyDetectorObject();
	virtual ~CBagEnergyDetectorObject();

	// Need private setinfo so we can parse energy detector fields
	PARSE_CODES SetInfo(bof_ifstream &istr);

	ERROR_CODE Update(CBofBitmap *, CBofPoint, CBofRect *, INT);

	ERROR_CODE Attach();

	VOID SetMsgTime(INT &nVal) { SetState(nVal); }
	INT GetMsgTime() { return GetState(); }

protected:
private:
	CBofString m_sEnergyTimeStr;
	CBofString m_sZhapsStr;
	CBofString m_sCauseStr;

	BOOL m_bTextInitialized;
};

// Special object, clue object.
class CBagLogClue : public CBagTextObject {
private:
	CBagVar *m_pStringVar1;
	CBagVar *m_pStringVar2;
	CBagVar *m_pStringVar3;
	CBagVar *m_pStringVar4;
	INT m_nSdevWidth;

public:
	CBagLogClue(const CBofString &sInit, INT nSdevWidth, INT nPointSize);
	virtual ~CBagLogClue() {}

	ERROR_CODE Attach();

	PARSE_CODES SetInfo(bof_ifstream &istr);

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = NULL, INT /*nMaskColor*/ = -1);
};

} // namespace Bagel

#endif
