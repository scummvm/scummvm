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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "cloudthread.h"
#include "common/debug.h"
#include "common/json.h"
#include "common/system.h"
#include "common/timer.h"

void example1();
void example2();
void example3();

void cloudThread(void *thread) {
	CloudThread *cloudThread = (CloudThread *)thread;
	cloudThread->handler();
};

void CloudThread::handler() {
	if (_firstTime) {
		_firstTime = false;

		example1();
		example2();
		example3();
	} else { }
}

void CloudThread::setTimeout(int interval) {
	Common::TimerManager *manager = g_system->getTimerManager();
	if (!manager->installTimerProc(cloudThread, interval, this, "Cloud Thread"))
		warning("Failed to create cloud thread");
}

void CloudThread::unsetTimeout() {
	Common::TimerManager *manager = g_system->getTimerManager();
	manager->removeTimerProc(cloudThread);
}

void CloudThread::start() {
	setTimeout(1000000); //in one second
}

/// SimpleJSON examples:

using Common::JSON;
using Common::JSONValue;
using Common::JSONArray;
using Common::JSONObject;

// Just some sample JSON text, feel free to change but could break demo
const char *EXAMPLE = "\
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
void example1() {
	// Parse example data
	JSONValue *value = JSON::parse(EXAMPLE);

	// Did it go wrong?
	if (value == NULL) {
		debug("Example code failed to parse, did you change it?\r\n");
	} else {
		// Retrieve the main object
		JSONObject root;
		if (value->isObject() == false) {
			debug("The root element is not an object, did you change the example?\r\n");
		} else {
			root = value->asObject();

			// Retrieving a string
			if (root.find("string_name") != root.end() && root["string_name"]->isString()) {
				debug("string_name:\r\n");
				debug("------------\r\n");
				debug(root["string_name"]->asString().c_str());
				debug("\r\n\r\n");
			}

			// Retrieving a boolean
			if (root.find("bool_second") != root.end() && root["bool_second"]->isBool()) {
				debug("bool_second:\r\n");
				debug("------------\r\n");
				debug(root["bool_second"]->asBool() ? "it's true!" : "it's false!");
				debug("\r\n\r\n");
			}

			// Retrieving an array
			if (root.find("array_letters") != root.end() && root["array_letters"]->isArray()) {
				JSONArray array = root["array_letters"]->asArray();
				debug("array_letters:\r\n");
				debug("--------------\r\n");
				for (unsigned int i = 0; i < array.size(); i++) {
					//wstringstream output;
					debug("[%d] => %s\r\n", i, array[i]->stringify().c_str());
				}
				debug("\r\n");
			}

			// Retrieving nested object
			if (root.find("sub_object") != root.end() && root["sub_object"]->isObject()) {
				debug("sub_object:\r\n");
				debug("-----------\r\n");
				debug(root["sub_object"]->stringify().c_str());
				debug("\r\n\r\n");
			}
		}

		delete value;
	}
}

// Example 3 : compact vs. prettyprint
void example2() {
	const char *EXAMPLE3 =
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
	JSONValue *value = JSON::parse(EXAMPLE3);
	if (value) {
		debug("-----------\r\n");
		debug(value->stringify().c_str());
		debug("\r\n");
		debug("-----------\r\n");
		debug(value->stringify(true).c_str());
		debug("\r\n");
		debug("-----------\r\n");
	}

	// Clean up
	delete value;
}

// Example 4 : List keys in an object.
void example3() {
	// Parse the example.
	JSONValue *main_object = JSON::parse(EXAMPLE);
	if (main_object == NULL) {
		debug("Example code failed to parse, did you change it?\r\n");
	} else if (!main_object->isObject()) {
		debug("Example code is not an object, did you change it?\r\n");
		delete main_object;
	} else {
		// Print the main object.
		debug("Main object:\r\n");
		debug(main_object->stringify(true).c_str());
		debug("-----------\r\n");

		// Fetch the keys and print them out.
		Common::Array<Common::String> keys = main_object->objectKeys();

		Common::Array<Common::String>::iterator iter = keys.begin();
		while (iter != keys.end()) {
			debug("Key: ");
			debug((*iter).c_str());
			debug("\r\n");

			// Get the key's value.
			JSONValue *key_value = main_object->child((*iter).c_str());
			if (key_value) {
				debug("Value: ");
				debug(key_value->stringify().c_str());
				debug("\r\n");
				debug("-----------\r\n");
			}

			// Next key.
			iter++;
		}

		delete main_object;
	}
}
