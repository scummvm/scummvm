#include "cloudthread.h"
#include "../../common/debug.h"
#include "../../common/json.h"

void example1();
void example2();
void example3();

void cloudThread(void *thread) {
	CloudThread *cloudThread = (CloudThread *)thread;
	cloudThread->work();
};

void CloudThread::work() {
	if(firstTime) {		
		firstTime = false;

		example1();
		example2();
		example3();
	} else {		
	}
}

/// SimpleJSON examples:

using Common::JSON;
using Common::JSONValue;
using Common::JSONArray;
using Common::JSONObject;

// Just some sample JSON text, feel free to change but could break demo
const char* EXAMPLE = "\
{ \
	\"string_name\" : \"string\tvalue and a \\\"quote\\\" and a unicode char \\u00BE and a c:\\\\path\\\\ or a \\/unix\\/path\\/ :D\", \
	\"bool_name\" : true, \
	\"bool_second\" : FaLsE, \
	\"null_name\" : nULl, \
	\"negative\" : -34.276, \
	\"sub_object\" : { \
						\"foo\" : \"abc\", \
						 \"bar\" : 1.35e2, \
						 \"blah\" : { \"a\" : \"A\", \"b\" : \"B\", \"c\" : \"C\" } \
					}, \
	\"array_letters\" : [ \"a\", \"b\", \"c\", [ 1, 2, 3  ]  ] \
}    ";

// Example 1
void example1()
{
	// Parse example data
	JSONValue *value = JSON::Parse(EXAMPLE);

	// Did it go wrong?
	if (value == NULL)
	{
		debug("Example code failed to parse, did you change it?\r\n");
	}
	else
	{
		// Retrieve the main object
		JSONObject root;
		if (value->IsObject() == false)
		{
			debug("The root element is not an object, did you change the example?\r\n");
		}
		else
		{
			root = value->AsObject();

			// Retrieving a string
			if (root.find("string_name") != root.end() && root["string_name"]->IsString())
			{
				debug("string_name:\r\n");
				debug("------------\r\n");
				debug(root["string_name"]->AsString().c_str());
				debug("\r\n\r\n");
			}

			// Retrieving a boolean
			if (root.find("bool_second") != root.end() && root["bool_second"]->IsBool())
			{
				debug("bool_second:\r\n");
				debug("------------\r\n");
				debug(root["bool_second"]->AsBool() ? "it's true!" : "it's false!");
				debug("\r\n\r\n");
			}

			// Retrieving an array
			if (root.find("array_letters") != root.end() && root["array_letters"]->IsArray())
			{
				JSONArray array = root["array_letters"]->AsArray();
				debug("array_letters:\r\n");
				debug("--------------\r\n");
				for (unsigned int i = 0; i < array.size(); i++)
				{
					//wstringstream output;
					debug("[%d] => %s\r\n", i, array[i]->Stringify().c_str());
				}
				debug("\r\n");
			}

			// Retrieving nested object
			if (root.find("sub_object") != root.end() && root["sub_object"]->IsObject())
			{
				debug("sub_object:\r\n");
				debug("-----------\r\n");
				debug(root["sub_object"]->Stringify().c_str());
				debug("\r\n\r\n");
			}
		}

		delete value;
	}
}

