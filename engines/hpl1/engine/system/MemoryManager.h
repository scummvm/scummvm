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

#ifndef HPL_MEMORY_MANAGER_H
#define HPL_MEMORY_MANAGER_H

#include <map>
#include <string>

namespace hpl {

//------------------------------------

class cAllocatedPointer {
public:
	cAllocatedPointer(void *apData, const std::string &asFile, int alLine, size_t alMemory);

	std::string msFile;
	int mlLine;
	size_t mlMemory;
	void *mpData;
};

//------------------------------------

typedef std::map<void *, cAllocatedPointer> tAllocatedPointerMap;
typedef tAllocatedPointerMap::iterator tAllocatedPointerMapIt;

//------------------------------------

class cMemoryManager {
public:
	static void *AddPointer(const cAllocatedPointer &aAllocatedPointer);

	static bool RemovePointer(void *apData);

	static void LogResults();

	static tAllocatedPointerMap m_mapPointers;
	static size_t mlTotalMemoryUsage;

	static bool mbLogDeletion;

	template<class T>
	static T *DeleteAndReturn(T *apData) {
		delete apData;
		return apData;
	}

	template<class T>
	static T *DeleteArrayAndReturn(T *apData) {
		delete[] apData;
		return apData;
	}

	template<class T>
	static T *FreeAndReturn(T *apData) {
		free(apData);
		return apData;
	}

	static void SetLogCreation(bool abX);
	static bool GetLogCreation() { return mbLogCreation; }

	static int GetCreationCount() { return mlCreationCount; }

private:
	static bool mbLogCreation;
	static int mlCreationCount;
};

//------------------------------------

//#define MEMORY_MANAGER_ACTIVE
#ifdef MEMORY_MANAGER_ACTIVE

#define hplNew(classType, constructor) \
	(classType *)hpl::cMemoryManager::AddPointer(hpl::cAllocatedPointer(new classType constructor, __FILE__, __LINE__, sizeof(classType)))

#define hplNewArray(classType, amount) \
	(classType *)hpl::cMemoryManager::AddPointer(hpl::cAllocatedPointer(new classType[amount], __FILE__, __LINE__, amount * sizeof(classType)))

#define hplMalloc(amount) \
	hpl::cMemoryManager::AddPointer(hpl::cAllocatedPointer(malloc(amount), __FILE__, __LINE__, amount))

#define hplDelete(data)                                                                              \
	{                                                                                                \
		if (hpl::cMemoryManager::RemovePointer(hpl::cMemoryManager::DeleteAndReturn(data)) == false) \
			hpl::Log("Deleting at '%s' %d\n", __FILE__, __LINE__);                                   \
	} // delete data;

#define hplDeleteArray(data)                                                                              \
	{                                                                                                     \
		if (hpl::cMemoryManager::RemovePointer(hpl::cMemoryManager::DeleteArrayAndReturn(data)) == false) \
			hpl::Log("Deleting at '%s' %d\n", __FILE__, __LINE__);                                        \
	} // delete [] data;

#define hplFree(data)                                                                              \
	{                                                                                              \
		if (hpl::cMemoryManager::RemovePointer(hpl::cMemoryManager::FreeAndReturn(data)) == false) \
			hpl::Log("Deleting at '%s' %d\n", __FILE__, __LINE__);                                 \
	} // free(data);

#else
#define hplNew(classType, constructor) \
	new classType constructor

#define hplNewArray(classType, amount) \
	new classType[amount]

#define hplMalloc(amount) \
	malloc(amount)

#define hplDelete(data) \
	delete data;

#define hplDeleteArray(data) \
	delete[] data;

#define hplFree(data) \
	free(data);

#endif

//------------------------------------

};     // namespace hpl
#endif // HPL_MEMORY_MANAGER_H
