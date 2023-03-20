#!/usr/bin/env python3

# This script generates a dists/scummvm.appdata.xml file with multilanguage support.
# The multilanguage data is extracted from po/*.po files

from xml.dom import minidom
import re
import os


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


doc = minidom.Document()

copyright_comment_elem = doc.createComment(
    " Copyright 2020-2023 The ScummVM Team ")
doc.appendChild(copyright_comment_elem)

component_elem = doc.createElement("component")
component_elem.setAttribute("type", "desktop")
doc.appendChild(component_elem)


id_elem = doc.createElement("id")
id_text = doc.createTextNode("scummvm.desktop")
id_elem.appendChild(id_text)
component_elem.appendChild(id_elem)

metadata_license_elem = doc.createElement("metadata_license")
metadata_license_text = doc.createTextNode("CC0-1.0")
metadata_license_elem.appendChild(metadata_license_text)
component_elem.appendChild(metadata_license_elem)

project_license_elem = doc.createElement("project_license")
project_license_text = doc.createTextNode("GPL-3.0-or-later")
project_license_elem.appendChild(project_license_text)
component_elem.appendChild(project_license_elem)

name_elem = doc.createElement("name")
name_text = doc.createTextNode("ScummVM")
name_elem.appendChild(name_text)
component_elem.appendChild(name_elem)

summary_elem = doc.createElement("summary")
summary_text = doc.createTextNode(
    "Interpreter for numerous adventure games and role-playing games")
summary_elem.appendChild(summary_text)
component_elem.appendChild(summary_elem)

# get file names of all .po files
file_names = []
for filename in os.listdir("../po/"):
    if filename.endswith(".po"):
        file_names.append(filename)

# summary translations
for file in file_names:
    summary = extract_summary(file)
    if (summary is None):
        continue
    summary_elem = doc.createElement("summary")
    summary_text = doc.createTextNode(summary)
    summary_elem.appendChild(summary_text)
    lang = file[0] + file[1]
    summary_elem.setAttribute("xml:lang", lang)
    component_elem.appendChild(summary_elem)

developer_name_elem = doc.createElement("developer_name")
developer_name_text = doc.createTextNode("The ScummVM Team")
developer_name_elem.appendChild(developer_name_text)
component_elem.appendChild(developer_name_elem)

provides_elem = doc.createElement("provides")
component_elem.appendChild(provides_elem)

id_elem = doc.createElement("id")
id_text = doc.createTextNode("scummvm.desktop")
id_elem.appendChild(id_text)
provides_elem.appendChild(id_elem)

description_elem = doc.createElement("description")
component_elem.appendChild(description_elem)

# paragraph 1
p_elem = doc.createElement("p")
p_text = doc.createTextNode('''ScummVM is a program which allows you to run a wide variety of classic
graphical point-and -click adventure games and role-playing games,
provided you already have their data files. The clever part about this:
ScummVM just replaces the executables shipped with the game,
allowing you to play them on systems for which they were never designed!''')
p_elem.appendChild(p_text)
description_elem.appendChild(p_elem)

# paragraph 1 translations
for file in file_names:
    par1 = extract_par1(file)
    if (par1 is None):
        continue

    # par1 also contains " (quotes) around the text; so we need to replace them with empty
    # character otherwise &quot; will appear in scummvm.appdata.xml generated file
    par1 = par1.replace('"', '')

    p_elem = doc.createElement("p")
    p_text = doc.createTextNode(par1)
    p_elem.appendChild(p_text)
    lang = file[0] + file[1]
    p_elem.setAttribute("xml:lang", lang)
    description_elem.appendChild(p_elem)

# paragraph 2
p_elem = doc.createElement("p")
p_text = doc.createTextNode('''Currently, ScummVM supports a huge library of adventures with over 250 games in total.
It supports many classics published by legendary studios like LucasArts, Sierra On-Line,
Revolution Software, Cyan, Inc. and Westwood Studios.''')
p_elem.appendChild(p_text)
description_elem.appendChild(p_elem)

# paragraph 2 translations
for file in file_names:
    par2 = extract_par2(file)
    if (par2 is None):
        continue
    par2 = par2.replace('"', '')
    p_elem = doc.createElement("p")
    p_text = doc.createTextNode(par2)
    p_elem.appendChild(p_text)
    lang = file[0] + file[1]
    p_elem.setAttribute("xml:lang", lang)
    description_elem.appendChild(p_elem)

# paragraph 3
p_elem = doc.createElement("p")
p_text = doc.createTextNode('''Next to ground-breaking titles like the Monkey Island series, Broken Sword,
Myst, Blade Runner and countless other games you will find
some really obscure adventures and truly hidden gems to explore.''')
p_elem.appendChild(p_text)
description_elem.appendChild(p_elem)

# paragraph 3 translations
for file in file_names:
    par3 = extract_par3(file)
    if (par3 is None):
        continue
    par3 = par3.replace('"', '')
    p_elem = doc.createElement("p")
    p_text = doc.createTextNode(par3)
    p_elem.appendChild(p_text)
    lang = file[0] + file[1]
    p_elem.setAttribute("xml:lang", lang)
    description_elem.appendChild(p_elem)

