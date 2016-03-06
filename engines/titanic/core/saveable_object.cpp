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
#include "titanic/core/link_item.h"
#include "titanic/core/list.h"
#include "titanic/core/message_target.h"
#include "titanic/core/movie_clip.h"
#include "titanic/core/multi_drop_target.h"
#include "titanic/core/node_item.h"
#include "titanic/core/project_item.h"
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
#include "titanic/game/bilge_succubus.h"
#include "titanic/game/bomb.h"
#include "titanic/game/bottom_of_well_monitor.h"
#include "titanic/game/bowl_unlocker.h"
#include "titanic/game/brain_slot.h"
#include "titanic/game/bridge_view.h"
#include "titanic/game/broken_pell_base.h"
#include "titanic/game/broken_pellerator.h"
#include "titanic/game/broken_pellerator_froz.h"
#include "titanic/game/cage.h"
#include "titanic/game/call_pellerator.h"
#include "titanic/game/captains_wheel.h"
#include "titanic/game/cdrom.h"
#include "titanic/game/cdrom_computer.h"
#include "titanic/game/cdrom_tray.h"
#include "titanic/game/cell_point_button.h"
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
#include "titanic/game/idle_summoner.h"
#include "titanic/game/lemon_dispensor.h"
#include "titanic/game/light.h"
#include "titanic/game/light_switch.h"
#include "titanic/game/little_lift_button.h"
#include "titanic/game/long_stick_dispenser.h"
#include "titanic/game/mail_man.h"
#include "titanic/game/missiveomat.h"
#include "titanic/game/missiveomat_button.h"
#include "titanic/game/musical_instrument.h"
#include "titanic/game/music_console_button.h"
#include "titanic/game/music_room_phonograph.h"
#include "titanic/game/music_room_stop_phonograph_button.h"
#include "titanic/game/music_system_lock.h"
#include "titanic/game/nav_helmet.h"
#include "titanic/game/navigation_computer.h"
#include "titanic/game/no_nut_bowl.h"
#include "titanic/game/nose_holder.h"
#include "titanic/game/null_port_hole.h"
#include "titanic/game/nut_replacer.h"
#include "titanic/game/pet_disabler.h"
#include "titanic/game/phonograph.h"
#include "titanic/game/phonograph_lid.h"
#include "titanic/game/play_music_button.h"
#include "titanic/game/play_on_act.h"
#include "titanic/game/port_hole.h"
#include "titanic/game/record_phonograph_button.h"
#include "titanic/game/replacement_ear.h"
#include "titanic/game/reserved_table.h"
#include "titanic/game/restaurant_cylinder_holder.h"
#include "titanic/game/restaurant_phonograph.h"
#include "titanic/game/room_item.h"
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
#include "titanic/game/starling_puret.h"
#include "titanic/game/start_action.h"
#include "titanic/game/stop_phonograph_button.h"
#include "titanic/game/sub_glass.h"
#include "titanic/game/sweet_bowl.h"
#include "titanic/game/television.h"
#include "titanic/game/third_class_canal.h"
#include "titanic/game/throw_tv_down_well.h"
#include "titanic/game/titania_still_control.h"
#include "titanic/game/tow_parrot_nav.h"
#include "titanic/game/up_lighter.h"
#include "titanic/game/useless_lever.h"
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
#include "titanic/game/parrot/parrot_succubus.h"
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
#include "titanic/gfx/chev_switch.h"
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
#include "titanic/gfx/pet_drag_chev.h"
#include "titanic/gfx/pet_graphic.h"
#include "titanic/gfx/pet_graphic2.h"
#include "titanic/gfx/pet_leaf.h"
#include "titanic/gfx/pet_mode_off.h"
#include "titanic/gfx/pet_mode_on.h"
#include "titanic/gfx/pet_mode_panel.h"
#include "titanic/gfx/pet_pannel1.h"
#include "titanic/gfx/pet_pannel2.h"
#include "titanic/gfx/pet_pannel3.h"
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
#include "titanic/gfx/volume_control.h"

#include "titanic/messages/messages.h"
#include "titanic/messages/auto_sound_event.h"
#include "titanic/messages/bilge_auto_sound_event.h"
#include "titanic/messages/bilge_dispensor_event.h"
#include "titanic/messages/door_auto_sound_event.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/messages/service_elevator_door.h"

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

#include "titanic/pet_control/pet_control.h"

#include "titanic/sound/auto_music_player.h"
#include "titanic/sound/auto_music_player_base.h"
#include "titanic/sound/auto_sound_player.h"
#include "titanic/sound/auto_sound_player_adsr.h"
#include "titanic/sound/background_sound_maker.h"
#include "titanic/sound/bird_song.h"
#include "titanic/sound/gondolier_song.h"
#include "titanic/sound/enter_view_toggles_other_music.h"
#include "titanic/sound/music_player.h"
#include "titanic/sound/node_auto_sound_player.h"
#include "titanic/sound/restricted_auto_music_player.h"
#include "titanic/sound/room_auto_sound_player.h"
#include "titanic/sound/season_noises.h"
#include "titanic/sound/seasonal_music_player.h"
#include "titanic/sound/titania_speech.h"
#include "titanic/sound/trigger_auto_music_player.h"
#include "titanic/sound/view_auto_sound_player.h"
#include "titanic/sound/view_toggles_other_music.h"
#include "titanic/sound/water_lapping_sounds.h"

#include "titanic/star_control/star_control.h"

