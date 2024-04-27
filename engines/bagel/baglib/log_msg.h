
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

class CBagLogResidue : public CBagTextObject {
protected:
	int m_nSdevWidth;

public:
	CBagLogResidue(int sdevWidth);
	virtual ~CBagLogResidue() {}

	void setSize(const CBofSize &size);

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);
};

#define mMsgTimeMask 0x3FFF
#define mMsgPlayedMask 0x4000

class CBagLogMsg : public CBagTextObject {
protected:
	CBofString m_sMsgSendee;
	CBofString m_sMsgTimeStr;
	int m_nSdevWidth;

public:
	CBagLogMsg(int sdevWidth);
	virtual ~CBagLogMsg() {}

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);

	/**
	 * Takes in info and then removes the relative information and returns the info
	 * without the relevant info.
	 */
	PARSE_CODES setInfo(bof_ifstream &istr);

	void setSize(const CBofSize &size);

	void setProperty(const CBofString &prop, int val);
	int getProperty(const CBofString &prop);

	void SetMsgSendee(const CBofString &sProp) {
		m_sMsgSendee = sProp;
	}
	CBofString GetMsgSendee() {
		return m_sMsgSendee;
	}

	void SetMsgTime(int &nVal) {
		int nState = GetState();
		SetState((nState & mMsgPlayedMask) | (nVal & mMsgTimeMask));
	}
	int GetMsgTime() {
		return GetState() & mMsgTimeMask;
	}

	void SetMsgPlayed(bool bPlayed) {
		int nState = GetState();
		SetState((nState & mMsgTimeMask) | (bPlayed == true ? mMsgPlayedMask : 0));
	}
	bool GetMsgPlayed() {
		return (GetState() & mMsgPlayedMask) != 0;
	}
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
	CBagLogSuspect(int sdevWidth);
	virtual ~CBagLogSuspect() {}

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);

	void setSize(const CBofSize &size);

	PARSE_CODES setInfo(bof_ifstream &istr);

	void setProperty(const CBofString &prop, int val);
	int getProperty(const CBofString &prop);

	void SetSusName(const CBofString &sProp) {
		m_sSusName = sProp;
	}
	void SetSusSpecies(const CBofString &sProp) {
		m_sSusSpecies = sProp;
	}
	void SetSusRoom(const CBofString &sProp) {
		m_sSusRoom = sProp;
	}

	void SetSusVP(bool bVal) {
		bVal == false ? SetState(GetState() & ~mSusVoicePrinted) : SetState(GetState() | mSusVoicePrinted);
	}
	bool GetSusVP() {
		return (GetState() & mSusVoicePrinted) != 0;
	}

	void SetSusRP(bool bVal) {
		bVal == false ? SetState(GetState() & ~mSusResiduePrinted) : SetState(GetState() | mSusResiduePrinted);
	}
	bool GetSusRP() {
		return (GetState() & mSusResiduePrinted) != 0;
	}

	void SetSusChecked(bool bVal) {
		bVal == false ? SetState(GetState() & ~mSusChecked) : SetState(GetState() | mSusChecked);
	}
	bool GetSusChecked() {
		return (GetState() & mSusChecked) != 0;
	}
};

class CBagLog : public CBagStorageDevBmp {
protected:
	// Queued messages waited to be played and inserted into SDEV
	CBofList<CBagObject *> *m_pQueued_Msgs;

	static CBagLog *m_bLastFloatPage;

public:
	CBagLog();
	virtual ~CBagLog();
	static void initialize() {
		m_bLastFloatPage = nullptr;
	}

	virtual CBagObject *OnNewUserObject(const CBofString &initStr);

	/**
	 * This is different for the log sdev Object are just queued for insertion
	 * and the message light starts blinking. The messages will be added
	 * to the sdev when the message light is clicked
	 * @return      Error result code
	 */
	virtual ErrorCode ActivateLocalObject(CBagObject *bagObj);

	/**
	 * Releases and deletes all the objects in the list
	 */
	ErrorCode ReleaseMsg();

	ErrorCode PlayMsgQue();

	/**
	 * Remove a (duplicate) message from the message queue
	 */
	bool RemoveFromMsgQueue(CBagObject *pObj);

	/**
	 * This function arranges the objects that are considered floating
	 * (no coordinates given in script).  It is virtual so that derived
	 * storage devices can do special things, like paging etc.
	 * @return Returns the next available location in the sdev,
	 * at this level the objects will go out of range of the sdev.
	 */
	CBofPoint ArrangeFloater(CBofPoint nPos, CBagObject *pObj);

	int GetCurFltPage();
	void SetCurFltPage(int fltPage);

	static void ArrangePages();
	static void InitArrangePages() {
		m_bLastFloatPage = nullptr;
	}
};

class CBagEnergyDetectorObject : public CBagTextObject {
public:
	CBagEnergyDetectorObject();
	virtual ~CBagEnergyDetectorObject();

	// Need private setinfo so we can parse energy detector fields
	PARSE_CODES setInfo(bof_ifstream &istr);

	ErrorCode update(CBofBitmap *, CBofPoint, CBofRect *, int);

	ErrorCode attach();

	void SetMsgTime(int &nVal) {
		SetState(nVal);
	}
	int GetMsgTime() {
		return GetState();
	}

protected:
private:
	CBofString m_sEnergyTimeStr;
	CBofString m_sZhapsStr;
	CBofString m_sCauseStr;

	bool m_bTextInitialized;
};

// Special object, clue object.
class CBagLogClue : public CBagTextObject {
private:
	CBagVar *m_pStringVar1;
	CBagVar *m_pStringVar2;
	CBagVar *m_pStringVar3;
	CBagVar *m_pStringVar4;
	int m_nSdevWidth;

public:
	CBagLogClue(const CBofString &sInit, int nSdevWidth, int nPointSize);
	virtual ~CBagLogClue() {}

	ErrorCode attach();

	PARSE_CODES setInfo(bof_ifstream &istr);

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);
};

} // namespace Bagel

#endif
