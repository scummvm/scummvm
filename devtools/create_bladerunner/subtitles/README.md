# Blade Runner Subtitles

# Blade Runner (1997) Subtitles Support
Some tools written in __Python 2.7__ to help add support for subtitles in Westwood's point and click adventure game Blade Runner (1997) for PC.

## quotesSpreadsheetCreator (quoteSpreadsheetCreator.py)
(requires python lib *xlwt*, *wave*)
A tool to gather all the speech audio filenames in an Excel file which will include a column with links to the audio file location on the PC. By Ctrl+MouseClick on that column's entries you should be able to listen to the corresponding wav file.
The output excel file *out.xls* should help with the transcription of all the spoken *in-game* quotes. It also provides extra quote information such as the corresponding actor ID and quote ID per quote.

Note 1: A lot of extra information has been added to the output Excel file maintained for the English transcription, such as whether a quote is unused or untriggered, the person a quote refers to (when applicable), as well as extra quotes that are not separate Audio files (AUD) in the game's archives but are part of a video file (VQA) or were text resources (TRE) for dialogue menus, UI etc. Therefore, this tool is provided here mostly for archiving purposes.
__The online Excel file is available here:__
https://docs.google.com/spreadsheets/d/17ew0YyhSwqcqZg6bXrIgz0GkA62dhgViHN15lOu5Hj8/edit?usp=sharing

Syntax Notes: 
1. The "-op" switch should be followed by the path to the folder where the WAV files should be exported; This folder path will also be used as input when the output Excel will be created (for the "INGQUO_E.TRE" sheet with the in-game quotes).
2. The "-ip" switch should be followed by the path to the game's folder, where the TLK and MIX files reside.
3. The "-ian" optional switch is followed by the path to the actornames.txt file -- if this is omitted then the file is assumed to reside in the current working directory.
4. Using the "-xwav" optional switch, this tool will export __ALL__ game's audio files (AUD) (that are either speech or speech-related) in a WAV format. This is expected to run for a few minutes and take up quite a lot of your HDD space (about 650MB).
5. Using the "-xtre" optional switch, the tool will add a sheet to the output Excel with the contents of each of the game's Text Resource files (TRE).
6. You may use both, either or neither of the "-xwav" and "-xtre" switches, depending on what you need to do.
7. The "--trace" optional switch enables extra debug messages to be printed. 

Usage:
```
python2.7 quoteSpreadsheetCreator.py -op folderpath_for_extracted_wav_Files [-ip folderpath_for_TLK_Files] [-ian pathToActorNamesTxt] [-m stringPathToReplaceFolderpathInExcelLinks] [-xwav] [-xtre] [--trace]
```
The tool __requires__ a valid path to the actornames.txt file; this file is included in the samples folder.


## mixResourceCreator (mixResourceCreator.py)
(requires python lib *xlrd*)
A tool to process the aforementioned Excel file with the dialogue transcriptions and output text resource files (TRE) that will be packed along with the external font (see fontCreator tool) into a SUBTITLES.MIX file. Currently, a modified version of the ScummVM's BladeRunner engine is required for this MIX file to work in-game. Multiple TRE files will be created intermediately in order to fully support subtitles in the game. One TRE file includes all in-game spoken quotes and the rest of them correspond to any VQA video sequence that contain voice acting.
Usage:
```
python2.7 mixResourceCreator.py -x excelWithTranscriptSheets.xls [-ian pathToActorNamesTxt] [-cft pathToConfigureFontsTranslationTxt] [--trace]
```
The tool __requires__ a valid path to the actornames.txt file, which is included in the samples folder.

Syntax Notes: 
1. The "-x" switch is followed by the path to the input Excel file (xls) which should contain the transcript sheet(s).
2. The "-ian" optional switch is followed by the path to the actornames.txt file -- if this is omitted then the file is assumed to reside in the current working directory.
3. The "-cft" optional switch is followed by the path to the text configuration file "configureFontsTranslation.txt" -- if this is omitted then the file is assumed to reside in the current working directory.
4. The "--trace" optional switch enables extra debug messages to be printed. 

