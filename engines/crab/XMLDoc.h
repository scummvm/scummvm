#pragma once

#include "common_header.h"
#include "filesystem.h"

class XMLDoc
{
	rapidxml::xml_document<char> doc;
	char *text;

public:
	XMLDoc(){ text = NULL; }
	XMLDoc(const std::string &filename){ text = NULL; Load(filename); }

	~XMLDoc() { delete[] text; }

	//Load the text from the specified file into the rapidxml format
	//Each function that references it must parse it there
	void Load(const std::string &filename);

	//Check if document is ready for parsing
	bool ready() const { return text != NULL; }

	const rapidxml::xml_document<>* Doc() const;
};