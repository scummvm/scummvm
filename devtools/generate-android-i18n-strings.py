#!/usr/bin/env python3

# This script generates dists/android/res/values-<qualifier>/strings.xml files
# to add multilanguage support for android strings in res/values/strings.xml file.
# It considers dists/android/res/values/strings.xml file as a base template to generate
# those files. Relevant translated strings.xml file will be automatically used based on
# the android system's language.
#
# Also, this script generates a fake cpp file (dists/android/strings.xml.cpp) with strings
# from dists/android/res/values/strings.xml wrapped inside _() to be picked up by
# gettext for weblate translations

import polib
import os
import re
import xml.etree.ElementTree as ET


def generate_fake_cpp():
    cpp_text = '''/* ScummVM - Graphic Adventure Engine
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
 * This is an auto generated dummy file used for sticking strings from
 * dists/android/res/values/strings.xml into our translation system
 *
 */
 
#include "common/translation.h" // For catching the file during POTFILES reviews\n
'''
    with open('../dists/android.strings.xml.cpp', 'w') as file:
        file.write(cpp_text)
        tree = ET.parse('../dists/android/res/values/strings.xml')
        root = tree.getroot()
        for string in root.findall('string'):
            if (string.attrib.get("translatable") != "false"):
                file.write(
                    f'static Common::U32String {string.attrib.get("name")} = _("{string.text}");\n')


def extract_translations(file):
    po_file = polib.pofile('../po/' + file + '.po')
    translations = {}
    for entry in po_file:
        if entry.msgid and entry.msgstr:
            translations[entry.msgid] = entry.msgstr
    return translations


def escape_special_characters(translated_string):
    '''Some characters like single quote (') have special usage in XML and hence must be escaped.\n
    See: https://developer.android.com/guide/topics/resources/string-resource.html#escaping_quotes
    '''
    escaped = translated_string.replace('@', '\\@')
    escaped = escaped.replace('?', '\\?')
    escaped = escaped.replace('\'', '\\\'')
    escaped = escaped.replace('\"', '\\\"')
    escaped = escaped.replace('\n', '\\n')
    escaped = escaped.replace('\t', '\\t')

    return escaped


def is_bcp47_language_code(language_code):
    pattern = r'^[a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*$'
    if re.match(pattern, language_code):
        return True
    else:
        return False


def get_lang_qualifier(file):
    '''Generates <qualifier> for res/values-<qualifier> directory as per the specs given here:
    https://developer.android.com/guide/topics/resources/providing-resources#AlternativeResources
    '''
    lang_qualifier = file[0] + file[1]
    if (is_bcp47_language_code(file)):
        subtags = file.split("-")
        lang_qualifier = "+".join(subtags)
        lang_qualifier = "b+" + lang_qualifier
    else:
        lang_qualifier = file.replace('_', '-r')
    return lang_qualifier


def generate_translated_xml(file):
    tree = ET.parse('../dists/android/res/values/strings.xml')
    root = tree.getroot()

    translations = extract_translations(file)

    for string in root.findall('string'):
        if string.text in translations:
            string.text = escape_special_characters(translations[string.text])
        else:
            root.remove(string)

    ET.indent(tree, '  ')

    dir = '../dists/android/res/values-' + get_lang_qualifier(file)

    if not os.path.exists(dir):
        os.makedirs(dir)

    tree.write(dir + '/strings.xml', encoding='utf-8', xml_declaration=True)


def get_po_files():
    po_file_names = []
    for filename in os.listdir("../po/"):
        if filename.endswith(".po"):
            if (filename != "be-tarask.po"):
                # This skips be-tarask file because there is a bug with be-tarask that gives this compile error:
                # AAPT: error: failed to deserialize resource table: configuration has invalid locale 'be-'.
                # See the open issue here: https://issuetracker.google.com/issues/234820481
                po_file_names.append(os.path.splitext(filename)[0])

    return po_file_names


def main():
    generate_fake_cpp()

    po_file_names = get_po_files()

    for file in po_file_names:
        generate_translated_xml(file)


if __name__ == '__main__':
    main()
