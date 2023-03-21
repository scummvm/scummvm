#include "stdafx.h"
#include "XMLDoc.h"

void XMLDoc::Load(const std::string &filename)
{
	if (ready())
		doc.clear();

	if (FileOpen(filename.c_str(), text))
		if (text != NULL)
			doc.parse<0>(text);
}

const rapidxml::xml_document<>* XMLDoc::Doc() const
{
	if (text != NULL)
		return &doc;
	else
		return NULL;
}