// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_geom_ext.h"
#include "../base/base_game.h"
#include "../base/base_file_manager.h"
#include "../utils/utils.h"
#include "../base/base_parser.h"
#include "ad_geom_ext_node.h"
#include "ad_types.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdGeomExt::AdGeomExt(BaseGame* in_gameRef) : BaseClass(in_gameRef)
{
}

//////////////////////////////////////////////////////////////////////////
AdGeomExt::~AdGeomExt(void)
{
	for(int i=0; i<m_Nodes.size(); i++)
	{
		if (m_Nodes[i])
		{
			delete m_Nodes[i];
		}
	}
	m_Nodes.clear();
}


//////////////////////////////////////////////////////////////////////////
bool AdGeomExt::LoadFile(char* Filename)
{
	byte* Buffer = BaseFileManager::getEngineInstance()->readWholeFile(Filename);
	if(Buffer==NULL)
	{
		_gameRef->LOG(0, "AdGeomExt::LoadFile failed for file '%s'", Filename);
		return false;
	}

	bool ret = LoadBuffer(Buffer);
	if(!ret)
	{
		_gameRef->LOG(0, "Error parsing geometry description file '%s'", Filename);
	}

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
	TOKEN_DEF (GEOMETRY)
	TOKEN_DEF (NODE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdGeomExt::LoadBuffer(byte* Buffer)
{
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE (GEOMETRY)
		TOKEN_TABLE (NODE)
	TOKEN_TABLE_END

	byte* params;
	int cmd;
	BaseParser parser;

	if(parser.getCommand ((char**)&Buffer, commands, (char**)&params)!=TOKEN_GEOMETRY)
	{
		_gameRef->LOG(0, "'GEOMETRY' keyword expected.");
		return false;
	}
	Buffer = params;


	while ((cmd = parser.getCommand ((char**)&Buffer, commands, (char**)&params)) > 0)
	{
		switch (cmd)
		{
			case TOKEN_NODE:
			{
				AdGeomExtNode* Node = new AdGeomExtNode(_gameRef);
				if(Node && Node->LoadBuffer(params, false))
				{
					m_Nodes.add(Node);
				}
				else
				{
					if (Node)
					{
						delete Node;
					}

					cmd = PARSERR_GENERIC;
				}
			}
			break;
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

	AddStandardNodes();
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool AdGeomExt::AddStandardNodes()
{
	AdGeomExtNode* Node;

	Node = new AdGeomExtNode(_gameRef);
	Node->SetupNode("walk_*", GEOM_WALKPLANE, true);
	m_Nodes.add(Node);

	Node = new AdGeomExtNode(_gameRef);
	Node->SetupNode("blk_*", GEOM_BLOCKED, false);
	m_Nodes.add(Node);

	Node = new AdGeomExtNode(_gameRef);
	Node->SetupNode("wpt_*", GEOM_WAYPOINT, false);
	m_Nodes.add(Node);

	return true;
}


//////////////////////////////////////////////////////////////////////////
AdGeomExtNode* AdGeomExt::MatchName(char* Name)
{
	if(!Name) return NULL;

	for(int i=0; i<m_Nodes.size(); i++)
	{
		if(m_Nodes[i]->MatchesName(Name)) return m_Nodes[i];
	}
	return NULL;
}

}