screenshots_elem = doc.createElement("screenshots")
component_elem.appendChild(screenshots_elem)

screenshot_elem = doc.createElement("screenshot")
screenshots_elem.appendChild(screenshot_elem)
screenshot_elem.setAttribute("type", "default")

image_elem = doc.createElement("image")
image_text = doc.createTextNode(
    "https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_1.png")
image_elem.appendChild(image_text)
screenshot_elem.appendChild(image_elem)

caption_elem = doc.createElement("caption")
caption_text = doc.createTextNode("ScummVM Launcher")
caption_elem.appendChild(caption_text)
screenshot_elem.appendChild(caption_elem)

screenshot_elem = doc.createElement("screenshot")
screenshots_elem.appendChild(screenshot_elem)

image_elem = doc.createElement("image")
image_text = doc.createTextNode(
    "https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_2.png")
image_elem.appendChild(image_text)
screenshot_elem.appendChild(image_elem)

caption_elem = doc.createElement("caption")
caption_text = doc.createTextNode(
    "ScummVM running 'Flight of the Amazon Queen'")
caption_elem.appendChild(caption_text)
screenshot_elem.appendChild(caption_elem)

screenshot_elem = doc.createElement("screenshot")
screenshots_elem.appendChild(screenshot_elem)

image_elem = doc.createElement("image")
image_text = doc.createTextNode(
    "https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_3.png")
image_elem.appendChild(image_text)
screenshot_elem.appendChild(image_elem)

caption_elem = doc.createElement("caption")
caption_text = doc.createTextNode("ScummVM running 'Beneath a Steel Sky'")
caption_elem.appendChild(caption_text)
screenshot_elem.appendChild(caption_elem)

screenshot_elem = doc.createElement("screenshot")
screenshots_elem.appendChild(screenshot_elem)

image_elem = doc.createElement("image")
image_text = doc.createTextNode(
    "https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_4.png")
image_elem.appendChild(image_text)
screenshot_elem.appendChild(image_elem)

caption_elem = doc.createElement("caption")
caption_text = doc.createTextNode(
    "ScummVM running 'The Curse of Monkey Island'")
caption_elem.appendChild(caption_text)
screenshot_elem.appendChild(caption_elem)

screenshot_elem = doc.createElement("screenshot")
screenshots_elem.appendChild(screenshot_elem)

image_elem = doc.createElement("image")
image_text = doc.createTextNode(
    "https://www.scummvm.org/frs/extras/storefront/screenshots/screenshot_5.png")
image_elem.appendChild(image_text)
screenshot_elem.appendChild(image_elem)

caption_elem = doc.createElement("caption")
caption_text = doc.createTextNode("ScummVM running a demo of 'Myst'")
caption_elem.appendChild(caption_text)
screenshot_elem.appendChild(caption_elem)

url_elem = doc.createElement("url")
url_text = doc.createTextNode("https://www.scummvm.org")
url_elem.appendChild(url_text)
url_elem.setAttribute("type", "homepage")
component_elem.appendChild(url_elem)

update_contact_elem = doc.createElement("update_contact")
update_contact_text = doc.createTextNode("scummvm-devel@lists.scummvm.org")
update_contact_elem.appendChild(update_contact_text)
component_elem.appendChild(update_contact_elem)

# content ratings
content_rating_elem = doc.createElement("content_rating")
content_rating_elem.setAttribute("type", "oars-1.1")
component_elem.appendChild(content_rating_elem)

content_attribute_ids = {
    "violence-cartoon": "none",
    "violence-fantasy": "none",
    "violence-realistic": "none",
    "violence-bloodshed": "none",
    "violence-sexual": "none",
    "violence-desecration": "none",
    "violence-slavery": "none",
    "violence-worship": "none",
    "drugs-alcohol": "none",
    "drugs-narcotics": "none",
    "drugs-tobacco": "none",
    "sex-nudity": "none",
    "sex-themes": "none",
    "sex-homosexuality": "none",
    "sex-prostitution": "none",
    "sex-adultery": "none",
    "sex-appearance": "none",
    "language-profanity": "none",
    "language-humor": "none",
    "language-discrimination": "none",
    "social-chat": "none",
    "social-info": "none",
    "social-audio": "none",
    "social-location": "none",
    "social-contacts": "none",
    "money-purchasing": "none",
    "money-gambling": "none",
}

for id in content_attribute_ids:
    content_attribute_elem = doc.createElement("content_attribute")
    content_attribute_text = doc.createTextNode(content_attribute_ids[id])
    content_attribute_elem.appendChild(content_attribute_text)
    content_attribute_elem.setAttribute("id", id)
    content_rating_elem.appendChild(content_attribute_elem)

# write to scummvm.appdata.xml file
with open("../dists/scummvm.appdata.xml", "wb") as f:
    f.write(doc.toprettyxml(indent="  ", encoding="UTF-8"))