The text configuration file "configureFontsTranslation.txt" a __text file that should be saved in a UTF-8 encoding (no BOM)__, that contains the following:
1. A key "targetEncoding" with a value of the name of the ASCII codepage that should be used for the character fonts (eg windows-1253).
2. Multiple lines with the key "fontNameAndOutOfOrderGlyphs" and a value that contains:
    * the name of a in-game Font file that should be included in the SUBTITLES.MIX
    * a '#' character after the font name
	* a list of comma separated tuples that specify the mapping of special (out of order) character (see fontCreator) to placeholder characters from the selected codepage.
	* eg. fontNameAndOutOfOrderGlyphs=SUBTLS_E#í:Ά,ñ:¥,â:¦,é:§,Ά:£


## fontCreator (fontCreator.py)
(requires python Image library *PIL*)
A tool to support __both__ the extraction of fonts from the game __and__ the creation of a font file (FON) for use with (currently) a modified version of ScummVM's BladeRunner engine (WIP) in order to resolve various issues with the available fonts (included in the game's own resource files). These issues include alignment, kerning, corrupted format, limited charset and unsupported characters -- especially for languages with too many non-Latin symbols in their alphabet.
This font tool's code is based off the Monkey Island Special Edition's Translator (https://github.com/ShadowNate/MISETranslator).
Usage:
```
Syntax A - To export game fonts to PNG images:
python2.7 fontCreator.py -ip folderpathForMIXFiles

Syntax B - To create the subtitle's font:
python2.7 fontCreator.py -im imageRowPNGFilename -om targetFONfilename [-oe pathToOverrideEncodingTxt] -pxLL minSpaceBetweenLettersInRowLeftToLeft -pxTT minSpaceBetweenLettersInColumnTopToTop -pxKn kerningForFirstDummyFontLetter -pxWS whiteSpaceWidthInPixels [--trace]
```
This tool __requires__ an override encoding text file in its Syntax B mode (subtitle font creation). You can specify the path to this file after a "-oe" switch. If you don't provide this path, the script will search for an "overrideEncoding.txt" file in the current working directory.
The override encoding file is a __text file that should be saved in a UTF-8 encoding (no BOM)__, that contains the following:
1. A key "targetEncoding" with a value of the name of the ASCII codepage that should be used for the character fonts (eg windows-1253).
2. A key "asciiCharList" with value the "all-characters" string with all the printable characters that will be used in-game, from the specified codepage. Keep in mind that:
    * The first such character (typically this is the '!' character) should be repeated twice!
    * All characters must belong to the specified codepage.
    * The order that the characters appear in the string should match their order in the ASCII table of the codepage.
    * You don't need to use all the characters of the specified codepage in your "all-characters" string.
    * For any special characters that don't appear in the target codepage (eg ñ, é, í, â don't appear in the Greek codepage), you'll have to decide on an ASCII value for them (one not used by another character appearing in-game). 
    In the "all-characters" string you should put as placeholders the actual characters from the specified codepage that correspond to the ASCII values you have decided above; The placeholder characters should also be in the proper order (by their ASCII value) inside the string.
3. A key "explicitKerningList" with value a list of comma separated tuples where each tuple specifies a character and its manually set kerning (x-offset) in pixels. Kerning can have integer (positive or negative) values. This list is optional and may be skipped, if you put a '-' instead of a list.
    * Example: explicitKerningList=i:-1
    * Don't use space(s) between the tuples!
4. A key "explicitWidthIncrement" with value a list of comma separated tuples  where each tuple specifies a character and its manually set extended width in pixels. This should be a positive integer value. You can skip this list by not writing anything in the file after the previous (manual kerning) list.
    * Example: explicitWidthIncrement=i:0,j:1,l:1
    * Don't use space(s) between the tuples!
5. A key "originalFontName" with the FON file's original name in the game (the one that it should replace). Use SUBLTS for the subtitles FON.
    * Example: originalFontName=SUBLTS	
6. A key "specialOutOfOrderGlyphsUTF8ToAsciiTargetEncoding" with value a list of comma separated tuples that indicates which character glyphs should replace the placeholder glyphs in your all-character string above.
    * Example: specialOutOfOrderGlyphsUTF8ToAsciiTargetEncoding=í:Ά,ñ:¥,â:¦,é:§,Ά:£
    * Don't use space(s) between the tuples!
There is a sample of such file in the source folder for the fontCreator tool.
	
__For the exporting the game fonts (to PNG) mode__, the valid syntax expects only one (1) argument:
1. folderpathForMIXFiles: is the path where the game's MIX files are located (STARTUP.MIX is required). The exported font files will be: 10PT.FON.PNG, TAHOMA18.FON.PNG, TAHOMA24.FON.PNG and KIA6PT.FON.PNG.

