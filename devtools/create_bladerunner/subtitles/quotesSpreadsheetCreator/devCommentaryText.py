#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#

my_module_version = "1.00"
my_module_name = "devCommentaryText"

# All game versions should have the English text pre-filled in the POGO sheet
DEV_AUDIO_COMMENTART_TUPLE_LIST = [
	("MA04VO1A.AUD", "Yes, this is McCoy's apartment. Probably one of the more pivotal areas in the game. You'll notice that if you walk McCoy into the bathroom on the right hand side over there a really weird thing happens. One of the things the player might not pick up on but is rather important is McCoy doesn't wash his hands every time he uses the bathroom. This can possibly lead the player to believe that: a) Ray McCoy is a Replicant who doesn't care about germs and, you know, doesn't use to wash his hands or b) McCoy is just kind of a slob."),
	("CT01VO1A.AUD", "Oh, yeah, this is the Chinatown scene with Howie's restaurant. You'll notice that Howie is moving his arms in the air. The thing we were trying to pull off here was that he was making the sushi but we had a lot of problems with the sushi going bad under the lights and it just wasn't a good scene, so what you're seeing in the final version here is we had to get rid of the lot of the fish and sushi and we're a little disappointed in this area."),
	("HC01VO1A.AUD", "This is an area known as Hawker's Circle. It's sort of a swamp pit, so to speak where a lot of the scum of the earth hang out. Notice again the constant Asian reminder here showing you that Asians are often living in poor rather dingy areas of the city.")
]

DEV_ISEZ_QUOTES_TUPLE_LIST = [
	("I_SEZ", "Blade Runner\nFrom the dark recesses of David Leary's imagination comes a game unlike any\nother. Blade Runner immerses you in the underbelly of future Los Angeles. Right\nfrom the start, the story pulls you in with graphic descriptions of a\ngrandmother doing the shimmy in her underwear, child molestation, brutal\ncold-blooded slaying of innocent animals, vomiting on desks, staring at a\nwoman's ass, the list goes on. And when the game starts, the real fun begins -\nshoot down-on-their-luck homeless people and toss them into a dumpster. Watch\nwith sadistic glee as a dog gets blown into chunky, bloody, bits by an\nexplosive, and even murder a shy little girl who loves you. If you think David\nLeary is sick, and you like sick, this is THE game for you.\n\nJW: Don't forget the wasting of helpless mutated cripples in the underground.\nIt's such a beautiful thing!\n\nDL: Go ahead.  Just keep beating that snarling pit bull...ignore the foam\naround his jaws. There's room on the top shelf of my fridge for at least one\nmore head... - Psychotic Dave\n"),
	("I_SEZ", "MG: Is David Leary a self-respecting human or is he powered by rechargeable\nBatteries?"),
	("I_SEZ", "JM: That McCoy--he's one funny guy! Jet-black fire truck, hehehehe..."),
	("I_SEZ", "JM: Did it have a huge, ugly piece of chrome on it?"),
	("I_SEZ", "JM: This officer has a talent for vivid metaphors."),
	("I_SEZ", "DL: What is that supposed to mean?  I didn't write this line..."),
	("I_SEZ", "MG: Hey, leave that officer alone. Can't you see he's busy?\nJM: (...mmm, donuts...)"),
	("I_SEZ", "MG: It's all fun and games until someone loses a tiger cub."),
	("I_SEZ", "JM: Chrome...is that what that is?"),
	("I_SEZ", "JM: It's hard to imagine that thing on either a car or a horse.\nMG: McCoy! What a witty chap...\nJM: He keeps me chuckling non-stop!"),
	("I_SEZ", "MG: Leaving already?  The fun is just beginning!"),
	("I_SEZ", "MG: We don't want any of that abstract art oozing out onto the street.")
]
#
#
#
class devCommentaryText:
	m_traceModeEnabled = True
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		self.m_traceModeEnabled = traceModeEnabled
		return
	
	def printTexts(self):
		print "AUDIO COMMENTARY"
		for (idTre, textTre) in DEV_AUDIO_COMMENTART_TUPLE_LIST:
			print "%s\t%s" % (idTre, textTre)
		print "I_SEZ QUOTES"
		for (idTre, textTre) in DEV_ISEZ_QUOTES_TUPLE_LIST:
			print "%s\t%s" % (idTre, textTre)
		return
		
	def getAudioCommentaryTextEntriesList(self):
		return DEV_AUDIO_COMMENTART_TUPLE_LIST
		
	def getISEZTextEntriesList(self):
		return DEV_ISEZ_QUOTES_TUPLE_LIST

if __name__ == '__main__':
	#	 main()
	print "[Debug] Running %s as main module" % (my_module_name)
	traceModeEnabled = False
	devCommentaryTextInstance = devCommentaryText(traceModeEnabled)
	devCommentaryTextInstance.printTexts()
	
else:
	#debug
	#print "[Debug] Running	 %s imported from another module" % (my_module_name)
	pass
	