#pragma once

#include "GameEventInfo.h"
#include "ImageManager.h"
#include "ParagraphData.h"
#include "PathfindingGrid.h"
#include "TMXMap.h"
#include "animset.h"
#include "button.h"
#include "common_header.h"
#include "fightmove.h"
#include "level_objects.h"
#include "person.h"
#include "sprite.h"
#include "talknotify.h"

namespace pyrodactyl {
namespace level {
class Level {
	// The .tmx file to import terrain from
	TMX::TMXMap terrain;

	// The pathfinding grid for the level
	PathfindingGrid pathfindingGrid;

	// The area we display to the player
	Rect camera;

	// The player sprite location in the object list
	unsigned int player_index;

	// The order in which to draw the sprites
	std::multimap<int, pyrodactyl::anim::Sprite *> obj_seq;

	// The file index which contains the fighting moves of all characters
	std::vector<std::string> anim_set;

	// The movement sets for sprites in levels
	std::vector<pyrodactyl::ai::MovementSet> move_set;

	// These sprites are only for animated objects and cannot be interacted with
	std::vector<pyrodactyl::anim::Sprite> background;

	// These sprites fly across the screen randomly
	std::vector<pyrodactyl::anim::Sprite> fly;

	// The id of the music track being played
	MusicInfo music;

	// Is the world map accessible from this level?
	MapVis showmap;

	// We disable exits when player is fighting enemies - this is used to check if we are in an exit area WHILE fighting enemies
	// If we are fighting inside exit area, the level switch is delayed until we walk out of exit and back in
	// to prevent instant level switch as soon as you beat all enemies
	bool inside_exit;

	bool first_hit;

	// Default sprite parameters
	pyrodactyl::ai::SpriteConstant sc_default;

	// Protected level functions
	bool CollidingWithObject(pyrodactyl::event::Info &info, std::string &id);
	bool CollidingWithLevel(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);
	bool PlayerInCombat(pyrodactyl::event::Info &info);
	void BattleAlert(pyrodactyl::event::Info &info);

	void SortObjectsToDraw();
	void MoveObject(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);

	void Think(pyrodactyl::event::Info &info, std::vector<pyrodactyl::event::EventResult> &result,
			   std::vector<pyrodactyl::event::EventSeqInfo> &end_seq, std::string &id);

	void DrawObjects(pyrodactyl::event::Info &info);
	void SetCamera();

	bool LayerVisible(pyrodactyl::anim::Sprite *obj);

public:
	// The objects in the level, and the player character
	std::vector<pyrodactyl::anim::Sprite> objects;

	// The notification text drawn if the player is able to talk to a sprite
	TalkNotify talk_notify;

	// Used for drawing the destination marker for point and click movement
	PlayerDestMarker dest_marker;

	// The location of this level on the world map
	Vector2i map_loc;

	// The clip revealed by this level on the world map
	struct MapClip {
		// Which world map is this clip added to?
		int id;

		// The clip itself
		Rect rect;

		MapClip() { id = 0; }
	} map_clip;

	// Used to draw ambient dialog
	pyrodactyl::ui::ParagraphData pop;

	// The path of the preview image
	std::string preview_path;

	Level() { Reset(); }
	~Level() {}

	void Reset();

	void Camera(int x, int y, int w, int h) {
		camera.x = x;
		camera.y = y;
		camera.w = w;
		camera.h = h;
	}
	Rect Camera() { return camera; }

	void PlayerStop() { objects[player_index].Stop(); }

	const std::string &PlayerID() { return objects[player_index].ID(); }
	void PlayerID(const std::string &ID, const int &X, const int &Y);

	void ShowMap(bool val) { showmap.normal = val; }
	bool ShowMap() { return showmap.current; }

	bool operator()(int i, int j);

	// This calculates the unlocked moves for each sprite in the level, and the visibility of objects
	void CalcProperties(pyrodactyl::event::Info &info);

	// Loading function
	void Load(const std::string &filename, pyrodactyl::event::Info &info, pyrodactyl::event::TriggerSet &game_over,
			  const int &player_x = -1, const int &player_y = -1);

	// One time load called first-time
	void LoadMoves(const std::string &filename);
	void LoadConst(const std::string &filename);

	// Used to see if a sprite collides with a rectangle
	void CalcTrigCollide(pyrodactyl::event::Info &info);

	// See if a player clicked on the sprite in contact
	bool ContainsClick(const std::string &id, const SDL_Event &Event);

	// Get index of a sprite in the object array
	pyrodactyl::anim::Sprite *GetSprite(const std::string &id);

	void HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
	LevelResult InternalEvents(pyrodactyl::event::Info &info, std::vector<pyrodactyl::event::EventResult> &result,
							   std::vector<pyrodactyl::event::EventSeqInfo> &end_seq, bool EventInProgress);

	void Draw(pyrodactyl::event::Info &info);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace level
} // End of namespace pyrodactyl