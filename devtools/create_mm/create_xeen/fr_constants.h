/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "constants.h"
#include "common/keyboard.h"

class FR : public LangConstants {
public:
	const char *CLOUDS_CREDITS() {
		return "\v012\t000\x3"
			   "c\f35Designed and Directed By:\n"
			   "\f17Jon Van Caneghem\x3l\n"
			   "\n"
			   "\t025\f35Programming:\n"
			   "\t035\f17Mark Caldwell\n"
			   "\t035Dave Hathaway\n"
			   "\n"
			   "\t025\f35Sound System & FX:\n"
			   "\t035\f17Todd Hendrix\n"
			   "\n"
			   "\t025\f35Music & Speech:\n"
			   "\t035\f17Tim Tully\n"
			   "\n"
			   "\t025\f35Writing:\n"
			   "\t035\f17Paul Rattner\n"
			   "\t035Debbie Murphy\n"
			   "\t035Jon Van Caneghem\v012\n"
			   "\n"
			   "\n"
			   "\t180\f35Graphics:\n"
			   "\t190\f17Louis Johnson\n"
			   "\t190Jonathan P. Gwyn\n"
			   "\t190Bonita Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n"
			   "\n"
			   "\t180\f35Testing:\n"
			   "\t190\f17Benjamin Bent\n"
			   "\t190Mario Escamilla\n"
			   "\t190Richard Espy\n"
			   "\t190Scott McDaniel\n"
			   "\t190Clayton Retzer\n"
			   "\t190Michael Suarez\x3"
			   "c";
	}

	const char *DARK_SIDE_CREDITS() {
		return "\v012\t000\x3"
			   "c\f35Design et R$aliser par:\n"
			   "\f17Jon Van Caneghem\x3l\n"
			   "\n"
			   "\t025\f35Programmation:\n"
			   "\t035\f17Mark Caldwell\n"
			   "\t035Dave Hathaway\n"
			   "\n"
			   "\t025\f35Son et FX:\n"
			   "\t035\f17Mike Heilemann\n"
			   "\n"
			   "\t025\f35Musique et parole:\n"
			   "\t035\f17Tim Tully\n"
			   "\n"
			   "\t025\f35Ecrit par:\n"
			   "\t035\f17Paul Rattner\n"
			   "\t035Debbie Van Caneghem\n"
			   "\t035Jon Van Caneghem\v012\n"
			   "\n"
			   "\n"
			   "\t180\f35Graphiques:\n"
			   "\t190\f17Jonathan P. Gwyn\n"
			   "\t190Bonita Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n"
			   "\n"
			   "\t180\f35Test:\n"
			   "\t190\f17Benjamin Bent\n"
			   "\t190Christian Dailey\n"
			   "\t190Mario Escamilla\n"
			   "\t190Marco Hunter\n"
			   "\t190Robert J. Lupo\n"
			   "\t190Clayton Retzer\n"
			   "\t190David Vela\x3"
			   "c";
	}

	const char *SWORDS_CREDITS1() {
		return "\v012\x3"
			   "c\f35Published By New World Computing, Inc.\f17\n"
			   "Developed By CATware, Inc.\x3l\n"
			   "\f01Design and Direction\t180Series Created by\n"
			   "\t020Bill Fawcett\t190John Van Caneghem\n"
			   "\n"
			   "\t010Story Contributions\t180Producer & Manual\n"
			   "\t020Ellen Guon\t190Dean Rettig\n"
			   "\n"
			   "\t010Programming & Ideas\t180Original Programming\n"
			   "\t020David Potter\t190Mark Caldwell\n"
			   "\t020Rod Retterath\t190Dave Hathaway\n"
			   "\n"
			   "\t010Manual Illustrations\t180Graphic Artists\n"
			   "\t020Todd Cameron Hamilton\t190Jonathan P. Gwyn\n"
			   "\t020James Clouse\t190Bonnie Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n";
	}

	const char *SWORDS_CREDITS2() {
		return "\f05\v012\t000\x3l\n"
			   "\t100Sound Programming\n"
			   "\t110Todd Hendrix\n"
			   "\n"
			   "\t100Music\n"
			   "\t110Tim Tully\n"
			   "\t110Quality Assurance Manager\n"
			   "\t110Peter Ryu\n"
			   "\t100Testers\n"
			   "\t110Walter Johnson\n"
			   "\t110Bryan Farina\n"
			   "\t110David Baton\n"
			   "\t110Jack Nalls\n";
	}

	const char *OPTIONS_MENU() {
		return "\r\x1\x3"
			   "c\fdOptions Might and Magic\n"
			   "%s of Xeen\x2\n"
			   "\v%.3dCopyright (c) %d NWC, Inc.\n"
			   "Tous droits R$serv$s\x1";
	}

	const char **GAME_NAMES() {
		static const char *_gameNames[] = { "Clouds", "Darkside", "World" };
		return _gameNames;
	}

	const char *THE_PARTY_NEEDS_REST() {
		return "\v012Le groupe a besoin de repos!";
	}

	const char *WHO_WILL() {
		return "\x3"
			   "c\v000\t000%s\n\n"
			   "Qui\n%s?\n\v055F1 - F%d";
	}

	const char *HOW_MUCH() {
		return "\x3"
			   "cCombien\n\n";
	}

	const char *WHATS_THE_PASSWORD() {
		return "\x3"
			   "cMot de Passe?\n"
			   "\n"
			   "Allez ] la page %u, ] la ligne\n"
			   "%u, et tapez le mot %u.\v067\t000Les espaces ne comptent pas comme mots ou lignes.  "
			   "Les mots ] trait d'union comptent pour un seul mot.  Toute ligne contenant du texte est une ligne."
			   "\x3"
			   "c\v040\t000\n";
	}

	const char *PASSWORD_INCORRECT() {
		return "\x3"
			   "c\v040\n"
			   "\f32Faux!\fd";
	}

	const char *IN_NO_CONDITION() {
		return "\v007%s n'est pas en condition!";
	}

	const char *NOTHING_HERE() {
		return "\x03" "c\v010Rien ici.";
	}

	const char **WHO_ACTIONS() {
		static const char *_whoActions[] = {
			"va chercher", "va ouvrir", "va boire", "va exploiter", "va toucher", "va lire", "va apprendre", "va prendre",
				"va cogner", "va voler", "va corrompre", "va payer", "va s'asseoir", "va essayer", "va tourner", "va se laver",
				"va d$truire", "va tirer", "va descendre", "va jeter une pi%ce", "va prier", "va rejoindre", "va agir",
				"va jouer", "va pousser", "va crocheter", "va crocheter", "va manger", "va signer", "va fermer", "va bloquer", "va essayer"
		};
		return _whoActions;
	}

	const char **WHO_WILL_ACTIONS() {
		static const char *_whoWillActions[] = {
			"Ouvrir Grille", "Ouvrir Porte", "Ouvrir Parchemin", "S$lect. Personnage"
		};
		return _whoWillActions;
	}

	const char **DIRECTION_TEXT_UPPER() {
		static const char *_directionTextUpper[] = { "NORD", "EST", "SUD", "OUEST" };
		return _directionTextUpper;
	}

	const char **DIRECTION_TEXT() {
		static const char *_directionText[] = { "Nord", "Est", "Sud", "Ouest" };
		return _directionText;
	}

	const char **RACE_NAMES() {
		static const char *_raceNames[] = { "Humain", "Elfe", "Nain", "Gnome", "D-Orc" };
		return _raceNames;
	}

	const char **ALIGNMENT_NAMES() {
		static const char *_alignmentNames[] = { "Bon", "Neutre", "Mal$fique" };
		return _alignmentNames;
	}

	const char **SEX_NAMES() {
		static const char *_sexNames[] = { "M&le", "Femelle" };
		return _sexNames;
	}

	const char **SKILL_NAMES() {
		static const char *_skillNames[] = {
			"Vol\t100", "Ma{tre d'Armes", "Astrologue", "Body Build", "Cartographe",
				"Crois$", "Sens Orient", "Linguiste", "Marchand", "Montagnard",
				"Navigateur", "Eclaireur", "Ma{tre Prieur", "Prestidigit",
				"Nageur", "Pisteur", "Prte scrte", "Sens Danger"
		};
		return _skillNames;
	}

	const char **CLASS_NAMES() {
		static const char *_classNames[] = {
			"Chevalier", "Paladin", "Archer", "Clerc", "Sorcier", "Voleur",
				"Ninja", "Barbare", "Druide", "Forestier", nullptr
		};
		return _classNames;
	}

	const char **CONDITION_NAMES_M() {
		static const char *_conditionNamesM[] = {
			"Envout$", "Coeur Bris$", "Faible", "Empoisonn$", "Infect$",
				"Fou", "Amoureux", "Saoul", "Endormi", "D$pressif", "Confus",
				"Paralys$", "Inconscient", "Mort", "Pierre", "Supprim$", "Bon"
		};
		return _conditionNamesM;
	}

