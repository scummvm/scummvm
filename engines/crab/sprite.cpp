#include "stdafx.h"
#include "sprite.h"

using namespace pyrodactyl::ai;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::input;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Sprite::Sprite() : img_size(1, 1), vel_mod(1.0f, 1.0f)
{
	dir = DIRECTION_LEFT;
	layer = -1;
	image = 0;
	Visible(true);
	damage_done = false;
	hover = false;
	//pathing.SetSprite(this);
}

//------------------------------------------------------------------------
// Purpose: Load sprite from XML and animations from the index of all animation files
//------------------------------------------------------------------------
void Sprite::Load(rapidxml::xml_node<char> *node, std::vector<std::string> &animations)
{
	using namespace pyrodactyl::image;

	if (NodeValid(node))
	{
		LoadStr(id, "id", node);

		//The amount by which the sprite x,y should be multiplied
		int multiply = 1;
		LoadNum(multiply, "multiply", node, false);

		LoadNum(pos.x, "x", node);
		LoadNum(pos.y, "y", node);

		pos.x *= multiply;
		pos.y *= multiply;

		LoadImgKey(image, "img", node);
		LoadNum(layer, "layer", node, false);

		Image dat;
		gImageManager.GetTexture(image, dat);
		img_size.x = dat.W();
		img_size.y = dat.H();

		unsigned int index = 0;
		LoadNum(index, "moveset", node);

		if (index < animations.size())
			anim_set.Load(animations.at(index));

		anim_set.fight.ListAttackMoves(ai_data.fight.attack);

		LoadDirection(dir, node);
		clip = anim_set.walk.Clip(dir);
		box_v = anim_set.walk.BoxV(dir);

		if (NodeValid("visible", node, false))
			visible.Load(node->first_node("visible"));

		if (NodeValid("movement", node, false))
			ai_data.walk.Load(node->first_node("movement"));

		if (NodeValid("popup", node, false))
			popup.Load(node->first_node("popup"));
	}
}

//------------------------------------------------------------------------
// Purpose: Move along x and y axis
//------------------------------------------------------------------------
void Sprite::Move(const SpriteConstant &sc)
{
	if (target.x == 0.0f && (vel.x > -sc.tweening && vel.x < sc.tweening))
		vel.x = 0.0f;
	else
	{
		vel.x += (target.x - vel.x) * sc.tweening;
		pos.x += vel.x;
	}

	if (target.y == 0.0f && (vel.y > -sc.tweening && vel.y < sc.tweening))
		vel.y = 0.0f;
	else
	{
		vel.y += (target.y - vel.y) * sc.tweening;
		pos.y += vel.y;
	}
}

//------------------------------------------------------------------------
// Purpose: Resolve collision with shapes excluding the level bounding rectangle
//------------------------------------------------------------------------
void Sprite::ResolveCollide()
{
	//NOTE: we don't check i->intersect here because we do that in the level functions
	for (auto i = collide_data.begin(); i != collide_data.end(); ++i)
	{
		Rect bounds = BoundRect();
		if (i->type == SHAPE_POLYGON)
		{
			pos.x -= i->data.x;
			pos.y -= i->data.y;
		}
		else
		{
			Direction d = bounds.ResolveY(i->data);
			if (d == DIRECTION_UP)
				pos.y -= i->data.y + i->data.h - anim_set.bounds.y + anim_set.AnchorY(dir) + 1;
			else if (d == DIRECTION_DOWN)
				pos.y -= i->data.y - bounds.h - anim_set.bounds.y + anim_set.AnchorY(dir) - 1;

			d = bounds.ResolveX(i->data);
			if (d == DIRECTION_LEFT)
				pos.x -= i->data.x + i->data.w - anim_set.bounds.x + anim_set.AnchorX(dir) + 1;
			else if (d == DIRECTION_RIGHT)
				pos.x -= i->data.x - bounds.w - anim_set.bounds.x + anim_set.AnchorX(dir) - 1;
		}
	}

	//Clear the collision data
	collide_data.clear();
}

