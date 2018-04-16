/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/carry/arm.h"
#include "titanic/carry/auditory_centre.h"
#include "titanic/carry/bowl_ear.h"
#include "titanic/carry/brain.h"
#include "titanic/carry/bridge_piece.h"
#include "titanic/carry/carry.h"
#include "titanic/carry/carry_parrot.h"
#include "titanic/carry/central_core.h"
#include "titanic/carry/chicken.h"
#include "titanic/carry/crushed_tv.h"
#include "titanic/carry/eye.h"
#include "titanic/carry/feathers.h"
#include "titanic/carry/fruit.h"
#include "titanic/carry/glass.h"
#include "titanic/carry/hammer.h"
#include "titanic/carry/head_piece.h"
#include "titanic/carry/hose.h"
#include "titanic/carry/hose_end.h"
#include "titanic/carry/key.h"
#include "titanic/carry/liftbot_head.h"
#include "titanic/carry/long_stick.h"
#include "titanic/carry/magazine.h"
#include "titanic/carry/maitred_left_arm.h"
#include "titanic/carry/maitred_right_arm.h"
#include "titanic/carry/mouth.h"
#include "titanic/carry/napkin.h"
#include "titanic/carry/nose.h"
#include "titanic/carry/note.h"
#include "titanic/carry/parcel.h"
#include "titanic/carry/perch.h"
#include "titanic/carry/phonograph_cylinder.h"
#include "titanic/carry/phonograph_ear.h"
#include "titanic/carry/photograph.h"
#include "titanic/carry/plug_in.h"
#include "titanic/carry/speech_centre.h"
#include "titanic/carry/sweets.h"
#include "titanic/carry/vision_centre.h"

#include "titanic/core/saveable_object.h"
#include "titanic/core/background.h"
#include "titanic/core/click_responder.h"
#include "titanic/core/dont_save_file_item.h"
#include "titanic/core/drop_target.h"
#include "titanic/core/file_item.h"
#include "titanic/core/game_object.h"
#include "titanic/core/game_object_desc_item.h"
#include "titanic/core/link_item.h"
#include "titanic/core/list.h"
#include "titanic/core/mail_man.h"
#include "titanic/core/message_target.h"
#include "titanic/support/movie_clip.h"
#include "titanic/core/multi_drop_target.h"
#include "titanic/core/node_item.h"
#include "titanic/core/project_item.h"
#include "titanic/core/room_item.h"
#include "titanic/core/saveable_object.h"
#include "titanic/core/static_image.h"
#include "titanic/core/turn_on_object.h"
#include "titanic/core/turn_on_play_sound.h"
#include "titanic/core/turn_on_turn_off.h"
#include "titanic/core/tree_item.h"
#include "titanic/core/view_item.h"

#include "titanic/game/announce.h"
#include "titanic/game/annoy_barbot.h"
#include "titanic/game/arb_background.h"
#include "titanic/game/arboretum_gate.h"
#include "titanic/game/auto_animate.h"
#include "titanic/game/bar_bell.h"
#include "titanic/game/bar_menu.h"
#include "titanic/game/bar_menu_button.h"
#include "titanic/game/belbot_get_light.h"
#include "titanic/npcs/bilge_succubus.h"
#include "titanic/game/bomb.h"
#include "titanic/game/bottom_of_well_monitor.h"
#include "titanic/game/bowl_unlocker.h"
#include "titanic/game/brain_slot.h"
#include "titanic/game/bridge_door.h"
#include "titanic/game/bridge_view.h"
#include "titanic/game/broken_pell_base.h"
#include "titanic/game/broken_pellerator.h"
#include "titanic/game/broken_pellerator_froz.h"
#include "titanic/game/cage.h"
#include "titanic/game/captains_wheel.h"
#include "titanic/game/cdrom.h"
#include "titanic/game/cdrom_computer.h"
#include "titanic/game/cdrom_tray.h"
#include "titanic/game/cell_point_button.h"
#include "titanic/game/chev_code.h"
#include "titanic/game/chev_panel.h"
#include "titanic/game/chicken_cooler.h"
#include "titanic/game/chicken_dispensor.h"
#include "titanic/game/close_broken_pel.h"
#include "titanic/game/computer.h"
#include "titanic/game/computer_screen.h"
#include "titanic/game/code_wheel.h"
#include "titanic/game/cookie.h"
#include "titanic/game/credits.h"
#include "titanic/game/credits_button.h"
#include "titanic/game/dead_area.h"
#include "titanic/game/desk_click_responder.h"
#include "titanic/game/doorbot_elevator_handler.h"
#include "titanic/game/doorbot_home_handler.h"
#include "titanic/game/ear_sweet_bowl.h"
#include "titanic/game/eject_phonograph_button.h"
#include "titanic/game/elevator_action_area.h"
#include "titanic/game/emma_control.h"
#include "titanic/game/empty_nut_bowl.h"
#include "titanic/game/end_credit_text.h"
#include "titanic/game/end_credits.h"
#include "titanic/game/end_explode_ship.h"
#include "titanic/game/end_game_credits.h"
#include "titanic/game/end_sequence_control.h"
#include "titanic/game/fan.h"
#include "titanic/game/fan_control.h"
#include "titanic/game/fan_decrease.h"
#include "titanic/game/fan_increase.h"
#include "titanic/game/fan_noises.h"
#include "titanic/game/floor_indicator.h"
#include "titanic/game/games_console.h"
#include "titanic/game/get_lift_eye2.h"
#include "titanic/game/glass_smasher.h"
#include "titanic/game/hammer_clip.h"
#include "titanic/game/hammer_dispensor.h"
#include "titanic/game/hammer_dispensor_button.h"
#include "titanic/game/head_slot.h"
#include "titanic/game/head_smash_event.h"
#include "titanic/game/head_smash_lever.h"
#include "titanic/game/head_spinner.h"
#include "titanic/game/idle_summoner.h"
#include "titanic/game/leave_sec_class_state.h"
#include "titanic/game/lemon_dispensor.h"
#include "titanic/game/light.h"
#include "titanic/game/light_switch.h"
#include "titanic/game/little_lift_button.h"
#include "titanic/game/long_stick_dispenser.h"
#include "titanic/game/missiveomat.h"
#include "titanic/game/missiveomat_button.h"
#include "titanic/game/movie_tester.h"
#include "titanic/game/musical_instrument.h"
#include "titanic/game/music_console_button.h"
#include "titanic/game/music_room_phonograph.h"
#include "titanic/game/music_room_stop_phonograph_button.h"
#include "titanic/game/music_system_lock.h"
#include "titanic/game/nav_helmet.h"
#include "titanic/game/nav_helmet_on.h"
#include "titanic/game/nav_helmet_off.h"
#include "titanic/game/navigation_computer.h"
#include "titanic/game/no_nut_bowl.h"
#include "titanic/game/nose_holder.h"
#include "titanic/game/null_port_hole.h"
#include "titanic/game/nut_replacer.h"
#include "titanic/game/pet_disabler.h"
#include "titanic/game/phonograph.h"
#include "titanic/game/phonograph_lid.h"
#include "titanic/game/place_holder_item.h"
#include "titanic/game/play_music_button.h"
#include "titanic/game/play_on_act.h"
#include "titanic/game/port_hole.h"
#include "titanic/game/record_phonograph_button.h"
#include "titanic/game/replacement_ear.h"
#include "titanic/game/reserved_table.h"
#include "titanic/game/restaurant_cylinder_holder.h"
#include "titanic/game/restaurant_phonograph.h"
#include "titanic/game/sauce_dispensor.h"
#include "titanic/game/search_point.h"
#include "titanic/game/season_background.h"
#include "titanic/game/season_barrel.h"
#include "titanic/game/seasonal_adjustment.h"
#include "titanic/game/service_elevator_window.h"
#include "titanic/game/ship_setting.h"
#include "titanic/game/ship_setting_button.h"
#include "titanic/game/show_cell_points.h"
#include "titanic/game/speech_dispensor.h"
#include "titanic/game/splash_animation.h"
#include "titanic/game/starling_puret.h"
#include "titanic/game/start_action.h"
#include "titanic/game/stop_phonograph_button.h"
#include "titanic/game/sub_glass.h"
#include "titanic/game/sub_wrapper.h"
#include "titanic/game/sweet_bowl.h"
#include "titanic/game/television.h"
#include "titanic/game/third_class_canal.h"
#include "titanic/game/throw_tv_down_well.h"
#include "titanic/game/titania_still_control.h"
#include "titanic/game/tow_parrot_nav.h"
#include "titanic/game/up_lighter.h"
#include "titanic/game/useless_lever.h"
#include "titanic/game/variable_list.h"
#include "titanic/game/volume_control.h"
#include "titanic/game/wheel_button.h"
#include "titanic/game/wheel_hotspot.h"
#include "titanic/game/wheel_spin.h"
#include "titanic/game/wheel_spin_horn.h"
#include "titanic/game/gondolier/gondolier_base.h"
#include "titanic/game/gondolier/gondolier_chest.h"
#include "titanic/game/gondolier/gondolier_face.h"
#include "titanic/game/gondolier/gondolier_mixer.h"
#include "titanic/game/gondolier/gondolier_slider.h"
#include "titanic/game/maitred/maitred_arm_holder.h"
#include "titanic/game/maitred/maitred_body.h"
#include "titanic/game/maitred/maitred_legs.h"
#include "titanic/game/maitred/maitred_prod_receptor.h"
#include "titanic/game/parrot/parrot_lobby_controller.h"
#include "titanic/game/parrot/parrot_lobby_link_updater.h"
#include "titanic/game/parrot/parrot_lobby_object.h"
#include "titanic/game/parrot/parrot_lobby_view_object.h"
#include "titanic/game/parrot/parrot_loser.h"
#include "titanic/game/parrot/parrot_nut_bowl_actor.h"
#include "titanic/game/parrot/parrot_nut_eater.h"
#include "titanic/game/parrot/parrot_perch_holder.h"
#include "titanic/npcs/parrot_succubus.h"
#include "titanic/game/parrot/parrot_trigger.h"
#include "titanic/game/parrot/player_meets_parrot.h"
#include "titanic/game/pet/pet.h"
#include "titanic/game/pet/pet_class1.h"
#include "titanic/game/pet/pet_class2.h"
#include "titanic/game/pet/pet_class3.h"
#include "titanic/game/pet/pet_lift.h"
#include "titanic/game/pet/pet_monitor.h"
#include "titanic/game/pet/pet_pellerator.h"
#include "titanic/game/pet/pet_position.h"
#include "titanic/game/pet/pet_sentinal.h"
#include "titanic/game/pet/pet_sounds.h"
#include "titanic/game/pet/pet_transition.h"
#include "titanic/game/pet/pet_transport.h"
#include "titanic/game/pickup/pick_up.h"
#include "titanic/game/pickup/pick_up_bar_glass.h"
#include "titanic/game/pickup/pick_up_hose.h"
#include "titanic/game/pickup/pick_up_lemon.h"
#include "titanic/game/pickup/pick_up_speech_centre.h"
#include "titanic/game/pickup/pick_up_vis_centre.h"
#include "titanic/game/placeholder/bar_shelf_vis_centre.h"
#include "titanic/game/placeholder/lemon_on_bar.h"
#include "titanic/game/placeholder/place_holder.h"
#include "titanic/game/placeholder/tv_on_bar.h"
#include "titanic/game/sgt/armchair.h"
#include "titanic/game/sgt/basin.h"
#include "titanic/game/sgt/bedfoot.h"
#include "titanic/game/sgt/bedhead.h"
#include "titanic/game/sgt/chest_of_drawers.h"
#include "titanic/game/sgt/desk.h"
#include "titanic/game/sgt/deskchair.h"
#include "titanic/game/sgt/drawer.h"
#include "titanic/game/sgt/sgt_doors.h"
#include "titanic/game/sgt/sgt_nav.h"
#include "titanic/game/sgt/sgt_navigation.h"
#include "titanic/game/sgt/sgt_restaurant_doors.h"
#include "titanic/game/sgt/sgt_state_control.h"
#include "titanic/game/sgt/sgt_state_room.h"
#include "titanic/game/sgt/sgt_tv.h"
#include "titanic/game/sgt/sgt_upper_doors_sound.h"
#include "titanic/game/sgt/toilet.h"
#include "titanic/game/sgt/vase.h"
#include "titanic/game/sgt/washstand.h"
#include "titanic/game/transport/gondolier.h"
#include "titanic/game/transport/lift.h"
#include "titanic/game/transport/lift_indicator.h"
#include "titanic/game/transport/pellerator.h"
#include "titanic/game/transport/service_elevator.h"
#include "titanic/game/transport/transport.h"
#include "titanic/gfx/act_button.h"
#include "titanic/gfx/changes_season_button.h"
#include "titanic/gfx/chev_left_off.h"
#include "titanic/gfx/chev_left_on.h"
#include "titanic/gfx/chev_right_off.h"
#include "titanic/gfx/chev_right_on.h"
#include "titanic/gfx/chev_send_rec_switch.h"
#include "titanic/gfx/edit_control.h"
#include "titanic/gfx/elevator_button.h"
#include "titanic/gfx/get_from_succ.h"
#include "titanic/gfx/helmet_on_off.h"
#include "titanic/gfx/home_photo.h"
#include "titanic/gfx/icon_nav_action.h"
#include "titanic/gfx/icon_nav_butt.h"
#include "titanic/gfx/icon_nav_down.h"
#include "titanic/gfx/icon_nav_image.h"
#include "titanic/gfx/icon_nav_left.h"
#include "titanic/gfx/icon_nav_receive.h"
#include "titanic/gfx/icon_nav_right.h"
#include "titanic/gfx/icon_nav_send.h"
#include "titanic/gfx/icon_nav_up.h"
#include "titanic/gfx/keybrd_butt.h"
#include "titanic/gfx/move_object_button.h"
#include "titanic/gfx/music_control.h"
#include "titanic/gfx/music_slider_pitch.h"
#include "titanic/gfx/music_slider_speed.h"
#include "titanic/gfx/music_switch.h"
#include "titanic/gfx/music_switch_inversion.h"
#include "titanic/gfx/music_switch_reverse.h"
#include "titanic/gfx/music_voice_mute.h"
#include "titanic/gfx/send_to_succ.h"
#include "titanic/gfx/sgt_selector.h"
#include "titanic/gfx/slider_button.h"
#include "titanic/gfx/small_chev_left_off.h"
#include "titanic/gfx/small_chev_left_on.h"
#include "titanic/gfx/small_chev_right_off.h"
#include "titanic/gfx/small_chev_right_on.h"
#include "titanic/gfx/status_change_button.h"
#include "titanic/gfx/st_button.h"
#include "titanic/gfx/toggle_button.h"
#include "titanic/gfx/text_down.h"
#include "titanic/gfx/text_skrew.h"
#include "titanic/gfx/text_up.h"
#include "titanic/gfx/toggle_switch.h"

