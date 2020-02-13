#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#

my_module_version = "1.00"
my_module_name = "devCommentaryText"

# All game versions should have the English text pre-filled in the POGO sheet
DEV_AUDIO_COMMENTARY_TUPLE_LIST = [
	("MA04VO1A.AUD", "*McCoy's apartment*"),
	("CT01VO1A.AUD", "*Chinatown*"),
	("HC01VO1A.AUD", "*Hawker's Circle*")
]

EXTRA_SPEECH_AUDIO_TUPLE_LIST = [
	('COLONY.AUD', "*Blimp Guy talk*"),
	('67_0470R.AUD', "*Crowd talk*"),
	('67_0480R.AUD', "*Crowd talk*"),
	('67_0500R.AUD', "*Crowd talk*"),
	('67_0540R.AUD', "*Crowd talk*"),
	('67_0560R.AUD', "*Crowd talk*"),
	('67_0870R.AUD', "*Crowd talk*"),
	('67_0880R.AUD', "*Crowd talk*"),
	('67_0900R.AUD', "*Crowd talk*"),
	('67_0940R.AUD', "*Crowd talk*"),
	('67_0960R.AUD', "*Crowd talk*"),
	('67_1070R.AUD', "*Crowd talk*"),
	('67_1080R.AUD', "*Crowd talk*"),
	('67_1100R.AUD', "*Crowd talk*"),
	('67_1140R.AUD', "*Crowd talk*"),
	('67_1160R.AUD', "*Crowd talk*")
]

# we use the spoken quote id that triggers the comment as an ID for the DEV commentary quote
# Ids 00-9990 and above correspond to clicks or other transitions (no corresponding spoken in-game quote)
DEV_ISEZ_QUOTES_TUPLE_LIST = [
	("IS-00-9990.AUD", "Blade Runner\nFrom the dark recesses of David Leary's imagination comes a game unlike any\nother. Blade Runner immerses you in the underbelly of future Los Angeles. Right\nfrom the start, the story pulls you in with graphic descriptions of a\ngrandmother doing the shimmy in her underwear, child molestation, brutal\ncold-blooded slaying of innocent animals, vomiting on desks, staring at a\nwoman's ass, the list goes on. And when the game starts, the real fun begins -\nshoot down-on-their-luck homeless people and toss them into a dumpster. Watch\nwith sadistic glee as a dog gets blown into chunky, bloody, bits by an\nexplosive, and even murder a shy little girl who loves you. If you think David\nLeary is sick, and you like sick, this is THE game for you.\n\nJW: Don't forget the wasting of helpless mutated cripples in the underground.\nIt's such a beautiful thing!\n\nDL: Go ahead.  Just keep beating that snarling pit bull...ignore the foam\naround his jaws. There's room on the top shelf of my fridge for at least one\nmore head... - Psychotic Dave\n"),
	("IS-99-1860.AUD", "MG: Is David Leary a self-respecting human or is he powered by rechargeable\nBatteries?"),  # voice-over
	("IS-99-1890.AUD", "JM: That McCoy--he's one funny guy! Jet-black fire truck, hehehehe..."), # voice-over
	("IS-23-0130.AUD", "JM: Did it have a huge, ugly piece of chrome on it?"), # Officer Leary
	("IS-23-0090.AUD", "JM: This officer has a talent for vivid metaphors."), # Officer Leary
	("IS-00-4540.AUD", "DL: What is that supposed to mean?  I didn't write this line..."),
	("IS-00-4515.AUD", "MG: Hey, leave that officer alone. Can't you see he's busy?\nJM: (...mmm, donuts...)"), # clicking on Leary after we get his statement
	("IS-23-0060.AUD", "MG: It's all fun and games until someone loses a tiger cub."), # Officer Leary
	("IS-00-9991.AUD", "JM: Chrome...is that what that is?"), # pick up chrome
	("IS-00-4510.AUD", "JM: It's hard to imagine that thing on either a car or a horse.\nMG: McCoy! What a witty chap...\nJM: He keeps me chuckling non-stop!"),
	("IS-00-9992.AUD", "MG: Leaving already?  The fun is just beginning!"), # leaving Runciter's zoo with Spinner
	("IS-00-4500.AUD", "MG: We don't want any of that abstract art oozing out onto the street.")
]
#
#
#
class devCommentaryText(object):
	m_traceModeEnabled = True
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		self.m_traceModeEnabled = traceModeEnabled
		return
	
	def printTexts(self):
		if self.m_traceModeEnabled:
			print "[Trace] Printing all dev commentary text"
		print "\nAUDIO COMMENTARY"
		print "------------------"
		for (idTre, textTre) in DEV_AUDIO_COMMENTARY_TUPLE_LIST:
			print "%s\t%s" % (idTre, textTre)
		print "\nEXTRA SPEECH AUDIO"
		print "------------------"
		for (idTre, textTre) in EXTRA_SPEECH_AUDIO_TUPLE_LIST:
			print "%s\t%s" % (idTre, textTre)
		print "\nI_SEZ QUOTES"
		print "------------------"
		for (idTre, textTre) in DEV_ISEZ_QUOTES_TUPLE_LIST:
			print "%s\t%s" % (idTre, textTre)
		return
		
	def getAudioCommentaryTextEntriesList(self):
		if self.m_traceModeEnabled:
			print "[Trace] getAudioCommentaryTextEntriesList"
		return DEV_AUDIO_COMMENTARY_TUPLE_LIST
		
	def getISEZTextEntriesList(self):
		
		if self.m_traceModeEnabled:
			print "[Trace] getISEZTextEntriesList"
		return DEV_ISEZ_QUOTES_TUPLE_LIST
		
	def getExtraSpeechAudioEntriesList(self):
		if self.m_traceModeEnabled:
			print "[Trace] getExtraSpeechAudioEntriesList"
		return EXTRA_SPEECH_AUDIO_TUPLE_LIST
#
#
#
if __name__ == '__main__':
	# main()
	print "[Debug] Running %s as main module" % (my_module_name)
	traceModeEnabled = False
	devCommentaryTextInstance = devCommentaryText(traceModeEnabled)
	devCommentaryTextInstance.printTexts()
else:
	#debug
	#print "[Debug] Running	 %s imported from another module" % (my_module_name)
	pass