__For the creation of subtitles' font mode__, there are six (6) mandatory launch arguments for the fontCreator tool:
1. imageRowPNGFilename: is the filename of the input PNG image file which should contain a row of (preferably) tab separated glyphs. Example: "Tahoma_18ShdwTranspThreshZero003-G5.png". Keep in mind that:
    * The first glyph should be repeated here too, as in the override encoding text file.
	* Background should be transparent.
	* All colors used in the character glyphs should not have any transparency value (eg from Gimp 2, set Layer->Transparency->Threshold alpha to 0). There's no partial transparency supported by the game. A pixel will either by fully transparent or fully opaque.
    * If you use special glyphs that are not in the specified ASCII codepage (eg ñ, é, í, â don't appear in the Greek codepage), then in this image file you should use the actual special glyphs - put them at the position of the placeholder characters in your "all-characters" string that you've specified in the override encoding text file.
2. targetFONfilename: Example: "SUBTLS_E.FON". Keep in mind that:
    * As of yet, only the SUBTLS_E.FON is supported by a modified (non-official) version of the BladeRunner ScummVM engine.
3. minSpaceBetweenLettersInRowLeftToLeft: This is a length (positive integer) in pixels that indicates the __minimum__ distance between the left-most side of a glyph and the left-most side of the immediate subsequent glyph in the input image PNG (row of glyphs) file.
This basically tells the tool how far (in the x axis) it can search for pixels that belong to the same glyph). You can input an approximate value here and adjust it based on the output of the tool (the tool should be able to detect ALL the glyphs in the PNG row image file and it will report how many it detected in its output)
4. minSpaceBetweenLettersInColumnTopToTop: This is a positive integer in pixels that indicates the __minimum__ distance between the top-most pixel of a glyph and the top-most pixel of a glyph on another row of the input image file.
It is highly recommended, though, that the input image file should contain only a single row of glyphs and this value should be higher than the maximum height among the glyphs, typically this should be set to approximately double the maximum height of glyph.
5. kerningForFirstDummyFontGlyph: This is an integer that explicitly indicates the kerning, ie. offset in pixels (on the x-axis) of the first glyph (the one that is repeated twice). This can be measured by observing the indent that your image processing app adds when you enter the first glyph (typically it should be only a few pixels)
6. whiteSpaceWidthInPixels: This is a positive integer value that sets the width in pixels for the single white space between words for the subtitles in-game.

The "--trace" optional switch enables extra debug messages to be printed. 

A suggested method of creating decent looking PNG with the row of glyphs for your subtitles' font is the following:
1. Create the font row in __GIMP__ 
    * Start with a __new__ empty image, with transparent background. Choose a large enough canvas width (you can adjust it later)     
    * Paste as a new layer a tab separated alphanumeric sequence with all your glyphs (as specified above). Choose white as the font's color.
    * Adjust your canvas' width and height to make sure all the glyphs are within its borders.
2. Add layers for shadows if necessary (recommended) by duplicating the original layer with the (white colored) glyphs to create layers that would be used for the shadow effect. Those layers should be __colorified__ as black and placed behind the original layer, displaced by one (1) pixel from eg. the top, right, left, and bottom (it's recommended to do this for all of those four).
3. __Merge all visible__ layers (maintaining an alpha channel for the background)
4. __Select all__ and __float the selection__, which should create a floating selection with all the letter glyphs.
    * __Promote that selection to a layer__ and __duplicate__ it.
5. Choose one of the duplicated layers and __COLORIFY__ it to pitch black.
    * __Set the transparency threshold__ of this black layer to 0.
6. Finally, place this completely black colored layer underneath the other one and __merge the visible__ layers.
7. Export your image to a PNG file. 
    
This should get rid of semi-transparent pixels while maintaining the "aliasing" effect. 
There could be a better way but this should work ok.


# Credits and Special Thanks
- All the developer guys from the ScummVM (https://github.com/scummvm/scummvm) team, and especially the ones involved in the implementation of the BladeRunner engine for ScummVM (madmoose, peterkohaut, sev and everyone else).
- The information provided in this blog (http://westwoodbladerunner.blogspot.ca) by Michael Liebscher.
- The creator of br-mixer (https://github.com/bdamer/br-mixer), Ben Damer, who also has a blog entry about the game resource file formats (http://afqa123.com/2015/03/07/deciphering-blade-runner/)
