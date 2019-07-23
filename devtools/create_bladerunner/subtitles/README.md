# Blade Runner (Westwood Studios, 1997) Subtitles Support
Some tools written in __Python 2.7__ to help add support for subtitles in Westwood's point and click adventure game Blade Runner (1997) for PC.
The official English, German, French, Italian and Spanish versions of the game should be supported.

## Quick guide for volunteer transcribers
1.  Please make sure you have an audio player installed in your Operating System (such as VLC).

2.  You will need python 2.7 installed for this guide, as well as the python libraries Pillow (or PIL), xlrd, xlwt, wave and xlutils.

3.  Using git, checkout the latest revision of the ScummVM repository from GitHub.

4.  Make sure you've created a Blade Runner game directory and added Blade Runner in ScummVM as per the instructions in the wiki (the actual required files are __1.TLK, 2.TLK, 3.TLK, A.TLK and SPCHSFX.TLK__: <https://wiki.scummvm.org/index.php?title=User_Manual/Installing_a_game_for_use_with_ScummVM#Blade_Runner>
    For this guide it is assumed that this folder is:
    ```console
    C:\Westwood\Blade Runner\
    ```

5.  Create a folder on your HDD to export all speech audio from the game. Eg. create:
    ```console
    C:\Westwood\Blade Runner\AUDIO_FRA\
    ```

6.  Export all speech audio from the game and create an Excel file (xls) with links to the audio files.
    *   You will need to have some free HDD space for this step (around 650MB).

    *   Using a command line interface (eg MSYS2) navigate to the ScummVM repository folder and issue a command like the one below, making sure you specify the correct target language code for your requirements; supported language codes are "EN\_ANY" for English, "FR\_FRA" for French, "DE\_DEU" for German, "IT\_ITA" for Italian, "ES\_ESP" for Spanish and "RU\_RUS" for Russian (version by Fargus Multimedia).

    *   In the following example we assume we need to export the audio from the French version of the game. This could take a few minutes to complete. Please refer to the [quotesSpreadsheetCreator](#quotesspreadsheetcreator-quotespreadsheetcreatorpy) section of this document for more command-line options for this tool.
        ```bash
        python2.7 devtools/create_bladerunner/subtitles/quotesSpreadsheetCreator/quoteSpreadsheetCreator.py -op "/c/Westwood/Blade Runner/AUDIO_FRA" -ip "/c/Westwood/Blade Runner" -ian "devtools/create_bladerunner/subtitles/common/actornames.txt" -xwav -ld FR_FRA
        ```

7.  When you've completed the above step, an Excel file should be created in your current folder, named "out-French.xls". Open this file with a spreadsheet editor app (tested with Microsoft Excel 2007 and LibreOffice Calc 6.2.4) and select the "INGQUO_#.TR#" sheet.
    *   Please keep the column A ("Filename") as is. Do not modify the contents of this column.

    *   Edit column B ("Quote") to put in the text for your transcript of the corresponding audio file

    *   Use column G ("ShortHandFileName") to play the audio file for the quote to be transcribed in the same line. You should be able to double click in MS Excel or Ctrl+Click on the specific cell; an audio player will be launched automatically to play the audio file.

    *   Note that the columns used by the tool that will create the final subtitles file are only columns A and B.

8.  The rest of the sheets in the Excel file, excluding the "SBTLVERS.TRE" which contains versioning info) are the sheets for the video cutscenes. We don't provide links to the audio cues for those subtitles, but we do provide the timings for the English version of these scenes which you can use as a guide, along with the English transcript available online here: <https://docs.google.com/spreadsheets/d/17ew0YyhSwqcqZg6bXrIgz0GkA62dhgViHN15lOu5Hj8/edit?usp=sharing>
    *   Note that the important columns used by the tool that will create the final subtitles file are in this case only columns A ("Frame Start"), B ("Frame End") and C ("Subtitle").

    *   Fill in column C with the subtitle text, and set the values in columns A and B to adjust when the subtitle should start and when it should stop being displayed respectively.

9.  You may edit the info in the "SBTLVERS.TRE" sheet to fill in the credits and a version number for your transcript.

10. If you're transcribing for a non-Latin language, you will need to create a special font for the subtitles in order for the game to display your transcript properly. Please, refer to the [fontCreator](#fontcreator-fontcreatorpy) section of this document for this process.

## Building and installing a SUBTITLES.MIX file with a "make" command
You need to follow these instructions:

