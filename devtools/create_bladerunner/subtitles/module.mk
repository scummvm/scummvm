
MODULE := devtools/create_bladerunner/subtitles

# Set the name of the final output
TOOL_OUTPUT := SUBTITLES.MIX
FONT_OUTPUT := SUBTLS_E.FON

BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER := $(srcdir)/devtools/create_bladerunner/subtitles
BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER := $(srcdir)/devtools/create_bladerunner/subtitles/sampleInput
INTERMEDIATE_RESOURCE_FILES_UI := OPTIONS.TR* DLGMENU.TR* SCORERS.TR* VK.TR* CLUES.TR* CRIMES.TR* ACTORS.TR* HELP.TR* AUTOSAVE.TR* ERRORMSG.TR* SPINDEST.TR* KIA.TR* KIACRED.TR* CLUETYPE.TR* ENDCRED.TR* POGO.TR* SBTLVERS.TR*
INTERMEDIATE_RESOURCE_FILES_SUBS := INGQUO_*.TR* WSTLGO_E.TR* BRLOGO_E.TR* INTRO_*.TR* MW_A_*.TR* MW_B01_*.TR* MW_B02_*.TR* MW_B03_*.TR* MW_B04_*.TR* MW_B05_*.TR* INTRGT_*.TR* MW_D_*.TR* MW_C01_*.TR* MW_C02_*.TR* MW_C03_*.TR* END04A_*.TR* END04B_*.TR* END04C_*.TR* END06_*.TR* END01A_*.TR* END01B_*.TR* END01C_*.TR* END01D_*.TR* END01E_*.TR* END01F_*.TR* END03_*.TR* TB_FLY_*.TR*
INPUT_TRANSCRIPT_FILENAME := englishTranscript.xls
INPUT_TRANSCRIPT_FILEPATH := $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_FILENAME)

ifeq (,$(wildcard $(INPUT_TRANSCRIPT_FILEPATH)))
INPUT_TRANSCRIPT_FILENAME := englishTranscript.xlsx
INPUT_TRANSCRIPT_FILEPATH := $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_FILENAME) 
endif

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
# This command sequence will erase any intermediate resource files (.TR*) at the end.
# The $(FONT_OUTPUT) file will not be erased.
$(TOOL_OUTPUT): $(FONT_OUTPUT) $(INPUT_TRANSCRIPT_FILEPATH) $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_AUX_CONF_FILENAME) $(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/common/actornames.txt
	$(info ---------)
	$(info Creating Blade Runner subtitles MIX file: $(TOOL_OUTPUT)...)
	$(info This process assumes that the folder: )
	$(info $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER))
	$(info contains: )
	$(info *. $(INPUT_TRANSCRIPT_FILENAME) - an XLS(X) (Excel) input file with the transcript)
	$(info *. $(INPUT_TRANSCRIPT_AUX_CONF_FILENAME) - a TXT (text) input file with configuration settings for the transcript processing)
	$(info If successful, a $(TOOL_OUTPUT) file will be created in your working directory)
	$(info Please, copy this $(TOOL_OUTPUT) into your Blade Runner game directory!)
	$(info ---------)
	$(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/mixResourceCreator/mixResourceCreator.py -x $(INPUT_TRANSCRIPT_FILEPATH) -ian $(BLADERUNNER_SUBTITLES_SCRIPTS_ROOT_FOLDER)/common/actornames.txt -cft $(BLADERUNNER_SUBTITLES_SAMPLE_INPUT_FOLDER)/$(INPUT_TRANSCRIPT_AUX_CONF_FILENAME) -ld EFIGS
	-$(RM) $(INTERMEDIATE_RESOURCE_FILES_UI) $(INTERMEDIATE_RESOURCE_FILES_SUBS)