	const char **CONDITION_NAMES_F() {
		static const char *_conditionNamesF[] = {
			"Envout$e", "Coeur Bris$", "Faible", "Empoisonn$e", "Infect$e",
				"Folle", "Amoureuse", "Saoule", "Endormie", "D$pressive", "Confuse",
				"Paralys$e", "Inconsciente", "Morte", "Pierre", "Supprim$e", "Bonne"
		};
		return _conditionNamesF;
	}

	const char *GOOD() {
		return "Bon";
	}

	const char *BLESSED() {
		return "\n\t020B$ni(e)\t095%+d";
	}

	const char *POWER_SHIELD() {
		return "\n\t020Puissant Bouclier\t095%+d";
	}

	const char *HOLY_BONUS() {
		return "\n\t020Bonus Saint\t095%+d";
	}

	const char *HEROISM() {
		return "\n\t020H$ro_sme\t095%+d";
	}

	const char *IN_PARTY() {
		return "\f15Au Groupe\fd";
	}

	const char *PARTY_DETAILS() {
		return "\015\003l\002\014"
			   "00"
			   "\013"
			   "001"
			   "\011"
			   "035%s"
			   "\013"
			   "009"
			   "\011"
			   "035%s"
			   "\013"
			   "017"
			   "\011"
			   "035%s"
			   "\013"
			   "025"
			   "\011"
			   "035%s"
			   "\013"
			   "001"
			   "\011"
			   "136%s"
			   "\013"
			   "009"
			   "\011"
			   "136%s"
			   "\013"
			   "017"
			   "\011"
			   "136%s"
			   "\013"
			   "025"
			   "\011"
			   "136%s"
			   "\013"
			   "044"
			   "\011"
			   "035%s"
			   "\013"
			   "052"
			   "\011"
			   "035%s"
			   "\013"
			   "060"
			   "\011"
			   "035%s"
			   "\013"
			   "068"
			   "\011"
			   "035%s"
			   "\013"
			   "044"
			   "\011"
			   "136%s"
			   "\013"
			   "052"
			   "\011"
			   "136%s"
			   "\013"
			   "060"
			   "\011"
			   "136%s"
			   "\013"
			   "068"
			   "\011"
			   "136%s";
	}

	const char *PARTY_DIALOG_TEXT() {
		return "%s\x2\x3"
			   "c\v106\t013Haut\t048Bas\t083\f37S\fduppr\t118\f37E\fdnlev"
			   "\t153\f37C\fdr$er\t188E\f37x\fdit\x1";
	}

	const char *NO_ONE_TO_ADVENTURE_WITH() {
		return "Vous n'avez personne pour l'aventure";
	}

	const char *YOUR_ROSTER_IS_FULL() {
		return "Votre liste est compl%te!";
	}

	const char *PLEASE_WAIT() {
		return "\fd\x03" "c\t000\v002Patientez S.V.P...";
	}

	const char *OOPS() {
		return "\003"
			   "c\011"
			   "000\013"
			   "002Oops...";
	}

	const char *BANK_TEXT() {
		return "\r\x02\x03" "c\v122\t013\f37D\fd$pos\t040\f37R\fdetir\t067ESC\x01\t000\v000Banque de Xeen\v015\nBanque\x03l\nOr\x03r\t000%s\x03l\nGems\x03r\t000%s\x03" "c\n\nGroupe\x03l\nOr\x03r\t000%s\x03l\nGems\x03r\t000%s";
	}

	const char *BLACKSMITH_TEXT() {
		return "\x01\r\x03" "c\v000\t000Options Magasin\t039\v027%s\x03l\v046\n\t011\f37R\fdegard\n\t000\v090Or\x03r\t000%s\x02\x03" "c\v122\t040ESC\x01";
	}

	const char *GUILD_NOT_MEMBER_TEXT() {
		return "\n\nVous devez #tre membre ici.";
	}

	const char *GUILD_TEXT() {
		return "\x03" "c\v027\t039%s\x03l\v046\n\t012\f37A\fdcheter\n\t012\f37I\fdnfo";
	}

	const char *TAVERN_TEXT() {
		return "\r\x03" "c\v000\t000Options Taverne \t039\v027%s%s\x03l\t000\v090Or\x03r\t000%s\x02\x03" "c\v122\t021\f37S\fdigner\t060ESC\x01";
	}

	const char *FOOD_AND_DRINK() {
		return "\x03l\t017\v046\f37B\fdoire\n\t017\f37N\fdourrit\n\t017\f37P\fdourb\n\t017\f37R\fdumeur";
	}

	const char *GOOD_STUFF() {
		return "\n\nPas Mal\n\nUne touche!";
	}

	const char *HAVE_A_DRINK() {
		return "\n\nPrenez un verre\n\nUne touche!";
	}

	const char *YOURE_DRUNK() {
		return "\n\nVous #tes sao=l\n\nUne touche!";
	}

	const char *TEMPLE_TEXT() {
		return "\r\x03" "c\v000\t000Options Temple\t039\v027%s\x03l\t000\v046\f37G\fdu$rir\x03r\t000%u\x03l\n\f37D\fdon\x03r\t000%u\x03l\nD$\f37s\fdenv\x03r\t000%s\x03l\t000\v090Or\x03r\t000%s\x02\x03" "c\v122\t040ESC\x01";
	}

	const char *EXPERIENCE_FOR_LEVEL() {
		return "%s a besoin %u exp$rience pour le niveau %u.";
	}

	const char *TRAINING_LEARNED_ALL() {
		return "%s a appris tout ce que nous savons!";
	}

	const char *ELIGIBLE_FOR_LEVEL() {
		return "%s est pr#t pour le niveau %u.\x03l\n\v081Co=t\x03r\t000%u";
	}

	const char *TRAINING_TEXT() {
		return "\r\x03" "cOptions Camp\n\n%s\x03l\v090\t000Or\x03r\t000%s\x02\x03" "c\v122\t021\f37S\fd'entrain\t060ESC\x01";
	}

	const char *GOLD_GEMS() {
		return "\x3"
			   "c\v000\t000%s\x3l\n"
			   "\n"
			   "Or\x3r\t000%s\x3l\n"
			   "Gems\x3r\t000%s\x2\x3"
			   "c\v096\t013\f37O\fdr\t040\f37G\fdems\t067ESC\x1";
	}

	const char *GOLD_GEMS_2() {
		return "\x3"
			   "c\v000\t000%s\x3l\n"
			   "\n"
			   "\x4"
			   "077Or\x3r\t000%s\x3l\n"
			   "\x4"
			   "077Gems\x3r\t000%s\x3l\t000\v051\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	const char **DEPOSIT_WITHDRAWL() {
		static const char *_depositWithdrawl[] = { "D$pot", "Retrait" };
		return _depositWithdrawl;
	}

	const char *NOT_ENOUGH_X_IN_THE_Y() {
		return "\x3"
			   "c\v012Pas assez de %s dans le %s!\x3l";
	}

	const char *NO_X_IN_THE_Y() {
		return "\x3"
			   "c\v012Pas de %s dans le %s!\x3l";
	}

	const char **STAT_NAMES() {
		static const char *_statNames[] = {
			"Pouvoir", "Intellect", "Personalit$", "Endurance", "Vitesse",
				"Pr$cision", "Chance", "Age", "Niveau", "Classe d'Armure", "Pts de Vie",
				"Pts de Sort", "R$sistances", "Talents", "D$corations", "Exp$rience"
		};
		return _statNames;
	}

	const char **CONSUMABLE_NAMES() {
		static const char *_consumableNames[] = { "Or", "Gems", "Nouriture", "Condition" };
		return _consumableNames;
	}

	const char **CONSUMABLE_GOLD_FORMS() {
		static const char *_consumableGoldForms[] = { "" };
		return _consumableGoldForms;
	}

	const char **CONSUMABLE_GEM_FORMS() {
		static const char *_consumableGemForms[] = { "" };
		return _consumableGemForms;
	}

	const char **WHERE_NAMES() {
		static const char *_whereNames[] = { "Groupe", "Banque" };
		return _whereNames;
	}

	const char *AMOUNT() {
		return "\x3"
			   "c\t000\v051Montant\x3l\n";
	}

	const char *FOOD_PACKS_FULL() {
		return "\v007Le sac est d$j] plein!";
	}

	const char *BUY_SPELLS() {
		return "\x03" "c\v027\t039%s\x03l\v046\n\t012\f37A\fdcheter\n\t012\f37I\fdnfo";
	}

	const char *GUILD_OPTIONS() {
		return "\r\f00\x03" "c\v000\t000Options Guilde: %s\x03l\t000\v090Or\x03r\t000%s\x02\x03" "c\v122\t040ESC\x01";
	}

	const char *NOT_A_SPELL_CASTER() {
		return "N'est pas un lanceur...";
	}

	const char *SPELLS_LEARNED_ALL() {
		return "Vous avez appris tout ce \n\t010que nous savons.";
	}

