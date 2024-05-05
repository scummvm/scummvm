
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

#ifndef BAGEL_BAGLIB_DOSSIER_OBJECT_H
#define BAGEL_BAGLIB_DOSSIER_OBJECT_H

#include "bagel/baglib/text_object.h"
#include "bagel/baglib/log_msg.h"

namespace Bagel {

class CBagDossierObject : public CBagTextObject {
private:
	CBofString _indexLine;
	CBofRect _indexRect;
	CBofRect _dossierRect;
	CBagObject *_residuePrintedObject;

	bool _dossierSetFl : 1;
	bool _notActiveFl : 1;
	bool _showIndexFl : 1;
	bool _dosRectInitFl : 1;

public:
	CBofString _sSuspectVar;

public:
	CBagDossierObject();
	virtual ~CBagDossierObject();

	/**
	 * Takes in info and then removes the relative information and returns
	 * the info without the relevant info.
	 */
	ParseCodes setInfo(CBagIfstream &istr) override;

	ErrorCode attach() override;
	ErrorCode detach() override;

	ErrorCode update(CBofBitmap *, CBofPoint, CBofRect * = nullptr, int = -1) override;

	void onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * = nullptr) override; // run menu if available

	CBofRect getRect() override;

	void setNotActive(bool b) {
		_notActiveFl = b;
	}
	bool getNotActive() const {
		return _notActiveFl;
	}

	void activateDossierObject(CBagLog *logWld);
	void deactivateDossierObject(CBagLog *logWld);

	void setResiduePrintedObject(CBagObject *p) {
		_residuePrintedObject = p;
	};

	static void deactivateCurDossier();
	void showDossierText();

	static void initialize();
	static CBagDossierObject *_curDossier;
};

} // namespace Bagel

#endif