// Example 3 : compact vs. prettyprint
void example2()
{
	const char* EXAMPLE3 =
		"{\
	 \"SelectedTab\":\"Math\",\
	 	\"Widgets\":[\
			{\"WidgetPosition\":[0,369,800,582],\"WidgetIndex\":1,\"WidgetType\":\"WidgetCheckbox.1\"},\
			{\"WidgetPosition\":[235,453,283,489],\"IsWidgetVisible\":-1,\"Caption\":\"On\",\"EnableCaption\":-1,\"Name\":\"F2.View\",\"CaptionPosition\":2,\"ControlWidth\":25,\"ControlHeight\":36,\"Font\":0,\"Value\":\"Off\",\"WidgetIndex\":2,\"WidgetType\":\"WidgetCheckbox.1\"},\
			{\"WidgetPosition\":[235,494,283,530],\"IsWidgetVisible\":-1,\"Caption\":\"On\",\"EnableCaption\":-1,\"Name\":\"F3.View\",\"CaptionPosition\":2,\"ControlWidth\":25,\"ControlHeight\":36,\"Font\":0,\"Value\":\"Off\",\"WidgetIndex\":3,\"WidgetType\":\"WidgetCheckbox.1\"},\
			{\"WidgetPosition\":[235,536,283,572],\"IsWidgetVisible\":-1,\"Caption\":\"On\",\"EnableCaption\":-1,\"Name\":\"F4.View\",\"CaptionPosition\":2,\"ControlWidth\":25,\"ControlHeight\":36,\"Font\":0,\"Value\":\"Off\",\"WidgetIndex\":4,\"WidgetType\":\"WidgetCheckbox.1\"},\
			{\"WidgetPosition\":[287,417,400,439],\"IsWidgetVisible\":-1,\"Caption\":\"\",\"EnableCaption\":0,\"Name\":\"F1.Equation\",\"CaptionPosition\":1,\"ControlWidth\":113,\"ControlHeight\":22,\"Font\":0,\"AlignText\":0,\"EnableBorder\":0,\"CaptionOnly\":0,\"Value\":\"FFT(C1)\",\"WidgetIndex\":9,\"WidgetType\":\"WidgetStaticText.1\"},\
			{\"WidgetPosition\":[191,409,230,445],\"IsWidgetVisible\":0,\"Caption\":\"F1\",\"EnableCaption\":0,\"Name\":\"F1.MeasureOpGui\",\"CaptionPosition\":1,\"ControlWidth\":39,\"ControlHeight\":36,\"Font\":0,\"ButtonOnly\":-1,\"PickerTitle\":\"Select Measurement To Graph\",\"Value\":\"Amplitude\",\"WidgetIndex\":17,\"WidgetType\":\"WidgetProcessorCombobox.1\"},\
			{\"WidgetPosition\":[191,409,230,445],\"IsWidgetVisible\":-1,\"Caption\":\"F1\",\"EnableCaption\":0,\"Name\":\"F1.Operator1gui\",\"CaptionPosition\":1,\"ControlWidth\":39,\"ControlHeight\":36,\"Font\":0,\"ButtonOnly\":-1,\"PickerTitle\":\"Select Math Operator\",\"Value\":\"FFT\",\"WidgetIndex\":25,\"WidgetType\":\"WidgetProcessorCombobox.1\"},\
			{\"WidgetPosition\":[191,452,230,487],\"IsWidgetVisible\":-1,\"Caption\":\"F2\",\"EnableCaption\":0,\"Name\":\"F2.Operator1gui\",\"CaptionPosition\":1,\"ControlWidth\":39,\"ControlHeight\":36,\"Font\":0,\"ButtonOnly\":-1,\"PickerTitle\":\"Select Math Operator\",\"Value\":\"Zoom\",\"WidgetIndex\":26,\"WidgetType\":\"WidgetProcessorCombobox.1\"}\
		]\
	 }";

	// Parse example data
	JSONValue *value = JSON::Parse(EXAMPLE3);
	if (value)
	{
		debug("-----------\r\n");
		debug(value->Stringify().c_str());
		debug("\r\n");
		debug("-----------\r\n");
		debug(value->Stringify(true).c_str());
		debug("\r\n");
		debug("-----------\r\n");
	}

	// Clean up
	delete value;
}

// Example 4 : List keys in an object.
void example3()
{
	// Parse the example.
	JSONValue *main_object = JSON::Parse(EXAMPLE);
	if (main_object == NULL)
	{
		debug("Example code failed to parse, did you change it?\r\n");
	}
	else if (!main_object->IsObject())
	{
		debug("Example code is not an object, did you change it?\r\n");
		delete main_object;
	}
	else
	{
		// Print the main object.
		debug("Main object:\r\n");
		debug(main_object->Stringify(true).c_str());
		debug("-----------\r\n");

		// Fetch the keys and print them out.
		Common::Array<Common::String> keys = main_object->ObjectKeys();

		Common::Array<Common::String>::iterator iter = keys.begin();
		while (iter != keys.end())
		{
			debug("Key: ");
			debug((*iter).c_str());
			debug("\r\n");

			// Get the key's value.
			JSONValue *key_value = main_object->Child((*iter).c_str());
			if (key_value)
			{
				debug("Value: ");
				debug(key_value->Stringify().c_str());
				debug("\r\n");
				debug("-----------\r\n");
			}

			// Next key.
			iter++;
		}

		delete main_object;
	}
}