	const char *SPELLS_FOR() {
		return "\r\fd%s\x2\x3"
			   "c\t000\v002Sorts pour %s";
	}

	const char *SPELL_LINES_0_TO_9() {
		return "\x2\x3l\v015\t0011\n2\n3\n4\n5\n6\n7\n8\n9\n0";
	}

	const char *SPELLS_DIALOG_SPELLS() {
		return "\x3l\v015"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l"
			   "\t004\v110%s - %u\x1";
	}

	const char *SPELL_PTS() {
		return "Pts de Sort";
	}

	const char *GOLD() {
		return "Or";
	}

	const char *SPELL_INFO() {
		return "\x3"
			   "c\f09%s\fd\x3l\n"
			   "\n"
			   "%s\x3"
			   "c\t000\v100Une touche!";
	}

	const char *SPELL_PURCHASE() {
		return "\x3l\v000\t000\fd%s  Souhaitez-vous acheter "
			   "\f09%s\fd pour %u?";
	}

	const char *MAP_TEXT() {
		return "\x3"
			   "c\v000\t000%s\x3l\v139"
			   "\t000X : %d\x3r\t000Y : %d\x3"
			   "c\t000%s";
	}

	const char *LIGHT_COUNT_TEXT() {
		return "\x3l\n\n\t024Lumi%re\x3r\t124%d";
	}

	const char *FIRE_RESISTENCE_TEXT() {
		return "%c%sFeu%s%u";
	}

	const char *ELECRICITY_RESISTENCE_TEXT() {
		return "%c%sElectricit$%s%u";
	}

	const char *COLD_RESISTENCE_TEXT() {
		return "%c%sFroid%s%u";
	}

	const char *POISON_RESISTENCE_TEXT() {
		return "%c%sPoison/Acid%s%u";
	}

	const char *CLAIRVOYANCE_TEXT() {
		return "%c%sClairvoyance%s";
	}

	const char *LEVITATE_TEXT() {
		return "%c%sLevitation%s";
	}

	const char *WALK_ON_WATER_TEXT() {
		return "%c%sMarche sur l'Eau";
	}

	const char *GAME_INFORMATION() {
		return "\r\x3"
			   "c\t000\v001\f37%s of Xeen\fd\n"
			   "Information Jeu\n"
			   "\n"
			   "Actuel: \f37%sdi\fd\n"
			   "\n"
			   "\t032Heure\t072Jour\t112Ann$e\n"
			   "\t032\f37%d:%02d%c\t072%u\t112%u\fd%s";
	}

	const char *WORLD_GAME_TEXT() {
		return "World";
	}

	const char *DARKSIDE_GAME_TEXT() {
		return "Darkside";
	}

	const char *CLOUDS_GAME_TEXT() {
		return "Clouds";
	}

	const char *SWORDS_GAME_TEXT() {
		return "Swords";
	}

	const char **WEEK_DAY_STRINGS() {
		static const char *_weekDayStrings[] = {
			"Dix", "Un", "Deux", "Trois", "Quatre", "Cinq", "Six", "Sept", "Huit", "Neuf"
		};
		return _weekDayStrings;
	}

	const char *CHARACTER_DETAILS() {
		return "\x3l\v041\t196%s\t000\v002%s : %s %s %s"
			   "\x3r\t053\v028\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f%02u%d\fd\t196\f15%u\fd\x3r"
			   "\t053\v051\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f%02u%u\fd\t196\f15%u\fd"
			   "\x3r\t053\v074\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f15%u\fd"
			   "\x3r\t053\v097\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f15%u jour%s\fd"
			   "\x3r\t053\v120\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f%02u%s\fd"
			   "\t230%s%s%s%s\fd";
	}

	const char **DAYS() {
		static const char *_days[] = { "", "s", "" };
		return _days;
	}

	const char *PARTY_GOLD() {
		return "Or du Groupe";
	}

	const char *PLUS_14() {
		return "\f14+";
	}

	const char *CHARACTER_TEMPLATE() {
		return "\x1\f00\r\x3l\t029\v018Pou\t080Pr$\t131P.V.\t196Exp$rience"
			   "\t029\v041Int\t080Cha\t131P.S.\t029\v064Per\t080Age"
			   "\t131Resis\t196Gems Groupe\t029\v087End\t080Niv\t131Talents"
			   "\t196Nourrit\t029\v110Vit\t080CA\t131D$cor\t196Condition\x3"
			   "c"
			   "\t290\v025\f37O\fdbjet\t290\v057\f37R"
			   "\fd$fer\t290\v089\f37E\fdchg\t290\v121Exit\x3l%s";
	}

	const char *EXCHANGING_IN_COMBAT() {
		return "\x03" "c\v007\t000Echange non autoris$ en combat!";
	}

