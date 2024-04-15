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

#include "bagel/baglib/expression.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/sound_object.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {

CBagVar CBagExpression::m_xTempVar("CBagExpr::TempVar", "", false);            // used as a default param


CBagExpression::CBagExpression(CBagExpression *pPrevExpr, bool bPrevNeg) {
	m_bNegative = false;
	m_xPrevExpression = pPrevExpr;

	m_bPrevNegative = (byte)bPrevNeg;
}

CBagExpression::~CBagExpression() {
}

bool CBagExpression::Evaluate(CBagVar *xLHOper, CBagVar *xRHOper, OPERATION xOper, CBagVar &xResult) {
	bool bRClocal = false;

	// If the variable is named "RANDOM", generate a random number for its value
	if (xLHOper->GetName() == "RANDOM")
		xLHOper->SetValue(g_engine->getRandomNumber());
	if (xRHOper->GetName() == "RANDOM")
		xRHOper->SetValue(g_engine->getRandomNumber());

	switch (xOper) {
	case NONE:
		break;

	case ASSIGN:
		bRClocal = OnAssign(xLHOper, xRHOper, xResult);
		break;

	case EQUAL:
		bRClocal = OnEqual(xLHOper, xRHOper, xResult);
		break;

	case NOTEQUAL:
		bRClocal = OnNotEqual(xLHOper, xRHOper, xResult);
		break;

	case LESSTHAN:
		bRClocal = OnLessThan(xLHOper, xRHOper, xResult);
		break;

	case LESSTHANEQUAL:
		bRClocal = OnLessThanEqual(xLHOper, xRHOper, xResult);
		break;

	case GREATERTHAN:
		bRClocal = OnGreaterThan(xLHOper, xRHOper, xResult);
		break;

	case GREATERTHANEQUAL:
		bRClocal = OnGreaterThanEqual(xLHOper, xRHOper, xResult);
		break;

	case PLUSASSIGN:
		bRClocal = OnPlusAssign(xLHOper, xRHOper, xResult);
		break;

	case MINUSASSIGN:
		bRClocal = OnMinusAssign(xLHOper, xRHOper, xResult);
		break;

	case PLUS:
		bRClocal = OnPlus(xLHOper, xRHOper, xResult);
		break;

	case MINUS:
		bRClocal = OnMinus(xLHOper, xRHOper, xResult);
		break;

	case MULTIPLY:
		bRClocal = OnMultiply(xLHOper, xRHOper, xResult);
		break;

	case DIVIDE:
		bRClocal = OnDivide(xLHOper, xRHOper, xResult);
		break;

	case MOD:
		bRClocal = OnMod(xLHOper, xRHOper, xResult);
		break;

	case CONTAINS:
		bRClocal = OnContains(xLHOper, xRHOper, xResult);
		break;

	case HAS:
		bRClocal = OnHas(xLHOper, xRHOper, xResult);
		break;

	case STATUS:
		bRClocal = OnStatus(xLHOper, xRHOper, xResult);
		break;

	default:
		break;
	}

	return bRClocal;
}


CBagVar *CBagExpression::GetVariable(int nPos) {
	CBagVar *pVar = m_cVarList.GetNodeItem(nPos);

	// Re-wrote because Left(), and Mid() were causing many allocations

	// If the variable is a reference (OBJ.PROPERTY)
	if (pVar->IsReference()) {
		char *p, szFront[256];
		Common::strcpy_s(szFront, pVar->GetName());

		if ((p = strstr(szFront, "~~")) != nullptr) {
			char szBack[256];
			Common::strcpy_s(szBack, p + 2);
			*p = '\0';

			CBofString sObject(szFront, 256);
			CBofString sProperty(szBack, 256);

			int nVal = SDEVMNGR->GetObjectValue(sObject, sProperty);
			pVar->SetValue(nVal);
		}
	}

	return pVar;
}


CBagExpression::OPERATION CBagExpression::GetOperation(int nPos) {
	Assert(false);
	return m_cOperList.GetNodeItem(nPos);
}


