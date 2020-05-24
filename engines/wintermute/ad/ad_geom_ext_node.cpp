// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_geom_ext_node.h"
#include "../base/base_parser.h"
#include "../base/base_game.h"
#include "../utils/utils.h"
#include "common/str.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode::AdGeomExtNode(BaseGame* inGame) : BaseClass(inGame)
{
	m_NamePattern = NULL;
	m_ReceiveShadows = false;
	m_Type = GEOM_GENERIC;
}

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode::~AdGeomExtNode(void)
{
	if (m_NamePattern)
	{
		delete[] m_NamePattern;
	}
}


TOKEN_DEF_START
	TOKEN_DEF (NODE)
	TOKEN_DEF (NAME)
	TOKEN_DEF (WALKPLANE)
	TOKEN_DEF (BLOCKED)
	TOKEN_DEF (WAYPOINT)
	TOKEN_DEF (RECEIVE_SHADOWS)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdGeomExtNode::LoadBuffer(byte* Buffer, bool Complete)
{
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE (NODE)
		TOKEN_TABLE (NAME)
		TOKEN_TABLE (WALKPLANE)
		TOKEN_TABLE (BLOCKED)
		TOKEN_TABLE (WAYPOINT)
		TOKEN_TABLE (RECEIVE_SHADOWS)
	TOKEN_TABLE_END

	byte* params;
	int cmd=2;
	BaseParser parser;

	if(Complete)
	{
		if(parser.getCommand ((char**)&Buffer, commands, (char**)&params)!=TOKEN_NODE)
		{
			_gameRef->LOG(0, "'NODE' keyword expected.");
			return false;
		}
		Buffer = params;
	}

	while ((cmd = parser.getCommand ((char**)&Buffer, commands, (char**)&params)) > 0)
	{
		switch (cmd)
		{
			case TOKEN_NAME:
				BaseUtils::setString(&m_NamePattern, (char*)params);
			break;

			case TOKEN_RECEIVE_SHADOWS:
				parser.scanStr((char*)params, "%b", &m_ReceiveShadows);
			break;

			case TOKEN_WALKPLANE:
			{
				bool IsWalkplane = false;
				parser.scanStr((char*)params, "%b", &IsWalkplane);
				if(IsWalkplane) m_Type = GEOM_WALKPLANE;
				break;
			}

			case TOKEN_BLOCKED:
			{
				bool IsBlocked = false;
				parser.scanStr((char*)params, "%b", &IsBlocked);
				if(IsBlocked) m_Type = GEOM_BLOCKED;
				break;
			}

			case TOKEN_WAYPOINT:
			{
				bool IsWaypoint = false;
				parser.scanStr((char*)params, "%b", &IsWaypoint);
				if(IsWaypoint) m_Type = GEOM_WAYPOINT;
				break;
			}			
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND)
	{
		_gameRef->LOG(0, "Syntax error in geometry description file");
		return false;
	}
	if (cmd == PARSERR_GENERIC)
	{
		_gameRef->LOG(0, "Error loading geometry description");
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool AdGeomExtNode::SetupNode(char* NamePattern, TGeomNodeType Type, bool ReceiveShadows)
{
	BaseUtils::setString(&m_NamePattern, NamePattern);
	m_Type = Type;
	m_ReceiveShadows = ReceiveShadows;

	return true;
}


//////////////////////////////////////////////////////////////////////////
bool AdGeomExtNode::MatchesName(char* Name)
{
	return Common::matchString(Name, m_NamePattern);
}

}