	const char *CURRENT_MAXIMUM_RATING_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Actuel / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u\n"
			   "\x3"
			   "cClassnt: %s";
	}

	const char *CURRENT_MAXIMUM_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Actuel / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u";
	}

	const char **RATING_TEXT() {
		static const char *_ratingText[] = {
			"Nonexistant", "Tr%s Pauvre", "Pauvre", "Tr%s Bas", "Bas", "Moyen", "Bon",
				"tr%s Bon", "Haut", "Tr%s Haut", "Magnifique", "Super", "Stup$fiant", "Incroyable",
				"Gigantesque", "Fantastique", "Terrible", "Ahurissant", "Monumental", "Formidable",
				// FIXME: Spelling incorrect. Collosal should be Colossal. Fixing changes game data and thus may cause issues
				"Collosal", "Imposant", "Impressionnant", "Ultime"
		};
		return _ratingText;
	}

	const char **BORN() {
		static const char *_born[] = { "N$ le", "" };
		return _born;
	}

	const char *AGE_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Actuel / Natural\n"
			   "\x3r\t057%u\x3l\t061/ %u\n"
			   "\x3"
			   "c%s: %u / %u\x1";
	}

	const char *LEVEL_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Actuel / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u\n"
			   "\x3"
			   "c%u Attaque%s/Tour\x1";
	}

	const char *RESISTENCES_TEXT() {
		return "\x2\x3"
			   "c%s\x3l\n"
			   "\t020Feu\t100%u\n"
			   "\t020Froid\t100%u\n"
			   "\t020Electricit$\t100%u\n"
			   "\t020Poison\t100%u\n"
			   "\t020Energ\t100%u\n"
			   "\t020Magie\t100%u";
	}

	const char *NONE() {
		return "\n\t020Aucune";
	}

	const char *EXPERIENCE_TEXT() {
		return "\x2\x3"
			   "c%s\x3l\n"
			   "\t010Actuel:\t070%u\n"
			   "\t010Proch Niv:\t070%s\x1";
	}

	const char *ELIGIBLE() {
		return "\f12Eligible\fd";
	}

	const char *IN_PARTY_IN_BANK() {
		return "\x2\x3"
			   "cLe Groupe a %s\n"
			   "%u et\n"
			   "%u en banque\x1\x3l";
	}

	const char **FOOD_ON_HAND() {
		static const char *_onHand[] = { "", "", "" };
		return _onHand;
	}

	const char *FOOD_TEXT() {
		return "\x2\x3"
			   "c%s Groupe\n"
			   "%u %s\n"
			   "Pour %u jour%s\x3l";
	}

	const char *EXCHANGE_WITH_WHOM() {
		return "\t010\v005Echanger avec qui?";
	}

	const char *QUICK_REF_LINE() {
		return "\v%3d\t007%u)\t027%s\t110%c%c%c\x3r\t160\f%02u%u\fd"
			   "\x3l\t170\f%02u%d\fd\t208\f%02u%u\fd\t247\f"
			   "%02u%u\fd\t270\f%02u%c%c%c%c\fd";
	}

	const char *QUICK_REFERENCE() {
		return "\r\x3"
			   "cR$f$rence Rapide\v012\x3l"
			   "\t007*\t027Nom\t110Cls\t140Niv\t176P.V."
			   "\t212P.S.\t241C.A.\t270Cond"
			   "%s%s%s%s%s%s%s%s"
			   "\v110\t064\x3"
			   "cOr\t144Gems\t224Nourrit\v119"
			   "\t064\f15%u\t144%u\t224%u jour%s\fd";
	}

	const char *ITEMS_DIALOG_TEXT1() {
		return "\r\x02\x03" "c\v021\t017\f37A\fdrme\t051A\f37r\fdmur\t085A\f37c\fdces\t119\f37D\fdivr\t153%s\t187%s\t221%s\t255%s\t289Exit";
	}

	const char *ITEMS_DIALOG_TEXT2() {
		return "\r\x02\x03" "c\v021\t017\f37A\fdrme\t051A\f37r\fdmur\t085A\f37c\fdces\t119\f37D\fdivr\t153\f37%s\t289Exit";
	}

	const char *ITEMS_DIALOG_LINE1() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\n";
	}

	const char *ITEMS_DIALOG_LINE2() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%u\n";
	}

	const char *BTN_BUY() {
		return "Ac\f37h\fdet";
	}

	const char *BTN_SELL() {
		return "\f37V\fdend";
	}

	const char *BTN_IDENTIFY() {
		return "\f37I\fddent";
	}

	const char *BTN_FIX() {
		return "R$\f37p\fdar";
	}

	const char *BTN_USE() {
		return "\f37U\fdtil";
	}

	const char *BTN_EQUIP() {
		return "\f37E\fdquip";
	}

	const char *BTN_REMOVE() {
		return "E\f37n\fdlev";
	}

	const char *BTN_DISCARD() {
		return "\f37J\fdet";
	}

	const char *BTN_QUEST() {
		return "\f37Q\fdu#t";
	}

	const char *BTN_ENCHANT() {
		return "En\fdchant";
	}

	const char *BTN_RECHARGE() {
		return "Re\fdchrg";
	}

	const char *BTN_GOLD() {
		return "O\fdr";
	}

	const char *ITEM_BROKEN() {
		return "\f32cass$(e) ";
	}

	const char *ITEM_CURSED() {
		return "\f09envout$(e) ";
	}

	const char *ITEM_OF() {
		return "de ";
	}

	const char **BONUS_NAMES() {
		static const char *_bonusNames[] = {
			"", "Destructeur de Dragon", "Eliminateur d'Immortel", "Destructeur de Golem",
				"Ecraseur de Sauterelle", "Tueur de Monstre", "Eliminateur de B#te"
		};
		return _bonusNames;
	}

	const char **WEAPON_NAMES() {
		static const char *_weaponNames[] = {
			nullptr, "longue $p$e ", "$p$e courte ", "dble tranchant ", "cimeterre ",
				"coutelat ", "sabre ", "matraque ", "hachette ", "katana ", "nunchakus ",
				"wakazashi ", "poignard ", "massue ", "fleau ", "gourdin ", "maul ", "lance ",
				"bardiche ", "glaive ", "hallebarde ", "pique ", "flamberge ", "trident ",
				"b&ton ", "marteau ", "naginata ", "hache  ", "grde hache ", "super hache ",
				"arc court ", "arc long ", "arbal%te ", "fronde ", "Ep$e Destruct Xeen ",
				"Longue Ep$e Ancienne ", "Poignard Ancien ", "Massue Ancienne ", "Lance Ancienne ",
				"B&ton Ancien ", "Arc Long Ancien "
		};
		return _weaponNames;
	}

	const char **ARMOR_NAMES() {
		static const char *_armorNames[] = {
			nullptr, "robes ", "$caille ", "maille anneaux ", "maille chaines ",
				"maille $clats ", "c[te de maille ", "cuirace ", "bouclier ",
				"casque ", "boots ", "manteau ", "cape ", "gants "
		};
		return _armorNames;
	}

	const char **ACCESSORY_NAMES() {
		static const char *_accessoryNames[] = {
			nullptr, "bague ", "ceinture ", "broche ", "m$daille ", "charm ", "cam$e ",
				"scarab$e ", "pendantif ", "collier ", "amulette "
		};
		return _accessoryNames;
	}

	const char **MISC_NAMES() {
		static const char *_miscNames[] = {
			nullptr, "baguette ", "bijoux ", "gem ", "boite ", "globe ", "trompe ", "pi%ce ",
				"baguette ", "sifflet ", "potion ", "parchemin ", "bogus", "bogus", "bogus",
				"bogus", "bogus", "bogus", "bogus", "bogus", "bogus", "bogus"
		};
		return _miscNames;
	}

	const char **SPECIAL_NAMES() {
		static const char *_specialNames[] = {
			nullptr, " lumi%re", " r$veil", " fl%ches", " premier soin", " poings", " boules feu", " endormant",
				" revivification", " gu$rissant", " $tincellant", " $clats", " insecticide", " nuages toxiques", " protect $lts",
				" douleur", " sautant", " contr[le b#te", " clairvoyance", " chge immortel", " l$vitation", " vue de sorcier", " b$nissant",
				" ident. monstre", " $clairant", " bonus saints", " cure puissante", " cures natures", " balises",
				" protection", " hero_sme", " hypnotisme", " marche sur l'eau", " froid mordant", " cherche monstre", " boules de feu",
				" rayons lunaire", " antidotes", " vapeur d'acide", " distortion temps", " endort dragon", " vaccination", " t$l$portation",
				" mort subite", " libre action", " arr#te golem", " salves poison", " essaims mortels", " abritant", " protection",
				" sorcellerie", " agr$able", " fl$au ardents", " rechargant", " congelant", " portail ville", " pierre ] la chair",
				" r$veil mort", " sublimation", " $p$es dan}antes", " rayons lunaires", " distortion masse", " lumi%re prismat",
				" enchante objt", " incin$rant", " mots saints", " r$surrection", " temp#tes", " m$gavoltage", " infernos",
				" rayons solaires", " implosions", " explosions $toiles", " des DIEUX!"
		};
		return _specialNames;
	}

	const char **ELEMENTAL_NAMES() {
		static const char *_elementalNames[] = {
			"Feu", "Elec", "Froid", "Acid/Poison", "Energ", "Magie"
		};
		return _elementalNames;
	}

	const char **ATTRIBUTE_NAMES() {
		static const char *_attributeNames[] = {
			"pouvoir", "Intellect", "Personalit$", "Vitesse", "pr$cision", "Chance",
				"Pts de Vie", "Pts de Sort", "Classe d'Armure", "Vol"
		};
		return _attributeNames;
	}

	const char **EFFECTIVENESS_NAMES() {
		static const char *_effectivenessNames[] = {
			nullptr, "Dragons", "Immortel", "Golems", "Sauterelles", "Monstres", "B#tes"
		};
		return _effectivenessNames;
	}

	const char **QUEST_ITEM_NAMES() {
		static const char *_questItemNames[] = {
			"Acte pour Chateau-Neuf",
				"Cl$ en Cristal Tour des Sorci%res",
				"Cl$ en squelette Tour de Darzog",
				"Cl$ enchant$e Tour de Haute Magie",
				"Amulette Bijoux Sphinx du Nord",
				"Pierre Mille Terreurs",
				"Pierre Golem d'Admission",
				"Pierre Yak d'Ouverture",
				"Sceptre de Xeen Distortion Temporelle",
				"Alacorne de Falista",
				"Elixir de Restauration",
				"Baguette de F$e Magique",
				"Tiare Princesse Roxanne",
				"Livre Saint d'Elvenkind",
				"Scarab$e de Vision",
				"Cristaux Pi$zo$lectricit$",
				"Parchemin Perspicacit$",
				"Racine Phirna",
				"Sifflet Os d'Orothin",
				"Pendatif Magique de Barok",
				"Cr&ne Manquant de Ligono",
				"Derni%re Fleur de l'Et$",
				"Derni%re Goutte de Pluie du Printemps",
				"Dernier Flocon de neige de l'Hiver",
				"Derni%re Feuille de l'Automne",
				"Roche de Lave",
				"M$ga Cr$dit du Roi",
				"Permis d'Excavation",
				"Poup$e Cupie",
				"Poup$e Pouvoir",
				"Poup$e Vitesse",
				"Poup$e Endurance",
				"Poup$e Pr$cision",
				"Poup$e Chance",
				"Gadget",
				"Laissez-passer Castelvue",
				"Laissez-passer Sablonique",
				"Laissez-passer Bergelac",
				"Laissez-passer N$cropolis",
				"Laissez-passer Olympe",
				"Cl$ Tour Occidentale",
				"Cl$ Tour M$ridionale",
				"Cl$ Tour Orientale",
				"Cl$ Tour Septentrionale",
				"Cl$ Tour d'Ellinger",
				"Cl$ Tour du Dragon",
				"Cl$ Tour Pierrenoire",
				"Cl$ Temple de d'Ecorce",
				"Cl$ Donjon des Ames Perdues",
				"Cl$ Ancienne Pyramide",
				"Cl$ Donjon de la Mort",
				"Amulette Sphinx M$ridional",
				"Orbe Pharaon Dragon",
				"Cube Pouvoir",
				"Carillon d'Ouverture",
				"Carte d'identit$ en Or",
				"Carte d'identit$ en argent",
				"Vautour -fuge",
				"Bride",
				"Bride Enchant$e",
				"Carte au tr$sor (aller en E1 x1, y11)",
				"",
				"Fausse carte",
				"Collier d'Onyx",
				"Oeuf de Dragon",
				"Tribble",
				"Statuette p$gase dor$e",
				"Statuette dragon dor$e",
				"Statuette griffon dor$e",
				"Calice Protection",
				"Joyau des Ages",
				"Rossignol de S$r$nit$",
				"Coeur de Sandro",
				"Anneau d'Ector",
				"Pommeau d'$meraude de Vespar",
				"Couronne Reine Kalindra",
				"Loupe de Caleb",
				"Boite ] &me",
				"Boite ] &me avec Corak dedans",
				"Rubis Brut",
				"Emeraude Brute",
				"Saphir Brut",
				"Diamant Brut",
				"Melon Monga",
				"Disque d'Energie"
		};
		return _questItemNames;
	}

	const char **QUEST_ITEM_NAMES_SWORDS() {
		static const char *_questItemNamesSwords[] = {
			"Pass to Hart", "Pass to Impery", "Laissez-passer Bergelac", "Laissez-passer N$cropolis", "Laissez-passer Olympe",
				"Cl$ Egouts de Hart", "Cl$ Pyramide de Rettig", "Cl$ Ancien Temple",
				"Cl$ Pyramide de Canegtut", "Cl$ Pyramide d'Ascihep", "Cl$ Tour du Dragon",
				"Cl$ Tour Pierrenoire", "Cl$ Temple de d'Ecorce", "Cl$ Donjon des Ames Perdues",
				"Cl$ Ancienne Pyramide", "Cl$ Donjon de la Mort", "Red Magic Hammer",
				"Green Magic Hammer", "Golden Magic Wand", "Silver Magic Hammer", "Magic Coin",
				"Ruby", "Diamond Mineral", "Emerald", "Sapphire", "Carte au tr$sor (aller en E1 x1, y11)",
				"NOTUSED", "Melon", "Princess Crown", "Emerald Wand", "Druid Carving", "High Sign",
				"Holy Wheel", "Double Cross", "Sky Hook", "Sacred Cow", "Staff of the Mountain",
				"Hard Rock", "Soft Rock", "Rock Candy", "Ivy Plant", "Spirit Gem", "Temple of Sun holy lamp oil",
				"Noams Hammer", "Positive Orb", "Negative Orb", "FireBane Staff", "Diamond Edged Pick",
				"Melon Monga", "Disque d'Energie", "Old XEEN Quest Item"
		};
		return _questItemNamesSwords;
	}

	const char *NOT_PROFICIENT() {
		return "\t000\v007\x3"
			   "c%ss n'est efficace avec un(e) %s!";
	}

	const char *NO_ITEMS_AVAILABLE() {
		return "\x03" "c\n\t000Pas d'objet disponible.";
	}

	const char **CATEGORY_NAMES() {
		static const char *_categoryNames[] = { "Armes", "Armures", "Accessoires", "Divers" };
		return _categoryNames;
	}

	const char *X_FOR_THE_Y() {
		return "\x01\fd\r%s\v000\t000%s pour %s %s%s\v011\x02%s%s%s%s%s%s%s%s%s\x01\fd";
	}

	const char *X_FOR_Y() {
		return "\x01\fd\r\x03l\v000\t000%s pour %s\x03r\t000%s\x03l\v011\x02%s%s%s%s%s%s%s%s%s\x01\fd";
	}

	const char *X_FOR_Y_GOLD() {
		return "\x01\fd\r\x03l\v000\t000%s pour %s\t150Or - %lu%s\x03l\v011\x02%s%s%s%s%s%s%s%s%s\x01\fd";
	}

	const char *FMT_CHARGES() {
		return "\x3rr\t000Charges\x3l";
	}

	const char *AVAILABLE_GOLD_COST() {
		return "\x01\fd\r\x03l\v000\t000Disponible %s\t150Or - %lu\x03r\t000Co=t\x03l\v011\x02%s%s%s%s%s%s%s%s%s\x01\fd";
	}

	const char *CHARGES() {
		return "Charges";
	}

	const char *COST() {
		return "Co=t";
	}

	const char **ITEM_ACTIONS() {
		static const char *_itemActions[] = {
			"Equiper", "Enlever", "Utiliser", "Jeter", "Enchant", "Recharge", "Or"
		};
		return _itemActions;
	}

	const char *WHICH_ITEM() {
		return "\t010\v005%s quel objet?";
	}

	const char *WHATS_YOUR_HURRY() {
		return "\v007Pourquoi si press$?\nAttendez de sortir de l]!";
	}

	const char *USE_ITEM_IN_COMBAT() {
		return "\v007Util: pour ] votre tour utiliser un objet en cour de Combat!";
	}

	const char *NO_SPECIAL_ABILITIES() {
		return "\v005\x3"
			   "c%s\fdn'a pas de talents sp$ciaux!";
	}

	const char *CANT_CAST_WHILE_ENGAGED() {
		return "\x3"
			   "c\v007Impossible de lancer %s pendant un combat!";
	}

	const char *EQUIPPED_ALL_YOU_CAN() {
		return "\x3"
			   "c\v007Vous ne pouvez plus mettre %ss!";
	}

	const char *REMOVE_X_TO_EQUIP_Y() {
		return "\x3"
			   "c\v007Enlevez %s pour mettre autre %s\b!";
	}

	const char *RING() {
		return "bague";
	}

	const char *MEDAL() {
		return "m$daille";
	}

	const char *CANNOT_REMOVE_CURSED_ITEM() {
		return "\x03Vous ne pouvez pas enlever un objet envout$!";
	}

	const char *CANNOT_DISCARD_CURSED_ITEM() {
		return "\x03xcVous ne pouvez pas jeter un objet envout$!";
	}

	const char *PERMANENTLY_DISCARD() {
		return "\v000\t000\x3lJeter pour toujours %s\fd?";
	}

	const char *BACKPACK_IS_FULL() {
		return "\v005\x3"
			   "c\fdLe sac ] dos de %s est rempli.";
	}

	const char **CATEGORY_BACKPACK_IS_FULL() {
		static const char *_categoryBackpackIsFull[] = {
			"\v010\t000\x3"
			"cLe sac ] arme de %s est plein.",
				"\v010\t000\x3"
				"cLe sac ] armure de %s est plein.",
				"\v010\t000\x3"
				"cLe sac ] accessoires de %s est rempli.",
				"\v010\t000\x3"
				"cLe sac ] divers de %s est plein."
		};
		return _categoryBackpackIsFull;
	}

	const char *BUY_X_FOR_Y_GOLD() {
		return "\x3l\v000\t000\fdAcheter %s\fd pour %u %s?";
	}

	const char *SELL_X_FOR_Y_GOLD() {
		return "\x3l\v000\t000\fdVendre %s\fd pour %u %s?";
	}

	const char **GOLDS() {
		static const char *_sellXForYGoldEndings[] = { "or", "" };
		return _sellXForYGoldEndings;
	}

	const char *NO_NEED_OF_THIS() {
		return "\v005\x3"
			   "c\fdNous n'avons pas besoin de cet(te) %s\f!";
	}

	const char *NOT_RECHARGABLE() {
		return "\v012\x3"
			   "c\fdPas rechargeable.  %s";
	}

	const char *NOT_ENCHANTABLE() {
		return "\v012\t000\x3"
			   "cPas d'enchantement possible.  %s";
	}

	const char *SPELL_FAILED() {
		return "Echec du Sort!";
	}

	const char *ITEM_NOT_BROKEN() {
		return "\fdCet objet n'est pas cass$!";
	}

	const char **FIX_IDENTIFY() {
		static const char *_fixIdentify[] = { "R$parer", "Identifier" };
		return _fixIdentify;
	}

	const char *FIX_IDENTIFY_GOLD() {
		return "\x3l\v000\t000%s %s\fd pour %u %s?";
	}

	const char *IDENTIFY_ITEM_MSG() {
		return "\fd\v000\t000\x3"
			   "cIdentifier Objet\x3l\n"
			   "\n"
			   "\v012%s\fd\n"
			   "\n"
			   "%s";
	}

	const char *ITEM_DETAILS() {
		return "Capacit$s\t132:\t140%s\n"
			   "Modification PV\t132:\t140%s\n"
			   "Dommage Physique\t132:\t140%s\n"
			   "Dommage El$ments\t132:\t140%s\n"
			   "R$sistance El$ments\t132:\t140%s\n"
			   "Bonus C.A.\t132:\t140%s\n"
			   "Attribution Bonus\t132:\t140%s\n"
			   "Pouvoir Special\t132:\t140%s";
	}

	const char *ALL() {
		return "Tout";
	}

	const char *FIELD_NONE() {
		return "Aucun";
	}

	const char *DAMAGE_X_TO_Y() {
		return "%d ] %d";
	}

	const char *ELEMENTAL_XY_DAMAGE() {
		return "%+d %s Dommage";
	}

	const char *ATTR_XY_BONUS() {
		return "%+d %s";
	}

	const char *EFFECTIVE_AGAINST() {
		return "x3 vs %s";
	}

	const char *QUESTS_DIALOG_TEXT() {
		return "\r\x02\x03" "c\v021\t017\f37O\fdbjets\t085\f37Q\fdu#tes\t153\f37A\fduto Notes\t221\f37H\fdaut\t255\f37B\fdas\t289Exit";
	}

	const char *CLOUDS_OF_XEEN_LINE() {
		return "\x08 \x08*-- \f04Nuages de Xeen\fd --";
	}

	const char *DARKSIDE_OF_XEEN_LINE() {
		return "\x08 \x08*-- \f04Darkside of Xeen\fd --";
	}

	const char *SWORDS_OF_XEEN_LINE() {
		return "\b \b*-- \f04Swords of Xeen\fd --";
	}

	const char *NO_QUEST_ITEMS() {
		return "\r\x01\fd\x03" "c\v000\t000Objets Qu#tes\x03l\x02\n\n\x03" "cPas d'Objet de Qu#tes";
	}

	const char *NO_CURRENT_QUESTS() {
		return "\x03" "c\v000\t000\n\nPas de Qu#tes en cours";
	}

	const char *NO_AUTO_NOTES() {
		return "\x3"
			   "cPas d'Auto Note";
	}

	const char *QUEST_ITEMS_DATA() {
		return "\r\x1\fd\x3"
			   "c\v000\t000Objets Qu#tes\x3l\x2\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s";
	}

	const char *CURRENT_QUESTS_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000Qu#tes en cours\x3l\x2\n"
			   "%s\n"
			   "\n"
			   "%s\n"
			   "\n"
			   "%s";
	}

	const char *AUTO_NOTES_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000Auto Notes\x3l\x2\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l";
	}

	const char *REST_COMPLETE() {
		return "\v000\t0008 hres passent.  Fin de Repos.\n"
			   "%s\n"
			   "%d nourrit; consomm$s.";
	}

	const char *PARTY_IS_STARVING() {
		return "\f07Le groupe meurt de faim!\fd";
	}

	const char *HIT_SPELL_POINTS_RESTORED() {
		return "PV et Pts de Sort restor$s.";
	}

	const char *TOO_DANGEROUS_TO_REST() {
		return "Trop dangeureux de se reposer ici!";
	}

	const char *SOME_CHARS_MAY_DIE() {
		return "Certains pourraient en mourir. Se reposer?";
	}

	const char *DISMISS_WHOM() {
		return "\t010\v005Renvoyer qui?";
	}

	const char *CANT_DISMISS_LAST_CHAR() {
		return "Vous ne pouvez pas renvoyer votre unique personnage!";
	}

	const char *DELETE_CHAR_WITH_ELDER_WEAPON() {
		return "\v000\t000Ce personnage a une Arme Ancienne et ne peut pas #tre supprim$!";
	}

	const char **REMOVE_DELETE() {
		static const char *_removeDelete[] = { "Enlever", "Supprimer" };
		return _removeDelete;
	}

	const char *REMOVE_OR_DELETE_WHICH() {
		return "\x3l\t010\v005%s quel personnage?";
	}

	const char *YOUR_PARTY_IS_FULL() {
		return "\v007Votre Groupe est complet!";
	}

	const char *HAS_SLAYER_SWORD() {
		return "\v000\t000Ce personnage a l'Ep$e Destruct Xeen et ne peut pas #tre supprim$!";
	}

	const char *SURE_TO_DELETE_CHAR() {
		return "Etes-vous s=r de vouloir supprimer %s le %s?";
	}

	const char *CREATE_CHAR_DETAILS() {
		return "\f04\x3"
			   "c\x2\t144\v119E\f37n\f04roler\t144\v149\f37C\f04r$er"
			   "\t144\v179\f37ESC\f04\x3l\x1\t195\v021P\f37o\f04u"
			   "\t195\v045\f37I\f04nt\t195\v069\f37P\f04er\t195\v093\f37E\f04nd"
			   "\t195\v116\f37V\f04it\t195\v140P\f37r\f04e\t195\v164C\f37h\f04a%s";
	}

	const char *NEW_CHAR_STATS() {
		return "\f04\x3l\t022\v148Race\t055: %s\n"
			   "\t022Sexe\t055: %s\n"
			   "\t022Class\t055:\n"
			   "\x3r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
			   "\t215\v151%d\t215\v175%d\x3l\t242\v020\f%.2dChevalier\t242\v031\f%.2d"
			   "Paladin\t242\v042\f%.2dArcher\t242\v053\f%.2dClerc\t242\v064\f%.2d"
			   "Sorcier\t242\v075\f%.2dVoleur\t242\v086\f%.2dNinja\t242\v097\f%.2d"
			   "Barbare\t242\v108\f%.2dDruide\t242\v119\f%.2dForestier\f04\x3"
			   "c"
			   "\t265\v142Talents\x3l\t223\v155%s\t223\v170%s%s";
	}

	const char *NAME_FOR_NEW_CHARACTER() {
		return "\x03" "cEntrez un Nom pour ce personnage\n\n";
	}

	const char *SELECT_CLASS_BEFORE_SAVING() {
		return "\v006\x3"
			   "cS$lectionnez une Classe avant de sauvegarder.\x3l";
	}

	const char *EXCHANGE_ATTR_WITH() {
		return "Echanger %s avec...";
	}

	const int *NEW_CHAR_SKILLS_OFFSET() {
		static const int _newCharSkillsOffset[] = { 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 };
		return _newCharSkillsOffset;
	}

	const char *COMBAT_DETAILS() {
		return "\r\f00\x3"
			   "c\v000\t000\x2"
			   "Combat%s%s%s\x1";
	}

	const char *NOT_ENOUGH_TO_CAST() {
		return "\x3"
			   "c\v010Pas assez de %s pour Lancer %s";
	}

	const char **SPELL_CAST_COMPONENTS() {
		static const char *_spellCastComponents[] = { "Points de Sort", "Gems" };
		return _spellCastComponents;
	}

	const char *CAST_SPELL_DETAILS() {
		return "\r\x2\x3"
			   "c\v122\t013\f37J\fdeter\t040\f37N\fdouv"
			   "\t067ESC\x1\t000\v000\x3"
			   "cJeter Sort\n"
			   "\n"
			   "%s\x3l\n"
			   "\n"
			   "Sort Pr#t:\x3"
			   "c\n"
			   "\n"
			   "\f09%s\fd\x2\x3l\n"
			   "\v082Co=t\x3r\t000%u/%u\x3l\n"
			   "Pts Sort\x3r\t000%u\x1";
	}

	const char *PARTY_FOUND() {
		return "\x3"
			   "cLe Groupe a trouv$:\n"
			   "\n"
			   "\x3r\t000%u Or\n"
			   "%u Gems";
	}

	const char *BACKPACKS_FULL_PRESS_KEY() {
		return "\v007\f12Attention!  Sac ] dos remplis!\fd\nUne touche";
	}

	const char *HIT_A_KEY() {
		return "\x03l\v120\t000\x04" "077\x03" "c\f37Une touche\fd";
	}

	const char *GIVE_TREASURE_FORMATTING() {
		return "\x3l\v060\t000\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	const char **FOUND() {
		static const char *_found[] = { " a trouv$", "" };
		return _found;
	}

	const char *X_FOUND_Y() {
		return "\v060\t000\x3"
			   "c%s%s: %s";
	}

	const char *ON_WHO() {
		return "\x03" "c\v009Sur Qui?";
	}

	const char *WHICH_ELEMENT1() {
		return "\r\x3"
			   "c\x1Quel El$ment?\x2\v034\t014\f15F\fdeu\t044"
			   "\f15E\fdlec\t074\f15Fr\fdoid\t104\f15A\fdcid\x1";
	}

	const char *WHICH_ELEMENT2() {
		return "\r\x3"
			   "cQuel El$ment?\x2\v034\t014\f15F\fdeu\t044"
			   "\f15E\fdlec\t074\f15Fr\fdoid\t104\f15A\fdcid\x1";
	}

	const char *DETECT_MONSTERS() {
		return "\x3"
			   "cD$tecter Monstres";
	}

	const char *LLOYDS_BEACON() {
		return "\r\x3"
			   "c\v000\t000\x01" "Balise Lloyd\n"
			   "\n"
			   "Dernier Lieu\n"
			   "\n"
			   "%s\x3l\n"
			   "x : %d\x3r\t000y : %d\x3"
			   "c\x2\v122\t021\f15P\fdlacer\t060\f15R\fdetour\x1";
	}

	const char *HOW_MANY_SQUARES() {
		return "\x3"
			   "cT$l$porter\n%s de combien de (1-9)\n";
	}

	const char *TOWN_PORTAL() {
		return "\x3"
			   "cPortail de Ville\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\n"
			   "\t0104. %s\n"
			   "\t0105. %s\x3"
			   "c\n"
			   "\n"
			   "Pour quel ville (1-5)\n"
			   "\n";
	}

	const char *TOWN_PORTAL_SWORDS() {
		return "\x3"
			   "cPortail de Ville\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\x3"
			   "c\n"
			   "\n"
			   "Pour quel ville (1-3)\n"
			   "\n";
	}

	const char *MONSTER_DETAILS() {
		return "\x3l\n"
			   "%s\x3"
			   "c\t100%s\t140%u\t180%u\x3r\t000%s";
	}

	const char **MONSTER_SPECIAL_ATTACKS() {
		static const char *_monsterSpecialAttacks[] = {
			"N$ant", "Magie", "Feu", "Elec", "Froid", "Poison", "Energ", "Maladie",
				"Saoul", "Endormi", "ObjEnvout", "Amoureu", "PtsSRet", "Envout", "Paralys",
				"Inconsc", "Confus", "ArmCass", "Malade", "Suppr", "Age+5", "Mort", "Pierre"
		};
		return _monsterSpecialAttacks;
	}

	const char *IDENTIFY_MONSTERS() {
		return "Nom\x3"
			   "c\t100PV\t140CA\t177#Atq\x3r\t000Special%s%s%s";
	}

	const char *MOONS_NOT_ALIGNED() {
		return "\x3"
			   "c\v012\t000Les lunes ne sont pas align$es. Le passage vers %s est indisponible";
	}

	const char *AWARDS_FOR() {
		return "\r\x1\fd\x3"
			   "c\v000\t000D$corations de %s, %s\x3l\x2\n"
			   "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";
	}

	const char *AWARDS_TEXT() {
		return "\r\x02\x03" "c\v021\t221\f37H\fdaut\t255\f37B\fdas\t289Exit";
	}

	const char *NO_AWARDS() {
		return "\x3"
			   "cPas de d$coration";
	}

	const char *WARZONE_BATTLE_MASTER() {
		return "Warzone\n\t125Ma{tre de Guerre";
	}

	const char *WARZONE_MAXED() {
		return "Quoi! Encore vous? allez chercher quelqu'un de votre niveau!";
	}

	const char *WARZONE_LEVEL() {
		return "Monstres de quel niveau? (1-10)\n";
	}

	const char *WARZONE_HOW_MANY() {
		return "Combien de monstres? (1-20)\n";
	}

	const char *PICKS_THE_LOCK() {
		return "\x3"
			   "c\v010%s %s la serrure!\nUne touche.";
	}

	const char **PICK_FORM() {
		static const char *_pickForm[] = { "crochette", "" };
		return _pickForm;
	}

	const char *UNABLE_TO_PICK_LOCK() {
		return "\x3"
			   "c\v010%s %s la serrure!\nUne touche.";
	}

	const char **UNABLE_TO_PICK_FORM() {
		static const char *_unableToPickForm[] = { "n'a pas pu crocheter", "" };
		return _unableToPickForm;
	}

	const char *CONTROL_PANEL_TEXT() {
		return "\x1\f00\x3"
			   "c\v000\t000Panneau de Contr[le\x3r"
			   "\v022\t045\f06C\fdharg:\t124\f06E\fdfx:"
			   "\v041\t045\f06S\fdauv:\t124\f06M\fdusic:"
			   "\v060\t045\f06Q\fduit:"
			   "\v080\t084Mr \f06W\fdizard:%s\t000\x1";
	}

	const char *CONTROL_PANEL_BUTTONS() {
		return "\x3"
			   "c\f11"
			   "\v022\t062char\t141%s"
			   "\v041\t062sauv\t141%s"
			   "\v060\t062exit"
			   "\v079\t102Aide\fd";
	}

	const char *ON() {
		return "\f15on\f11";
	}

	const char *OFF() {
		return "\f32off\f11";
	}

	const char *CONFIRM_QUIT() {
		return "Etes-vous s=r de vouloir quitter?";
	}

	const char *MR_WIZARD() {
		return "Vous voulez vraiment l'aide de Mr. Wizard?";
	}

	const char *NO_LOADING_IN_COMBAT() {
		return "Chargement non autoris$ en cours de combat!";
	}

	const char *NO_SAVING_IN_COMBAT() {
		return "Sauvegarde non autoris$ en cours de combat!";
	}

	const char *QUICK_FIGHT_TEXT() {
		return "\r\fd\x3"
			   "c\v000\t000Combat Rapide\n\n"
			   "%s\x3l\n\n"
			   "Actuel\x3r\n"
			   "\t000%s\x2\x3"
			   "c\v122\t021\f37S\f04uivant\t060Exit\x1";
	}

	const char **QUICK_FIGHT_OPTIONS() {
		static const char *_quickFightOptions[] = { "Attaquer", "Lancer", "Bloquer", "Courir" };
		return _quickFightOptions;
	}

	const char **WORLD_END_TEXT() {
		static const char *_worldEndText[] = {
			"\n\n\n\n\n\n\n"
			"F$licitation Aventuriers!\n\n"
			"Que la c$r$monie d'unification commence!",
				("Et alors le bruit que la Proph%tie $tait sur le point de se r$aliser se r$pendit partout sur les terres de Xeen."),
				"Ils virent en grand nombre pour #tre t$moin de l'$v$nement.",
				"\v026Le Dragon Pharaon pr$sida la c$r$monie.",
				"\v026La Reine Kalindra pr$senta le Cube de Pouvoir.",
				"\v026Prince Roland pr$senta le Sceptre de Xeen.",
				"\v026Ensemble, ils plac%rent le Cube de Pouvoir...",
				"\v026et le Sceptre, sur l'Altar de la Jonction.",
				"La proph$tie accomplie, les deux c[t$s de Xeen furent unis en un seul",
		};
		return _worldEndText;
	}

	const char *WORLD_CONGRATULATIONS() {
		return "\x3"
			   "cF$licitations\n\n"
			   "Votre Score Final est:\n\n"
			   "%010lu\n"
			   "\x3l\n"
			   "Envoyez ce score au QG des Anciens o vous deviendrez une l$gende!\n\n"
			   "Quartier G$n$ral des Anciens\n"
			   "PPS/New World Computing, Inc.\n"
			   "150, bd. Haussmann\n"
			   "75008 Paris";
	}

	const char *WORLD_CONGRATULATIONS2() {
		return "\n\n\n\n\n\n"
			   "Mais... encore autre chose!\n"
			   "\n\n"
			   "Notez le  message\n"
			   "\"%s\"\n"
			   "avec votre score final et vous aurez peut-#tre un bonus.";
	}

	const char *CLOUDS_CONGRATULATIONS1() {
		return "\f23\x03l\v000\t000Envoyez ce score QG des Anciens ou vous deviendrez une l$gende!\f33\x03" "c\v070\t000Une touche";
	}

	const char *CLOUDS_CONGRATULATIONS2() {
		return "\f23\x03l\v000\t000Quartier G$n$rale des Anciens\nPPS/New World Computing, Inc.\n150, bd. Haussmann\n75008 Paris\f33\x03" "c\v070\t000Une touche";
	}

	const char **GOOBER() {
		static const char *_goober[] = {
			"", "Je suis un Goober!", "Je suis un Super Goober!"
		};
		return _goober;
	}

	const char *DIFFICULTY_TEXT() {
		return "\v000\t000\x03" "cS$lection des pr$f$rences";
	}

	const char *SAVE_OFF_LIMITS() {
		return "\x03" "c\v002\t000Les Dieux de la restauration de jeu juge cette zone hors limites!\nD$sol$, sauvegarde impossible dans ce labyrinthe.";
	}

	const char *CLOUDS_INTRO1() {
		return "\f00\v082\t040\x3"
			   "cRoi Burlock\v190\t040Paysans\v082\t247"
			   "Lord Xeen\v190\t258La b#te de Xeen\v179\t150Crodo";
	}

	const char *DARKSIDE_ENDING1() {
		return "\n\x3"
			   "cF$licitations\n"
			   "\n"
			   "Votre Score Final est:\n"
			   "\n"
			   "%010lu\n"
			   "\x3"
			   "l\n"
			   "Envoyez ce score au QG de Anciens o vous deviendrez une l$gende!\n"
			   "\n"
			   "Quartier G$n$ral des Anciens\n"
			   "PPS/New World Computing, Inc.\n"
			   "150, bd. Haussmann\n"
			   "75008 Paris";
	}

	const char *DARKSIDE_ENDING2() {
		return "\nAventuriers,\n\nJ'ai sauvegard$ votre partie dans Castlevue.\n\nLe Monde de Xeen a besoin de vous!\n\nChargez votre partie et venez me voir dans la Grande Pyramide pour plus d'instructions";
	}

	const char *PHAROAH_ENDING_TEXT1() {
		return "\fd\v001\t001%s\x3"
			   "c\t000\v180Une touche!\x3"
			   "l";
	}

	const char *PHAROAH_ENDING_TEXT2() {
		return "\f04\v000\t000%s\x3"
			   "c\t000\v180Une touche!\x3"
			   "l\fd";
	}

	const char **CLOUDS_MAE_NAMES() {
		static const char *_maeNames[131] = { "" };
		return _maeNames;
	}

	const char **CLOUDS_MIRROR_LOCATIONS() {
		static const char *_mirrLocs[59] = { "" };
		return _mirrLocs;
	}

	const char **CLOUDS_MAP_NAMES() {
		static const char *_mapNames[] = {
			"",
		};
		return _mapNames;
	}

	const char **CLOUDS_MONSTERS() {
		static const char *_monsters[91] = { "" };
		return _monsters;
	}

	const char **CLOUDS_SPELLS() {
		static const char *_spells[77] = { "" };
		return _spells;
	}

	class FR_KeyConstants : public KeyConstants {
	public:
		class FR_DialogsCharInfo : public DialogsCharInfo {
		public:
			int KEY_ITEM()     { return Common::KEYCODE_o; }
			int KEY_QUICK()    { return Common::KEYCODE_r; }
			int KEY_EXCHANGE() { return Common::KEYCODE_e; }
		};
		FR_DialogsCharInfo *dialogsCharInfo() {
			if (!_dci)_dci = new FR_DialogsCharInfo();
			return _dci;
		}

		class FR_DialogsControlPanel : public DialogsControlPanel {
		public:
			int KEY_FXON()     { return Common::KEYCODE_e; }
			int KEY_MUSICON()  { return Common::KEYCODE_m; }
			int KEY_LOAD()     { return Common::KEYCODE_c; }
			int KEY_SAVE()     { return Common::KEYCODE_s; }
			int KEY_QUIT()     { return Common::KEYCODE_q; }
			int KEY_MRWIZARD() { return Common::KEYCODE_w; }
		};
		FR_DialogsControlPanel *dialogsControlPanel() {
			if (!_dcp) _dcp = new FR_DialogsControlPanel();
			return _dcp;
		}

		class FR_DialogsCreateChar : public DialogsCreateChar {
		public:
			int KEY_ROLL()   { return Common::KEYCODE_n; }
			int KEY_CREATE() { return Common::KEYCODE_c; }
			int KEY_MGT()    { return Common::KEYCODE_o; }
			int KEY_INT()    { return Common::KEYCODE_i; }
			int KEY_PER()    { return Common::KEYCODE_p; }
			int KEY_END()    { return Common::KEYCODE_e; }
			int KEY_SPD()    { return Common::KEYCODE_v; }
			int KEY_ACY()    { return Common::KEYCODE_r; }
			int KEY_LCK()    { return Common::KEYCODE_h; }
		};
		FR_DialogsCreateChar *dialogsCreateChar() {
			if (!_dcc) _dcc = new FR_DialogsCreateChar();
			return _dcc;
		}

		class FR_DialogsDifficulty : public DialogsDifficulty {
		public:
			int KEY_ADVENTURER() { return Common::KEYCODE_a; }
			int KEY_WARRIOR()    { return Common::KEYCODE_w; }
		};
		FR_DialogsDifficulty *dialogsDifficulty() {
			if (!_dd) _dd = new FR_DialogsDifficulty();
			return _dd;
		}

		class FR_DialogsItems : public DialogsItems {
		public:
			int KEY_WEAPONS()   { return Common::KEYCODE_a; }
			int KEY_ARMOR()     { return Common::KEYCODE_r; }
			int KEY_ACCESSORY() { return Common::KEYCODE_c; }
			int KEY_MISC()      { return Common::KEYCODE_d; }
			int KEY_ENCHANT()   { return Common::KEYCODE_e; }
			int KEY_USE()       { return Common::KEYCODE_u; }
			int KEY_BUY()       { return Common::KEYCODE_h; }
			int KEY_SELL()      { return Common::KEYCODE_v; }
			int KEY_IDENTIFY()  { return Common::KEYCODE_i; }
			int KEY_FIX()       { return Common::KEYCODE_p; }
			int KEY_EQUIP()     { return Common::KEYCODE_e; }
			int KEY_REM()       { return Common::KEYCODE_n; }
			int KEY_DISC()      { return Common::KEYCODE_j; }
			int KEY_QUEST()     { return Common::KEYCODE_q; }
			int KEY_RECHRG()    { return Common::KEYCODE_r; }
			int KEY_GOLD()      { return Common::KEYCODE_g; }
		};
		FR_DialogsItems *dialogsItems() {
			if (!_di) _di = new FR_DialogsItems();
			return _di;
		}

		class FR_DialogsParty : public DialogsParty {
		public:
			int KEY_DELETE() { return Common::KEYCODE_d; }
			int KEY_REMOVE() { return Common::KEYCODE_r; }
			int KEY_CREATE() { return Common::KEYCODE_c; }
			int KEY_EXIT()   { return Common::KEYCODE_x; }
		};
		FR_DialogsParty *dialogsParty() {
			if (!_dp) _dp = new FR_DialogsParty();
			return _dp;
		}

		class FR_DialogsQuests : public DialogsQuests {
		public:
			int KEY_QUEST_ITEMS()    { return Common::KEYCODE_i; }
			int KEY_CURRENT_QUESTS() { return Common::KEYCODE_q; }
			int KEY_AUTO_NOTES()     { return Common::KEYCODE_a; }
		};
		FR_DialogsQuests *dialogsQuests() {
			if (!_dq) _dq = new FR_DialogsQuests();
			return _dq;
		}

		class FR_DialogsQuickFight : public DialogsQuickFight {
		public:
			int KEY_NEXT() { return Common::KEYCODE_s; }
		};
		FR_DialogsQuickFight *dialogsQuickFight() {
			if (!_dqf) _dqf = new FR_DialogsQuickFight();
			return _dqf;
		}

		class FR_DialogsSpells : public DialogsSpells {
		public:
			int KEY_CAST()   { return Common::KEYCODE_j; }
			int KEY_NEW()    { return Common::KEYCODE_n; }
			int KEY_FIRE()   { return Common::KEYCODE_f; }
			int KEY_ELEC()   { return Common::KEYCODE_e; }
			int KEY_COLD()   { return Common::KEYCODE_c; }
			int KEY_ACID()   { return Common::KEYCODE_a; }
			int KEY_SET()    { return Common::KEYCODE_t; }
			int KEY_RETURN() { return Common::KEYCODE_r; }
		};
		FR_DialogsSpells *dialogsSpells() {
			if (!_ds) _ds = new FR_DialogsSpells();
			return _ds;
		}

		class FR_Locations : public Locations {
		public:
			int KEY_DEP()        { return Common::KEYCODE_d; }
			int KEY_WITH()       { return Common::KEYCODE_r; }
			int KEY_GOLD()       { return Common::KEYCODE_o; }
			int KEY_GEMS()       { return Common::KEYCODE_e; }
			int KEY_BROWSE()     { return Common::KEYCODE_r; }
			int KEY_BUY_SPELLS() { return Common::KEYCODE_a; }
			int KEY_SPELL_INFO() { return Common::KEYCODE_i; }
			int KEY_SIGN_IN()    { return Common::KEYCODE_s; }
			int KEY_DRINK()      { return Common::KEYCODE_b; }
			int KEY_FOOD()       { return Common::KEYCODE_n; }
			int KEY_TIP()        { return Common::KEYCODE_p; }
			int KEY_RUMORS()     { return Common::KEYCODE_r; }
			int KEY_HEAL()       { return Common::KEYCODE_g; }
			int KEY_DONATION()   { return Common::KEYCODE_d; }
			int KEY_UNCURSE()    { return Common::KEYCODE_s; }
			int KEY_TRAIN()      { return Common::KEYCODE_s; }
		};
		FR_Locations *locations() {
			if (!_l) _l = new FR_Locations();
			return _l;
		}

		class FR_CloudsOfXeenMenu : public CloudsOfXeenMenu {
		public:
			int KEY_START_NEW_GAME() { return Common::KEYCODE_s; }
			int KEY_LOAD_GAME()      { return Common::KEYCODE_l; }
			int KEY_SHOW_CREDITS()   { return Common::KEYCODE_c; }
			int KEY_VIEW_ENDGAME()   { return Common::KEYCODE_e; }
		};
		FR_CloudsOfXeenMenu *cloudsOfXeenMenu() {
			if (!_soxm)
				_soxm = new FR_CloudsOfXeenMenu();
			return _soxm;
		}

		virtual ~FR_KeyConstants() {
			delete _dci;
			delete _dcp;
			delete _dcc;
			delete _dd;
			delete _di;
			delete _dp;
			delete _dq;
			delete _dqf;
			delete _ds;
			delete _l;
			delete _soxm;
		}

	private:
		FR_DialogsCharInfo     *_dci = NULL;
		FR_DialogsControlPanel *_dcp = NULL;
		FR_DialogsCreateChar   *_dcc = NULL;
		FR_DialogsDifficulty   *_dd = NULL;
		FR_DialogsItems        *_di = NULL;
		FR_DialogsParty        *_dp = NULL;
		FR_DialogsQuests       *_dq = NULL;
		FR_DialogsQuickFight   *_dqf = NULL;
		FR_DialogsSpells       *_ds = NULL;
		FR_Locations           *_l = NULL;
		FR_CloudsOfXeenMenu    *_soxm = NULL;
	};

	FR_KeyConstants *keyConstants() {
		if (!_kc)_kc = new FR_KeyConstants();
		return _kc;
	}

	virtual ~FR() {
		delete _kc;
	}

private:
	FR_KeyConstants *_kc = NULL;
};
