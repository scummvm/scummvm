#pragma once

#include "engines/crab/common_header.h"
#include "engines/crab/filesystem.h"

namespace Crab {

class XMLDoc
{
	rapidxml::xml_document<char> doc;
	char *text;

public:
	XMLDoc(){ text = NULL; }
	XMLDoc(const Common::String &filename){ text = NULL; Load(filename); }

	~XMLDoc() { delete[] text; }

	//Load the text from the specified file into the rapidxml format
	//Each function that references it must parse it there
	void Load(const Common::String &filename);

	//Check if document is ready for parsing
	bool ready() const { return text != NULL; }

	const rapidxml::xml_document<>* Doc() const;
};

} // End of namespace Crab
