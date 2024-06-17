
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
	int _sdevWidth;

public:
	CBagLogResidue(int sdevWidth);
	virtual ~CBagLogResidue() {}

	void setSize(const CBofSize &size) override;

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;
};

#define MSG_TIME_MASK 0x3FFF
#define MSG_PLAYED_MASK 0x4000

class CBagLogMsg : public CBagTextObject {
protected:
	CBofString _msgSendee;
	CBofString _msgTimeStr;
	int _sdevWidth;

public:
	CBagLogMsg(int sdevWidth);
	virtual ~CBagLogMsg() {}

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;

	/**
	 * Takes in info and then removes the relative information and returns the info
	 * without the relevant info.
	 */
	ParseCodes setInfo(CBagIfstream &istr) override;

	void setSize(const CBofSize &size) override;

	void setProperty(const CBofString &prop, int val) override;
	int getProperty(const CBofString &prop) override;

	void setMsgSendee(const CBofString &sendee) {
		_msgSendee = sendee;
	}
	CBofString getMsgSendee() const {
		return _msgSendee;
	}

	void setMsgTime(int &msgTime) {
		const int state = getState();
		setState((state & MSG_PLAYED_MASK) | (msgTime & MSG_TIME_MASK));
	}

	int getMsgTime() {
		return getState() & MSG_TIME_MASK;
	}

	void setMsgPlayed(bool playedFl) {
		const int state = getState();
		setState((state & MSG_TIME_MASK) | (playedFl == true ? MSG_PLAYED_MASK : 0));
	}
	bool getMsgPlayed() {
		return (getState() & MSG_PLAYED_MASK) != 0;
	}
};

#define mSusChecked 0x0001
#define mSusVoicePrinted 0x0002
#define mSusResiduePrinted 0x0004

class CBagLogSuspect : public CBagTextObject {
protected:
	int _nSdevWidth;

	CBofString _susName;
	CBofString _susSpecies;
	CBofString _susRoom;

public:
	CBagLogSuspect(int sdevWidth);
	virtual ~CBagLogSuspect() {}

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;

	void setSize(const CBofSize &size) override;

	ParseCodes setInfo(CBagIfstream &istr) override;

	void setProperty(const CBofString &prop, int val) override;
	int getProperty(const CBofString &prop) override;

	void setSusName(const CBofString &susName) {
		_susName = susName;
	}
	void setSusSpecies(const CBofString &susSpecies) {
		_susSpecies = susSpecies;
	}
	void setSusRoom(const CBofString &susRoom) {
		_susRoom = susRoom;
	}

	void setSusVoicePrinted(bool bVal) {
		bVal == false ? setState(getState() & ~mSusVoicePrinted) : setState(getState() | mSusVoicePrinted);
	}
	bool getSusVoicePrinted() {
		return (getState() & mSusVoicePrinted) != 0;
	}

	void setSusResiduePrinted(bool bVal) {
		bVal == false ? setState(getState() & ~mSusResiduePrinted) : setState(getState() | mSusResiduePrinted);
	}
	bool getSusResiduePrinted() {
		return (getState() & mSusResiduePrinted) != 0;
	}

	void setSusChecked(bool bVal) {
		bVal == false ? setState(getState() & ~mSusChecked) : setState(getState() | mSusChecked);
	}
	bool getSusChecked() {
		return (getState() & mSusChecked) != 0;
	}
};

class CBagLog : public CBagStorageDevBmp {
protected:
	// Queued messages waited to be played and inserted into SDEV
	CBofList<CBagObject *> *_queuedMsgList;

	static CBagLog *_lastFloatPage;

public:
	CBagLog();
	virtual ~CBagLog();
	static void initialize() {
		_lastFloatPage = nullptr;
	}

	CBagObject *onNewUserObject(const CBofString &initStr) override;

	/**
	 * This is different for the log sdev Object are just queued for insertion
	 * and the message light starts blinking. The messages will be added
	 * to the sdev when the message light is clicked
	 * @return      Error result code
	 */
	ErrorCode activateLocalObject(CBagObject *bagObj) override;

	/**
	 * Releases and deletes all the objects in the list
	 */
	ErrorCode releaseMsg();

	ErrorCode playMsgQueue();

	/**
	 * Remove a (duplicate) message from the message queue
	 */
	bool removeFromMsgQueue(CBagObject *bagObj);

	/**
	 * This function arranges the objects that are considered floating
	 * (no coordinates given in script).  It is virtual so that derived
	 * storage devices can do special things, like paging etc.
	 * @return Returns the next available location in the sdev,
	 * at this level the objects will go out of range of the sdev.
	 */
	CBofPoint arrangeFloater(CBofPoint &pos, CBagObject *bagObj) override;

	int getCurFltPage();
	void setCurFltPage(int fltPage);

	static void arrangePages();
	static void initArrangePages() {
		_lastFloatPage = nullptr;
	}
};

class CBagEnergyDetectorObject : public CBagTextObject {
public:
	CBagEnergyDetectorObject();
	virtual ~CBagEnergyDetectorObject();

	// Need private setinfo so we can parse energy detector fields
	ParseCodes setInfo(CBagIfstream &istr) override;

	ErrorCode update(CBofBitmap *, CBofPoint, CBofRect *, int) override;

	ErrorCode attach() override;

	void setMsgTime(int &nVal) {
		setState(nVal);
	}
	int getMsgTime() {
		return getState();
	}

private:
	CBofString _energyTimeStr;
	CBofString _zhapsStr;
	CBofString _causeStr;

	bool _textInitializedFl;
};

// Special object, clue object.
class CBagLogClue : public CBagTextObject {
private:
	CBagVar *_stringVar1;
	CBagVar *_stringVar2;
	CBagVar *_stringVar3;
	CBagVar *_stringVar4;
	int _sdevWidth;

public:
	CBagLogClue(const CBofString &initStr, int sdevWidth, int pointSize);
	virtual ~CBagLogClue() {}

	ErrorCode attach() override;

	ParseCodes setInfo(CBagIfstream &istr) override;

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;
};

} // namespace Bagel

#endif