bool CBagExpression::Evaluate(bool bNeg, CBagVar &xResult) {
	bool bRClocal = false;

	// There must be an expression for every variable after the first
	Assert(m_cVarList.GetCount() - 1 == m_cOperList.GetCount());

	int nVCount = 0;

	CBagVar *xLHOper = GetVariable(nVCount++);
	xResult = *xLHOper;

	bool bRCparent = true;
	if (m_xPrevExpression) {
		bRCparent = m_xPrevExpression->Evaluate(m_bPrevNegative, xResult);
	}

	if (bRCparent) {
		bool bVal = false;
		int nECount = 0;

		while (nVCount < m_cVarList.GetCount()) {
			CBagVar *xRHOper = GetVariable(nVCount++);
			OPERATION xOper = m_cOperList.GetNodeItem(nECount++);
			CBagVar *xRHOper2;

			switch (xOper) {
			case AND:
				xRHOper2 = GetVariable(nVCount++);
				xOper = m_cOperList.GetNodeItem(nECount++);
				bVal = Evaluate(xRHOper, xRHOper2, xOper, xResult);

				bRClocal &= bVal;
				break;

			case OR:
				xRHOper2 = GetVariable(nVCount++);
				xOper = m_cOperList.GetNodeItem(nECount++);
				bVal = Evaluate(xRHOper, xRHOper2, xOper, xResult);

				bRClocal |= bVal;
				break;

			default:
				bRClocal = Evaluate(xLHOper, xRHOper, xOper, xResult);
				break;
			}
		}

		if (bNeg)
			// Evaluate before and with parent
			bRClocal = !bRClocal;

		bRClocal &= bRCparent;
	}

	return bRClocal;
}

bool CBagExpression::EvalLeftToRight(bool bNeg, CBagVar &xResult) {
	bool bRClocal = false;
	CBagVar stLHOper;
	OPERATION xOper = NONE;

	// There must be an expression for every variable after the first
	Assert(m_cVarList.GetCount() - 1 == m_cOperList.GetCount());

	int nVCount = 0;

	CBagVar *xLHOper = GetVariable(nVCount++);
	xResult = *xLHOper;

	bool bRCparent = true;
	if (m_xPrevExpression) {
		bRCparent = m_xPrevExpression->Evaluate(m_bPrevNegative, xResult);
	}

	if (bRCparent) {
		bool bFirstTime = true;
		int nECount = 0;
		while (nVCount < m_cVarList.GetCount()) {
			CBagVar *xRHOper = GetVariable(nVCount++);
			OPERATION xPrevOper = xOper;      // save previous operator
			xOper = m_cOperList.GetNodeItem(nECount++);

			if (bFirstTime) {
				stLHOper = *xLHOper;
				bFirstTime = false;
			} else {
				// Based on what we have for a previous operator, either use
				// the left hand expression or the result of the previous expression.
				switch (xPrevOper) {
				case MINUS:
				case MULTIPLY:
				case DIVIDE:
				case MOD:
				case PLUS:
					stLHOper = xResult;
					break;
				case NONE:
				case ASSIGN:
				case EQUAL:
				case NOTEQUAL:
				case LESSTHAN:
				case LESSTHANEQUAL:
				case GREATERTHAN:
				case GREATERTHANEQUAL:
				case PLUSASSIGN:
				case MINUSASSIGN:
				case CONTAINS:
				case HAS:
				case STATUS:
				default:
					stLHOper = *xLHOper;
					break;
				}
			}

			bool bVal;
			CBagVar *xRHOper2;
			switch (xOper) {

			case AND:
				xRHOper2 = GetVariable(nVCount++);
				xOper = m_cOperList.GetNodeItem(nECount++);
				bVal = Evaluate(xRHOper, xRHOper2, xOper, xResult);

				bRClocal &= bVal;
				break;

			case OR:
				xRHOper2 = GetVariable(nVCount++);
				xOper = m_cOperList.GetNodeItem(nECount++);
				bVal = Evaluate(xRHOper, xRHOper2, xOper, xResult);

				// or this don't not it!!!
				bRClocal |= bVal;
				break;

			default:
				bRClocal = Evaluate(&stLHOper, xRHOper, xOper, xResult);
				break;
			}
		}

		if (bNeg)
			// Evaluate before and with parent
			bRClocal = !bRClocal;

		bRClocal &= bRCparent;
	}

	return bRClocal;
}


bool CBagExpression::NegEvaluate(CBagVar &xResult) {
	return Evaluate(false, xResult);
}


bool CBagExpression::OnAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar & /*xResultOper*/) {
	//int nIndex;
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	char szBuf[256];
	Common::strcpy_s(szBuf, xRHOper->GetValue());
	Assert(strlen(szBuf) < 256);
	CBofString sBuf(szBuf, 256);

	xLHOper->SetValue(sBuf);

	return true;
}


bool CBagExpression::OnEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	bool bVal = xLHOper->GetValue() == xRHOper->GetValue();
	xResultOper.SetBoolValue(bVal);

	return bVal;
}


bool CBagExpression::OnNotEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));
	bool bVal = xLHOper->GetValue() != xRHOper->GetValue();
	xResultOper.SetBoolValue(bVal);

	return bVal;
}


