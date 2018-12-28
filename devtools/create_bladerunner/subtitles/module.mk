
MODULE := devtools/create_bladerunner/subtitles

# Set the name of the final output
TOOL_OUTPUT := SUBTITLES.MIX
FONT_OUTPUT := SUBTLS_E.FON

BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER := $(srcdir)/devtools/create_bladerunner/subtitles
BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER := $(srcdir)/devtools/create_bladerunner/subtitles/sampleInput
INTERMEDIATE_RESOURCE_FILES_UI := "OPTIONS.TRE" "DLGMENU.TRE" "SCORERS.TRE" "VK.TRE" "CLUES.TRE" "CRIMES.TRE" "ACTORS.TRE" "HELP.TRE" "AUTOSAVE.TRE" "ERRORMSG.TRE" "SPINDEST.TRE" "KIA.TRE" "KIACRED.TRE" "CLUETYPE.TRE" "ENDCRED.TRE" "POGO.TRE"
INTERMEDIATE_RESOURCE_FILES_SUBS := "INGQUO_E.TRE" "WSTLGO_E.TRE" "BRLOGO_E.TRE" "INTRO_E.TRE" "MW_A_E.TRE" "MW_B01_E.TRE" "MW_B02_E.TRE" "MW_B03_E.TRE" "MW_B04_E.TRE" "MW_B05_E.TRE" "INTRGT_E.TRE" "MW_D_E.TRE" "MW_C01_E.TRE" "MW_C02_E.TRE" "MW_C03_E.TRE" "END04A_E.TRE" "END04B_E.TRE" "END04C_E.TRE" "END06_E.TRE" "END01A_E.TRE" "END01B_E.TRE" "END01C_E.TRE" "END01D_E.TRE" "END01E_E.TRE" "END01F_E.TRE" "END03_E.TRE"
INPUT_TRANSCRIPT_FILENAME := englishTranscript.xls
INPUT_TRANSCRIPT_AUX_CONF_FILENAME := configureFontsTranslation.txt
INPUT_FONT_GLYPHS_PNG_FILENAME := subtitlesFont.png
INPUT_FONT_GLYPHS_PNG_AUX_CONF_FILENAME := overrideEncodingSUBLTS.txt

$(MODULE): $(TOOL_OUTPUT) 
#
# Font file creation from an input PNG image named $(INPUT_FONT_GLYPHS_PNG_FILENAME)
# The $(INPUT_FONT_GLYPHS_PNG_AUX_CONF_FILENAME) is used to configure the font creation
# also the command line switches pxLL, pxTT, pxKn, pxWS configure additional aspects for the font creation
$(FONT_OUTPUT): $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_FONT_GLYPHS_PNG_FILENAME) $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_FONT_GLYPHS_PNG_AUX_CONF_FILENAME)
	$(info ---------)
	$(info Creating Blade Runner subtitles font $(FONT_OUTPUT)...)
	$(info This process assumes that the folder: )
	$(info $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER))
	$(info contains: )
	$(info *. $(INPUT_FONT_GLYPHS_PNG_FILENAME) - a PNG (image) input file with the Font glyphs)
	$(info *. $(INPUT_FONT_GLYPHS_PNG_AUX_CONF_FILENAME) - a TXT (text) input file with configuration settings for the glyph image processing)
	$(info If successful, a $(FONT_OUTPUT) file will be created in your working directory)
	$(info This is an intermediate file. You don't need to copy this in your Blade Runner game directory)
	$(info ---------)
	$(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/fontCreator/fontCreator.py -im $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_FONT_GLYPHS_PNG_FILENAME) -oe $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_FONT_GLYPHS_PNG_AUX_CONF_FILENAME) -om $(FONT_OUTPUT) -pxLL 42 -pxTT 30 -pxKn 1 -pxWS 7
	
# Creation of final output mix file SUBTILES.MIX
# The MIX file will pack the fonts file $(FONT_OUTPUT) as well as resources created from the transcript (EXCEL) file $(INPUT_TRANSCRIPT_FILENAME)
# The $(INPUT_TRANSCRIPT_AUX_CONF_FILENAME) file is used to configure the creation of the mix file
# This command sequence will erase any intermediate resource files (.TRE) at the end.
# The $(FONT_OUTPUT) file will not be erased.
$(TOOL_OUTPUT): $(FONT_OUTPUT) $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_FILENAME) $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_AUX_CONF_FILENAME) $(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/common/actornames.txt
	$(info ---------)
	$(info Creating Blade Runner subtitles MIX file: $(TOOL_OUTPUT)...)
	$(info This process assumes that the folder: )
	$(info $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER))
	$(info contains: )
	$(info *. $(INPUT_TRANSCRIPT_FILENAME) - an XLS (Excel) input file with the transcript)
	$(info *. $(INPUT_TRANSCRIPT_AUX_CONF_FILENAME) - a TXT (text) input file with configuration settings for the transcript processing)
	$(info If successful, a $(TOOL_OUTPUT) file will be created in your working directory)
	$(info Please, copy this $(TOOL_OUTPUT) into your Blade Runner game directory!)
	$(info ---------)
	$(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/mixResourceCreator/mixResourceCreator.py -x $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_FILENAME) -ian $(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/common/actornames.txt -cft $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_AUX_CONF_FILENAME)
	-$(RM) $(INTERMEDIATE_RESOURCE_FILES_UI) $(INTERMEDIATE_RESOURCE_FILES_SUBS)
