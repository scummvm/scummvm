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


def get_summary_translations(po_file_names):
    summary_translations = ""

    # first_translation is used to determine the indentation (first translation will not require any indentation)
    first_translation = True

    for file in po_file_names:
        summary = extract_summary(file)
        if (summary is None):
            continue

        lang = '"' + file[0] + file[1] + '"'
        summary_translations += ('' if first_translation else '  ') + '<summary xml:lang=' + \
            lang + '>' + summary + '</summary>\n'
        first_translation = False

    summary_translations = summary_translations.rstrip('\n')

    return summary_translations


def substitute_summary_translations(summary_translations, xml):
    pattern = r'<summary xml:lang="xy">I18N: One line summary as shown in \*nix distributions<\/summary>'

    appdata_xml = re.sub(pattern, summary_translations, xml)

    return appdata_xml


def get_parx_translations(x, po_file_names):
    parx_translations = ""
    first_translation = True

    for file in po_file_names:
        parx = ""
        if (x == 1):
            parx = extract_par1(file)
        elif (x == 2):
            parx = extract_par2(file)
        else:
            parx = extract_par3(file)

        if (parx is None):
            continue

        # parx also contains " (quotes) around the text; so we need to replace them with empty character
        # otherwise " (quotes) will appear in scummvm.appdata.xml generated file
        parx = parx.replace('"', '')

        lang = '"' + file[0] + file[1] + '"'
        parx_translations += ('' if first_translation else '    ') + '<p xml:lang=' + \
            lang + '>' + parx + '</p>\n'
        first_translation = False

    parx_translations = parx_translations.rstrip('\n')

    return parx_translations


def substitute_parx_translations(x, parx_translations, xml):
    pattern = r'<p xml:lang="xy">I18N: 1 of 3 paragraph of ScummVM description in \*nix distributions<\/p>'

    if (x == 2):
        pattern = r'<p xml:lang="xy">I18N: 2 of 3 paragraph of ScummVM description in \*nix distributions<\/p>'
    elif (x == 3):
        pattern = r'<p xml:lang="xy">I18N: 3 of 3 paragraph of ScummVM description in \*nix distributions<\/p>'

    appdata_xml = re.sub(pattern, parx_translations, xml)
    return appdata_xml


def get_po_files():
    po_file_names = []
    for filename in os.listdir("../po/"):
        if filename.endswith(".po"):
            po_file_names.append(filename)

    po_file_names.sort()

    return po_file_names


def main():
    po_file_names = get_po_files()

    summary_translations = get_summary_translations(po_file_names)
    xml = substitute_summary_translations(
        summary_translations, appdata_xml_template)

    par1_translations = get_parx_translations(1, po_file_names)
    xml = substitute_parx_translations(1, par1_translations, xml)

    par2_translations = get_parx_translations(2, po_file_names)
    xml = substitute_parx_translations(2, par2_translations, xml)

    par3_translations = get_parx_translations(3, po_file_names)
    xml = substitute_parx_translations(3, par3_translations, xml)

    # write to scummvm.appdata.xml file
    with open("../dists/scummvm.appdat3a.xml", "w") as f:
        f.write(xml)


if __name__ == '__main__':
    main()