bool CBagExpression::OnLessThan(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));
	bool bVal = xLHOper->GetNumValue() < xRHOper->GetNumValue();
	xResultOper.SetBoolValue(bVal);
	return bVal;
}


bool CBagExpression::OnGreaterThan(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));
	bool bVal = xLHOper->GetNumValue() > xRHOper->GetNumValue();
	xResultOper.SetBoolValue(bVal);
	return bVal;
}


bool CBagExpression::OnLessThanEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));
	bool bVal = xLHOper->GetNumValue() <= xRHOper->GetNumValue();
	xResultOper.SetBoolValue(bVal);
	return bVal;
}


bool CBagExpression::OnGreaterThanEqual(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	bool bVal = xLHOper->GetNumValue() >= xRHOper->GetNumValue();
	xResultOper.SetBoolValue(bVal);
	return bVal;
}


bool CBagExpression::OnPlusAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();
		xLHOper->SetValue(nLHO + nRHO);
		xResultOper.SetValue(xLHOper->GetNumValue());
	}

	return true;
}


bool CBagExpression::OnMinusAssign(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();
		xLHOper->SetValue(nLHO - nRHO);
		xResultOper.SetValue(xLHOper->GetNumValue());
	}

	return true;
}


bool CBagExpression::OnContains(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar & /*xResultOper*/) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	CBagStorageDev *pSDev;
	if ((pSDev = SDEVMNGR->GetStorageDevice(xLHOper->GetValue())) == nullptr)
		return false;

	CBagObject *pObj;
	if ((pObj = pSDev->GetObject(xRHOper->GetValue())) == nullptr)
		return false;

	if (pObj->IsActive())
		return true;

	return false;
}

bool CBagExpression::OnHas(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar & /*xResultOper*/) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	CBagStorageDev *pSDev = SDEVMNGR->GetStorageDevice(xLHOper->GetValue());
	if (pSDev == nullptr)
		return false;

	CBagObject *pObj = pSDev->GetObjectByType(xRHOper->GetValue(), true);
	if (pObj == nullptr)
		return false;

	return true;
}

bool CBagExpression::OnStatus(CBagVar *pLHOper, CBagVar * /*pRHOper*/, CBagVar & /*xResultOper*/) {
	Assert(pLHOper != nullptr);

	CBagStorageDev *pSDev = SDEVMNGR->GetStorageDeviceContaining(pLHOper->GetValue());
	if (pSDev == nullptr)
		return false;

	CBagObject *pObj = pSDev->GetObject(pLHOper->GetValue());
	if (pObj == nullptr)
		return false;

	return false;

}
bool CBagExpression::OnCurrSDev(CBagVar * /*xLHOper*/, CBagVar * /*xRHOper*/, CBagVar & /*xResultOper*/) {
	return true;
}


bool CBagExpression::OnPlus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();
		xResultOper.SetValue(nLHO + nRHO);
	}

	return true;
}


bool CBagExpression::OnMinus(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();
		xResultOper.SetValue(nLHO - nRHO);
	}

	return true;
}


bool CBagExpression::OnMultiply(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();

		xResultOper.SetValue(nLHO * nRHO);
	}

	return true;
}


bool CBagExpression::OnDivide(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();

		// Divide by Zero error?
		Assert(nRHO != 0);

		xResultOper.SetValue(nLHO / nRHO);
	}

	return true;
}


bool CBagExpression::OnMod(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar &xResultOper) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	if (xLHOper->IsNumeric() && xRHOper->IsNumeric()) {
		int nLHO = xLHOper->GetNumValue();
		int nRHO = xRHOper->GetNumValue();

		// Divide by Zero error?
		Assert(nRHO != 0);

		xResultOper.SetValue(nLHO % nRHO);
	}

	return true;
}


bool CBagExpression::OnAnd(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar & /*xResultOper*/) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	return (!xLHOper->GetValue().Find("true") && !xRHOper->GetValue().Find("true"));
}


bool CBagExpression::OnOr(CBagVar *xLHOper, CBagVar *xRHOper, CBagVar & /*xResultOper*/) {
	Assert((xLHOper != nullptr) && (xRHOper != nullptr));

	return (!xLHOper->GetValue().Find("true") || !xRHOper->GetValue().Find("true"));
}


