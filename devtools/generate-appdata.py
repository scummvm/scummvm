#!/usr/bin/env python3

# This script generates a dists/scummvm.appdata.xml file with multilanguage support.
# The multilanguage data is extracted from po/*.po files

import re
import os

appdata_xml_template = '''<?xml version="1.0" encoding="UTF-8"?>
<!-- Copyright 2020-2023 The ScummVM Team -->
<component type="desktop">
  <id>scummvm.desktop</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-3.0-or-later</project_license>
  <name>ScummVM</name>
  <summary>Interpreter for numerous adventure games and role-playing games</summary>
  <summary xml:lang="xy">I18N: One line summary as shown in *nix distributions</summary>
  <developer_name>The ScummVM Team</developer_name>
  <provides>
    <id>scummvm.desktop</id>
  </provides>
  <description>
    <p>
      ScummVM is a program which allows you to run a wide variety of classic
      graphical point-and-click adventure games and role-playing games,
      provided you already have their data files. The clever part about this:
      ScummVM just replaces the executables shipped with the game,
      allowing you to play them on systems for which they were never designed!
    </p>
    <p xml:lang="xy">I18N: 1 of 3 paragraph of ScummVM description in *nix distributions</p>
    <p>
      Currently, ScummVM supports a huge library of adventures with over 250 games in total.
      It supports many classics published by legendary studios like LucasArts, Sierra On-Line,
      Revolution Software, Cyan, Inc. and Westwood Studios.
    </p>
    <p xml:lang="xy">I18N: 2 of 3 paragraph of ScummVM description in *nix distributions</p>
    <p>
      Next to ground-breaking titles like the Monkey Island series, Broken Sword,
      Myst, Blade Runner and countless other games you will find
      some really obscure adventures and truly hidden gems to explore.
    </p>
    <p xml:lang="xy">I18N: 3 of 3 paragraph of ScummVM description in *nix distributions</p>
  </description>
  <screenshots>
    <screenshot type="default">
      <image>https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_1.png</image>
      <caption>ScummVM Launcher</caption>
    </screenshot>
    <screenshot>
      <image>https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_2.png</image>
      <caption>ScummVM running 'Flight of the Amazon Queen'</caption>
    </screenshot>
    <screenshot>
      <image>https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_3.png</image>
      <caption>ScummVM running 'Beneath a Steel Sky'</caption>
    </screenshot>
    <screenshot>
      <image>https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_4.png</image>
      <caption>ScummVM running 'The Curse of Monkey Island'</caption>
    </screenshot>
    <screenshot>
      <image>https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_5.png</image>
      <caption>ScummVM running a demo of 'Myst'</caption>
    </screenshot>
  </screenshots>
  <url type="homepage">https://www.scummvm.org</url>
  <update_contact>scummvm-devel@lists.scummvm.org</update_contact>
  <content_rating type="oars-1.1">
    <content_attribute id="violence-cartoon">none</content_attribute>
    <content_attribute id="violence-fantasy">none</content_attribute>
    <content_attribute id="violence-realistic">none</content_attribute>
    <content_attribute id="violence-bloodshed">none</content_attribute>
    <content_attribute id="violence-sexual">none</content_attribute>
    <content_attribute id="violence-desecration">none</content_attribute>
    <content_attribute id="violence-slavery">none</content_attribute>
    <content_attribute id="violence-worship">none</content_attribute>
    <content_attribute id="drugs-alcohol">none</content_attribute>
    <content_attribute id="drugs-narcotics">none</content_attribute>
    <content_attribute id="drugs-tobacco">none</content_attribute>
    <content_attribute id="sex-nudity">none</content_attribute>
    <content_attribute id="sex-themes">none</content_attribute>
    <content_attribute id="sex-homosexuality">none</content_attribute>
    <content_attribute id="sex-prostitution">none</content_attribute>
    <content_attribute id="sex-adultery">none</content_attribute>
    <content_attribute id="sex-appearance">none</content_attribute>
    <content_attribute id="language-profanity">none</content_attribute>
    <content_attribute id="language-humor">none</content_attribute>
    <content_attribute id="language-discrimination">none</content_attribute>
    <content_attribute id="social-chat">none</content_attribute>
    <content_attribute id="social-info">none</content_attribute>
    <content_attribute id="social-audio">none</content_attribute>
    <content_attribute id="social-location">none</content_attribute>
    <content_attribute id="social-contacts">none</content_attribute>
    <content_attribute id="money-purchasing">none</content_attribute>
    <content_attribute id="money-gambling">none</content_attribute>
  </content_rating>
</component>
'''