//------------------------------------------------------------------------
// Purpose: Resolve collision with the level bounding rectangle
//------------------------------------------------------------------------
void Sprite::ResolveInside(Rect collider)
{
	Rect bounds = BoundRect();
	Direction d = bounds.ResolveX(collider);

	if (d == DIRECTION_RIGHT)
		pos.x = collider.x - anim_set.bounds.x + anim_set.AnchorX(dir) + 1;
	else if (d == DIRECTION_LEFT)
		pos.x = collider.x + collider.w - anim_set.bounds.x - bounds.w + anim_set.AnchorX(dir) - 1;

	bounds = BoundRect();
	d = bounds.ResolveY(collider);

	if (d == DIRECTION_DOWN)
		pos.y = collider.y - anim_set.bounds.y + anim_set.AnchorY(dir) + 1;
	else if (d == DIRECTION_UP)
		pos.y = collider.y + collider.h - anim_set.bounds.y - bounds.h + anim_set.AnchorY(dir) - 1;
}

//------------------------------------------------------------------------
// Purpose: Various rectangles of the sprite
// Note: Every sprite in the game uses bottom center coordinates:
// draw_x = x - clip.w/2, draw_y = y - clip.h
//------------------------------------------------------------------------

//Used for walking and level geometry collision
Rect Sprite::BoundRect()
{
	Rect rect;
	rect.x = pos.x + anim_set.bounds.x - anim_set.AnchorX(dir);
	rect.y = pos.y + anim_set.bounds.y - anim_set.AnchorY(dir);
	rect.w = anim_set.bounds.w;
	rect.h = anim_set.bounds.h;
	return rect;
}

//Used for fighting
Rect Sprite::BoxV()
{
	Rect rect;
	rect.x = pos.x + box_v.x;
	rect.y = pos.y + box_v.y;
	rect.w = box_v.w;
	rect.h = box_v.h;
	return rect;
}

//Used for fighting
Rect Sprite::BoxD()
{
	Rect rect;
	rect.x = pos.x + box_d.x;
	rect.y = pos.y + box_d.y;
	rect.w = box_d.w;
	rect.h = box_d.h;
	return rect;
}

//Used for drawing object notifications over stuff
Rect Sprite::PosRect()
{
	Rect rect;
	rect.x = pos.x - anim_set.AnchorX(dir);
	rect.y = pos.y - anim_set.AnchorY(dir);
	rect.w = clip.w;
	rect.h = clip.h;
	return rect;
}

//The range rectangle is relative to the bounding rectangle, and needs to be updated according to the actual position
//Used for enemy sprite to find their required spot to attack the player
Rect Sprite::RangeRect(const Rect &bounds, const Range &range)
{
	Rect rect;
	rect.x = bounds.x + range.val[dir].x;
	rect.y = bounds.y + range.val[dir].y;
	rect.w = range.val[dir].w;
	rect.h = range.val[dir].h;

	return rect;
}

//Used for focusing the camera on the sprite
Vector2i Sprite::CamFocus()
{
	Vector2i v;
	v.x = pos.x + anim_set.focus.x;
	v.y = pos.y + anim_set.focus.y;

	return v;
}

