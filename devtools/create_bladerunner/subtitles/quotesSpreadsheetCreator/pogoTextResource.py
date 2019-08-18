#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#

my_module_version = "1.00"
my_module_name = "pogoTextResource"

# All game versions should have the English text pre-filled in the POGO sheet
POGO_TEXT_RESOURCE_TUPLE_LIST = [
	(0, "Air Conditioning"),
	(1, "Amy Shoopman"),
	(2, "Andy B. and the Milk Carton Kids"),
	(3, "Area 51"),
	(4, "Aspirin"),
	(5, "Babylon 5"),
	(6, "Bandit"),
	(7, "Bauer Inline Skates"),
	(8, "Bill Randolph"),
	(9, "Bill (Mr. Motorola) and Sarah"),
	(10, "Boo Berry and Frankenberry"),
	(11, "Brett W. Sperry"),
	(12, "Brianhead Ski Resort"),
	(13, "Bubba"),
	(14, "Bubbles"),
	(15, "Building 2 Parking"),
	(16, "The Buke"),
	(17, "Chan \"The Emporer\" Lee"),
	(18, "Cheezy Poofs"),
	(19, "Chuck \"Walter\" Karras"),
	(20, "Cinco De Mayo"),
	(21, "Club Med"),
	(22, "Code Complete"),
	(23, "Coffee Pub, Las Vegas"),
	(24, "Coke"),
	(25, "Coin Magic"),
	(26, "Count Chocula"),
	(27, "Dad"),
	(28, "David Arkenstone"),
	(29, "Digital Camera"),
	(30, "Direct X Team"),
	(31, "Denis and Joanne Dyack"),
	(32, "Blue Bayou, Disneyland"),
	(33, "Dongle-Boy"),
	(34, "Doves and Sparrows"),
	(35, "Dovey"),
	(36, "Draracles"),
	(37, "Dry Air"),
	(38, "Ed Del Castillo"),
	(39, "Eric \"Kick Ass\" Cartman"),
	(40, "FHM"),
	(41, "Fog City Diner"),
	(42, "Fog Studios"),
	(43, "Gatorade"),
	(44, "Gandhi Cuisine of India"),
	(45, "Giant Lava Lamp"),
	(46, "Eric and Nancy Gooch"),
	(47, "Grayford Family"),
	(48, "Comet Hale-Bopp"),
	(49, "Joseph B. Hewitt IV"),
	(50, "Hercules"),
	(51, "Hillbilly Jeopardy"),
	(52, "Home Cookin'"),
	(53, "Hooey Stick"),
	(54, "The Hypnotist"),
	(55, "Insects on the Move"),
	(56, "Intel"),
	(57, "James Hong"),
	(58, "Jasmine"),
	(59, "The Mysterious Cockatiel"),
	(60, "Joe's Frog"),
	(61, "Jed"),
	(62, "Jeeps"),
	(63, "Jeeter"),
	(64, "Jeff Brown"),
	(65, "JoeB"),
	(66, "Joe-Bob McClintock"),
	(67, "Joseph Turkel"),
	(68, "Jose Cuervo"),
	(69, "Juggling Balls"),
	(70, "Keith Parkinson"),
	(71, "Khan"),
	(72, "King of the Hill"),
	(73, "Kurt O. and the Toothbrush Squad"),
	(74, "Leonard and Shirley Legg"),
	(75, "Leroy"),
	(76, "Brion James"),
	(77, "Louis and his \"friend\""),
	(78, "M.C. Crammer and Janie"),
	(79, "Men's Room Magna-Doodle"),
	(80, "Mark and Deepti Rowland"),
	(81, "Metro Pizza, Las Vegas"),
	(82, "Matt Vella"),
	(83, "Maui"),
	(84, "1 Million Candlepower Spotlight"),
	(85, "Mom"),
	(86, "Movie-makers"),
	(87, "Mr. Nonsense"),
	(88, "Needles"),
	(89, "Nerf Weaponry"),
	(90, "Nimbus"),
	(91, "Norm Vordahl"),
	(92, "KNPR"),
	(93, "Olive Garden"),
	(94, "Onkyo"),
	(95, "Orangey"),
	(96, "Osbur, the Human Resource Manager"),
	(97, "Our Cheery Friend Leary"),
	(98, "Ousted Gnome King"),
	(99, "Pepsi"),
	(100, "Peta Wilson"),
	(101, "Pogo the Mockingbird"),
	(102, "Poker Nights"),
	(103, "Pirates"),
	(104, "Playmate Lingerie Calendar"),
	(105, "Pop-Ice"),
	(106, "Powerhouse Gym"),
	(107, "Rade McDowell"),
	(108, "Red Rock Canyon"),
	(109, "Refrigeration"),
	(110, "Rhoda"),
	(111, "Richard and Kimberly Weier"),
	(112, "Ridley Scott"),
	(113, "Ruud the Dude"),
	(114, "Our old pal Rick Parks"),
	(115, "Ruby's Diner"),
	(116, "Savatage"),
	(117, "Scully and Mulder"),
	(118, "Sean Young"),
	(119, "Seinfeld"),
	(120, "The Shadow"),
	(121, "Shakes"),
	(122, "Shorts"),
	(123, "Silly Putty"),
	(124, "The Simpsons"),
	(125, "Thomas Christensen"),
	(126, "We love you Steve Wetherill!!!"),
	(127, "Skank"),
	(128, "Slice"),
	(129, "SSG"),
	(130, "Steve and Anne Tall"),
	(131, "South Park"),
	(132, "Snap 'n Pops"),
	(133, "Sneaker"),
	(134, "Star Wars Trilogy"),
	(135, "Nonstop Summer Pool Parties"),
	(136, "Sunsets"),
	(137, "T-Bone and Angie"),
	(138, "T-shirts"),
	(139, "Julio Schembari, Tango Pools"),
	(140, "The Thermostat Key"),
	(141, "The Wizard"),
	(142, "Tomb Raider"),
	(143, "Tom Elmer II"),
	(144, "Tujia Linden"),
	(145, "Turbo"),
	(146, "Tweeter"),
	(147, "Twonky"),
	(148, "Ty and Judy Coon"),
	(149, "The Courtyard"),
	(150, "U.F.C."),
	(151, "Uli Boehnke"),
	(152, "Virgil"),
	(153, "Virtual Boy"),
	(154, "Westwood Offroad Excursion Team"),
	(155, "William Sanderson"),
	(156, "Xena"),
	(157, "Zion National Park"),
	(158, "We 3 coders give special thanks to:")
 ]


class pogoTextResource(object):
	m_traceModeEnabled = True
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True):
		self.m_traceModeEnabled = traceModeEnabled
		return
	
	def printPogo(self):
		if self.m_traceModeEnabled:
			print "[Trace] printing Pogo..."
		for (idTre, textTre) in POGO_TEXT_RESOURCE_TUPLE_LIST:
			print "%s\t%s" % (idTre, textTre)
		return
		
	def getPogoEntriesList(self):
		if self.m_traceModeEnabled:
			print "[Trace] getPogoEntriesList()"
		return POGO_TEXT_RESOURCE_TUPLE_LIST
		
#
#
#
if __name__ == '__main__':
	# main()
	print "[Debug] Running %s as main module" % (my_module_name)
	traceModeEnabled = False
	pogoTRInstance = pogoTextResource(traceModeEnabled)
	pogoTRInstance.printPogo()
else:
	#debug
	#print "[Debug] Running	 %s imported from another module" % (my_module_name)
	pass
	