1.  Download the online Excel transcript and save it as "englishTranscript.xlsx" into the "devtools\create_bladerunner\subtitles\sampleInput" folder.
    __The online Excel file is available here:__
    <https://docs.google.com/spreadsheets/d/17ew0YyhSwqcqZg6bXrIgz0GkA62dhgViHN15lOu5Hj8/edit?usp=sharing>

2.  Edit your font glyphs PNG file (or use the provided one in the sampleInput folder). This file should be stored as "subtitlesFont.png" into the "devtools\create\_bladerunner\subtitles\sampleInput" folder.

3.  Create an overrideEncodingSUBLTS.txt file in the sampleInput folder. This is a configuration file for the font file creation. A sample is provided in the sampleInput folder and documentation about this is below in this document (see "override encoding text file" in fontCreator).

4.  Create a configureFontsTranslation.txt in the sampleInput folder. A configuration file for the MIX file creation. A sample is provided in the sampleInput folder and documentation about this is below in this document (see "text configuration file" in mixResourceCreator).  

5.  From the ScummVM root folder run:
    ```bash
    make devtools/create_bladerunner/subtitles
    ```

6.  You may need to install some required Python 2 libraries such as Pillow (or PIL), xlrd, xlwt and wave (which is part of the Python 2 Standard Library).

7.  Copy the output file "SUBTITLES.MIX", created in the ScummVM root folder, into your Blade Runner game directory.

8.  Launch the Blade Runner game using ScummVM.