//------------------------------------------------------------------------
// Purpose: Draw the sprite
//------------------------------------------------------------------------
void Sprite::Draw(pyrodactyl::event::Info &info, const Rect &camera)
{
	using namespace pyrodactyl::image;
	using namespace pyrodactyl::text;

	int x = pos.x - camera.x - anim_set.AnchorX(dir), y = pos.y - camera.y - anim_set.AnchorY(dir);

	//Draw the shadow image relative to the bottom center of the sprite
	ShadowOffset sh = anim_set.Shadow(dir);
	if (sh.valid)
	{
		//Draw using custom offset
		gImageManager.Draw(x + clip.w / 2 - anim_set.shadow.size.x + sh.x,
			y + clip.h - anim_set.shadow.size.y + sh.y,
			anim_set.shadow.img);
	}
	else
	{
		//Draw using default offset
		gImageManager.Draw(x + clip.w / 2 - anim_set.shadow.size.x + anim_set.shadow.offset.x,
			y + clip.h - anim_set.shadow.size.y + anim_set.shadow.offset.y,
			anim_set.shadow.img);
	}

	gImageManager.Draw(x, y, image, &clip, anim_set.Flip(dir));
	img_eff.Draw(x, y);

	if (GameDebug)
	{
		//Nice boxes for the frames and box_v, box_d
		Rect bounds = BoundRect(), vul = BoxV(), dmg = BoxD(), pos = PosRect();
		bounds.Draw(-camera.x, -camera.y);
		pos.Draw(-camera.x, -camera.y, 255, 255, 255);
		dmg.Draw(-camera.x, -camera.y, 255, 0, 0);
		vul.Draw(-camera.x, -camera.y, 0, 0, 255);

		FightMove fm;
		if (anim_set.fight.NextMove(fm))
		{
			Rect actual_range;
			actual_range.x = bounds.x + fm.ai.range.val[dir].x;
			actual_range.y = bounds.y + fm.ai.range.val[dir].y;
			actual_range.w = fm.ai.range.val[dir].w;
			actual_range.h = fm.ai.range.val[dir].h;

			actual_range.Draw(-camera.x, -camera.y, 255, 0, 255);
		}

		/*int health = 0;
		info.StatGet(id, pyrodactyl::stat::STAT_HEALTH, health);
		gTextManager.Draw(x + 60.0f, y - 100.0f, NumberToString(health), 0);

		pyrodactyl::people::PersonState state = info.State(id);
		gTextManager.Draw(x, y - 60.0f, NumberToString(state), 0);

		gTextManager.Draw(x + 120.0f, y - 60.0f, NumberToString(ai_data.dest.y), 0);*/

		if (pathing.m_vSolution.size() > 0)
		{
			for (auto iter = pathing.m_vSolution.begin(); iter != pathing.m_vSolution.end(); ++iter)
			{
				bool nextToWall = false;
				for (auto neighbor = (*iter)->neighborNodes.begin(); neighbor != (*iter)->neighborNodes.end(); ++neighbor)
				{
					if ((*neighbor)->GetMovementCost() < 0 || (*neighbor)->GetMovementCost() > 1)
					{
						nextToWall = true;
						break;
					}
				}

				if (nextToWall)
					(*iter)->GetRect().Draw(-camera.x, -camera.y, 0, 0, 0, 254);
				else
					(*iter)->GetRect().Draw(-camera.x, -camera.y, 200, 200, 0, 254);
			}

			pathing.m_pGoalTile->GetRect().Draw(-camera.x, -camera.y, 0, 0, 200, 254);
			pathing.m_pStartTile->GetRect().Draw(-camera.x, -camera.y, 0, 200, 0, 254);

			//Draw adjacencies to the goal tile.
			/*	for(auto neighbor = pathing.m_pGoalTile->neighborNodes.begin(); neighbor != pathing.m_pGoalTile->neighborNodes.end(); ++neighbor)
				{
				(*neighbor)->GetRect().Draw(-camera.x, -camera.y, 200, 0, 0, 254);
				}*/
		}
		Rect destinationRect = Rect((int)pathing.destination.x - 5,
			(int)pathing.destination.y - 5,
			10,
			10);
		destinationRect.Draw(-camera.x, -camera.y, 0, 200, 0, 254);
	}
}