#include "titanic/messages/messages.h"
#include "titanic/messages/auto_sound_event.h"
#include "titanic/messages/bilge_auto_sound_event.h"
#include "titanic/messages/bilge_dispensor_event.h"
#include "titanic/messages/door_auto_sound_event.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/messages/service_elevator_door.h"

#include "titanic/moves/call_pellerator.h"
#include "titanic/moves/enter_bomb_room.h"
#include "titanic/moves/enter_bridge.h"
#include "titanic/moves/enter_exit_first_class_state.h"
#include "titanic/moves/enter_exit_mini_lift.h"
#include "titanic/moves/enter_exit_sec_class_mini_lift.h"
#include "titanic/moves/enter_exit_view.h"
#include "titanic/moves/enter_sec_class_state.h"
#include "titanic/moves/exit_arboretum.h"
#include "titanic/moves/exit_bridge.h"
#include "titanic/moves/exit_lift.h"
#include "titanic/moves/exit_pellerator.h"
#include "titanic/moves/exit_state_room.h"
#include "titanic/moves/exit_tiania.h"
#include "titanic/moves/move_player_in_parrot_room.h"
#include "titanic/moves/move_player_to.h"
#include "titanic/moves/move_player_to_from.h"
#include "titanic/moves/multi_move.h"
#include "titanic/moves/pan_from_pel.h"
#include "titanic/moves/restaurant_pan_handler.h"
#include "titanic/moves/restricted_move.h"
#include "titanic/moves/scraliontis_table.h"
#include "titanic/moves/trip_down_canal.h"

#include "titanic/npcs/barbot.h"
#include "titanic/npcs/bellbot.h"
#include "titanic/npcs/callbot.h"
#include "titanic/npcs/deskbot.h"
#include "titanic/npcs/doorbot.h"
#include "titanic/npcs/liftbot.h"
#include "titanic/npcs/maitre_d.h"
#include "titanic/npcs/mobile.h"
#include "titanic/npcs/parrot.h"
#include "titanic/npcs/starlings.h"
#include "titanic/npcs/succubus.h"
#include "titanic/npcs/summon_bots.h"
#include "titanic/npcs/titania.h"
#include "titanic/npcs/true_talk_npc.h"

#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_drag_chev.h"
#include "titanic/pet_control/pet_graphic.h"
#include "titanic/pet_control/pet_graphic2.h"
#include "titanic/pet_control/pet_leaf.h"
#include "titanic/pet_control/pet_mode_off.h"
#include "titanic/pet_control/pet_mode_on.h"
#include "titanic/pet_control/pet_mode_panel.h"
#include "titanic/pet_control/pet_pannel1.h"
#include "titanic/pet_control/pet_pannel2.h"
#include "titanic/pet_control/pet_pannel3.h"
#include "titanic/pet_control/pet_show_translation.h"
#include "titanic/pet_control/pet_translation.h"

#include "titanic/sound/auto_music_player.h"
#include "titanic/sound/auto_music_player_base.h"
#include "titanic/sound/auto_sound_player.h"
#include "titanic/sound/auto_sound_player_adsr.h"
#include "titanic/sound/background_sound_maker.h"
#include "titanic/sound/bird_song.h"
#include "titanic/sound/dome_from_top_of_well.h"
#include "titanic/sound/gondolier_song.h"
#include "titanic/sound/enter_view_toggles_other_music.h"
#include "titanic/sound/music_player.h"
#include "titanic/sound/node_auto_sound_player.h"
#include "titanic/sound/restricted_auto_music_player.h"
#include "titanic/sound/room_auto_sound_player.h"
#include "titanic/sound/room_trigger_auto_music_player.h"
#include "titanic/sound/season_noises.h"
#include "titanic/sound/seasonal_music_player.h"
#include "titanic/sound/titania_speech.h"
#include "titanic/sound/trigger_auto_music_player.h"
#include "titanic/sound/view_auto_sound_player.h"
#include "titanic/sound/view_toggles_other_music.h"
#include "titanic/sound/water_lapping_sounds.h"

#include "titanic/star_control/star_control.h"
#include "titanic/support/time_event_info.h"

