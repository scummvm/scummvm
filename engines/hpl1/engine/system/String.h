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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_STRING_H
#define HPL_STRING_H

#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

class cString {
public:
	/**
	 * Converts ascii to unicode
	 */
	static tWString To16Char(const tString &asString);

	/**
	 * Converts unicode to ascii
	 */
	static tString To8Char(const tWString &awsString);

	/**
	 * Gets a 16 bit string from an string of numbers
	 */
	static tWString Get16BitFromArray(const tString &asArray);

	/**
	 * Get the sub string
	 * \param asString The string to get the sub from. The method error check so the params are correct.
	 * \param alStart The character to start at.
	 * \param alCount The number of character to copy to the sub. -1 = all til end.
	 * \return
	 */
	static tString Sub(const tString &asString, int alStart, int alCount = -1);
	static tWString SubW(const tWString &asString, int alStart, int alCount = -1);
	/**
	 * Get the file extension of a string
	 * \param aString extension (for example ".exe"). If none "" is returned.
	 * \return
	 */
	static tString GetFileExt(tString aString);
	static tWString GetFileExtW(tWString aString);
	/**
	 * Sets the file extension. "" removes extension, for example "test.exe" -> "test"
	 * \param aString New string
	 * \param aExt Extension, both ".exe" and "exe" works
	 * \return
	 */
	static tString SetFileExt(tString aString, tString aExt);
	static tWString SetFileExtW(tWString aString, tWString aExt);

	/**
	 * Gets the file filename in for example: "/files/test/this.txt"
	 * \param aString The Filename
	 * \return
	 */
	static tString GetFileName(tString aString);
	static tWString GetFileNameW(tWString aString);
	/**
	 * Removes the filename from a path
	 * \param aString
	 * \return
	 */
	static tString GetFilePath(tString aString);
	static tWString GetFilePathW(tWString aString);
	/**
	 * Sets the path for a file.
	 * \param aString
	 * \param aPath New path
	 * \return
	 */
	static tString SetFilePath(tString aString, tString aPath);
	static tWString SetFilePathW(tWString aString, tWString aPath);
	/**
	 * Converts a string to lower case.
	 * \param aString
	 * \return
	 */
	static tString ToLowerCase(tString aString);
	static tWString ToLowerCaseW(tWString aString);

	/**
	 *
	 * \param aString The string to do the replacement on
	 * \param asOldChar The char to replace (one character only!)
	 * \param asNewChar The char to replace with (one character only!)
	 * \return
	 */
	static tString ReplaceCharTo(tString aString, tString asOldChar, tString asNewChar);
	static tWString ReplaceCharToW(tWString aString, tWString asOldChar, tWString asNewChar);

	/**
	 *
	 * \param aString The string to do the replacement on
	 * \param asOldString The char to replace
	 * \param asNewString The char to replace with
	 * \return
	 */
	static tString ReplaceStringTo(tString aString, tString asOldString, tString asNewString);

	static tString ToString(const char *asString, tString asDefault);
	static int ToInt(const char *asString, int alDefault);
	static bool ToBool(const char *asString, bool abDefault);
	static float ToFloat(const char *asString, float afDefault);
	static cColor ToColor(const char *asString, const cColor &aDefault);
	static cVector2f ToVector2f(const char *asString, const cVector2f &avDefault);
	static cVector3f ToVector3f(const char *asString, const cVector3f &avDefault);
	static cVector2l ToVector2l(const char *asString, const cVector2l &avDefault);
	static cVector3l ToVector3l(const char *asString, const cVector3l &avDefault);
	static cMatrixf ToMatrixf(const char *asString, const cMatrixf &a_mtxDefault);

	static tString ToString(int alX);
	static tString ToString(float afX);

	static tWString ToStringW(int alX);
	static tWString ToStringW(float afX);

	/**
	 * Get a vector of ints from a string such as "1, 2, 3".
	 * Valid separators are ' ', '\n', '\t' and ','
	 * \param &asData The string
	 * \param avVec a vector the values will be appended to.
	 * \param apSeparators a pointer to a string with chars to override the default separators
	 */
	static tIntVec &GetIntVec(const tString &asData, tIntVec &avVec, tString *apSeparators = NULL);

	/**
	 * Get a vector of ints from a string such as "1, 2, 3".
	 * Valid separators are ' ', '\n', '\t' and ','
	 * \param &asData The string
	 * \param avVec a vector the values will be appended to.
	 * \param apSeparators a pointer to a string with chars to override the default separators
	 */
	static tUIntVec &GetUIntVec(const tString &asData, tUIntVec &avVec, tString *apSeparators = NULL);

	/**
	 * Get a vector of floats from a string such as "1, 2, 3".
	 * Valid separators are ' ', '\n', '\t' and ','
	 * \param &asData The string
	 * \param avVec a vector the values will be appended to.
	 * \param apSeparators a pointer to a string with chars to override the default separators
	 */
	static tFloatVec &GetFloatVec(const tString &asData, tFloatVec &avVec, tString *apSeparators = NULL);

	/**
	 * Get a vector of strings from a string such as "one, two, three".
	 * Valid separators are ' ', '\n', '\t' and ','
	 * \param &asData
	 * \param avVec
	 * \param apSeparators a pointer to a string with chars to override the default separators
	 */
	static tStringVec &GetStringVec(const tString &asData, tStringVec &avVec, tString *apSeparators = NULL);

	/**
	 * Gets the last character of the string.
	 * \param aString
	 * \return
	 */
	static tString GetLastChar(tString aString);
	static tWString GetLastCharW(tWString aString);
	/**
	 * Get the last pos where aChar is found.
	 * \param aString
	 * \param aChar
	 * \return >=0 if string is found else -1
	 */
	static int GetLastStringPos(tString aString, tString aChar);
	static int GetLastStringPosW(tWString aString, tWString aChar);

	static void UIntStringToArray(unsigned int *apArray, const char *apString, int alSize);
	static void FloatStringToArray(float *apArray, const char *apString, int alSize);

private:
};

} // namespace hpl

#endif // HPL_STRING_H