namespace Titanic {

Common::HashMap<Common::String, CSaveableObject::CreateFunction> * 
	CSaveableObject::_classList = nullptr;

#define DEFFN(T) CSaveableObject *Function##T() { return new T(); }
#define ADDFN(T) (*_classList)[#T] = Function##T

DEFFN(CArm)
DEFFN(CAuditoryCentre)
DEFFN(CBowlEar)
DEFFN(CBrain)
DEFFN(CBridgePiece)
DEFFN(CCarry)
DEFFN(CCarryParrot)
DEFFN(CCentralCore)
DEFFN(CChicken)
DEFFN(CChickenCooler)
DEFFN(CCrushedTV)
DEFFN(CEar)
DEFFN(CEye)
DEFFN(CFeathers)
DEFFN(CFruit)
DEFFN(CGlass)
DEFFN(CHammer)
DEFFN(CHeadPiece)
DEFFN(CHose)
DEFFN(CHoseEnd)
DEFFN(CKey)
DEFFN(CLiftbotHead)
DEFFN(CLongStick)
DEFFN(CMagazine)
DEFFN(CMaitreDLeftArm)
DEFFN(CMaitreDRightArm)
DEFFN(CMouth)
DEFFN(CNapkin)
DEFFN(CNose)
DEFFN(CNote)
DEFFN(CParcel)
DEFFN(CPerch)
DEFFN(CPhonographCylinder)
DEFFN(CPhonographEar)
DEFFN(CPhotograph)
DEFFN(CPlugIn)
DEFFN(CSpeechCentre)
DEFFN(CSweets)
DEFFN(CVisionCentre)

DEFFN(CBackground)
DEFFN(CClickResponder)
DEFFN(CDontSaveFileItem)
DEFFN(CDropTarget)
DEFFN(CFileItem)
DEFFN(CFileListItem)
DEFFN(CLinkItem)
DEFFN(CMessageTarget)
DEFFN(CMovieClip)
DEFFN(CMovieClipList)
DEFFN(CMultiDropTarget)
DEFFN(CNodeItem)
DEFFN(CProjectItem)
DEFFN(CStaticImage)
DEFFN(CTurnOnObject)
DEFFN(CTurnOnPlaySound)
DEFFN(CTurnOnTurnOff)
DEFFN(CTreeItem)
DEFFN(CViewItem)

DEFFN(CAnnounce)
DEFFN(CAnnoyBarbot)
DEFFN(CArbBackground)
DEFFN(CArboretumGate)
DEFFN(CAutoAnimate)
DEFFN(CBarBell)
DEFFN(CBarMenu)
DEFFN(CBarMenuButton)
DEFFN(CBelbotGetLight)
DEFFN(CBilgeSuccUBus)
DEFFN(CBomb)
DEFFN(CBottomOfWellMonitor)
DEFFN(CBowlUnlocker)
DEFFN(CBrainSlot)
DEFFN(CBridgeView)
DEFFN(CBrokenPellBase)
DEFFN(CBrokenPellerator)
DEFFN(CBrokenPelleratorFroz)
DEFFN(CCage)
DEFFN(CCallPellerator)
DEFFN(CCaptainsWheel)
DEFFN(CCDROM)
DEFFN(CCDROMComputer)
DEFFN(CCDROMTray)
DEFFN(CCellPointButton)
DEFFN(CChickenDispensor)
DEFFN(CCloseBrokenPel)
DEFFN(CodeWheel)
DEFFN(CComputer)
DEFFN(CCookie)
DEFFN(CComputerScreen)
DEFFN(CCredits)
DEFFN(CCreditsButton)
DEFFN(CDeadArea)
DEFFN(CDeskClickResponder)
DEFFN(CDoorbotElevatorHandler)
DEFFN(CDoorbotHomeHandler)
DEFFN(CEarSweetBowl)
DEFFN(CEjectPhonographButton)
DEFFN(CElevatorActionArea)
DEFFN(CEmmaControl)
DEFFN(CEmptyNutBowl)
DEFFN(CEndCreditText)
DEFFN(CEndCredits)
DEFFN(CEndExplodeShip)
DEFFN(CEndGameCredits)
DEFFN(CEndSequenceControl)
DEFFN(CFan)
DEFFN(CFanControl)
DEFFN(CFanDecrease)
DEFFN(CFanIncrease)
DEFFN(CFanNoises)
DEFFN(CFloorIndicator)
DEFFN(CGamesConsole)
DEFFN(CGetLiftEye2)
DEFFN(CGlassSmasher)
DEFFN(CHammerClip)
DEFFN(CHammerDispensor)
DEFFN(CHammerDispensorButton)
DEFFN(CHeadSlot)
DEFFN(CHeadSmashEvent)
DEFFN(CHeadSmashLever)
DEFFN(CIdleSummoner)
DEFFN(CLemonDispensor)
DEFFN(CLight)
DEFFN(CLightSwitch)
DEFFN(CLittleLiftButton)
DEFFN(CLongStickDispenser)
DEFFN(CMailMan)
DEFFN(CMissiveOMat)
DEFFN(CMissiveOMatButton)
DEFFN(CMusicalInstrument)
DEFFN(CMusicConsoleButton)
DEFFN(CMusicRoomPhonograph)
DEFFN(CMusicRoomStopPhonographButton)
DEFFN(CMusicSystemLock)
DEFFN(CNavHelmet)
DEFFN(CNavigationComputer)
DEFFN(CNoNutBowl)
DEFFN(CNoseHolder)
DEFFN(CNullPortHole)
DEFFN(CNutReplacer)
DEFFN(CPetDisabler)
DEFFN(CPhonograph)
DEFFN(CPhonographLid)
DEFFN(CPlayMusicButton)
DEFFN(CPlayOnAct)
DEFFN(CPortHole)
DEFFN(CRecordPhonographButton)
DEFFN(CReplacementEar)
DEFFN(CReservedTable)
DEFFN(CRestaurantCylinderHolder)
DEFFN(CRestaurantPhonograph)
DEFFN(CRoomItem)
DEFFN(CSauceDispensor)
DEFFN(CSearchPoint)
DEFFN(CSeasonBackground)
DEFFN(CSeasonBarrel)
DEFFN(CSeasonalAdjustment)
DEFFN(CServiceElevatorWindow)
DEFFN(CShipSetting)
DEFFN(CShipSettingButton)
DEFFN(CShowCellpoints)
DEFFN(CSpeechDispensor)
DEFFN(CStarlingPuret)
DEFFN(CStartAction)
DEFFN(CStopPhonographButton)
DEFFN(CSUBGlass)
DEFFN(CSweetBowl)
DEFFN(CTelevision)
DEFFN(CThirdClassCanal)
DEFFN(CThrowTVDownWell)
DEFFN(CTitaniaStillControl)
DEFFN(CTOWParrotNav)
DEFFN(CUpLighter)
DEFFN(CUselessLever)
DEFFN(CWheelButton)
DEFFN(CWheelHotSpot)
DEFFN(CWheelSpin)
DEFFN(CWheelSpinHorn)
DEFFN(CGondolierBase)
DEFFN(CGondolierChest)
DEFFN(CGondolierFace)
DEFFN(CGondolierMixer)
DEFFN(CGondolierSlider)
DEFFN(CMaitreDArmHolder)
DEFFN(CMaitreDBody)
DEFFN(CMaitreDLegs)
DEFFN(CMaitreDProdReceptor)
DEFFN(CParrotLobbyController)
DEFFN(CParrotLobbyLinkUpdater)
DEFFN(CParrotLobbyObject)
DEFFN(CParrotLobbyViewObject)
DEFFN(CParrotLoser)
DEFFN(CParrotNutBowlActor)
DEFFN(CParrotNutEater)
DEFFN(CParrotPerchHolder)
DEFFN(CParrotSuccUBus)
DEFFN(CParrotTrigger)
DEFFN(CPlayerMeetsParrot)
DEFFN(CPET)
DEFFN(CPETClass1)
DEFFN(CPETClass2)
DEFFN(CPETClass3)
DEFFN(CPetControl)
DEFFN(CPetDragChev)
DEFFN(CPetGraphic)
DEFFN(CPetGraphic2)
DEFFN(PETLeaf)
DEFFN(CPETLift)
DEFFN(CPETMonitor)
DEFFN(CPETPellerator)
DEFFN(CPETPosition)
DEFFN(CPETSentinal)
DEFFN(CPETSounds)
DEFFN(CPETTransition)
DEFFN(CPETTransport)
DEFFN(CPickUp)
DEFFN(CPickUpBarGlass)
DEFFN(CPickUpHose)
DEFFN(CPickUpLemon)
DEFFN(CPickUpSpeechCentre)
DEFFN(CPickUpVisCentre)
DEFFN(CBarShelfVisCentre)
DEFFN(CLemonOnBar)
DEFFN(CPlaceHolder)
DEFFN(CTVOnBar)
DEFFN(CArmchair)
DEFFN(CBasin)
DEFFN(CBedfoot)
DEFFN(CBedhead)
DEFFN(CChestOfDrawers)
DEFFN(CDesk)
DEFFN(CDeskchair)
DEFFN(CDrawer)
DEFFN(CSGTDoors)
DEFFN(SGTNav)
DEFFN(CSGTNavigation)
DEFFN(CSGTRestaurantDoors)
DEFFN(CSGTStateControl)
DEFFN(CSGTStateRoom)
DEFFN(CSGTTV)
DEFFN(CSGTUpperDoorsSound)
DEFFN(CToilet)
DEFFN(CVase)
DEFFN(CWashstand)

DEFFN(CGondolier)
DEFFN(CLift)
DEFFN(CLiftindicator)
DEFFN(CPellerator)
DEFFN(CServiceElevator)
DEFFN(CTransport)

DEFFN(CActButton)
DEFFN(CChangesSeasonButton)
DEFFN(CChevLeftOff)
DEFFN(CChevLeftOn)
DEFFN(CChevRightOff)
DEFFN(CChevRightOn)
DEFFN(CChevSendRecSwitch)
DEFFN(CChevSwitch)
DEFFN(CEditControl)
DEFFN(CElevatorButton)
DEFFN(CGetFromSucc)
DEFFN(CHelmetOnOff)
DEFFN(CHomePhoto)
DEFFN(CIconNavAction)
DEFFN(CIconNavButt)
DEFFN(CIconNavDown)
DEFFN(CIconNavImage)
DEFFN(CIconNavLeft)
DEFFN(CIconNavReceive)
DEFFN(CIconNavRight)
DEFFN(CIconNavSend)
DEFFN(CIconNavUp)
DEFFN(CKeybrdButt)
DEFFN(CMoveObjectButton)
DEFFN(CMusicControl)
DEFFN(CMusicSlider)
DEFFN(CMusicSliderPitch)
DEFFN(CMusicSliderSpeed)
DEFFN(CMusicSwitch)
DEFFN(CMusicSwitchInversion)
DEFFN(CMusicSwitchReverse)
DEFFN(CMusicVoiceMute)
DEFFN(CPetModeOff)
DEFFN(CPetModeOn)
DEFFN(CPetModePanel)
DEFFN(CPetPannel1)
DEFFN(CPetPannel2)
DEFFN(CPetPannel3)
DEFFN(CSendToSucc)
DEFFN(CSGTSelector)
DEFFN(CSliderButton)
DEFFN(CSmallChevLeftOff)
DEFFN(CSmallChevLeftOn)
DEFFN(CSmallChevRightOff)
DEFFN(CSmallChevRightOn)
DEFFN(CStatusChangeButton)
DEFFN(CSTButton)
DEFFN(CTextDown)
DEFFN(CTextSkrew)
DEFFN(CTextUp)
DEFFN(CToggleButton)
DEFFN(CToggleSwitch)
DEFFN(CVolumeControl)

DEFFN(CActMsg)
DEFFN(CActivationmsg)
DEFFN(CAddHeadPieceMsg)
DEFFN(CAnimateMaitreDMsg)
DEFFN(CArboretumGateMsg)
DEFFN(CArmPickedUpFromTableMsg)
DEFFN(CAutoSoundEvent)
DEFFN(CBilgeAutoSoundEvent)
DEFFN(CBilgeDispensorEvent)
DEFFN(CBodyInBilgeRoomMsg)
DEFFN(CBowlStateChange)
DEFFN(CCarryObjectArrivedMsg)
DEFFN(CChangeSeasonMsg)
DEFFN(CCheckAllPossibleCodes)
DEFFN(CCheckChevCode)
DEFFN(CChildDragEndMsg)
DEFFN(CChildDragMoveMsg)
DEFFN(CChildDragStartMsg)
DEFFN(CClearChevPanelBits)
DEFFN(CCorrectMusicPlayedMsg)
DEFFN(CCreateMusicPlayerMsg)
DEFFN(CCylinderHolderReadyMsg)
DEFFN(CDeactivationMsg)
DEFFN(CDeliverCCarryMsg)
DEFFN(CDisableMaitreDProdReceptor)
DEFFN(CDismissBotMsg)
DEFFN(CDoffNavHelmet)
DEFFN(CDonNavHelmet)
DEFFN(CDoorAutoSoundEvent)
DEFFN(CDoorbotNeededInElevatorMsg)
DEFFN(CDoorbotNeededInHomeMsg)
DEFFN(CDropobjectMsg)
DEFFN(CDropZoneGotObjectMsg)
DEFFN(CDropZoneLostObjectMsg)
DEFFN(CEditControlMsg)
DEFFN(CEjectCylinderMsg)
DEFFN(CErasePhonographCylinderMsg)
DEFFN(CFreshenCookieMsg)
DEFFN(CGetChevClassBits)
DEFFN(CGetChevClassNum)
DEFFN(CGetChevCodeFromRoomNameMsg)
DEFFN(CGetChevFloorBits)
DEFFN(CGetChevFloorNum)
DEFFN(CGetChevLiftBits)
DEFFN(CGetChevLiftNum)
DEFFN(CGetChevRoomBits)
DEFFN(CGetChevRoomNum)
DEFFN(CHoseConnectedMsg)
DEFFN(CInitializeAnimMsg)
DEFFN(CIsEarBowlPuzzleDone)
DEFFN(CIsHookedOnMsg)
DEFFN(CIsParrotPresentMsg)
DEFFN(CKeyCharMsg)
DEFFN(CLemonFallsFromTreeMsg)
DEFFN(CLightsMsg)
DEFFN(CLockPhonographMsg)
DEFFN(CMaitreDDefeatedMsg)
DEFFN(CMaitreDHappyMsg)
DEFFN(CMissiveOMatActionMsg)
DEFFN(CMouseMsg)
DEFFN(CMouseMoveMsg)
DEFFN(CMouseButtonMsg)
DEFFN(CMouseButtonDownMsg)
DEFFN(CMouseButtonUpMsg)
DEFFN(CMouseButtonDoubleClickMsg)
DEFFN(CMouseDragMsg)
DEFFN(CMouseDragStartMsg)
DEFFN(CMouseDragMoveMsg)
DEFFN(CMouseDragEndMsg)
DEFFN(CMoveToStartPosMsg)
DEFFN(CMovieEndMsg)
DEFFN(CMovieFrameMsg)
DEFFN(CMusicHasStartedMsg)
DEFFN(CMusicHasStoppedMsg)
DEFFN(CMusicSettingChangedMsg)
DEFFN(CNPCPlayAnimationMsg)
DEFFN(CNPCPlayIdleAnimationMsg)
DEFFN(CNPCPlayTalkingAnimationMsg)
DEFFN(CNPCQueueIdleAnimMsg)
DEFFN(CNutPuzzleMsg)
DEFFN(COnSummonBotMsg)
DEFFN(COpeningCreditsMsg)
DEFFN(CPETDeliverMsg)
DEFFN(CPETGainedObjectMsg)
DEFFN(CPETHelmetOnOffMsg)
DEFFN(CPETKeyboardOnOffMsg)
DEFFN(CPETLostObjectMsg)
DEFFN(CPETObjectSelectedMsg)
DEFFN(CPETObjectStateMsg)
DEFFN(CPETPhotoOnOffMsg)
DEFFN(CPETPlaySoundMsg)
DEFFN(CPETReceiveMsg)
DEFFN(CPETSetStarDestinationMsg)
DEFFN(CPETStarFieldLockMsg)
DEFFN(CPETStereoFieldOnOffMsg)
DEFFN(CPETTargetMsg)
DEFFN(CPanningAwayFromParrotMsg)
DEFFN(CParrotSpeakMsg)
DEFFN(CParrotTriesChickenMsg)
DEFFN(CPassOnDragStartMsg)
DEFFN(CPhonographPlayMsg)
DEFFN(CPhonographReadyToPlayMsg)
DEFFN(CPhonographRecordMsg)
DEFFN(CPhonographStopMsg)
DEFFN(CPlayRangeMsg)
DEFFN(CPlayerTriesRestaurantTableMsg)
DEFFN(CPreSaveMsg)
DEFFN(CProdMaitreDMsg)
DEFFN(CPumpingMsg)
DEFFN(CPutBotBackInHisBoxMsg)
DEFFN(CPutParrotBackMsg)
DEFFN(CPuzzleSolvedMsg)
DEFFN(CQueryCylinderHolderMsg)
DEFFN(CQueryCylinderMsg)
DEFFN(CQueryCylinderNameMsg)
DEFFN(CQueryCylinderTypeMsg)
DEFFN(CQueryMusicControlSettingMsg)
DEFFN(CQueryPhonographState)
DEFFN(CRecordOntoCylinderMsg)
DEFFN(CRemoveFromGameMsg)
DEFFN(CReplaceBowlAndNutsMsg)
DEFFN(CRestaurantMusicChanged)
DEFFN(CSendCCarryMsg)
DEFFN(CSenseWorkingMsg)
DEFFN(CServiceElevatorDoor)
DEFFN(CServiceElevatorFloorChangeMsg)
DEFFN(CServiceElevatorFloorRequestMsg)
DEFFN(CServiceElevatorMsg)
DEFFN(CSetChevButtonImageMsg)
DEFFN(CSetChevClassBits)
DEFFN(CSetChevFloorBits)
DEFFN(CSetChevLiftBits)
DEFFN(CSetChevPanelBitMsg)
DEFFN(CSetChevPanelButtonsMsg)
DEFFN(CSetChevRoomBits)
DEFFN(CSetMusicControlsMsg)
DEFFN(CSetVarMsg)
DEFFN(CSetVolumeMsg)
DEFFN(CShipSettingMsg)
DEFFN(CShowTextMsg)
DEFFN(CSignalObject)
DEFFN(CSpeechFallsFromTreeMsg)
DEFFN(CStartMusicMsg)
DEFFN(CStatusChangeMsg)
DEFFN(CStopMusicMsg)
DEFFN(CSubAcceptCCarryMsg)
DEFFN(CSubDeliverCCarryMsg)
DEFFN(CSubSendCCarryMsg)
DEFFN(CSUBTransition)
DEFFN(CSubTurnOffMsg)
DEFFN(CSubTurnOnMsg)
DEFFN(CSummonBotMsg)
DEFFN(CSummonBotQuerryMsg)
DEFFN(CTakeHeadPieceMsg)
DEFFN(CTextInputMsg)
DEFFN(CTimeDilationMsg)
DEFFN(CTimeMsg)
DEFFN(CTitleSequenceEndedMsg)
DEFFN(CTransitMsg)
DEFFN(CTransportMsg)
DEFFN(CTriggerAutoMusicPlayerMsg)
DEFFN(CTriggerNPCEvent)
DEFFN(CTrueTalkGetAnimSetMsg)
DEFFN(CTrueTalkGetAssetDetailsMsg)
DEFFN(CTrueTalkGetStateValueMsg)
DEFFN(CTrueTalkNotifySpeechEndedMsg)
DEFFN(CTrueTalkNotifySpeechStartedMsg)
DEFFN(CTrueTalkQueueUpAnimSetMsg)
DEFFN(CTrueTalkSelfQueueAnimSetMsg)
DEFFN(CTrueTalkTriggerActionMsg)
DEFFN(CTurnOff)
DEFFN(CTurnOn)
DEFFN(CUse)
DEFFN(CUseWithCharMsg)
DEFFN(CUseWithOtherMsg)
DEFFN(CVirtualKeyCharMsg)
DEFFN(CVisibleMsg)

DEFFN(CEnterBombRoom)
DEFFN(CEnterBridge)
DEFFN(CEnterExitFirstClassState)
DEFFN(CEnterExitMiniLift)
DEFFN(CEnterExitSecClassMiniLift)
DEFFN(CEnterExitView)
DEFFN(CEnterSecClassState)
DEFFN(CExitArboretum)
DEFFN(CExitBridge)
DEFFN(CExitLift)
DEFFN(CExitPellerator)
DEFFN(CExitStateRoom)
DEFFN(CExitTiania)
DEFFN(CMovePlayerInParrotRoom)
DEFFN(CMovePlayerTo)
DEFFN(CMovePlayerToFrom)
DEFFN(CMultiMove)
DEFFN(CPanFromPel)
DEFFN(CRestaurantPanHandler)
DEFFN(CScraliontisTable)
DEFFN(CRestrictedMove)
DEFFN(CTripDownCanal)

DEFFN(CBarbot)
DEFFN(CBellBot)
DEFFN(CCallBot)
DEFFN(CDeskbot)
DEFFN(CDoorbot)
DEFFN(CLiftBot)
DEFFN(CMaitreD)
DEFFN(CMobile)
DEFFN(CParrot)
DEFFN(CStarlings)
DEFFN(CSummonBots)
DEFFN(CSuccUBus)
DEFFN(CTitania)
DEFFN(CAutoMusicPlayer)
DEFFN(CAutoMusicPlayerBase)
DEFFN(CAutoSoundPlayer)
DEFFN(CAutoSoundPlayerADSR)
DEFFN(CBackgroundSoundMaker)
DEFFN(CBirdSong)
DEFFN(CEnterViewTogglesOtherMusic)
DEFFN(CGondolierSong)
DEFFN(CMusicPlayer)
DEFFN(CNodeAutoSoundPlayer)
DEFFN(CRestrictedAutoMusicPlayer)
DEFFN(CRoomAutoSoundPlayer)
DEFFN(CSeasonNoises)
DEFFN(CSeasonalMusicPlayer)
DEFFN(CTitaniaSpeech)
DEFFN(CTriggerAutoMusicPlayer)
DEFFN(CViewAutoSoundPlayer)
DEFFN(CViewTogglesOtherMusic)
DEFFN(CWaterLappingSounds)
DEFFN(CStarControl);

void CSaveableObject::initClassList() {
	_classList = new Common::HashMap<Common::String, CreateFunction>();
	ADDFN(CArm);
	ADDFN(CAuditoryCentre);
	ADDFN(CBowlEar);
	ADDFN(CBrain);
	ADDFN(CBridgePiece);
	ADDFN(CCarry);
	ADDFN(CCarryParrot);
	ADDFN(CCentralCore);
	ADDFN(CChicken);
	ADDFN(CChickenCooler);
	ADDFN(CCrushedTV);
	ADDFN(CEar);
	ADDFN(CEye);
	ADDFN(CFeathers);
	ADDFN(CFruit);
	ADDFN(CGlass);
	ADDFN(CHammer);
	ADDFN(CHeadPiece);
	ADDFN(CHose);
	ADDFN(CHoseEnd);
	ADDFN(CKey);
	ADDFN(CLiftbotHead);
	ADDFN(CLongStick);
	ADDFN(CMagazine);
	ADDFN(CMaitreDLeftArm);
	ADDFN(CMaitreDRightArm);
	ADDFN(CMouth);
	ADDFN(CNapkin);
	ADDFN(CNose);
	ADDFN(CNote);
	ADDFN(CParcel);
	ADDFN(CPerch);
	ADDFN(CPhonographCylinder);
	ADDFN(CPhonographEar);
	ADDFN(CPhotograph);
	ADDFN(CPlugIn);
	ADDFN(CSpeechCentre);
	ADDFN(CSweets);
	ADDFN(CVisionCentre);

	ADDFN(CBackground);
	ADDFN(CClickResponder);
	ADDFN(CDontSaveFileItem);
	ADDFN(CDropTarget);
	ADDFN(CFileItem);
	ADDFN(CFileListItem);
	ADDFN(CLinkItem);
	ADDFN(CMessageTarget);
	ADDFN(CMovieClip);
	ADDFN(CMovieClipList);
	ADDFN(CMultiDropTarget);
	ADDFN(CNodeItem);
	ADDFN(CProjectItem);
	ADDFN(CStaticImage);
	ADDFN(CTurnOnObject);
	ADDFN(CTreeItem);
	ADDFN(CTurnOnPlaySound);
	ADDFN(CTurnOnTurnOff);
	ADDFN(CViewItem);

	ADDFN(CAnnounce);
	ADDFN(CAnnoyBarbot);
	ADDFN(CArbBackground);
	ADDFN(CArboretumGate);
	ADDFN(CAutoAnimate);
	ADDFN(CBarBell);
	ADDFN(CBarMenu);
	ADDFN(CBarMenuButton);
	ADDFN(CBelbotGetLight);
	ADDFN(CBilgeSuccUBus);
	ADDFN(CBomb);
	ADDFN(CBottomOfWellMonitor);
	ADDFN(CBrainSlot);
	ADDFN(CBowlUnlocker);
	ADDFN(CBridgeView);
	ADDFN(CBrokenPellBase);
	ADDFN(CBrokenPellerator);
	ADDFN(CBrokenPelleratorFroz);
	ADDFN(CCage);
	ADDFN(CCallPellerator);
	ADDFN(CCaptainsWheel);
	ADDFN(CCDROM);
	ADDFN(CCDROMComputer);
	ADDFN(CCDROMTray);
	ADDFN(CCellPointButton);
	ADDFN(CChickenDispensor);
	ADDFN(CodeWheel);
	ADDFN(CComputerScreen);
	ADDFN(CCloseBrokenPel);
	ADDFN(CComputer);
	ADDFN(CCookie);
	ADDFN(CCredits);
	ADDFN(CCreditsButton);
	ADDFN(CDeadArea);
	ADDFN(CDeskClickResponder);
	ADDFN(CDoorbotElevatorHandler);
	ADDFN(CDoorbotHomeHandler);
	ADDFN(CDropTarget);
	ADDFN(CEarSweetBowl);
	ADDFN(CEjectPhonographButton);
	ADDFN(CElevatorActionArea);
	ADDFN(CEmmaControl);
	ADDFN(CEmptyNutBowl);
	ADDFN(CEndCreditText);
	ADDFN(CEndCredits);
	ADDFN(CEndExplodeShip);
	ADDFN(CEndGameCredits);
	ADDFN(CEndSequenceControl);
	ADDFN(CFan);
	ADDFN(CFanControl);
	ADDFN(CFanDecrease);
	ADDFN(CFanIncrease);
	ADDFN(CFanNoises);
	ADDFN(CFloorIndicator);
	ADDFN(CGamesConsole);
	ADDFN(CGetLiftEye2);
	ADDFN(CGlassSmasher);
	ADDFN(CHammerClip);
	ADDFN(CHammerDispensor);
	ADDFN(CHammerDispensorButton);
	ADDFN(CHeadSlot);
	ADDFN(CHeadSmashEvent);
	ADDFN(CHeadSmashLever);
	ADDFN(CIdleSummoner);
	ADDFN(CLemonDispensor);
	ADDFN(CLight);
	ADDFN(CLightSwitch);
	ADDFN(CLittleLiftButton);
	ADDFN(CLongStickDispenser);
	ADDFN(CMailMan);
	ADDFN(CMissiveOMat);
	ADDFN(CMissiveOMatButton);
	ADDFN(CMusicalInstrument);
	ADDFN(CMusicConsoleButton);
	ADDFN(CMusicRoomPhonograph);
	ADDFN(CMusicRoomStopPhonographButton);
	ADDFN(CMusicSystemLock);
	ADDFN(CNavHelmet);
	ADDFN(CNavigationComputer);
	ADDFN(CNoNutBowl);
	ADDFN(CNoseHolder);
	ADDFN(CNullPortHole);
	ADDFN(CNutReplacer);
	ADDFN(CPetDisabler);
	ADDFN(CPhonograph);
	ADDFN(CPhonographLid);
	ADDFN(CPlayMusicButton);
	ADDFN(CPlayOnAct);
	ADDFN(CPortHole);
	ADDFN(CRecordPhonographButton);
	ADDFN(CReplacementEar);
	ADDFN(CReservedTable);
	ADDFN(CRestaurantCylinderHolder);
	ADDFN(CRestaurantPhonograph);
	ADDFN(CRoomItem);
	ADDFN(CSauceDispensor);
	ADDFN(CSearchPoint);
	ADDFN(CSeasonBackground);
	ADDFN(CSeasonBarrel);
	ADDFN(CSeasonalAdjustment);
	ADDFN(CServiceElevatorWindow);
	ADDFN(CShipSetting);
	ADDFN(CShipSettingButton);
	ADDFN(CShowCellpoints);
	ADDFN(CSpeechDispensor);
	ADDFN(CStarlingPuret);
	ADDFN(CStartAction);
	ADDFN(CStopPhonographButton);
	ADDFN(CSUBGlass);
	ADDFN(CSweetBowl);
	ADDFN(CTelevision);
	ADDFN(CThirdClassCanal);
	ADDFN(CThrowTVDownWell);
	ADDFN(CTitaniaStillControl);
	ADDFN(CTOWParrotNav);
	ADDFN(CUpLighter);
	ADDFN(CUselessLever);
	ADDFN(CWheelButton);
	ADDFN(CWheelHotSpot);
	ADDFN(CWheelSpin);
	ADDFN(CWheelSpinHorn);
	ADDFN(CGondolierBase);
	ADDFN(CGondolierChest);
	ADDFN(CGondolierFace);
	ADDFN(CGondolierMixer);
	ADDFN(CGondolierSlider);
	ADDFN(CMaitreDArmHolder);
	ADDFN(CMaitreDBody);
	ADDFN(CMaitreDLegs);
	ADDFN(CMaitreDProdReceptor);
	ADDFN(CParrotLobbyController);
	ADDFN(CParrotLobbyLinkUpdater);
	ADDFN(CParrotLobbyObject);
	ADDFN(CParrotLobbyViewObject);
	ADDFN(CParrotLoser);
	ADDFN(CParrotNutBowlActor);
	ADDFN(CParrotNutEater);
	ADDFN(CParrotPerchHolder);
	ADDFN(CParrotSuccUBus);
	ADDFN(CParrotTrigger);
	ADDFN(CPlayerMeetsParrot);
	ADDFN(CPET);
	ADDFN(CPETClass1);
	ADDFN(CPETClass2);
	ADDFN(CPETClass3);
	ADDFN(CPetControl);
	ADDFN(CPetDragChev);
	ADDFN(CPetGraphic);
	ADDFN(CPetGraphic2);
	ADDFN(PETLeaf);
	ADDFN(CPETLift);
	ADDFN(CPETMonitor);
	ADDFN(CPETPellerator);
	ADDFN(CPETPosition);
	ADDFN(CPETSentinal);
	ADDFN(CPETSounds);
	ADDFN(CPETTransition);
	ADDFN(CPETTransport);
	ADDFN(CPickUp);
	ADDFN(CPickUpBarGlass);
	ADDFN(CPickUpHose);
	ADDFN(CPickUpLemon);
	ADDFN(CPickUpSpeechCentre);
	ADDFN(CPickUpVisCentre);
	ADDFN(CBarShelfVisCentre);
	ADDFN(CLemonOnBar);
	ADDFN(CPlaceHolder);
	ADDFN(CTVOnBar);
	ADDFN(CArmchair);
	ADDFN(CBasin);
	ADDFN(CBedfoot);
	ADDFN(CBedhead);
	ADDFN(CChestOfDrawers);
	ADDFN(CDesk);
	ADDFN(CDeskchair);
	ADDFN(CDrawer);
	ADDFN(CSGTDoors);
	ADDFN(SGTNav);
	ADDFN(CSGTNavigation);
	ADDFN(CSGTRestaurantDoors);
	ADDFN(CSGTStateControl);
	ADDFN(CSGTStateRoom);
	ADDFN(CSGTTV);
	ADDFN(CSGTUpperDoorsSound);
	ADDFN(CToilet);
	ADDFN(CVase);
	ADDFN(CWashstand);

	ADDFN(CGondolier);
	ADDFN(CLift);
	ADDFN(CLiftindicator);
	ADDFN(CPellerator);
	ADDFN(CServiceElevator);
	ADDFN(CTransport);

	ADDFN(CActButton);
	ADDFN(CChangesSeasonButton);
	ADDFN(CChevLeftOff);
	ADDFN(CChevLeftOn);
	ADDFN(CChevRightOff);
	ADDFN(CChevRightOn);
	ADDFN(CChevSendRecSwitch);
	ADDFN(CChevSwitch);
	ADDFN(CEditControl);
	ADDFN(CElevatorButton);
	ADDFN(CGetFromSucc);
	ADDFN(CHelmetOnOff);
	ADDFN(CHomePhoto);
	ADDFN(CIconNavAction);
	ADDFN(CIconNavButt);
	ADDFN(CIconNavDown);
	ADDFN(CIconNavImage);
	ADDFN(CIconNavLeft);
	ADDFN(CIconNavReceive);
	ADDFN(CIconNavRight);
	ADDFN(CIconNavSend);
	ADDFN(CIconNavUp);
	ADDFN(CKeybrdButt);
	ADDFN(CMoveObjectButton);
	ADDFN(CMusicControl);
	ADDFN(CMusicSlider);
	ADDFN(CMusicSliderPitch);
	ADDFN(CMusicSliderSpeed);
	ADDFN(CMusicSwitch);
	ADDFN(CMusicSwitchInversion);
	ADDFN(CMusicSwitchReverse);
	ADDFN(CMusicVoiceMute);
	ADDFN(CPetModeOff);
	ADDFN(CPetModeOn);
	ADDFN(CPetModePanel);
	ADDFN(CPetPannel1);
	ADDFN(CPetPannel2);
	ADDFN(CPetPannel3);
	ADDFN(CSendToSucc);
	ADDFN(CSGTSelector);
	ADDFN(CSliderButton);
	ADDFN(CSmallChevLeftOff);
	ADDFN(CSmallChevLeftOn);
	ADDFN(CSmallChevRightOff);
	ADDFN(CSmallChevRightOn);
	ADDFN(CStatusChangeButton);
	ADDFN(CSTButton);
	ADDFN(CTextDown);
	ADDFN(CTextSkrew);
	ADDFN(CTextUp);
	ADDFN(CToggleButton);
	ADDFN(CToggleSwitch);
	ADDFN(CVolumeControl);

	ADDFN(CActMsg);
	ADDFN(CActivationmsg);
	ADDFN(CAddHeadPieceMsg);
	ADDFN(CAnimateMaitreDMsg);
	ADDFN(CArboretumGateMsg);
	ADDFN(CArmPickedUpFromTableMsg);
	ADDFN(CBodyInBilgeRoomMsg);
	ADDFN(CBowlStateChange);
	ADDFN(CCarryObjectArrivedMsg);
	ADDFN(CChangeSeasonMsg);
	ADDFN(CCheckAllPossibleCodes);
	ADDFN(CCheckChevCode);
	ADDFN(CChildDragEndMsg);
	ADDFN(CChildDragMoveMsg);
	ADDFN(CChildDragStartMsg);
	ADDFN(CClearChevPanelBits);
	ADDFN(CCorrectMusicPlayedMsg);
	ADDFN(CCreateMusicPlayerMsg);
	ADDFN(CCylinderHolderReadyMsg);
	ADDFN(CDeactivationMsg);
	ADDFN(CDeliverCCarryMsg);
	ADDFN(CDisableMaitreDProdReceptor);
	ADDFN(CDismissBotMsg);
	ADDFN(CDoffNavHelmet);
	ADDFN(CDonNavHelmet);
	ADDFN(CDoorbotNeededInElevatorMsg);
	ADDFN(CDoorbotNeededInHomeMsg);
	ADDFN(CDropobjectMsg);
	ADDFN(CDropZoneGotObjectMsg);
	ADDFN(CDropZoneLostObjectMsg);
	ADDFN(CEditControlMsg);
	ADDFN(CEjectCylinderMsg);
	ADDFN(CErasePhonographCylinderMsg);
	ADDFN(CFreshenCookieMsg);
	ADDFN(CGetChevClassBits);
	ADDFN(CGetChevClassNum);
	ADDFN(CGetChevCodeFromRoomNameMsg);
	ADDFN(CGetChevFloorBits);
	ADDFN(CGetChevFloorNum);
	ADDFN(CGetChevLiftBits);
	ADDFN(CGetChevLiftNum);
	ADDFN(CGetChevRoomBits);
	ADDFN(CGetChevRoomNum);
	ADDFN(CHoseConnectedMsg);
	ADDFN(CInitializeAnimMsg);
	ADDFN(CIsEarBowlPuzzleDone);
	ADDFN(CIsHookedOnMsg);
	ADDFN(CIsParrotPresentMsg);
	ADDFN(CKeyCharMsg);
	ADDFN(CLemonFallsFromTreeMsg);
	ADDFN(CLightsMsg);
	ADDFN(CLockPhonographMsg);
	ADDFN(CMaitreDDefeatedMsg);
	ADDFN(CMaitreDHappyMsg);
	ADDFN(CMissiveOMatActionMsg);
	ADDFN(CMouseMsg);
	ADDFN(CMouseMoveMsg);
	ADDFN(CMouseButtonMsg);
	ADDFN(CMouseButtonDownMsg);
	ADDFN(CMouseButtonUpMsg);
	ADDFN(CMouseButtonDoubleClickMsg);
	ADDFN(CMouseDragMsg);
	ADDFN(CMouseDragStartMsg);
	ADDFN(CMouseDragMoveMsg);
	ADDFN(CMouseDragEndMsg);
	ADDFN(CMoveToStartPosMsg);
	ADDFN(CMovieEndMsg);
	ADDFN(CMovieFrameMsg);
	ADDFN(CMusicHasStartedMsg);
	ADDFN(CMusicHasStoppedMsg);
	ADDFN(CMusicSettingChangedMsg);
	ADDFN(CNPCPlayAnimationMsg);
	ADDFN(CNPCPlayIdleAnimationMsg);
	ADDFN(CNPCPlayTalkingAnimationMsg);
	ADDFN(CNPCQueueIdleAnimMsg);
	ADDFN(CNutPuzzleMsg);
	ADDFN(COnSummonBotMsg);
	ADDFN(COpeningCreditsMsg);
	ADDFN(CPETDeliverMsg);
	ADDFN(CPETGainedObjectMsg);
	ADDFN(CPETHelmetOnOffMsg);
	ADDFN(CPETKeyboardOnOffMsg);
	ADDFN(CPETLostObjectMsg);
	ADDFN(CPETObjectSelectedMsg);
	ADDFN(CPETObjectStateMsg);
	ADDFN(CPETPhotoOnOffMsg);
	ADDFN(CPETPlaySoundMsg);
	ADDFN(CPETReceiveMsg);
	ADDFN(CPETSetStarDestinationMsg);
	ADDFN(CPETStarFieldLockMsg);
	ADDFN(CPETStereoFieldOnOffMsg);
	ADDFN(CPETTargetMsg);
	ADDFN(CPanningAwayFromParrotMsg);
	ADDFN(CParrotSpeakMsg);
	ADDFN(CParrotTriesChickenMsg);
	ADDFN(CPassOnDragStartMsg);
	ADDFN(CPhonographPlayMsg);
	ADDFN(CPhonographReadyToPlayMsg);
	ADDFN(CPhonographRecordMsg);
	ADDFN(CPhonographStopMsg);
	ADDFN(CPlayRangeMsg);
	ADDFN(CPlayerTriesRestaurantTableMsg);
	ADDFN(CPreSaveMsg);
	ADDFN(CProdMaitreDMsg);
	ADDFN(CPumpingMsg);
	ADDFN(CPutBotBackInHisBoxMsg);
	ADDFN(CPutParrotBackMsg);
	ADDFN(CPuzzleSolvedMsg);
	ADDFN(CQueryCylinderHolderMsg);
	ADDFN(CQueryCylinderMsg);
	ADDFN(CQueryCylinderNameMsg);
	ADDFN(CQueryCylinderTypeMsg);
	ADDFN(CQueryMusicControlSettingMsg);
	ADDFN(CQueryPhonographState);
	ADDFN(CRecordOntoCylinderMsg);
	ADDFN(CRemoveFromGameMsg);
	ADDFN(CReplaceBowlAndNutsMsg);
	ADDFN(CRestaurantMusicChanged);
	ADDFN(CSendCCarryMsg);
	ADDFN(CSenseWorkingMsg);
	ADDFN(CServiceElevatorDoor);
	ADDFN(CServiceElevatorFloorChangeMsg);
	ADDFN(CServiceElevatorFloorRequestMsg);
	ADDFN(CServiceElevatorMsg);
	ADDFN(CSetChevButtonImageMsg);
	ADDFN(CSetChevClassBits);
	ADDFN(CSetChevFloorBits);
	ADDFN(CSetChevLiftBits);
	ADDFN(CSetChevPanelBitMsg);
	ADDFN(CSetChevPanelButtonsMsg);
	ADDFN(CSetChevRoomBits);
	ADDFN(CSetMusicControlsMsg);
	ADDFN(CSetVarMsg);
	ADDFN(CSetVolumeMsg);
	ADDFN(CShipSettingMsg);
	ADDFN(CShowTextMsg);
	ADDFN(CSignalObject);
	ADDFN(CSpeechFallsFromTreeMsg);
	ADDFN(CStartMusicMsg);
	ADDFN(CStatusChangeMsg);
	ADDFN(CStopMusicMsg);
	ADDFN(CSubAcceptCCarryMsg);
	ADDFN(CSubDeliverCCarryMsg);
	ADDFN(CSubSendCCarryMsg);
	ADDFN(CSUBTransition);
	ADDFN(CSubTurnOffMsg);
	ADDFN(CSubTurnOnMsg);
	ADDFN(CSummonBotMsg);
	ADDFN(CSummonBotQuerryMsg);
	ADDFN(CTakeHeadPieceMsg);
	ADDFN(CTextInputMsg);
	ADDFN(CTimeDilationMsg);
	ADDFN(CTimeMsg);
	ADDFN(CTitleSequenceEndedMsg);
	ADDFN(CTransitMsg);
	ADDFN(CTransportMsg);
	ADDFN(CTriggerAutoMusicPlayerMsg);
	ADDFN(CTriggerNPCEvent);
	ADDFN(CTrueTalkGetAnimSetMsg);
	ADDFN(CTrueTalkGetAssetDetailsMsg);
	ADDFN(CTrueTalkGetStateValueMsg);
	ADDFN(CTrueTalkNotifySpeechEndedMsg);
	ADDFN(CTrueTalkNotifySpeechStartedMsg);
	ADDFN(CTrueTalkQueueUpAnimSetMsg);
	ADDFN(CTrueTalkSelfQueueAnimSetMsg);
	ADDFN(CTrueTalkTriggerActionMsg);
	ADDFN(CTurnOff);
	ADDFN(CTurnOn);
	ADDFN(CUse);
	ADDFN(CUseWithCharMsg);
	ADDFN(CUseWithOtherMsg);
	ADDFN(CVirtualKeyCharMsg);
	ADDFN(CVisibleMsg);

	ADDFN(CEnterBombRoom);
	ADDFN(CEnterBridge);
	ADDFN(CEnterExitFirstClassState);
	ADDFN(CEnterExitMiniLift);
	ADDFN(CEnterExitSecClassMiniLift);
	ADDFN(CEnterExitView);
	ADDFN(CEnterSecClassState);
	ADDFN(CExitArboretum);
	ADDFN(CExitBridge);
	ADDFN(CExitLift);
	ADDFN(CExitPellerator);
	ADDFN(CExitStateRoom);
	ADDFN(CExitTiania);
	ADDFN(CMovePlayerInParrotRoom);
	ADDFN(CMovePlayerTo);
	ADDFN(CMovePlayerToFrom);
	ADDFN(CMultiMove);
	ADDFN(CPanFromPel);
	ADDFN(CRestaurantPanHandler);
	ADDFN(CScraliontisTable);
	ADDFN(CRestrictedMove);
	ADDFN(CTripDownCanal);

	ADDFN(CBarbot);
	ADDFN(CBellBot);
	ADDFN(CCallBot);
	ADDFN(CDeskbot);
	ADDFN(CDoorbot);
	ADDFN(CMaitreD);
	ADDFN(CLiftBot);
	ADDFN(CMobile);
	ADDFN(CParrot);
	ADDFN(CStarlings);
	ADDFN(CSuccUBus);
	ADDFN(CSummonBots);
	ADDFN(CTitania);

	ADDFN(CAutoMusicPlayer);
	ADDFN(CAutoSoundEvent);
	ADDFN(CAutoMusicPlayerBase);
	ADDFN(CAutoSoundPlayer);
	ADDFN(CAutoSoundPlayerADSR);
	ADDFN(CBackgroundSoundMaker);
	ADDFN(CBilgeAutoSoundEvent);
	ADDFN(CBilgeDispensorEvent);
	ADDFN(CBirdSong);
	ADDFN(CDoorAutoSoundEvent);
	ADDFN(CGondolierSong);
	ADDFN(CEnterViewTogglesOtherMusic);
	ADDFN(CGondolierSong);
	ADDFN(CMusicPlayer);
	ADDFN(CNodeAutoSoundPlayer);
	ADDFN(CRestrictedAutoMusicPlayer);
	ADDFN(CRoomAutoSoundPlayer);
	ADDFN(CSeasonNoises);
	ADDFN(CSeasonalMusicPlayer);
	ADDFN(CAutoMusicPlayer);
	ADDFN(CTitaniaSpeech);
	ADDFN(CTriggerAutoMusicPlayer);
	ADDFN(CViewAutoSoundPlayer);
	ADDFN(CViewTogglesOtherMusic);
	ADDFN(CWaterLappingSounds);
	ADDFN(CStarControl);
}

void CSaveableObject::freeClassList() {
	delete _classList;
}

CSaveableObject *CSaveableObject::createInstance(const Common::String &name) {
	return (*_classList)[name]();
}

void CSaveableObject::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
}

void CSaveableObject::load(SimpleFile *file) {
	file->readNumber();
}

void CSaveableObject::saveHeader(SimpleFile *file, int indent) const {
	file->writeClassStart(getClassName(), indent);
}

void CSaveableObject::saveFooter(SimpleFile *file, int indent) const {
	file->writeClassEnd(indent);
}

} // End of namespace Titanic
