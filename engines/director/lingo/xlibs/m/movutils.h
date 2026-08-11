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

#ifndef DIRECTOR_LINGO_XLIBS_MOVUTILS_H
#define DIRECTOR_LINGO_XLIBS_MOVUTILS_H

namespace Director {

class MovUtilsXObject : public Object<MovUtilsXObject> {
public:
	MovUtilsXObject(ObjectType objType);
};

namespace MovUtilsXObj {

extern const char *const xlibNames[];
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_getVolName(int nargs);
void m_getSystemPath(int nargs);
void m_getWindowsPath(int nargs);
void m_setDefaultPath(int nargs);
void m_getChildWindowNames(int nargs);
void m_getNamedWindowHdl(int nargs);
void m_drawLine(int nargs);
void m_drawOval(int nargs);
void m_drawRect(int nargs);
void m_drawRoundRect(int nargs);
void m_drawPoly(int nargs);
void m_drawPie(int nargs);
void m_printLandscape(int nargs);
void m_noPunct(int nargs);
void m_toUpperCase(int nargs);
void m_toLowerCase(int nargs);
void m_trimWhiteChars(int nargs);
void m_dollarFormat(int nargs);
void m_getWordStart(int nargs);
void m_getWordEnd(int nargs);
void m_getLineStart(int nargs);
void m_getLineEnd(int nargs);
void m_isAlphaNum(int nargs);
void m_isAlpha(int nargs);
void m_isUpper(int nargs);
void m_isLower(int nargs);
void m_isDigit(int nargs);
void m_isPunctuation(int nargs);
void m_isWhiteSpace(int nargs);
void m_isPrintable(int nargs);
void m_isGraphic(int nargs);
void m_isControl(int nargs);
void m_isHex(int nargs);
void m_bitSet(int nargs);
void m_bitTest(int nargs);
void m_bitClear(int nargs);
void m_bitShiftL(int nargs);
void m_bitShiftR(int nargs);
void m_bitAnd(int nargs);
void m_bitOr(int nargs);
void m_bitXOr(int nargs);
void m_bitNot(int nargs);
void m_bitStringToNumber(int nargs);
void m_stageToCast(int nargs);
void m_stageToDIB(int nargs);
void m_stageToPICT(int nargs);
void m_cRtoCRLF(int nargs);
void m_cRLFtoCR(int nargs);
void m_getMessage(int nargs);

} // End of namespace MovUtilsXObj

} // End of namespace Director

#endif