void Sprite::DrawPopup(pyrodactyl::ui::ParagraphData &pop, const Rect &camera)
{
	//This is different from draw because we draw the popup centered over the head
	int x = pos.x - camera.x - anim_set.AnchorX(dir) + (anim_set.bounds.w / 2);
	int y = pos.y - camera.y - anim_set.AnchorY(dir);

	popup.Draw(x, y, pop, camera);
}
//------------------------------------------------------------------------
// Purpose: Handle the movement in a level for the player only
//------------------------------------------------------------------------
void Sprite::HandleEvents(Info &info, const Rect &camera, const SpriteConstant &sc, const SDL_Event &Event)
{
	int num = 0;
	info.StatGet(id, pyrodactyl::stat::STAT_SPEED, num); ++num;
	float player_speed = static_cast<float>(num);

	//This is for Diablo style hold-mouse-button-in-direction-of-movement
	//This is only used if - point and click movement isn't being used, cursor is not inside the hud, the cursor is a normal cursor and the mouse is pressed
	if (!ai_data.dest.active && !gMouse.inside_hud && !gMouse.hover && gMouse.Pressed())
	{
		//To find where the click is w.r.t sprite, we need to see where it is being drawn
		int x = pos.x - camera.x - anim_set.AnchorX(dir), y = pos.y - camera.y - anim_set.AnchorY(dir);

		//Just use the bound rectangle dimensions
		Rect b = BoundRect();
		int w = b.w, h = b.h;

		//X axis
		if (gMouse.motion.x > x + w)  XVel(player_speed * sc.walk_vel_mod.x);
		else if (gMouse.motion.x < x) XVel(-player_speed * sc.walk_vel_mod.x);
		else                          XVel(0.0f);

		//Y axis
		if (gMouse.motion.y > y + h)  YVel(player_speed * sc.walk_vel_mod.y);
		else if (gMouse.motion.y < y) YVel(-player_speed * sc.walk_vel_mod.y);
		else                          YVel(0.0f);
	}
	else //Keyboard movement
	{
		//Disable destination as soon as player presses a direction key
		//X axis
		if (gInput.State(IG_LEFT))
		{
			ai_data.dest.active = false;
			XVel(-player_speed * sc.walk_vel_mod.x);
		}
		else if (gInput.State(IG_RIGHT))
		{
			ai_data.dest.active = false;
			XVel(player_speed * sc.walk_vel_mod.x);
		}
		else if (!ai_data.dest.active)
			XVel(0.0f);

		//Y axis
		if (gInput.State(IG_UP))
		{
			ai_data.dest.active = false;
			YVel(-player_speed * sc.walk_vel_mod.y);
		}
		else if (gInput.State(IG_DOWN))
		{
			ai_data.dest.active = false;
			YVel(player_speed * sc.walk_vel_mod.y);
		}
		else if (!ai_data.dest.active)
			YVel(0.0f);
	}

	UpdateMove(input.HandleEvents(Event));

	//This is to prevent one frame of drawing with incorrect parameters
	Animate(info);
}

//------------------------------------------------------------------------
// Purpose: Set destination for sprite movement
//------------------------------------------------------------------------
void Sprite::SetDestPathfinding(const Vector2i &dest, bool reachable)
{
	ai_data.Dest(dest, true);
	pathing.SetDestination(dest, reachable);
}

//------------------------------------------------------------------------
// Purpose: Walking animation
//------------------------------------------------------------------------
void Sprite::Walk(const pyrodactyl::people::PersonState &pst)
{
	img_eff.visible = false;

	bool first_x = true;

	if (ai_data.dest.active)
	{
		Rect b = BoundRect();
		if (pos.x - ai_data.dest.x > -b.w && pos.x - ai_data.dest.x < b.w)
			first_x = false;
	}

	bool reset = anim_set.walk.Type(vel, dir, pst, first_x);

	if (reset)
		anim_set.walk.ResetClip(dir);

	Walk(reset);
}

void Sprite::Walk(const bool &reset)
{
	if (anim_set.walk.UpdateClip(dir, reset))
	{
		clip = anim_set.walk.Clip(dir);
		box_v = anim_set.walk.BoxV(dir);
	}
}

//------------------------------------------------------------------------
// Purpose: Decide which animation to play
//------------------------------------------------------------------------
void Sprite::Animate(Info &info)
{
	if (input.Idle())
		Walk(info.State(id));
	else
		UpdateFrame(info.State(id));
}

void Sprite::Animate(const pyrodactyl::people::PersonState &pst)
{
	Walk(pst);
}

