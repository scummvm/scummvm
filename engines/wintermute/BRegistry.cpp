/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include <fstream>
#include "BGame.h"
#include "BRegistry.h"
#include "PathUtil.h"
#include "StringUtil.h"
#include "tinyxml.h"
#include "utils.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBRegistry::CBRegistry(CBGame *inGame): CBBase(inGame) {
	m_IniName = NULL;

	SetIniName("./wme.ini");
	LoadValues(true);
}


//////////////////////////////////////////////////////////////////////////
CBRegistry::~CBRegistry() {
	SaveValues();
	delete[] m_IniName;
	m_IniName = NULL;
}



//////////////////////////////////////////////////////////////////////////
AnsiString CBRegistry::ReadString(const AnsiString &subKey, const AnsiString &key, const AnsiString &init) {
	AnsiString ret = "";

#ifdef __WIN32__
	// check ini file first (so what we can use project files on windows)
	char buffer[32768];
	GetPrivateProfileString(subKey.c_str(), key.c_str(), init.c_str(), buffer, 32768, m_IniName);
	ret = AnsiString(buffer);

	if (buffer != init) return ret;
#endif

	bool found = false;
	ret = GetValue(m_LocalValues, subKey, key, found);
	if (!found) ret = GetValue(m_Values, subKey, key, found);
	if (!found) ret = init;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CBRegistry::WriteString(const AnsiString &subKey, const AnsiString &key, const AnsiString &value) {
	m_Values[subKey][key] = value;
	return true;
}


//////////////////////////////////////////////////////////////////////////
int CBRegistry::ReadInt(const AnsiString &subKey, const AnsiString &key, int init) {
#ifdef __WIN32__
	int ret = GetPrivateProfileInt(subKey.c_str(), key.c_str(), init, m_IniName);
	if (ret != init) return ret;
#endif

	AnsiString val = ReadString(subKey, key, "");
	if (val.empty()) return init;
	else return atoi(val.c_str());
}


//////////////////////////////////////////////////////////////////////////
bool CBRegistry::WriteInt(const AnsiString &subKey, const AnsiString &key, int value) {
	WriteString(subKey, key, StringUtil::ToString(value));
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CBRegistry::ReadBool(const AnsiString &subKey, const AnsiString &key, bool init) {
	return (ReadInt(subKey, key, (int)init) != 0);
}


//////////////////////////////////////////////////////////////////////////
bool CBRegistry::WriteBool(const AnsiString &subKey, const AnsiString &key, bool value) {
	return WriteInt(subKey, key, (int)value);
}


//////////////////////////////////////////////////////////////////////////
void CBRegistry::SetIniName(char *Name) {
	delete[] m_IniName;
	m_IniName = NULL;

	if (strchr(Name, '\\') == NULL && strchr(Name, '/') == NULL) {
		m_IniName = new char [strlen(Name) + 3];
		sprintf(m_IniName, "./%s", Name);
	} else {
		m_IniName = new char [strlen(Name) + 1];
		strcpy(m_IniName, Name);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBRegistry::GetIniName() {
	return m_IniName;
}

//////////////////////////////////////////////////////////////////////////
void CBRegistry::LoadValues(bool local) {
	if (local) LoadXml("settings.xml", m_LocalValues);
	else LoadXml(PathUtil::Combine(Game->GetDataDir(), "settings.xml"), m_Values);
}

//////////////////////////////////////////////////////////////////////////
void CBRegistry::SaveValues() {
	SaveXml(PathUtil::Combine(Game->GetDataDir(), "settings.xml"), m_Values);
}

//////////////////////////////////////////////////////////////////////////
void CBRegistry::SetBasePath(const char *basePath) {
	m_BasePath = PathUtil::GetFileNameWithoutExtension(basePath);

	LoadValues(false);
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBRegistry::GetValue(PathValueMap &values, const AnsiString path, const AnsiString &key, bool &found) {
	found = false;
	PathValueMap::iterator it = values.find(path);
	if (it == values.end()) return "";

	KeyValuePair pairs = (*it)._value;
	KeyValuePair::iterator keyIt = pairs.find(key);
	if (keyIt == pairs.end()) return "";
	else {
		found = true;
		return (*keyIt)._value;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBRegistry::LoadXml(const AnsiString fileName, PathValueMap &values) {
	TiXmlDocument doc(fileName.c_str());
	if (!doc.LoadFile()) return;

	TiXmlElement *rootElem = doc.RootElement();
	if (!rootElem || Common::String(rootElem->Value()) != "Settings") // TODO: Avoid this strcmp-use. (Hack for now, since we might drop TinyXML all together)
		return;

	for (TiXmlElement *pathElem = rootElem->FirstChildElement(); pathElem != NULL; pathElem = pathElem->NextSiblingElement()) {
		for (TiXmlElement *keyElem = pathElem->FirstChildElement(); keyElem != NULL; keyElem = keyElem->NextSiblingElement()) {
			values[Common::String(pathElem->Value())][Common::String(keyElem->Value())] = keyElem->GetText();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CBRegistry::SaveXml(const AnsiString fileName, PathValueMap &values) {
	CBUtils::CreatePath(fileName.c_str());

	TiXmlDocument doc;
	doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));

	TiXmlElement *root = new TiXmlElement("Settings");
	doc.LinkEndChild(root);

	PathValueMap::iterator pathIt;
	for (pathIt = m_Values.begin(); pathIt != m_Values.end(); ++pathIt) {
		TiXmlElement *pathElem = new TiXmlElement((*pathIt)._key.c_str());
		root->LinkEndChild(pathElem);


		KeyValuePair pairs = (*pathIt)._value;
		KeyValuePair::iterator keyIt;
		for (keyIt = pairs.begin(); keyIt != pairs.end(); ++keyIt) {
			TiXmlElement *keyElem = new TiXmlElement((*keyIt)._key.c_str());
			pathElem->LinkEndChild(keyElem);

			keyElem->LinkEndChild(new TiXmlText((*keyIt)._value.c_str()));
		}
	}


	TiXmlPrinter printer;
	doc.Accept(&printer);

	std::ofstream stream;
	stream.open(fileName.c_str());

	if (!stream.is_open()) return;
	else {
		stream << printer.CStr();
		stream.close();
	}
}

} // end of namespace WinterMute