def extract_summary(file):
    with open('../po/' + file) as f:
        content = f.read()
    pattern = r'#: dists\/scummvm\.appdata\.xml\.cpp:32\nmsgid ".+"\nmsgstr "(.+)"'
    summary_match = re.search(pattern, content)
    if summary_match:
        summary = summary_match.group(1)
        return summary.strip()
    else:
        return None


def extract_par1(file):
    with open('../po/' + file) as f:
        content = f.read()
    pattern = r'#: dists\/scummvm\.appdata\.xml\.cpp:37\nmsgid ""\n(.+\n)*msgstr ""\n((.+\n)*)'
    par1_match = re.search(pattern, content)
    if par1_match:
        par1 = par1_match.group(2)
        if par1:
            return par1.strip()
        else:
            return None
    else:
        return None


def extract_par2(file):
    with open('../po/' + file) as f:
        content = f.read()
    pattern = r'#: dists\/scummvm\.appdata\.xml\.cpp:45\nmsgid ""\n(.+\n)*msgstr ""\n((.+\n)*)'
    par2_match = re.search(pattern, content)
    if par2_match:
        par2 = par2_match.group(2)
        if par2:
            return par2.strip()
        else:
            return None
    else:
        return None


def extract_par3(file):
    with open('../po/' + file) as f:
        content = f.read()
    pattern = r'#: dists\/scummvm\.appdata\.xml\.cpp:51\nmsgid ""\n(.+\n)*msgstr ""\n((.+\n)*)'
    par3_match = re.search(pattern, content)
    if par3_match:
        par3 = par3_match.group(2)
        if par3:
            return par3.strip()
        else:
            return None
    else:
        return None


# get file names of all .po files
file_names = []
for filename in os.listdir("../po/"):
    if filename.endswith(".po"):
        file_names.append(filename)

file_names.sort()

# summary translations
summary_translations = ""

# first_translation is used to determine the indentation (first translation will not require any indentation)
first_translation = True

for file in file_names:
    summary = extract_summary(file)
    if (summary is None):
        continue

    lang = '"' + file[0] + file[1] + '"'
    summary_translations += ('' if first_translation else '  ') + '<summary xml:lang=' + \
        lang + '>' + summary + '</summary>\n'
    first_translation = False

summary_translations = summary_translations.rstrip('\n')

regex_pattern = r'<summary xml:lang="xy">I18N: One line summary as shown in \*nix distributions<\/summary>'

appdata_xml_template = re.sub(
    regex_pattern, summary_translations, appdata_xml_template)

# paragraph 1 translations
par1_translations = ""
first_translation = True

for file in file_names:
    par1 = extract_par1(file)
    if (par1 is None):
        continue

    # par1 also contains " (quotes) around the text; so we need to replace them with empty character
    # otherwise " (quotes) will appear in scummvm.appdata.xml generated file
    par1 = par1.replace('"', '')

    lang = '"' + file[0] + file[1] + '"'
    par1_translations += ('' if first_translation else '    ') + '<p xml:lang=' + \
        lang + '>' + par1 + '</p>\n'
    first_translation = False

par1_translations = par1_translations.rstrip('\n')

regex_pattern = r'<p xml:lang="xy">I18N: 1 of 3 paragraph of ScummVM description in \*nix distributions<\/p>'

appdata_xml_template = re.sub(
    regex_pattern, par1_translations, appdata_xml_template)

# paragraph 2 translations
par2_translations = ""
first_translation = True

for file in file_names:
    par2 = extract_par2(file)
    if (par2 is None):
        continue
    par2 = par2.replace('"', '')

    lang = '"' + file[0] + file[1] + '"'
    par2_translations += ('' if first_translation else '    ') + '<p xml:lang=' + \
        lang + '>' + par2 + '</p>\n'
    first_translation = False

par2_translations = par2_translations.rstrip('\n')

regex_pattern = r'<p xml:lang="xy">I18N: 2 of 3 paragraph of ScummVM description in \*nix distributions<\/p>'

appdata_xml_template = re.sub(
    regex_pattern, par2_translations, appdata_xml_template)

# paragraph 3 translations
par3_translations = ""
first_translation = True

for file in file_names:
    par3 = extract_par3(file)
    if (par3 is None):
        continue
    par3 = par3.replace('"', '')

    lang = '"' + file[0] + file[1] + '"'
    par3_translations += ('' if first_translation else '    ') + '<p xml:lang=' + \
        lang + '>' + par3 + '</p>\n'
    first_translation = False

par3_translations = par3_translations.rstrip('\n')

regex_pattern = r'<p xml:lang="xy">I18N: 3 of 3 paragraph of ScummVM description in \*nix distributions<\/p>'

appdata_xml_template = re.sub(
    regex_pattern, par3_translations, appdata_xml_template)

# write to scummvm.appdata.xml file
with open("../dists/scummvm.appdata.xml", "w") as f:
    f.write(appdata_xml_template)