//------------------------------------------------------------------------
// Purpose: We need to find if the vulnerable area of this sprite collides
// with hitbox (usually the damage area of another sprite)
//------------------------------------------------------------------------
bool Sprite::FightCollide(Rect hitbox, Rect enemy_bounds, Range &range, const SpriteConstant &sc)
{
	Rect bounds = BoundRect();

	if (range.valid)
	{
		Rect actual_range = RangeRect(bounds, range);

		//The second part is a sanity check so the stray hitbox of a sprite 1000 pixels below does not cause damage
		if (hitbox.Collide(actual_range) && abs(bounds.y + bounds.h - enemy_bounds.y - enemy_bounds.h) < sc.plane_w)
			return true;
	}
	else
	{
		if (hitbox.Collide(bounds) && abs(bounds.y + bounds.h - enemy_bounds.y - enemy_bounds.h) < sc.plane_w)
			return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Update the frame info of the sprite
//------------------------------------------------------------------------
void Sprite::UpdateFrame(const pyrodactyl::people::PersonState &pst, const bool &repeat)
{
	FrameUpdateResult res = anim_set.fight.UpdateFrame(dir);
	if (res == FUR_SUCCESS)
	{
		AssignFrame();
	}
	else if (res == FUR_FAIL)
	{
		damage_done = false;
		Stop();

		if (repeat == false)
			ResetFrame(pst);
		else
			anim_set.fight.FrameIndex(0);
	}
}

void Sprite::AssignFrame()
{
	FightAnimFrame faf;
	if (anim_set.fight.CurFrame(faf, dir))
	{
		clip = faf.clip;
		BoxV(faf.box_v);
		BoxD(faf.box_d);

		pos.x += faf.delta.x;
		pos.y += faf.delta.y;

		input.state = faf.state;
	}
}

//------------------------------------------------------------------------
// Purpose: Update the move info of the player sprite
//------------------------------------------------------------------------
void Sprite::UpdateMove(const FightAnimationType &combo)
{
	if (combo != FA_IDLE)
	{
		if (input.Idle())
			ForceUpdateMove(combo);
		else
		{
			FightAnimFrame faf;
			if (anim_set.fight.CurFrame(faf, dir))
				if (faf.branch)
					ForceUpdateMove(combo);
		}
	}
}

void Sprite::ForceUpdateMove(const FightAnimationType &combo)
{
	unsigned int index = anim_set.fight.FindMove(combo, input.state);
	ForceUpdateMove(index);
}

//------------------------------------------------------------------------
// Purpose: Update the move info of the AI or player sprite
//------------------------------------------------------------------------
void Sprite::UpdateMove(const unsigned int &index)
{
	if (input.Idle())
		ForceUpdateMove(index);
}

void Sprite::ForceUpdateMove(const unsigned int &index)
{
	if (anim_set.fight.ForceUpdate(index, input, dir))
	{
		//This sets the sprite input to the current move input
		anim_set.fight.CurCombo(input);

		Stop();
		AssignFrame();
	}
	else
		input.Reset();
}
//------------------------------------------------------------------------
// Purpose: Reset the frame info of the sprite
//------------------------------------------------------------------------
void Sprite::ResetFrame(const pyrodactyl::people::PersonState &pst)
{
	input.Reset();
	Walk(true);
	anim_set.fight.Reset();

	box_d.w = 0;
	box_d.h = 0;
}

//------------------------------------------------------------------------
// Purpose: Check if both sprites in the same plane
//------------------------------------------------------------------------
bool Sprite::DamageValid(Sprite &s, const SpriteConstant &sc)
{
	//Get the y coordinates where these sprites are standing
	float Y = pos.y + clip.h, SY = s.pos.y + s.clip.h;

	if (abs(Y - SY) < sc.plane_w)
		return true;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Checks about dealing damage to sprite
//------------------------------------------------------------------------
void Sprite::CalcProperties(Info &info)
{
	visible.Evaluate(info);
	anim_set.fight.Evaluate(info);
}

//------------------------------------------------------------------------
// Purpose: Checks about dealing damage to sprite
//------------------------------------------------------------------------
bool Sprite::TakingDamage(Sprite &sp, const SpriteConstant &sc)
{
	if (DamageValid(sp, sc))
		if (BoxV().w > 0 && BoxV().h > 0 && sp.BoxD().w > 0 && sp.BoxD().h > 0)
			if (BoxV().Collide(sp.BoxD()))
				return true;

	/*std::string words = NumberToString(BoxV().x) + " " + NumberToString(BoxV().y)+ " " + NumberToString(BoxV().w)
	+ " " + NumberToString(BoxV().h) + "\n" + NumberToString(sp.BoxD().x) + " " + NumberToString(sp.BoxD().y)
	+ " " + NumberToString(sp.BoxD().w) + " " + NumberToString(sp.BoxD().h) + "\n";
	fprintf(stdout,words.c_str());*/

	return false;
}

//------------------------------------------------------------------------
// Purpose: We know we are taking damage, now is the time to update stats
//------------------------------------------------------------------------
void Sprite::TakeDamage(Info &info, Sprite &s)
{
	using namespace pyrodactyl::stat;
	using namespace pyrodactyl::music;

	FightMove f;
	if (s.anim_set.fight.CurMove(f) && info.PersonValid(s.ID()) && info.PersonValid(id))
	{
		int dmg = -1 * (f.eff.dmg + info.PersonGet(s.ID()).stat.val[STAT_ATTACK].cur - info.PersonGet(id).stat.val[STAT_DEFENSE].cur);
		if (dmg >= 0)
			dmg = -1;

		info.StatChange(id, STAT_HEALTH, dmg);

		int health = 1;
		info.StatGet(id, STAT_HEALTH, health);

		//Play death animation if dead, hurt animation otherwise
		if (health <= 0 && f.eff.death != -1)
			ForceUpdateMove(f.eff.death);
		else if (f.eff.hurt != -1)
			ForceUpdateMove(f.eff.hurt);

		gMusicManager.PlayEffect(f.eff.hit, 0);
		img_eff = f.eff.img;
	}

	Stop();
	s.damage_done = true;
}

//------------------------------------------------------------------------
// Purpose: We have 2 sprites, *this and s.
// Check damage between s.box_v and this->box_d and vice versa
//------------------------------------------------------------------------
void Sprite::ExchangeDamage(Info &info, Sprite &s, const SpriteConstant &sc)
{
	using namespace pyrodactyl::people;
	using namespace pyrodactyl::stat;

	//This object is taking damage from s
	if (!s.damage_done && TakingDamage(s, sc))
		TakeDamage(info, s);

	//Is the other sprite taking damage from this sprite?
	if (!damage_done && s.TakingDamage(*this, sc))
		s.TakeDamage(info, *this);

	//We change the animation to dying in order to give time to the death animation to play out
	int num = 0;
	info.StatGet(s.ID(), STAT_HEALTH, num);
	if (num <= 0)
	{
		info.State(s.ID(), PST_DYING);
		info.StatChange(s.ID(), STAT_HEALTH, 1);
	}

	info.StatGet(id, STAT_HEALTH, num);
	if (num <= 0)
	{
		info.State(id, PST_DYING);
		info.StatChange(id, STAT_HEALTH, 1);
	}
}

//------------------------------------------------------------------------
// Purpose: Update status of ambient dialog via popup object
//------------------------------------------------------------------------
void Sprite::InternalEvents(Info &info, const std::string &player_id, std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq)
{
	popup.InternalEvents(info, player_id, result, end_seq);
}

//------------------------------------------------------------------------
// Purpose: Save all sprite positions to save file
//------------------------------------------------------------------------
void Sprite::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	root->append_attribute(doc.allocate_attribute("id", id.c_str()));
	root->append_attribute(doc.allocate_attribute("x", gStrPool.Get(pos.x)));
	root->append_attribute(doc.allocate_attribute("y", gStrPool.Get(pos.y)));
}

//------------------------------------------------------------------------
// Purpose: Load all sprite positions from save file
//------------------------------------------------------------------------
void Sprite::LoadState(rapidxml::xml_node<char> *node)
{
	LoadNum(pos.x, "x", node);
	LoadNum(pos.y, "y", node);
}