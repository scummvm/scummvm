#!/usr/bin/env python3

# This script generates dists/org.scummvm.scummvm.metainfo.xml file with multilanguage support.
# The multilanguage data is extracted from po/*.po files

import re
import os
import xml.sax.saxutils

METAINFO_OUTPUT_FILE = 'dists/org.scummvm.scummvm.metainfo.xml'
METAINFO_XML_TEMPLATE = '''<?xml version="1.0" encoding="UTF-8"?>
<!-- Copyright 2020-2025 The ScummVM Team -->
<component type="desktop-application">
  <id>org.scummvm.scummvm</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-3.0-or-later</project_license>
  <name>ScummVM</name>
  <summary>Interpreter for numerous adventure games and role-playing games</summary>
  <summary xml:lang="xy">I18N: One line summary as shown in *nix distributions</summary>
  <developer_name>The ScummVM Team</developer_name>
  <launchable type="desktop-id">org.scummvm.scummvm.desktop</launchable>
  <provides>
    <binary>scummvm</binary>
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

SUMMARY_TAG = 'dists/org.scummvm.scummvm.metainfo.xml.cpp:32'
SUMMARY_PAT = r'  <summary xml:lang="xy">I18N: One line summary as shown in *nix distributions</summary>'
PAR_TAGS = [
    'dists/org.scummvm.scummvm.metainfo.xml.cpp:37', # Paragraph 1
    'dists/org.scummvm.scummvm.metainfo.xml.cpp:45', # Paragraph 2
    'dists/org.scummvm.scummvm.metainfo.xml.cpp:51', # Paragraph 3
]
PAR_PATS = [
    r'    <p xml:lang="xy">I18N: 1 of 3 paragraph of ScummVM description in *nix distributions</p>',
    r'    <p xml:lang="xy">I18N: 2 of 3 paragraph of ScummVM description in *nix distributions</p>',
    r'    <p xml:lang="xy">I18N: 3 of 3 paragraph of ScummVM description in *nix distributions</p>',
]

BASE_PATH = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))

def extract_po_line(file, tag):
    with open(os.path.join(BASE_PATH, 'po', file), 'r') as f:
        content = f.read()
    pattern = r'#: {0}\nmsgid ".+"\nmsgstr "(.+)"\n'.format(re.escape(tag))
    line_match = re.search(pattern, content)
    if line_match:
        return line_match.group(1)
    else:
        return None


def extract_po_par(file, tag):
    with open(os.path.join(BASE_PATH, 'po', file), 'r') as f:
        content = f.read()
    pattern = r'#: {0}\nmsgid ""\n(?:".*"\n)+msgstr ""\n((?:".+"\n)+)'.format(re.escape(tag))
    par_match = re.search(pattern, content)
    if par_match:
        par = par_match.group(1)
        # Remove trailing \n
        par = par[:-1]
        # Remove quotes at start and end of line
        return [line[1:-1] for line in par.split('\n')]
    else:
        return None


def po_to_lang(po_file_name):
    # Remove .po extension
    lang = po_file_name[:-3]

    region_subtag = None
    variant_subtag = None
    # we use - for locale modifier (tarask)
    if '-' in lang:
        lang, variant_subtag = lang.split('-', maxsplit=1)

    if '_' in lang:
        lang, region_subtag = lang.split('_', maxsplit=1)

    primary_subtag = lang

    assert(len(primary_subtag) == 2)
    assert(region_subtag is None or len(region_subtag) == 2)
    assert(variant_subtag is None or 6 <= len(variant_subtag) <= 8)

    lang = primary_subtag.lower()
    if region_subtag:
        lang += '-' + region_subtag.upper()
    if variant_subtag:
        lang += '-' + variant_subtag.lower()

    return primary_subtag, lang


def get_summary_translations(po_file_names):
    summary_translations = []

    for file, lang in po_file_names.items():
        summary = extract_po_line(file, SUMMARY_TAG)
        if summary is None:
            continue

        summary = xml.sax.saxutils.escape(summary)

        summary_translations.append('  <summary xml:lang="{0}">{1}</summary>'.format(
                lang, summary))

    return '\n'.join(summary_translations)


def substitute_summary_translations(po_file_names, xml):
    summary_translations = get_summary_translations(po_file_names)
    return xml.replace(SUMMARY_PAT, summary_translations)


def get_parx_translations(po_file_names, tag):
    parx_translations = []

    for file, lang in po_file_names.items():
        parx = extract_po_par(file, tag)

        if parx is None:
            continue

        # In XML a newline will be replace by a space character
        # Join everything making it pretty and remove trailing spaces
        parx = '\n'.join(line.rstrip(' ') for line in parx)
        parx = xml.sax.saxutils.escape(parx)

        parx_translations.append('    <p xml:lang="{0}">{1}</p>'.format(lang, parx))

    return '\n'.join(parx_translations)


def substitute_parx_translations(po_file_names, xml):
    for tag, pat in zip(PAR_TAGS, PAR_PATS):
        parx_translations = get_parx_translations(po_file_names, tag)
        xml = xml.replace(pat, parx_translations)
    return xml


def get_po_files():
    po_file_names = []
    for filename in os.listdir(os.path.join(BASE_PATH, 'po')):
        if filename.endswith(".po"):
            po_file_names.append(filename)

    po_file_names.sort()

    po_langs = {}
    last_primary = None
    last_file = None
    for file in po_file_names:
        primary_subtag, lang = po_to_lang(file)
        if last_primary != primary_subtag:
            # We are sorted so it's a new primary group
            last_primary = primary_subtag
            did_dedup = False
            # Try with primary subtag first
            po_langs[file] = primary_subtag
        else:
            if not did_dedup:
                # Fix last file name because we have duplicate
                po_langs[last_file] = po_to_lang(last_file)[1]
                did_dedup = True
            # We got a duplicate lang code: deduplicate
            po_langs[file] = lang
        last_file = file

    return po_langs


def main():
    po_file_names = get_po_files()

    xml = substitute_summary_translations(
        po_file_names, METAINFO_XML_TEMPLATE)

    xml = substitute_parx_translations(po_file_names, xml)

    with open(os.path.join(BASE_PATH, METAINFO_OUTPUT_FILE), 'w') as f:
        f.write(xml)


if __name__ == '__main__':
    main()