namespace Titanic {

CSaveableObject *ClassDef::create() {
	return new CSaveableObject();
}

/*------------------------------------------------------------------------*/

CSaveableObject::ClassListMap *CSaveableObject::_classList;
CSaveableObject::ClassDefList *CSaveableObject::_classDefs;

#define DEFFN(T) CSaveableObject *Function##T() { return new T(); } \
	ClassDef *T::_type
#define ADDFN(CHILD, PARENT) \
	CHILD::_type = new TypeTemplate<CHILD>(#CHILD, PARENT::_type); \
	_classDefs->push_back(CHILD::_type); \
	(*_classList)[#CHILD] = Function##CHILD

DEFFN(CArm);
DEFFN(CAuditoryCentre);
DEFFN(CBowlEar);
DEFFN(CBrain);
DEFFN(CBridgePiece);
DEFFN(CCarry);
DEFFN(CCarryParrot);
DEFFN(CCentralCore);
DEFFN(CChicken);
DEFFN(CCrushedTV);
DEFFN(CEar);
DEFFN(CEye);
DEFFN(CFeathers);
DEFFN(CFruit);
DEFFN(CGlass);
DEFFN(CHammer);
DEFFN(CHeadPiece);
DEFFN(CHose);
DEFFN(CHoseEnd);
DEFFN(CKey);
DEFFN(CLiftbotHead);
DEFFN(CLongStick);
DEFFN(CMagazine);
DEFFN(CMaitreDLeftArm);
DEFFN(CMaitreDRightArm);
DEFFN(CMouth);
DEFFN(CNapkin);
DEFFN(CNose);
DEFFN(CNote);
DEFFN(CParcel);
DEFFN(CPerch);
DEFFN(CPhonographCylinder);
DEFFN(CPhonographEar);
DEFFN(CPhotograph);
DEFFN(CPlugIn);
DEFFN(CSpeechCentre);
DEFFN(CSweets);
DEFFN(CVisionCentre);

DEFFN(CBackground);
DEFFN(CClickResponder);
DEFFN(CDontSaveFileItem);
DEFFN(CDropTarget);
DEFFN(CFileItem);
DEFFN(CFileListItem);
DEFFN(CGameObject);
DEFFN(CGameObjectDescItem);
DEFFN(CLinkItem);
DEFFN(ListItem);
DEFFN(CMailMan);
DEFFN(CMessageTarget);
DEFFN(CMovieClip);
DEFFN(CMovieRangeInfo);
DEFFN(CMultiDropTarget);
DEFFN(CNamedItem);
DEFFN(CNodeItem);
DEFFN(CProjectItem);
DEFFN(CResourceKey);
DEFFN(CRoomItem);
DEFFN(CSaveableObject);
DEFFN(CStaticImage);
DEFFN(CTurnOnObject);
DEFFN(CTurnOnPlaySound);
DEFFN(CTurnOnTurnOff);
DEFFN(CTreeItem);
DEFFN(CViewItem);

DEFFN(CAnnounce);
DEFFN(CAnnoyBarbot);
DEFFN(CArbBackground);
DEFFN(CArboretumGate);
DEFFN(CAutoAnimate);
DEFFN(CBarBell);
DEFFN(CBarMenu);
DEFFN(CBarMenuButton);
DEFFN(CBelbotGetLight);
DEFFN(CBilgeSuccUBus);
DEFFN(CBomb);
DEFFN(CBottomOfWellMonitor);
DEFFN(CBowlUnlocker);
DEFFN(CBrainSlot);
DEFFN(CBridgeDoor);
DEFFN(CBridgeView);
DEFFN(CBrokenPellBase);
DEFFN(CBrokenPellerator);
DEFFN(CBrokenPelleratorFroz);
DEFFN(CCage);
DEFFN(CCallPellerator);
DEFFN(CCaptainsWheel);
DEFFN(CCDROM);
DEFFN(CCDROMComputer);
DEFFN(CCDROMTray);
DEFFN(CCellPointButton);
DEFFN(CChevCode);
DEFFN(CChevPanel);
DEFFN(CChickenCooler);
DEFFN(CChickenDispensor);
DEFFN(CCloseBrokenPel);
DEFFN(CodeWheel);
DEFFN(CComputer);
DEFFN(CComputerScreen);
DEFFN(CCookie);
DEFFN(CCredits);
DEFFN(CCreditsButton);
DEFFN(CDeadArea);
DEFFN(CDeskClickResponder);
DEFFN(CDoorbotElevatorHandler);
DEFFN(CDoorbotHomeHandler);
DEFFN(CEarSweetBowl);
DEFFN(CEjectPhonographButton);
DEFFN(CElevatorActionArea);
DEFFN(CEmmaControl);
DEFFN(CEmptyNutBowl);
DEFFN(CEndCreditText);
DEFFN(CEndCredits);
DEFFN(CEndExplodeShip);
DEFFN(CEndGameCredits);
DEFFN(CEndSequenceControl);
DEFFN(CFan);
DEFFN(CFanControl);
DEFFN(CFanDecrease);
DEFFN(CFanIncrease);
DEFFN(CFanNoises);
DEFFN(CFloorIndicator);
DEFFN(CGamesConsole);
DEFFN(CGetLiftEye2);
DEFFN(CGlassSmasher);
DEFFN(CHammerClip);
DEFFN(CHammerDispensor);
DEFFN(CHammerDispensorButton);
DEFFN(CHeadSlot);
DEFFN(CHeadSmashEvent);
DEFFN(CHeadSmashLever);
DEFFN(CHeadSpinner);
DEFFN(CIdleSummoner);
DEFFN(CLeaveSecClassState);
DEFFN(CLemonDispensor);
DEFFN(CLight);
DEFFN(CLightSwitch);
DEFFN(CLittleLiftButton);
DEFFN(CLongStickDispenser);
DEFFN(CMissiveOMat);
DEFFN(CMissiveOMatButton);
DEFFN(CMovieTester);
DEFFN(CMusicalInstrument);
DEFFN(CMusicConsoleButton);
DEFFN(CMusicRoomPhonograph);
DEFFN(CMusicRoomStopPhonographButton);
DEFFN(CMusicSystemLock);
DEFFN(CNavHelmet);
DEFFN(CNavHelmetOn);
DEFFN(CNavHelmetOff);
DEFFN(CNavigationComputer);
DEFFN(CNoNutBowl);
DEFFN(CNoseHolder);
DEFFN(CNullPortHole);
DEFFN(CNutReplacer);
DEFFN(CPetDisabler);
DEFFN(CPhonograph);
DEFFN(CPhonographLid);
DEFFN(CPlaceHolderItem);
DEFFN(CPlayMusicButton);
DEFFN(CPlayOnAct);
DEFFN(CPortHole);
DEFFN(CRecordPhonographButton);
DEFFN(CReplacementEar);
DEFFN(CReservedTable);
DEFFN(CRestaurantCylinderHolder);
DEFFN(CRestaurantPhonograph);
DEFFN(CSauceDispensor);
DEFFN(CSearchPoint);
DEFFN(CSeasonBackground);
DEFFN(CSeasonBarrel);
DEFFN(CSeasonalAdjustment);
DEFFN(CServiceElevatorWindow);
DEFFN(CShipSetting);
DEFFN(CShipSettingButton);
DEFFN(CShowCellpoints);
DEFFN(CSpeechDispensor);
DEFFN(CSplashAnimation);
DEFFN(CStarlingPuret);
DEFFN(CStartAction);
DEFFN(CStopPhonographButton);
DEFFN(CSUBGlass);
DEFFN(CSUBWrapper);
DEFFN(CSweetBowl);
DEFFN(CTelevision);
DEFFN(CThirdClassCanal);
DEFFN(CThrowTVDownWell);
DEFFN(CTitaniaStillControl);
DEFFN(CTOWParrotNav);
DEFFN(CUpLighter);
DEFFN(CUselessLever);
DEFFN(CVariableListItem);
DEFFN(CVolumeControl);
DEFFN(CWheelButton);
DEFFN(CWheelHotSpot);
DEFFN(CWheelSpin);
DEFFN(CWheelSpinHorn);
DEFFN(CGondolierBase);
DEFFN(CGondolierChest);
DEFFN(CGondolierFace);
DEFFN(CGondolierMixer);
DEFFN(CGondolierSlider);
DEFFN(CMaitreDArmHolder);
DEFFN(CMaitreDBody);
DEFFN(CMaitreDLegs);
DEFFN(CMaitreDProdReceptor);
DEFFN(CParrotLobbyController);
DEFFN(CParrotLobbyLinkUpdater);
DEFFN(CParrotLobbyObject);
DEFFN(CParrotLobbyViewObject);
DEFFN(CParrotLoser);
DEFFN(CParrotNutBowlActor);
DEFFN(CParrotNutEater);
DEFFN(CParrotPerchHolder);
DEFFN(CParrotSuccUBus);
DEFFN(CParrotTrigger);
DEFFN(CPlayerMeetsParrot);
DEFFN(CPET);
DEFFN(CPETClass1);
DEFFN(CPETClass2);
DEFFN(CPETClass3);
DEFFN(CPetControl);
DEFFN(CPetDragChev);
DEFFN(CPetGraphic);
DEFFN(CPetGraphic2);
DEFFN(PETLeaf);
DEFFN(CPETLift);
DEFFN(CPETMonitor);
DEFFN(CPETPellerator);
DEFFN(CPETPosition);
DEFFN(CPETSentinal);
DEFFN(CPETSounds);
DEFFN(CPETTransition);
DEFFN(CPETTransport);
DEFFN(CPickUp);
DEFFN(CPickUpBarGlass);
DEFFN(CPickUpHose);
DEFFN(CPickUpLemon);
DEFFN(CPickUpSpeechCentre);
DEFFN(CPickUpVisCentre);
DEFFN(CBarShelfVisCentre);
DEFFN(CLemonOnBar);
DEFFN(CPlaceHolder);
DEFFN(CTVOnBar);
DEFFN(CArmchair);
DEFFN(CBasin);
DEFFN(CBedfoot);
DEFFN(CBedhead);
DEFFN(CChestOfDrawers);
DEFFN(CDesk);
DEFFN(CDeskchair);
DEFFN(CDrawer);
DEFFN(CSGTDoors);
DEFFN(SGTNav);
DEFFN(CSGTNavigation);
DEFFN(CSGTRestaurantDoors);
DEFFN(CSGTStateControl);
DEFFN(CSGTStateRoom);
DEFFN(CSGTTV);
DEFFN(CSGTUpperDoorsSound);
DEFFN(CToilet);
DEFFN(CVase);
DEFFN(CWashstand);

DEFFN(CGondolier);
DEFFN(CLift);
DEFFN(CLiftindicator);
DEFFN(CPellerator);
DEFFN(CServiceElevator);
DEFFN(CTransport);

DEFFN(CActButton);
DEFFN(CChangesSeasonButton);
DEFFN(CChevLeftOff);
DEFFN(CChevLeftOn);
DEFFN(CChevRightOff);
DEFFN(CChevRightOn);
DEFFN(CChevSendRecSwitch);
DEFFN(CEditControl);
DEFFN(CElevatorButton);
DEFFN(CGetFromSucc);
DEFFN(CHelmetOnOff);
DEFFN(CHomePhoto);
DEFFN(CIconNavAction);
DEFFN(CIconNavButt);
DEFFN(CIconNavDown);
DEFFN(CIconNavImage);
DEFFN(CIconNavLeft);
DEFFN(CIconNavReceive);
DEFFN(CIconNavRight);
DEFFN(CIconNavSend);
DEFFN(CIconNavUp);
DEFFN(CKeybrdButt);
DEFFN(CMoveObjectButton);
DEFFN(CMusicControl);
DEFFN(CMusicSlider);
DEFFN(CMusicSliderPitch);
DEFFN(CMusicSliderSpeed);
DEFFN(CMusicSwitch);
DEFFN(CMusicSwitchInversion);
DEFFN(CMusicSwitchReverse);
DEFFN(CMusicVoiceMute);
DEFFN(CPetModeOff);
DEFFN(CPetModeOn);
DEFFN(CPetModePanel);
DEFFN(CPetPannel1);
DEFFN(CPetPannel2);
DEFFN(CPetPannel3);
DEFFN(CPETShowTranslation);
DEFFN(CSendToSucc);
DEFFN(CSGTSelector);
DEFFN(CSliderButton);
DEFFN(CSmallChevLeftOff);
DEFFN(CSmallChevLeftOn);
DEFFN(CSmallChevRightOff);
DEFFN(CSmallChevRightOn);
DEFFN(CStatusChangeButton);
DEFFN(CSTButton);
DEFFN(CTextDown);
DEFFN(CTextSkrew);
DEFFN(CTextUp);
DEFFN(CToggleButton);
DEFFN(CToggleSwitch);

DEFFN(CActMsg);
DEFFN(CActivationmsg);
DEFFN(CAddHeadPieceMsg);
DEFFN(CAnimateMaitreDMsg);
DEFFN(CArboretumGateMsg);
DEFFN(CArmPickedUpFromTableMsg);
DEFFN(CAutoSoundEvent);
DEFFN(CBilgeAutoSoundEvent);
DEFFN(CBilgeDispensorEvent);
DEFFN(CBodyInBilgeRoomMsg);
DEFFN(CBowlStateChangeMsg);
DEFFN(CCarryObjectArrivedMsg);
DEFFN(CChangeMusicMsg);
DEFFN(CChangeSeasonMsg);
DEFFN(CCheckAllPossibleCodes);
DEFFN(CCheckChevCode);
DEFFN(CChildDragEndMsg);
DEFFN(CChildDragMoveMsg);
DEFFN(CChildDragStartMsg);
DEFFN(CClearChevPanelBits);
DEFFN(CCorrectMusicPlayedMsg);
DEFFN(CCreateMusicPlayerMsg);
DEFFN(CCylinderHolderReadyMsg);
DEFFN(CDeactivationMsg);
DEFFN(CDeliverCCarryMsg);
DEFFN(CDisableMaitreDProdReceptor);
DEFFN(CDismissBotMsg);
DEFFN(CDoffNavHelmet);
DEFFN(CDonNavHelmet);
DEFFN(CDoorAutoSoundEvent);
DEFFN(CDoorbotNeededInElevatorMsg);
DEFFN(CDoorbotNeededInHomeMsg);
DEFFN(CDropObjectMsg);
DEFFN(CDropZoneGotObjectMsg);
DEFFN(CDropZoneLostObjectMsg);
DEFFN(CEditControlMsg);
DEFFN(CEnterNodeMsg);
DEFFN(CEnterRoomMsg);
DEFFN(CEnterViewMsg);
DEFFN(CEjectCylinderMsg);
DEFFN(CErasePhonographCylinderMsg);
DEFFN(CFrameMsg);
DEFFN(CFreshenCookieMsg);
DEFFN(CGetChevClassBits);
DEFFN(CGetChevClassNum);
DEFFN(CGetChevCodeFromRoomNameMsg);
DEFFN(CGetChevFloorBits);
DEFFN(CGetChevFloorNum);
DEFFN(CGetChevLiftBits);
DEFFN(CGetChevLiftNum);
DEFFN(CGetChevRoomBits);
DEFFN(CGetChevRoomNum);
DEFFN(CHoseConnectedMsg);
DEFFN(CInitializeAnimMsg);
DEFFN(CIsEarBowlPuzzleDone);
DEFFN(CIsHookedOnMsg);
DEFFN(CIsParrotPresentMsg);
DEFFN(CKeyCharMsg);
DEFFN(CLeaveNodeMsg);
DEFFN(CLeaveRoomMsg);
DEFFN(CLeaveViewMsg);
DEFFN(CLemonFallsFromTreeMsg);
DEFFN(CLightsMsg);
DEFFN(CLoadSuccessMsg);
DEFFN(CLockPhonographMsg);
DEFFN(CMaitreDDefeatedMsg);
DEFFN(CMaitreDHappyMsg);
DEFFN(CMessage);
DEFFN(CMissiveOMatActionMsg);
DEFFN(CMouseMsg);
DEFFN(CMouseMoveMsg);
DEFFN(CMouseButtonMsg);
DEFFN(CMouseButtonDownMsg);
DEFFN(CMouseButtonUpMsg);
DEFFN(CMouseDoubleClickMsg);
DEFFN(CMouseDragMsg);
DEFFN(CMouseDragStartMsg);
DEFFN(CMouseDragMoveMsg);
DEFFN(CMouseDragEndMsg);
DEFFN(CMouseWheelMsg);
DEFFN(CMovementMsg);
DEFFN(CMoveToStartPosMsg);
DEFFN(CMovieEndMsg);
DEFFN(CMovieFrameMsg);
DEFFN(CMusicHasStartedMsg);
DEFFN(CMusicHasStoppedMsg);
DEFFN(CMusicSettingChangedMsg);
DEFFN(CNPCPlayAnimationMsg);
DEFFN(CNPCPlayIdleAnimationMsg);
DEFFN(CNPCPlayTalkingAnimationMsg);
DEFFN(CNPCQueueIdleAnimMsg);
DEFFN(CNutPuzzleMsg);
DEFFN(COnSummonBotMsg);
DEFFN(COpeningCreditsMsg);
DEFFN(CPETDeliverMsg);
DEFFN(CPETGainedObjectMsg);
DEFFN(CPETHelmetOnOffMsg);
DEFFN(CPETKeyboardOnOffMsg);
DEFFN(CPETLostObjectMsg);
DEFFN(CPETObjectSelectedMsg);
DEFFN(CPETObjectStateMsg);
DEFFN(CPETPhotoOnOffMsg);
DEFFN(CPETPlaySoundMsg);
DEFFN(CPETReceiveMsg);
DEFFN(CPETSetStarDestinationMsg);
DEFFN(CPETStarFieldLockMsg);
DEFFN(CPETStereoFieldOnOffMsg);
DEFFN(CPETTargetMsg);
DEFFN(CPETUpMsg);
DEFFN(CPETDownMsg);
DEFFN(CPETLeftMsg);
DEFFN(CPETRightMsg);
DEFFN(CPETActivateMsg);
DEFFN(CPanningAwayFromParrotMsg);
DEFFN(CParrotSpeakMsg);
DEFFN(CParrotTriesChickenMsg);
DEFFN(CPassOnDragStartMsg);
DEFFN(CPhonographPlayMsg);
DEFFN(CPhonographReadyToPlayMsg);
DEFFN(CPhonographRecordMsg);
DEFFN(CPhonographStopMsg);
DEFFN(CPlayRangeMsg);
DEFFN(CPlayerTriesRestaurantTableMsg);
DEFFN(CPreEnterNodeMsg);
DEFFN(CPreEnterRoomMsg);
DEFFN(CPreEnterViewMsg);
DEFFN(CPreSaveMsg);
DEFFN(CProdMaitreDMsg);
DEFFN(CPumpingMsg);
DEFFN(CPutBotBackInHisBoxMsg);
DEFFN(CPutParrotBackMsg);
DEFFN(CPuzzleSolvedMsg);
DEFFN(CQueryCylinderHolderMsg);
DEFFN(CQueryCylinderMsg);
DEFFN(CQueryCylinderNameMsg);
DEFFN(CQueryCylinderTypeMsg);
DEFFN(CQueryMusicControlSettingMsg);
DEFFN(CQueryPhonographState);
DEFFN(CRecordOntoCylinderMsg);
DEFFN(CRemoveFromGameMsg);
DEFFN(CReplaceBowlAndNutsMsg);
DEFFN(CRestaurantMusicChanged);
DEFFN(CSendCCarryMsg);
DEFFN(CSenseWorkingMsg);
DEFFN(CServiceElevatorDoor);
DEFFN(CServiceElevatorFloorChangeMsg);
DEFFN(CServiceElevatorFloorRequestMsg);
DEFFN(CServiceElevatorMsg);
DEFFN(CSetChevButtonImageMsg);
DEFFN(CSetChevClassBits);
DEFFN(CSetChevFloorBits);
DEFFN(CSetChevLiftBits);
DEFFN(CSetChevPanelBitMsg);
DEFFN(CSetChevPanelButtonsMsg);
DEFFN(CSetChevRoomBits);
DEFFN(CSetFrameMsg);
DEFFN(CSetMusicControlsMsg);
DEFFN(CSetVarMsg);
DEFFN(CSetVolumeMsg);
DEFFN(CShipSettingMsg);
DEFFN(CShowTextMsg);
DEFFN(CSignalObject);
DEFFN(CSpeechFallsFromTreeMsg);
DEFFN(CStartMusicMsg);
DEFFN(CStatusChangeMsg);
DEFFN(CStopMusicMsg);
DEFFN(CSubAcceptCCarryMsg);
DEFFN(CSubDeliverCCarryMsg);
DEFFN(CSubSendCCarryMsg);
DEFFN(CSUBTransition);
DEFFN(CSubTurnOffMsg);
DEFFN(CSubTurnOnMsg);
DEFFN(CSummonBotMsg);
DEFFN(CSummonBotQueryMsg);
DEFFN(CTakeHeadPieceMsg);
DEFFN(CTextInputMsg);
DEFFN(CTimeDilationMsg);
DEFFN(CTimeMsg);
DEFFN(CTimerMsg);
DEFFN(CTitleSequenceEndedMsg);
DEFFN(CTransitMsg);
DEFFN(CTranslateObjectMsg);
DEFFN(CTransportMsg);
DEFFN(CTriggerAutoMusicPlayerMsg);
DEFFN(CTriggerNPCEvent);
DEFFN(CTrueTalkGetAnimSetMsg);
DEFFN(CTrueTalkGetAssetDetailsMsg);
DEFFN(CTrueTalkGetStateValueMsg);
DEFFN(CTrueTalkNotifySpeechEndedMsg);
DEFFN(CTrueTalkNotifySpeechStartedMsg);
DEFFN(CTrueTalkQueueUpAnimSetMsg);
DEFFN(CTrueTalkSelfQueueAnimSetMsg);
DEFFN(CTrueTalkTriggerActionMsg);
DEFFN(CTurnOff);
DEFFN(CTurnOn);
DEFFN(CUse);
DEFFN(CUseWithCharMsg);
DEFFN(CUseWithOtherMsg);
DEFFN(CVirtualKeyCharMsg);
DEFFN(CVisibleMsg);
DEFFN(CCheckCodeWheelsMsg);

DEFFN(CEnterBombRoom);
DEFFN(CEnterBridge);
DEFFN(CEnterExitFirstClassState);
DEFFN(CEnterExitMiniLift);
DEFFN(CEnterExitSecClassMiniLift);
DEFFN(CEnterExitView);
DEFFN(CEnterSecClassState);
DEFFN(CExitArboretum);
DEFFN(CExitBridge);
DEFFN(CExitLift);
DEFFN(CExitPellerator);
DEFFN(CExitStateRoom);
DEFFN(CExitTiania);
DEFFN(CMovePlayerInParrotRoom);
DEFFN(CMovePlayerTo);
DEFFN(CMovePlayerToFrom);
DEFFN(CMultiMove);
DEFFN(CPanFromPel);
DEFFN(CRestaurantPanHandler);
DEFFN(CScraliontisTable);
DEFFN(CRestrictedMove);
DEFFN(CTripDownCanal);

DEFFN(CBarbot);
DEFFN(CBellBot);
DEFFN(CCallBot);
DEFFN(CCharacter);
DEFFN(CDeskbot);
DEFFN(CDoorbot);
DEFFN(CLiftBot);
DEFFN(CMaitreD);
DEFFN(CMobile);
DEFFN(CParrot);
DEFFN(CRobotController);
DEFFN(CStarlings);
DEFFN(CSummonBots);
DEFFN(CSuccUBus);
DEFFN(CTitania);
DEFFN(CTrueTalkNPC);
DEFFN(CAutoMusicPlayer);
DEFFN(CAutoMusicPlayerBase);
DEFFN(CAutoSoundPlayer);
DEFFN(CAutoSoundPlayerADSR);
DEFFN(CBackgroundSoundMaker);
DEFFN(CBirdSong);
DEFFN(CDomeFromTopOfWell);
DEFFN(CEnterViewTogglesOtherMusic);
DEFFN(CGondolierSong);
DEFFN(CMusicPlayer);
DEFFN(CNodeAutoSoundPlayer);
DEFFN(CRestrictedAutoMusicPlayer);
DEFFN(CRoomAutoSoundPlayer);
DEFFN(CRoomTriggerAutoMusicPlayer);
DEFFN(CSeasonNoises);
DEFFN(CSeasonalMusicPlayer);
DEFFN(CTitaniaSpeech);
DEFFN(CTriggerAutoMusicPlayer);
DEFFN(CViewAutoSoundPlayer);
DEFFN(CViewTogglesOtherMusic);
DEFFN(CWaterLappingSounds);
DEFFN(CStarControl);
DEFFN(CTimeEventInfo);

void CSaveableObject::initClassList() {
	_classDefs = new ClassDefList();
	_classList = new ClassListMap();

	CSaveableObject::_type = new TypeTemplate<CSaveableObject>("CSaveableObject", nullptr);
	_classDefs->push_back(CSaveableObject::_type);
	(*_classList)["CSaveableObject"] = FunctionCSaveableObject;

	// Setup the type definitions for each class. Note that these have to be
	// in order of hierarchy from ancestor class to descendent
	ADDFN(CMessage, CSaveableObject);
	ADDFN(CMessageTarget, CSaveableObject);
	ADDFN(CResourceKey, CSaveableObject);
	ADDFN(ListItem, CSaveableObject);
	ADDFN(CTreeItem, CMessageTarget);
	ADDFN(CFileItem, CTreeItem);
	ADDFN(CGameObjectDescItem, CTreeItem);
	ADDFN(CDontSaveFileItem, CFileItem);
	ADDFN(CProjectItem, CFileItem);
	ADDFN(CNamedItem, CTreeItem);
	ADDFN(CRoomItem, CNamedItem);
	ADDFN(CGameObject, CNamedItem);
	ADDFN(CLinkItem, CNamedItem);
	ADDFN(CNodeItem, CNamedItem);
	ADDFN(CPlaceHolderItem, CNamedItem);
	ADDFN(CViewItem, CNamedItem);
	ADDFN(CBackground, CGameObject);
	ADDFN(CClickResponder, CGameObject);
	ADDFN(CDropTarget, CGameObject);
	ADDFN(CFileListItem, ListItem);
	ADDFN(CMailMan, CGameObject);
	ADDFN(CMovieClip, ListItem);
	ADDFN(CMovieRangeInfo, ListItem);
	ADDFN(CMultiDropTarget, CDropTarget);
	ADDFN(CStartAction, CBackground);
	ADDFN(CEditControl, CGameObject);
	ADDFN(CToggleButton, CBackground);
	ADDFN(CToggleSwitch, CGameObject);
	ADDFN(CPlaceHolder, CGameObject);

	ADDFN(CCarry, CGameObject);
	ADDFN(CArm, CCarry);
	ADDFN(CBrain, CCarry);
	ADDFN(CBridgePiece, CCarry);
	ADDFN(CCarryParrot, CCarry);
	ADDFN(CChicken, CCarry);
	ADDFN(CCrushedTV, CCarry);
	ADDFN(CFeathers, CCarry);
	ADDFN(CFruit, CCarry);
	ADDFN(CGlass, CCarry);
	ADDFN(CHammer, CCarry);
	ADDFN(CHeadPiece, CCarry);
	ADDFN(CHose, CCarry);
	ADDFN(CKey, CCarry);
	ADDFN(CLiftbotHead, CCarry);
	ADDFN(CLongStick, CCarry);
	ADDFN(CMagazine, CCarry);
	ADDFN(CNapkin, CCarry);
	ADDFN(CNote, CCarry);
	ADDFN(CParcel, CCarry);
	ADDFN(CPhonographCylinder, CCarry);
	ADDFN(CPhotograph, CCarry);
	ADDFN(CPlugIn, CCarry);
	ADDFN(CSweets, CCarry);
	ADDFN(CMaitreDLeftArm, CArm);
	ADDFN(CMaitreDRightArm, CArm);
	ADDFN(CCentralCore, CBrain);
	ADDFN(CSpeechCentre, CBrain);
	ADDFN(CVisionCentre, CBrain);
	ADDFN(CAuditoryCentre, CBrain);
	ADDFN(CPerch, CCentralCore);
	ADDFN(CEar, CHeadPiece);
	ADDFN(CBowlEar, CEar);
	ADDFN(CPhonographEar, CEar);
	ADDFN(CEye, CHeadPiece);
	ADDFN(CMouth, CHeadPiece);
	ADDFN(CNose, CHeadPiece);
	ADDFN(CHoseEnd, CHose);

	ADDFN(CStaticImage, CGameObject);
	ADDFN(CTurnOnObject, CBackground);
	ADDFN(CTurnOnPlaySound, CTurnOnObject);
	ADDFN(CTurnOnTurnOff, CBackground);
	ADDFN(CAnnounce, CGameObject);
	ADDFN(CAnnoyBarbot, CGameObject);
	ADDFN(CArbBackground, CBackground);
	ADDFN(CArboretumGate, CBackground);
	ADDFN(CAutoAnimate, CBackground);
	ADDFN(CBarBell, CGameObject);
	ADDFN(CBarMenu, CGameObject);
	ADDFN(CBarMenuButton, CGameObject);
	ADDFN(CBelbotGetLight, CGameObject);
	ADDFN(CBomb, CBackground);
	ADDFN(CBottomOfWellMonitor, CGameObject);
	ADDFN(CBowlUnlocker, CGameObject);
	ADDFN(CBrainSlot, CGameObject);
	ADDFN(CBridgeDoor, CGameObject);
	ADDFN(CBridgeView, CBackground);
	ADDFN(CBrokenPellBase, CBackground);
	ADDFN(CBrokenPellerator, CBrokenPellBase);
	ADDFN(CBrokenPelleratorFroz, CBrokenPellBase);
	ADDFN(CCage, CBackground);
	ADDFN(CCallPellerator, CGameObject);
	ADDFN(CCaptainsWheel, CBackground);
	ADDFN(CCDROM, CGameObject);
	ADDFN(CCDROMComputer, CGameObject);
	ADDFN(CCDROMTray, CGameObject);
	ADDFN(CCellPointButton, CBackground);
	ADDFN(CChevCode, CGameObject);
	ADDFN(CChevPanel, CGameObject);
	ADDFN(CChickenCooler, CGameObject);
	ADDFN(CChickenDispensor, CBackground);
	ADDFN(CodeWheel, CBomb);
	ADDFN(CCloseBrokenPel, CBackground);
	ADDFN(CComputer, CBackground);
	ADDFN(CComputerScreen, CGameObject);
	ADDFN(CCookie, CGameObject);
	ADDFN(CCredits, CGameObject);
	ADDFN(CCreditsButton, CBackground);
	ADDFN(CDeadArea, CGameObject);
	ADDFN(CDeskClickResponder, CClickResponder);
	ADDFN(CDoorbotElevatorHandler, CGameObject);
	ADDFN(CDoorbotHomeHandler, CGameObject);
	ADDFN(CDropTarget, CGameObject);
	ADDFN(CElevatorActionArea, CGameObject);
	ADDFN(CEmmaControl, CBackground);
	ADDFN(CEmptyNutBowl, CGameObject);
	ADDFN(CEndCreditText, CGameObject);
	ADDFN(CEndCredits, CGameObject);
	ADDFN(CEndExplodeShip, CGameObject);
	ADDFN(CEndGameCredits, CGameObject);
	ADDFN(CEndSequenceControl, CGameObject);
	ADDFN(CFan, CGameObject);
	ADDFN(CFanControl, CGameObject);
	ADDFN(CFanDecrease, CGameObject);
	ADDFN(CFanIncrease, CGameObject);
	ADDFN(CFanNoises, CGameObject);
	ADDFN(CFloorIndicator, CGameObject);
	ADDFN(CGamesConsole, CBackground);
	ADDFN(CGetLiftEye2, CGameObject);
	ADDFN(CGlassSmasher, CGameObject);
	ADDFN(CHammerClip, CGameObject);
	ADDFN(CHammerDispensor, CBackground);
	ADDFN(CHammerDispensorButton, CStartAction);
	ADDFN(CHeadSlot, CGameObject);
	ADDFN(CHeadSmashEvent, CBackground);
	ADDFN(CHeadSmashLever, CBackground);
	ADDFN(CHeadSpinner, CGameObject);
	ADDFN(CIdleSummoner, CGameObject);
	ADDFN(CLeaveSecClassState, CGameObject);
	ADDFN(CLemonDispensor, CBackground);
	ADDFN(CLight, CBackground);
	ADDFN(CLightSwitch, CBackground);
	ADDFN(CLittleLiftButton, CBackground);
	ADDFN(CLongStickDispenser, CGameObject);
	ADDFN(CMissiveOMat, CGameObject);
	ADDFN(CMissiveOMatButton, CEditControl);
	ADDFN(CMovieTester, CGameObject);
	ADDFN(CMusicalInstrument, CBackground);
	ADDFN(CMusicSystemLock, CDropTarget);
	ADDFN(CNavHelmet, CGameObject);
	ADDFN(CNavHelmetOn, CGameObject);
	ADDFN(CNavHelmetOff, CGameObject);
	ADDFN(CNavigationComputer, CGameObject);
	ADDFN(CNoNutBowl, CBackground);
	ADDFN(CNoseHolder, CDropTarget);
	ADDFN(CNullPortHole, CClickResponder);
	ADDFN(CNutReplacer, CGameObject);
	ADDFN(CPetDisabler, CGameObject);
	ADDFN(CPhonographLid, CGameObject);
	ADDFN(CPlayMusicButton, CBackground);
	ADDFN(CPlayOnAct, CBackground);
	ADDFN(CPortHole, CGameObject);
	ADDFN(CRecordPhonographButton, CBackground);
	ADDFN(CReplacementEar, CBackground);
	ADDFN(CReservedTable, CGameObject);
	ADDFN(CRestaurantCylinderHolder, CDropTarget);
	ADDFN(CSauceDispensor, CBackground);
	ADDFN(CSearchPoint, CGameObject);
	ADDFN(CSeasonBackground, CBackground);
	ADDFN(CSeasonBarrel, CBackground);
	ADDFN(CSeasonalAdjustment, CBackground);
	ADDFN(CServiceElevatorWindow, CBackground);
	ADDFN(CShipSetting, CBackground);
	ADDFN(CShipSettingButton, CGameObject);
	ADDFN(CShowCellpoints, CGameObject);
	ADDFN(CSpeechDispensor, CBackground);
	ADDFN(CSplashAnimation, CGameObject);
	ADDFN(CStarlingPuret, CGameObject);
	ADDFN(CStopPhonographButton, CBackground);
	ADDFN(CSUBGlass, CGameObject);
	ADDFN(CSUBWrapper, CGameObject);
	ADDFN(CSweetBowl, CGameObject);
	ADDFN(CEarSweetBowl, CSweetBowl);
	ADDFN(CTelevision, CBackground);
	ADDFN(CThirdClassCanal, CBackground);
	ADDFN(CThrowTVDownWell, CGameObject);
	ADDFN(CTitaniaStillControl, CGameObject);
	ADDFN(CTOWParrotNav, CGameObject);
	ADDFN(CUpLighter, CDropTarget);
	ADDFN(CUselessLever, CToggleButton);
	ADDFN(CVariableListItem, ListItem);
	ADDFN(CVolumeControl, CGameObject);
	ADDFN(CWheelButton, CBackground);
	ADDFN(CWheelHotSpot, CBackground);
	ADDFN(CWheelSpin, CBackground);
	ADDFN(CWheelSpinHorn, CWheelSpin);
	ADDFN(CGondolierBase, CGameObject);
	ADDFN(CGondolierChest, CGondolierBase);
	ADDFN(CGondolierFace, CGondolierBase);
	ADDFN(CGondolierMixer, CGondolierBase);
	ADDFN(CGondolierSlider, CGondolierBase);
	ADDFN(CMaitreDArmHolder, CDropTarget);
	ADDFN(CMaitreDProdReceptor, CGameObject);
	ADDFN(CMaitreDBody, CMaitreDProdReceptor);
	ADDFN(CMaitreDLegs, CMaitreDProdReceptor);
	ADDFN(CParrotLobbyObject, CGameObject);
	ADDFN(CParrotLobbyController, CParrotLobbyObject);
	ADDFN(CParrotLobbyLinkUpdater, CParrotLobbyObject);
	ADDFN(CParrotLobbyViewObject, CParrotLobbyObject);
	ADDFN(CParrotLoser, CGameObject);
	ADDFN(CParrotNutBowlActor, CGameObject);
	ADDFN(CParrotNutEater, CGameObject);
	ADDFN(CParrotPerchHolder, CMultiDropTarget);
	ADDFN(CParrotTrigger, CGameObject);
	ADDFN(CPlayerMeetsParrot, CGameObject);
	ADDFN(CPET, CGameObject);
	ADDFN(CPETClass1, CGameObject);
	ADDFN(CPETClass2, CGameObject);
	ADDFN(CPETClass3, CGameObject);
	ADDFN(CPETMonitor, CGameObject);
	ADDFN(CPETPosition, CGameObject);
	ADDFN(CPETSentinal, CGameObject);
	ADDFN(CPETSounds, CGameObject);
	ADDFN(CPETTransition, CGameObject);
	ADDFN(CPETTransport, CGameObject);
	ADDFN(CPETPellerator, CPETTransport);
	ADDFN(CPETLift, CPETTransport);
	ADDFN(CPickUp, CGameObject);
	ADDFN(CPickUpBarGlass, CPickUp);
	ADDFN(CPickUpHose, CPickUp);
	ADDFN(CPickUpLemon, CPickUp);
	ADDFN(CPickUpSpeechCentre, CPickUp);
	ADDFN(CPickUpVisCentre, CPickUp);
	ADDFN(CBarShelfVisCentre, CPlaceHolder);
	ADDFN(CLemonOnBar, CPlaceHolder);
	ADDFN(CTVOnBar, CPlaceHolder);
	ADDFN(CSGTStateRoom, CBackground);
	ADDFN(CArmchair, CSGTStateRoom);
	ADDFN(CBasin, CSGTStateRoom);
	ADDFN(CBedfoot, CSGTStateRoom);
	ADDFN(CBedhead, CSGTStateRoom);
	ADDFN(CChestOfDrawers, CSGTStateRoom);
	ADDFN(CDesk, CSGTStateRoom);
	ADDFN(CDeskchair, CSGTStateRoom);
	ADDFN(CDrawer, CSGTStateRoom);
	ADDFN(CSGTDoors, CGameObject);
	ADDFN(SGTNav, CSGTStateRoom);
	ADDFN(CSGTNavigation, CGameObject);
	ADDFN(CSGTRestaurantDoors, CGameObject);
	ADDFN(CSGTStateControl, CBackground);
	ADDFN(CSGTTV, CSGTStateRoom);
	ADDFN(CSGTUpperDoorsSound, CClickResponder);
	ADDFN(CToilet, CSGTStateRoom);
	ADDFN(CVase, CSGTStateRoom);
	ADDFN(CWashstand, CSGTStateRoom);

	ADDFN(CPetGraphic, CGameObject);
	ADDFN(CPetGraphic2, CGameObject);
	ADDFN(CSTButton, CBackground);
	ADDFN(CActButton, CSTButton);
	ADDFN(CChangesSeasonButton, CSTButton);
	ADDFN(CElevatorButton, CSTButton);
	ADDFN(CIconNavButt, CPetGraphic);
	ADDFN(CIconNavImage, CPetGraphic);
	ADDFN(CIconNavReceive, CPetGraphic);
	ADDFN(CIconNavSend, CPetGraphic);
	ADDFN(CChevLeftOff, CToggleSwitch);
	ADDFN(CChevLeftOn, CToggleSwitch);
	ADDFN(CChevRightOff, CToggleSwitch);
	ADDFN(CChevRightOn, CToggleSwitch);
	ADDFN(CChevSendRecSwitch, CToggleSwitch);
	ADDFN(CGetFromSucc, CToggleSwitch);
	ADDFN(CHelmetOnOff, CToggleSwitch);
	ADDFN(CHomePhoto, CToggleSwitch);
	ADDFN(CIconNavAction, CToggleSwitch);
	ADDFN(CIconNavDown, CToggleSwitch);
	ADDFN(CIconNavLeft, CToggleSwitch);
	ADDFN(CIconNavRight, CToggleSwitch);
	ADDFN(CIconNavUp, CToggleSwitch);
	ADDFN(CKeybrdButt, CToggleSwitch);
	ADDFN(CMoveObjectButton, CSTButton);
	ADDFN(CMusicControl, CBackground);
	ADDFN(CMusicSlider, CMusicControl);
	ADDFN(CMusicSliderPitch, CMusicSlider);
	ADDFN(CMusicSliderSpeed, CMusicSlider);
	ADDFN(CMusicSwitch, CMusicControl);
	ADDFN(CMusicSwitchInversion, CMusicSwitch);
	ADDFN(CMusicSwitchReverse, CMusicSwitch);
	ADDFN(CMusicVoiceMute, CMusicControl);
	ADDFN(CPetControl, CGameObject);
	ADDFN(CPetDragChev, CPetGraphic2);
	ADDFN(PETLeaf, CGameObject);
	ADDFN(CPetModeOff, CToggleSwitch);
	ADDFN(CPetModeOn, CToggleSwitch);
	ADDFN(CPetModePanel, CToggleSwitch);
	ADDFN(CPetPannel1, CPetGraphic);
	ADDFN(CPetPannel2, CPetGraphic);
	ADDFN(CPetPannel3, CPetGraphic);
	ADDFN(CPETShowTranslation, CGameObject);
	ADDFN(CSendToSucc, CToggleSwitch);
	ADDFN(CSGTSelector, CPetGraphic);
	ADDFN(CSliderButton, CSTButton);
	ADDFN(CSmallChevLeftOff, CToggleSwitch);
	ADDFN(CSmallChevLeftOn, CToggleSwitch);
	ADDFN(CSmallChevRightOff, CToggleSwitch);
	ADDFN(CSmallChevRightOn, CToggleSwitch);
	ADDFN(CStatusChangeButton, CSTButton);
	ADDFN(CTextDown, CPetGraphic);
	ADDFN(CTextSkrew, CPetGraphic);
	ADDFN(CTextUp, CPetGraphic);

	ADDFN(CActMsg, CMessage);
	ADDFN(CActivationmsg, CMessage);
	ADDFN(CAddHeadPieceMsg, CMessage);
	ADDFN(CAnimateMaitreDMsg, CMessage);
	ADDFN(CArboretumGateMsg, CMessage);
	ADDFN(CArmPickedUpFromTableMsg, CMessage);
	ADDFN(CAutoSoundEvent, CGameObject);
	ADDFN(CBilgeAutoSoundEvent, CAutoSoundEvent);
	ADDFN(CBilgeDispensorEvent, CAutoSoundEvent);
	ADDFN(CBodyInBilgeRoomMsg, CMessage);
	ADDFN(CBowlStateChangeMsg, CMessage);
	ADDFN(CCarryObjectArrivedMsg, CMessage);
	ADDFN(CChangeMusicMsg, CMessage);
	ADDFN(CChangeSeasonMsg, CMessage);
	ADDFN(CCheckAllPossibleCodes, CMessage);
	ADDFN(CCheckChevCode, CMessage);
	ADDFN(CChildDragEndMsg, CMessage);
	ADDFN(CChildDragMoveMsg, CMessage);
	ADDFN(CChildDragStartMsg, CMessage);
	ADDFN(CClearChevPanelBits, CMessage);
	ADDFN(CCorrectMusicPlayedMsg, CMessage);
	ADDFN(CCreateMusicPlayerMsg, CMessage);
	ADDFN(CCylinderHolderReadyMsg, CMessage);
	ADDFN(CDeactivationMsg, CMessage);
	ADDFN(CDeliverCCarryMsg, CMessage);
	ADDFN(CDisableMaitreDProdReceptor, CMessage);
	ADDFN(CDismissBotMsg, CMessage);
	ADDFN(CDoffNavHelmet, CMessage);
	ADDFN(CDonNavHelmet, CMessage);
	ADDFN(CDoorAutoSoundEvent, CAutoSoundEvent);
	ADDFN(CDoorbotNeededInElevatorMsg, CMessage);
	ADDFN(CDoorbotNeededInHomeMsg, CMessage);
	ADDFN(CDropObjectMsg, CMessage);
	ADDFN(CDropZoneGotObjectMsg, CMessage);
	ADDFN(CDropZoneLostObjectMsg, CMessage);
	ADDFN(CEditControlMsg, CMessage);
	ADDFN(CEnterNodeMsg, CMessage);
	ADDFN(CEnterRoomMsg, CMessage);
	ADDFN(CEnterViewMsg, CMessage);
	ADDFN(CEjectCylinderMsg, CMessage);
	ADDFN(CErasePhonographCylinderMsg, CMessage);
	ADDFN(CFrameMsg, CMessage);
	ADDFN(CFreshenCookieMsg, CMessage);
	ADDFN(CGetChevClassBits, CMessage);
	ADDFN(CGetChevClassNum, CMessage);
	ADDFN(CGetChevCodeFromRoomNameMsg, CMessage);
	ADDFN(CGetChevFloorBits, CMessage);
	ADDFN(CGetChevFloorNum, CMessage);
	ADDFN(CGetChevLiftBits, CMessage);
	ADDFN(CGetChevLiftNum, CMessage);
	ADDFN(CGetChevRoomBits, CMessage);
	ADDFN(CGetChevRoomNum, CMessage);
	ADDFN(CHoseConnectedMsg, CMessage);
	ADDFN(CInitializeAnimMsg, CMessage);
	ADDFN(CIsEarBowlPuzzleDone, CMessage);
	ADDFN(CIsHookedOnMsg, CMessage);
	ADDFN(CIsParrotPresentMsg, CMessage);
	ADDFN(CKeyCharMsg, CMessage);
	ADDFN(CLeaveNodeMsg, CMessage);
	ADDFN(CLeaveRoomMsg, CMessage);
	ADDFN(CLeaveViewMsg, CMessage);
	ADDFN(CLemonFallsFromTreeMsg, CMessage);
	ADDFN(CLightsMsg, CMessage);
	ADDFN(CLoadSuccessMsg, CMessage);
	ADDFN(CLockPhonographMsg, CMessage);
	ADDFN(CMaitreDDefeatedMsg, CMessage);
	ADDFN(CMaitreDHappyMsg, CMessage);
	ADDFN(CMissiveOMatActionMsg, CMessage);
	ADDFN(CMouseMsg, CMessage);
	ADDFN(CMouseMoveMsg, CMouseMsg);
	ADDFN(CMouseButtonMsg, CMouseMsg);
	ADDFN(CMouseButtonDownMsg, CMouseButtonMsg);
	ADDFN(CMouseButtonUpMsg, CMouseButtonMsg);
	ADDFN(CMouseDoubleClickMsg, CMouseButtonMsg);
	ADDFN(CMouseDragMsg, CMouseMsg);
	ADDFN(CMouseDragStartMsg, CMouseDragMsg);
	ADDFN(CMouseDragMoveMsg, CMouseDragMsg);
	ADDFN(CMouseDragEndMsg, CMouseDragMsg);
	ADDFN(CMouseWheelMsg, CMouseMsg);
	ADDFN(CMovementMsg, CMessage);
	ADDFN(CMoveToStartPosMsg, CMessage);
	ADDFN(CMovieEndMsg, CMessage);
	ADDFN(CMovieFrameMsg, CMessage);
	ADDFN(CMusicHasStartedMsg, CMessage);
	ADDFN(CMusicHasStoppedMsg, CMessage);
	ADDFN(CMusicSettingChangedMsg, CMessage);
	ADDFN(CNPCPlayAnimationMsg, CMessage);
	ADDFN(CNPCPlayIdleAnimationMsg, CMessage);
	ADDFN(CNPCPlayTalkingAnimationMsg, CMessage);
	ADDFN(CNPCQueueIdleAnimMsg, CMessage);
	ADDFN(CNutPuzzleMsg, CMessage);
	ADDFN(COnSummonBotMsg, CMessage);
	ADDFN(COpeningCreditsMsg, CMessage);
	ADDFN(CPETDeliverMsg, CMessage);
	ADDFN(CPETGainedObjectMsg, CMessage);
	ADDFN(CPETHelmetOnOffMsg, CMessage);
	ADDFN(CPETKeyboardOnOffMsg, CMessage);
	ADDFN(CPETLostObjectMsg, CMessage);
	ADDFN(CPETObjectSelectedMsg, CMessage);
	ADDFN(CPETObjectStateMsg, CMessage);
	ADDFN(CPETPhotoOnOffMsg, CMessage);
	ADDFN(CPETPlaySoundMsg, CMessage);
	ADDFN(CPETReceiveMsg, CMessage);
	ADDFN(CPETSetStarDestinationMsg, CMessage);
	ADDFN(CPETStarFieldLockMsg, CMessage);
	ADDFN(CPETStereoFieldOnOffMsg, CMessage);
	ADDFN(CPETTargetMsg, CMessage);
	ADDFN(CPETUpMsg, CPETTargetMsg);
	ADDFN(CPETDownMsg, CPETTargetMsg);
	ADDFN(CPETLeftMsg, CPETTargetMsg);
	ADDFN(CPETRightMsg, CPETTargetMsg);
	ADDFN(CPETActivateMsg, CPETTargetMsg);
	ADDFN(CPanningAwayFromParrotMsg, CMessage);
	ADDFN(CParrotSpeakMsg, CMessage);
	ADDFN(CParrotTriesChickenMsg, CMessage);
	ADDFN(CPassOnDragStartMsg, CMessage);
	ADDFN(CPhonographPlayMsg, CMessage);
	ADDFN(CPhonographReadyToPlayMsg, CMessage);
	ADDFN(CPhonographRecordMsg, CMessage);
	ADDFN(CPhonographStopMsg, CMessage);
	ADDFN(CPlayRangeMsg, CMessage);
	ADDFN(CPlayerTriesRestaurantTableMsg, CMessage);
	ADDFN(CEnterNodeMsg, CMessage);
	ADDFN(CEnterRoomMsg, CMessage);
	ADDFN(CEnterViewMsg, CMessage);
	ADDFN(CPreEnterNodeMsg, CMessage);
	ADDFN(CPreEnterRoomMsg, CMessage);
	ADDFN(CPreEnterViewMsg, CMessage);
	ADDFN(CPreSaveMsg, CMessage);
	ADDFN(CProdMaitreDMsg, CMessage);
	ADDFN(CPumpingMsg, CMessage);
	ADDFN(CPutBotBackInHisBoxMsg, CMessage);
	ADDFN(CPutParrotBackMsg, CMessage);
	ADDFN(CPuzzleSolvedMsg, CMessage);
	ADDFN(CQueryCylinderHolderMsg, CMessage);
	ADDFN(CQueryCylinderMsg, CMessage);
	ADDFN(CQueryCylinderNameMsg, CMessage);
	ADDFN(CQueryCylinderTypeMsg, CMessage);
	ADDFN(CQueryMusicControlSettingMsg, CMessage);
	ADDFN(CQueryPhonographState, CMessage);
	ADDFN(CRecordOntoCylinderMsg, CMessage);
	ADDFN(CRemoveFromGameMsg, CMessage);
	ADDFN(CReplaceBowlAndNutsMsg, CMessage);
	ADDFN(CRestaurantMusicChanged, CMessage);
	ADDFN(CSendCCarryMsg, CMessage);
	ADDFN(CSenseWorkingMsg, CMessage);
	ADDFN(CServiceElevatorDoor, CMessage);
	ADDFN(CServiceElevatorFloorChangeMsg, CMessage);
	ADDFN(CServiceElevatorFloorRequestMsg, CMessage);
	ADDFN(CServiceElevatorMsg, CMessage);
	ADDFN(CSetChevButtonImageMsg, CMessage);
	ADDFN(CSetChevClassBits, CMessage);
	ADDFN(CSetChevFloorBits, CMessage);
	ADDFN(CSetChevLiftBits, CMessage);
	ADDFN(CSetChevPanelBitMsg, CMessage);
	ADDFN(CSetChevPanelButtonsMsg, CMessage);
	ADDFN(CSetChevRoomBits, CMessage);
	ADDFN(CSetFrameMsg, CMessage);
	ADDFN(CSetMusicControlsMsg, CMessage);
	ADDFN(CSetVarMsg, CMessage);
	ADDFN(CSetVolumeMsg, CMessage);
	ADDFN(CShipSettingMsg, CMessage);
	ADDFN(CShowTextMsg, CMessage);
	ADDFN(CSignalObject, CMessage);
	ADDFN(CSpeechFallsFromTreeMsg, CMessage);
	ADDFN(CStartMusicMsg, CMessage);
	ADDFN(CStatusChangeMsg, CMessage);
	ADDFN(CStopMusicMsg, CMessage);
	ADDFN(CSubAcceptCCarryMsg, CMessage);
	ADDFN(CSubDeliverCCarryMsg, CMessage);
	ADDFN(CSubSendCCarryMsg, CMessage);
	ADDFN(CSUBTransition, CMessage);
	ADDFN(CSubTurnOffMsg, CMessage);
	ADDFN(CSubTurnOnMsg, CMessage);
	ADDFN(CSummonBotMsg, CMessage);
	ADDFN(CSummonBotQueryMsg, CMessage);
	ADDFN(CTakeHeadPieceMsg, CMessage);
	ADDFN(CTextInputMsg, CMessage);
	ADDFN(CTimeDilationMsg, CMessage);
	ADDFN(CTimeMsg, CMessage);
	ADDFN(CTimerMsg, CTimeMsg);
	ADDFN(CTitleSequenceEndedMsg, CMessage);
	ADDFN(CTransitMsg, CMessage);
	ADDFN(CTranslateObjectMsg, CMessage);
	ADDFN(CTransportMsg, CMessage);
	ADDFN(CTriggerAutoMusicPlayerMsg, CMessage);
	ADDFN(CTriggerNPCEvent, CMessage);
	ADDFN(CTrueTalkGetAnimSetMsg, CMessage);
	ADDFN(CTrueTalkGetAssetDetailsMsg, CMessage);
	ADDFN(CTrueTalkGetStateValueMsg, CMessage);
	ADDFN(CTrueTalkNotifySpeechEndedMsg, CMessage);
	ADDFN(CTrueTalkNotifySpeechStartedMsg, CMessage);
	ADDFN(CTrueTalkQueueUpAnimSetMsg, CMessage);
	ADDFN(CTrueTalkSelfQueueAnimSetMsg, CMessage);
	ADDFN(CTrueTalkTriggerActionMsg, CMessage);
	ADDFN(CTurnOff, CMessage);
	ADDFN(CTurnOn, CMessage);
	ADDFN(CUse, CMessage);
	ADDFN(CUseWithCharMsg, CMessage);
	ADDFN(CUseWithOtherMsg, CMessage);
	ADDFN(CVirtualKeyCharMsg, CMessage);
	ADDFN(CVisibleMsg, CMessage);
	ADDFN(CCheckCodeWheelsMsg, CMessage);

	ADDFN(CMovePlayerTo, CGameObject);
	ADDFN(CMovePlayerToFrom, CGameObject);
	ADDFN(CEnterBombRoom, CMovePlayerTo);
	ADDFN(CExitArboretum, CMovePlayerTo);
	ADDFN(CExitBridge, CMovePlayerTo);
	ADDFN(CExitStateRoom, CMovePlayerTo);
	ADDFN(CMovePlayerInParrotRoom, CMovePlayerTo);
	ADDFN(CExitTiania, CMovePlayerTo);
	ADDFN(CMultiMove, CMovePlayerTo);
	ADDFN(CRestaurantPanHandler, CMovePlayerTo);
	ADDFN(CPanFromPel, CMovePlayerTo);
	ADDFN(CRestrictedMove, CMovePlayerTo);
	ADDFN(CTripDownCanal, CMovePlayerTo);
	ADDFN(CEnterBridge, CGameObject);
	ADDFN(CEnterExitFirstClassState, CGameObject);
	ADDFN(CEnterExitMiniLift, CSGTNavigation);
	ADDFN(CEnterExitSecClassMiniLift, CGameObject);
	ADDFN(CEnterExitView, CGameObject);
	ADDFN(CEnterSecClassState, CGameObject);
	ADDFN(CExitLift, CGameObject);
	ADDFN(CExitPellerator, CGameObject);
	ADDFN(CScraliontisTable, CRestaurantPanHandler);

	ADDFN(CCharacter, CGameObject);
	ADDFN(CStarlings, CCharacter);
	ADDFN(CTrueTalkNPC, CCharacter);
	ADDFN(CBarbot, CTrueTalkNPC);
	ADDFN(CBellBot, CTrueTalkNPC);
	ADDFN(CCallBot, CGameObject);
	ADDFN(CDeskbot, CTrueTalkNPC);
	ADDFN(CDoorbot, CTrueTalkNPC);
	ADDFN(CMaitreD, CTrueTalkNPC);
	ADDFN(CLiftBot, CTrueTalkNPC);
	ADDFN(CMobile, CCharacter);
	ADDFN(CParrot, CTrueTalkNPC);
	ADDFN(CRobotController, CGameObject);
	ADDFN(CSuccUBus, CTrueTalkNPC);
	ADDFN(CBilgeSuccUBus, CSuccUBus);
	ADDFN(CParrotSuccUBus, CSuccUBus);
	ADDFN(CSummonBots, CRobotController);
	ADDFN(CTitania, CCharacter);
	ADDFN(CTransport, CMobile);
	ADDFN(CGondolier, CTransport);
	ADDFN(CLift, CTransport);
	ADDFN(CLiftindicator, CLift);
	ADDFN(CPellerator, CTransport);
	ADDFN(CServiceElevator, CTransport);

	ADDFN(CMusicPlayer, CGameObject);
	ADDFN(CAutoMusicPlayerBase, CGameObject);
	ADDFN(CAutoSoundPlayer, CGameObject);
	ADDFN(CBackgroundSoundMaker, CGameObject);
	ADDFN(CGondolierSong, CGameObject);
	ADDFN(CTriggerAutoMusicPlayer, CGameObject);
	ADDFN(CMusicConsoleButton, CMusicPlayer);
	ADDFN(CPhonograph, CMusicPlayer);
	ADDFN(CRestaurantPhonograph, CPhonograph);
	ADDFN(CMusicRoomPhonograph, CRestaurantPhonograph);
	ADDFN(CEjectPhonographButton, CBackground);
	ADDFN(CMusicRoomStopPhonographButton, CEjectPhonographButton);
	ADDFN(CAutoMusicPlayer, CAutoMusicPlayerBase);
	ADDFN(CAutoSoundPlayerADSR, CAutoSoundPlayer);
	ADDFN(CNodeAutoSoundPlayer, CAutoSoundPlayer);
	ADDFN(CViewAutoSoundPlayer, CAutoSoundPlayer);
	ADDFN(CRoomAutoSoundPlayer, CAutoSoundPlayer);
	ADDFN(CRestrictedAutoMusicPlayer, CAutoMusicPlayer);
	ADDFN(CRoomTriggerAutoMusicPlayer, CTriggerAutoMusicPlayer);
	ADDFN(CEnterViewTogglesOtherMusic, CTriggerAutoMusicPlayer);
	ADDFN(CViewTogglesOtherMusic, CEnterViewTogglesOtherMusic);
	ADDFN(CWaterLappingSounds, CRoomAutoSoundPlayer);
	ADDFN(CBirdSong, CRoomAutoSoundPlayer);
	ADDFN(CDomeFromTopOfWell, CViewAutoSoundPlayer);
	ADDFN(CGondolierSong, CRoomAutoSoundPlayer);
	ADDFN(CSeasonNoises, CViewAutoSoundPlayer);
	ADDFN(CSeasonalMusicPlayer, CAutoMusicPlayerBase);
	ADDFN(CTitaniaSpeech, CGameObject);
	ADDFN(CStarControl, CGameObject);
	ADDFN(CTimeEventInfo, ListItem);
}

void CSaveableObject::freeClassList() {
	ClassDefList::iterator i;
	for (i = _classDefs->begin(); i != _classDefs->end(); ++i)
		delete *i;

	delete _classDefs;
	delete _classList;
}

CSaveableObject *CSaveableObject::createInstance(const Common::String &name) {
	return (*_classList)[name]();
}

void CSaveableObject::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);
}

void CSaveableObject::load(SimpleFile *file) {
	file->readNumber();
}

void CSaveableObject::saveHeader(SimpleFile *file, int indent) {
	file->writeClassStart(getType()->_className, indent);
}

void CSaveableObject::saveFooter(SimpleFile *file, int indent) {
	file->writeClassEnd(indent);
}

bool CSaveableObject::isInstanceOf(const ClassDef *classDef) const {
	for (ClassDef *def = getType(); def != nullptr; def = def->_parent) {
		if (def == classDef)
			return true;
	}

	return false;
}

} // End of namespace Titanic