PARSE_CODES CBagExpression::SetInfo(bof_ifstream &istr) {
	char szBuf[256];
	szBuf[0] = 0;
	CBofString sStr(szBuf, 256);

	char szErrStr[256];
	Common::strcpy_s(szErrStr, "Error in expression ");
	CBofString errStr(szErrStr, 256);

	PARSE_CODES rc = PARSING_DONE;
	bool bDone = false;
	OPERATION xOper;

	while (!bDone && rc == PARSING_DONE) {
		istr.EatWhite();
		int ch = istr.peek();
		switch (ch) {
		case '(': {
			istr.Get();
			istr.EatWhite();

			GetAlphaNumFromStream(istr, sStr);
			CBagVar *pVar = VARMNGR->GetVariable(sStr);
			if (!pVar) {                             // this must be a reference, make a new variable
				if (sStr.Find("~~") > 0) {
					pVar = new CBagVar;
					pVar->SetName(sStr);
					pVar->SetReference();
				} else {                             // this is an error condition, constants can only be rhopers
					//CBofString strName("Constant");
					//strName += sStr;
					pVar = new CBagVar;
					pVar->SetName(sStr);
					pVar->SetValue(sStr);
					pVar->SetConstant();
				}
			}
			m_cVarList.AddToTail(pVar);

			istr.EatWhite();
			ch = istr.peek();
			while ((ch != ')') && rc == PARSING_DONE) {
				GetOperatorFromStream(istr, xOper);
				if (xOper == NONE) {
					rc = UNKNOWN_TOKEN;
					errStr = "Bad operator:";
					break;
				}
				m_cOperList.AddToTail(xOper);

				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				pVar = VARMNGR->GetVariable(sStr);
				if (!pVar) {
					if (sStr.Find("~~") > 0) {             // this must be a reference, make a new variable
						pVar = new CBagVar;
						pVar->SetName(sStr);
						pVar->SetReference();
					} else {                            // this must be a constant, make a new variable
						pVar = new CBagVar;
						pVar->SetName(sStr);
						pVar->SetValue(sStr);
						pVar->SetConstant();
					}
				}
				m_cVarList.AddToTail(pVar);

				istr.EatWhite();
			} // while parsing inner circle

			if (ch == ')') {
				istr.Get();
				bDone = true;
			}
			break;
		}

		case 'N':
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("NOT")) {
				m_bNegative = (byte)!m_bNegative;
				istr.EatWhite();
				break;
			}
		default:
			rc = UNKNOWN_TOKEN;
			break;
		}
	}

	if (rc != PARSING_DONE) {
		ParseAlertBox(istr, "Error in expression:", __FILE__, __LINE__);
	}

	istr.EatWhite();

	return rc;
}


ErrorCode CBagExpression::GetOperatorFromStream(bof_ifstream &istr, OPERATION &xOper) {
	ErrorCode rc = ERR_NONE;

	char szLocalBuff[256];
	szLocalBuff[0] = 0;

	CBofString sStr(szLocalBuff, 256);

	xOper = NONE;

	istr.EatWhite();
	GetOperStrFromStream(istr, sStr);

	if (sStr.IsEmpty()) {
		GetAlphaNumFromStream(istr, sStr);
		istr.EatWhite();
	}

	if (!sStr.Find("-=")) {
		xOper = MINUSASSIGN;

	} else if (!sStr.Find("+=")) {
		xOper = PLUSASSIGN;

	} else if (!sStr.Find(">=")) {
		xOper = GREATERTHANEQUAL;

	} else if (!sStr.Find("<=")) {
		xOper = LESSTHANEQUAL;

	} else if (!sStr.Find("!=")) {
		xOper = NOTEQUAL;

	} else if (!sStr.Find("==")) {
		xOper = EQUAL;

	} else if (!sStr.Find(">")) {
		xOper = GREATERTHAN;

	} else if (!sStr.Find("<")) {
		xOper = LESSTHAN;

	} else if (!sStr.Find("=")) {
		xOper = ASSIGN;

	} else if (!sStr.Find("+")) {
		xOper = PLUS;

	} else if (!sStr.Find("-")) {
		xOper = MINUS;

	} else if (!sStr.Find("*")) {
		xOper = MULTIPLY;

	} else if (!sStr.Find("/")) {
		xOper = DIVIDE;

	} else if (!sStr.Find("%")) {
		xOper = MOD;

	} else if (!sStr.Find("CONTAINS")) {
		// Sdev contains object
		xOper = CONTAINS;

	} else if (!sStr.Find("HAS")) {
		// Sdev has type of object
		xOper = HAS;

	} else if (!sStr.Find("OR")) {
		xOper = OR;

	} else if (!sStr.Find("STATUS")) {
		xOper = STATUS;

	} else if (!sStr.Find("AND")) {
		xOper = AND;
	}

	if (xOper == NONE)
		rc = ERR_UNKNOWN;

	return rc;
}

} // namespace Bagel