## quotesSpreadsheetCreator (quoteSpreadsheetCreator.py)
(requires python libs *xlwt* (tested with version 1.3.0), *wave* (included in the Python 2 Standard Library)

A tool to gather all the speech audio filenames in an Excel file which will include a column with links to the audio file location on the PC. By Ctrl+MouseClick on that column's entries you should be able to listen to the corresponding WAV file.

The output Excel file *out.xls* should help with the transcription of all the spoken *in-game* quotes. It also provides extra quote information such as the corresponding actor ID and quote ID per quote.

Note 1: A lot of extra information has been added to the output Excel file maintained for the English transcription, such as whether a quote is unused or untriggered, the person a quote refers to (when applicable), as well as extra quotes that are not separate Audio files (AUD) in the game's archives but are part of a video file (VQA) or were text resources (TRx) for dialogue menus, UI etc. Therefore, this tool is provided here mostly for archiving purposes.

__The online Excel file is available here:__
<https://docs.google.com/spreadsheets/d/17ew0YyhSwqcqZg6bXrIgz0GkA62dhgViHN15lOu5Hj8/edit?usp=sharing>

Syntax Notes:

1.  The "-op" switch should be followed by the path to the folder where the WAV files should be exported; This folder path will also be used as input when the output Excel will be created (for the "INGQUO\_x.TRx" sheet with the in-game quotes).

2.  The "-ip" switch should be followed by the path to the game's folder, where the TLK and MIX files reside.

3.  The "-ian" optional switch is followed by the path to the actornames.txt file -- if this is omitted then the file is assumed to reside in the current working directory.

4.  The "-ld" optional switch is followed by a language description for the language of the game you are exporting Text Resources from. This switch is meaningful when you also use the "-xtre" switch to export Text Resource files.
    *   Valid language values are: EN\_ANY, DE\_DEU, FR\_FRA, IT\_ITA, ES\_ESP, RU\_RUS 

    *   Default language value is: EN\_ANY (English)

5.  The "-xwav" optional switch will export __ALL__ game's audio files (AUD) (that are either speech or speech-related) in a WAV format. This is expected to run for a few minutes and take up quite a lot of your HDD space (around 650MB).

6.  The "-xtre" optional switch will add extra sheets to the output Excel with the contents of each of the game's Text Resource files (TRx) (on sheet per TRx file).

7.  The "-xdevs" optional switch will add a sheet for Developer Commentary text and some additional voice-overs from SFX.MIX.

8.  The "-xpogo" optional switch will add a sheet for the POGO text.

9.  You may use all, a subset or none of the "-xwav", "-xtre", "-xpogo", "-xdevs" switches, depending on what you need to do.

10. The "--trace" optional switch enables extra debug messages to be printed. 

Usage:
```bash
python2.7 quoteSpreadsheetCreator.py -op folderpath_for_exported_wav_Files [-ip folderpath_for_TLK_Files] [-ian pathToActorNamesTxt] [-m stringPathToReplaceFolderpathInExcelLinks] [-ld languageDescription] [-xwav] [-xtre] [--trace]
```

The tool __requires__ a valid path to the actornames.txt file; this file is included in the samples folder.

## mixResourceCreator (mixResourceCreator.py)
(requires python libs *xlrd* (tested with version 1.2.0), *xlwt* (tested with version 1.3.0), *xlutils* (tested with version 2.0.0))

A tool to process the aforementioned Excel file with the dialogue transcriptions and output text resource files (TRx) that will be packed along with the external font (see fontCreator tool) into a SUBTITLES.MIX file. Multiple TRx files will be created intermediately in order to fully support subtitles in the game. One TRx file includes all in-game spoken quotes and the rest of them correspond to any VQA video sequence that contain voice acting.

Usage:
```bash
python2.7 mixResourceCreator.py -x excelWithTranscriptSheets.xlsx [-ian pathToActorNamesTxt] [-cft pathToConfigureFontsTranslationTxt] [--trace]
```

The tool __requires__ a valid path to the actornames.txt file, which is included in the samples folder.

Syntax Notes: 

1.  The "-x" switch is followed by the path to the input Excel file (xls or xlsx) which should contain the transcript sheet(s).

2.  The "-ian" optional switch is followed by the path to the actornames.txt file -- if this is omitted then the file is assumed to reside in the current working directory.

3.  The "-cft" optional switch is followed by the path to the text configuration file "configureFontsTranslation.txt" -- if this is omitted then the file is assumed to reside in the current working directory.

4.  The "-ld" optional switch is followed by a language description for the language of the game you are exporting Text Resources from. This switch is meaningful when you also use the "-xtre" switch to export Text Resource files.
    *   Valid language values are: EN\_ANY, DE\_DEU, FR\_FRA, IT\_ITA, ES\_ESP, RU\_RUS, EFIGS

    *   Default language value is: EN\_ANY (English)

5.  The "--trace" optional switch enables extra debug messages to be printed. 

The __text configuration file "configureFontsTranslation.txt"__ is a __text file that should be saved in a UTF-8 encoding (no BOM)__, that contains the following:

1.  Multiple lines (practically one per imported Font type) with the key "fontNameAndOutOfOrderGlyphs" and a '\#'-separated value that contains:
    *   the name of an in-game Font "type" that will be included in the SUBTITLES.MIX resource archive. You could also have lines for Font "types" not imported in the SUBTITLES.MIX, but are already Font types used by the game (eg. KIA6PT) especially when you intend to add translated/ updated TRx files in the SUBTITLES.MIX (however even in that case, there are default settings for those fonts that the script will use if you don't specify alternative settings in this file to override them). Valid values can be: SUBTLS\_E (for all subtitles), KIA6PT, TAHOMA (for both TAHOMA18 and TAHOMA24), SYSTEM (this is basically for the ERRORMSG.TRx).

    *   a '\#' character after the font type

    *   the name of the 8-bit string encoding (codepage) (eg. cp437 or windows-1252) that should be used to store the texts that uses this particular font "type" in the game's TRx text resource files. Valid values can be the standard encodings listed in python's page:
        <https://docs.python.org/2.7/library/codecs.html#standard-encodings>

    *   a '\#' character after the target Encoding

    *   a list of comma separated tuples that specify the mapping of special (out of order) character to placeholder characters from the selected codepage. See fontCreator section for more details on this.

    *   For example: 
        ```text
        fontNameAndOutOfOrderGlyphs=SUBTLS_E#windows-1253#í:Ά,ñ:¥,â:¦,é:§,Ά:£
        fontNameAndOutOfOrderGlyphs=KIA6PT#cp437#
        fontNameAndOutOfOrderGlyphs=TAHOMA#cp437#
        fontNameAndOutOfOrderGlyphs=SYSTEM#latin-1#
        ```

Note: for font files (FON) that you have created or edited with the fontCreator tool (e.g for the "SUBTLS\_E.FON" file for subtitles, or another in-game font file eg. KIA6PT, TAHOMA) you __should copy the 8-bit encoding and the comma separated out of order character tuples from the respective "override encoding" text file that you used with the fontCreator tool for each new/ edited font in the configureFontsTranslation.txt fields__. 

Additionally, __all the new and edited fonts (FON files that were output by the fontCreator script) should be in your working directory__ in order to include them in the SUBTITLES.MIX. 

It's important to keep the naming of those files unchanged. __Supported name values for imported FON files__ are: SUBTLS\_E.FON, KIA6PT.FON, TAHOMA18.FON, TAHOMA24.FON and SYSTEM.FON (practically you won't be using the last one).

## fontCreator (fontCreator.py)
(requires python Image library *Pillow* (tested with version 5.4.1))

A tool to support __both__ the exporting of fonts from the game (to PNG images) __and__ the creation of a font file (FON) in order to resolve various issues with the available fonts (included in the game's own resource files). These issues include alignment, kerning, corrupted format, limited charset and unsupported characters -- especially for languages with too many non-Latin symbols in their alphabet.
This font tool's code is based off the Monkey Island Special Edition's Translator (<https://github.com/ShadowNate/MISETranslator>).

Usage:

Syntax A - To export game fonts to PNG images:
```bash
python2.7 fontCreator.py -ip folderpathForMIXFiles
```

Syntax B - To create the subtitle's font:
```bash
python2.7 fontCreator.py -im imageRowPNGFilename -om targetFONfilename [-oe pathToOverrideEncodingTxt] -pxLL minSpaceBetweenLettersInRowLeftToLeft -pxTT minSpaceBetweenLettersInColumnTopToTop -pxKn kerningForFirstDummyFontLetter -pxWS whiteSpaceWidthInPixels [--trace]
```

This tool __requires an override encoding text file__ in its Syntax B mode (subtitle font creation). You can specify the path to this file after a "-oe" switch. If you don't provide this path, the script will search for an "overrideEncoding.txt" file in the current working directory.

The override encoding file is a __text file that should be saved in a UTF-8 encoding (no BOM)__, that contains the following:

1.  A key "targetEncoding" with a value of the name of the ASCII codepage (8-bit encoding) that should be used for the character fonts (eg windows-1253). Valid values can be the standard encodings listed in python's page:
    <https://docs.python.org/2.7/library/codecs.html#standard-encodings>

2.  A key "asciiCharList" with value the "all-characters" string with all the printable characters that will be used in-game, from the specified codepage. Keep in mind that:
    *   The first such character (typically this is the '!' character) should be repeated twice!

    *   All characters must belong to the specified codepage.

    *   The order that the characters appear in the string should match their order in the ASCII table of the codepage.

    *   You don't need to use all the characters of the specified codepage in your "all-characters" string.

    *   For any special characters that don't appear in the target codepage (eg ñ, é, í, â don't appear in the Greek codepage), you'll have to decide on an ASCII value for them (one not used by another character appearing in-game). 

    *   In the "all-characters" string you should put as placeholders the actual characters from the specified codepage that correspond to the ASCII values you have decided above; The placeholder characters should also be in the proper order (by their ASCII value) inside the string.

3.  A key "explicitKerningList" with value a list of comma separated tuples where each tuple specifies a character and its manually set kerning (x-offset) in pixels. Kerning can have integer (positive or negative) values. This list is optional and may be skipped, if you put a '-' instead of a list.
    *   Example: explicitKerningList=i:-1

    *   Don't use space(s) between the tuples!

4.  A key "explicitWidthIncrement" with value a list of comma separated tuples  where each tuple specifies a character and its manually set extended width in pixels. This should be a positive integer value. You can skip this list by not writing anything in the file after the previous (manual kerning) list.
    *   Example: explicitWidthIncrement=i:0,j:1,l:1

    *   Don't use space(s) between the tuples!

5.  A key "originalFontName" with the FON file's original name in the game (the one that it should replace). Use SUBLTS for the subtitles FON.
    *   Example: originalFontName=SUBLTS	

6.  A key "specialOutOfOrderGlyphsUTF8ToAsciiTargetEncoding" with value a list of comma separated tuples that indicates which character glyphs should replace the placeholder glyphs in your all-character string above.
    *   Example: specialOutOfOrderGlyphsUTF8ToAsciiTargetEncoding=í:Ά,ñ:¥,â:¦,é:§,Ά:£

    *   Don't use space(s) between the tuples!

There is a sample of such file in the source folder for the fontCreator tool.

__For the exporting of the game fonts (to PNG) mode__, the valid syntax expects only one (1) argument:

1.  folderpathForMIXFiles: is the path where the game's MIX files are located (STARTUP.MIX is required). The exported font files will be: 10PT.FON.PNG, TAHOMA18.FON.PNG, TAHOMA24.FON.PNG and KIA6PT.FON.PNG.

__For the creation of subtitles' font mode__, there are six (6) mandatory launch arguments for the fontCreator tool:

1.  imageRowPNGFilename: is the filename of the input PNG image file which should contain a row of (preferably) tab separated glyphs. Example: "Tahoma\_18ShdwTranspThreshZero003-G5.png". Keep in mind that:
    *   The first glyph should be repeated here too, as in the override encoding text file.

    *   Background should be transparent.

    *   All colors used in the character glyphs should not have any transparency value (eg from Gimp 2, set Layer->Transparency->Threshold alpha to 0). There's no partial transparency supported by the game. A pixel will either by fully transparent or fully opaque.

    *   If you use special glyphs that are not in the specified ASCII codepage (eg ñ, é, í, â don't appear in the Greek codepage), then in this image file you should use the actual special glyphs - put them at the position of the placeholder characters in your "all-characters" string that you've specified in the override encoding text file.

2.  targetFONfilename: Example: "SUBTLS\_E.FON" for the subtitles.

3.  minSpaceBetweenLettersInRowLeftToLeft: This is a length (positive integer) in pixels that indicates the __minimum__ distance between the left-most side of a glyph and the left-most side of the immediate subsequent glyph in the input image PNG (row of glyphs) file.

    This basically tells the tool how far (in the x axis) it can search for pixels that belong to the same glyph). You can input an approximate value here and adjust it based on the output of the tool (the tool should be able to detect ALL the glyphs in the PNG row image file and it will report how many it detected in its output)

4.  minSpaceBetweenLettersInColumnTopToTop: This is a positive integer in pixels that indicates the __minimum__ distance between the top-most pixel of a glyph and the top-most pixel of a glyph on another row of the input image file.

    It is highly recommended, though, that the input image file should contain only a single row of glyphs and this value should be higher than the maximum height among the glyphs, typically this should be set to approximately double the maximum height of glyph.

5.  kerningForFirstDummyFontGlyph: This is an integer that explicitly indicates the kerning, ie. offset in pixels (on the x-axis) of the first glyph (the one that is repeated twice). This can be measured by observing the indent that your image processing app adds when you enter the first glyph (typically it should be only a few pixels)

6.  whiteSpaceWidthInPixels: This is a positive integer value that sets the width in pixels for the single white space between words for the subtitles in-game.

The "--trace" optional switch enables extra debug messages to be printed. 

A suggested method of creating decent looking PNG with the row of glyphs for your subtitles' font is the following:

1.  Create the font row in __GIMP__
    *   Start with a __new__ empty image, with transparent background. Choose a large enough canvas width (you can adjust it later)     

    *   Paste as a new layer a tab separated alphanumeric sequence with all your glyphs (as specified above). Choose white as the font's color.

    *   Adjust your canvas' width and height to make sure all the glyphs are within its borders.

2.  Add layers for shadows if necessary (recommended) by duplicating the original layer with the (white colored) glyphs to create layers that would be used for the shadow effect. Those layers should be __colorified__ as black and placed behind the original layer, displaced by one (1) pixel from eg. the top, right, left, and bottom (it's recommended to do this for all of those four).

3.  __Merge all visible__ layers (maintaining an alpha channel for the background)

4.  __Select all__ and __float the selection__, which should create a floating selection with all the letter glyphs.
    *   __Promote that selection to a layer__ and __duplicate__ it.

5.  Choose one of the duplicated layers and __COLORIFY__ it to pitch black.
    *   __Set the transparency threshold__ of this black layer to 0.

6.  Finally, place this completely black colored layer underneath the other one and __merge the visible__ layers.

7.  Export your image to a PNG file. 

This should get rid of semi-transparent pixels while maintaining the "aliasing" effect. 
There could be a better way but this should work ok.

## Credits and Special Thanks
*   All the developer guys from the ScummVM (<https://github.com/scummvm/scummvm>) team, and especially the ones involved in the implementation of the BladeRunner engine for ScummVM (madmoose, peterkohaut, sev and everyone else).

*   The information provided in this blog (<http://westwoodbladerunner.blogspot.ca>) by Michael Liebscher.

*   The creator of br-mixer (<https://github.com/bdamer/br-mixer>), Ben Damer, who also has a blog entry about the game resource file formats (<http://afqa123.com/2015/03/07/deciphering-blade-runner/>)
