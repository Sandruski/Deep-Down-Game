#include "p2Defs.h"
#include "p2Log.h"

#include "j1Module.h"
#include "j1App.h"

#include "j1EntityFactory.h"
#include "j1Render.h"

#include "Entity.h"
#include "Imp.h"
#include "CatPeasant.h"
#include "Monkey.h"
#include "Cat.h"
#include "j1Textures.h"
#include "j1Scene.h"
#include "j1Map.h"
#include "j1Pathfinding.h"
#include "j1Collision.h"
#include "j1Gui.h"

#define SPAWN_MARGIN 50

j1EntityFactory::j1EntityFactory()
{
	name.create("entities");

	for (uint i = 0; i < MAX_ENTITIES; ++i)
		entities[i] = nullptr;

}

// Destructor
j1EntityFactory::~j1EntityFactory()
{
}

bool j1EntityFactory::Awake(pugi::xml_node& config) {

	bool ret = true;
	
	pugi::xml_node node = config.child("spritesheets").child("spritesheet");

	// Load texture paths
	CatPeasant_spritesheet = node.attribute("name").as_string();
	node = node.next_sibling("spritesheet");
	Monkey_spritesheet = node.attribute("name").as_string();
	node = node.next_sibling("spritesheet");
	Imp_spritesheet = node.attribute("name").as_string();
	node = node.next_sibling("spritesheet");
	Player_spritesheet = node.attribute("name").as_string();
	node = node.next_sibling("spritesheet");
	Cat_spritesheet = node.attribute("name").as_string();
	//_load_texture_paths

	//PLAYER
	pugi::xml_node general_node = config.child("types").child("player").child("general");
	pugi::xml_node actual_node;

	actual_node = general_node.child("coll_offset");
	player.coll_offset = { actual_node.attribute("x").as_int(), actual_node.attribute("y").as_int(), actual_node.attribute("w").as_int(), actual_node.attribute("h").as_int() };
	player.gravity = general_node.child("gravity").attribute("value").as_float();
	actual_node = general_node.child("speed");
	player.speed = { actual_node.attribute("x").as_float(), actual_node.attribute("y").as_float() };
	player.check_collision_offset = general_node.child("check_collision").attribute("offset").as_uint();

	// Load animations
	pugi::xml_node animations_node = config.child("types").child("player").child("animations");

	//idle
	node = animations_node.child("idle");
	player.idle.speed = node.attribute("speed").as_float();
	player.idle.loops = node.attribute("loops").as_bool();
	player.coll_size = { node.child("frame").attribute("w").as_int(), node.child("frame").attribute("h").as_int() };
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//idle2
	node = animations_node.child("idle2");
	player.idle2.speed = node.attribute("speed").as_float();
	player.idle2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.idle2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//forward
	node = animations_node.child("forward");
	player.forward.speed = node.attribute("speed").as_float();
	player.forward.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.forward.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//backward
	node = animations_node.child("backward");
	player.backward.speed = node.attribute("speed").as_float();
	player.backward.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.backward.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//crouch
	node = animations_node.child("crouch");
	player.crouch.speed = node.attribute("speed").as_float();
	player.crouch.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.crouch.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//crouch2
	node = animations_node.child("crouch2");
	player.crouch2.speed = node.attribute("speed").as_float();
	player.crouch2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.crouch2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//shot
	node = animations_node.child("shot");
	player.shot.speed = node.attribute("speed").as_float();
	player.shot.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.shot.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//shot2
	node = animations_node.child("shot2");
	player.shot2.speed = node.attribute("speed").as_float();
	player.shot2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.shot2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//crouchShot
	node = animations_node.child("crouchShot");
	player.crouchShot.speed = node.attribute("speed").as_float();
	player.crouchShot.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.crouchShot.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//crouchShot2
	node = animations_node.child("crouchShot2");
	player.crouchShot2.speed = node.attribute("speed").as_float();
	player.crouchShot2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.crouchShot2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//jump
	node = animations_node.child("jump");
	player.jump.speed = node.attribute("speed").as_float();
	player.jump.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.jump.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//jump2
	node = animations_node.child("jump2");
	player.jump2.speed = node.attribute("speed").as_float();
	player.jump2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.jump2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//punished
	node = animations_node.child("punished");
	player.punished.speed = node.attribute("speed").as_float();
	player.punished.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.punished.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//punished2
	node = animations_node.child("punished2");
	player.punished2.speed = node.attribute("speed").as_float();
	player.punished2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.punished2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}


	//dash
	node = animations_node.child("dash");
	player.dash.speed = node.attribute("speed").as_float();
	player.dash.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.dash.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//dash2
	node = animations_node.child("dash2");
	player.dash2.speed = node.attribute("speed").as_float();
	player.dash2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.dash2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//firstAttack
	node = animations_node.child("firstAttack");
	player.firstAttack.speed = node.attribute("speed").as_float();
	player.firstAttack.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.firstAttack.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//firstAttack2
	node = animations_node.child("firstAttack2");
	player.firstAttack2.speed = node.attribute("speed").as_float();
	player.firstAttack2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.firstAttack2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//secondAttack
	node = animations_node.child("secondAttack");
	player.secondAttack.speed = node.attribute("speed").as_float();
	player.secondAttack.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.secondAttack.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//secondAttack2
	node = animations_node.child("secondAttack2");
	player.secondAttack2.speed = node.attribute("speed").as_float();
	player.secondAttack2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.secondAttack2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//thirdAttack
	node = animations_node.child("thirdAttack");
	player.thirdAttack.speed = node.attribute("speed").as_float();
	player.thirdAttack.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.thirdAttack.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//thirdAttack2
	node = animations_node.child("thirdAttack2");
	player.thirdAttack2.speed = node.attribute("speed").as_float();
	player.thirdAttack2.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		player.thirdAttack2.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}
	//_load_animations
	//_PLAYER

	// IMP
	general_node = config.child("types").child("player").child("general");

	actual_node = general_node.child("coll_offset");
	imp.coll_offset = { actual_node.attribute("x").as_int(), actual_node.attribute("y").as_int(), actual_node.attribute("w").as_int(), actual_node.attribute("h").as_int() };
	imp.lives = general_node.child("lives").attribute("value").as_int();
	actual_node = general_node.child("pathfinding_affect_area");
	imp.enemy_pathfinding_affect_area = { actual_node.child("enemy").attribute("x").as_int(), actual_node.child("enemy").attribute("y").as_int(),actual_node.child("enemy").attribute("w").as_int(),actual_node.child("enemy").attribute("h").as_int() };
	imp.player_pathfinding_affect_area = { actual_node.child("player").attribute("x").as_int(), actual_node.child("player").attribute("y").as_int(),actual_node.child("player").attribute("w").as_int(),actual_node.child("player").attribute("h").as_int() };
	actual_node = general_node.child("pathfinding_speed");
	imp.pathfinding_slow_speed = actual_node.attribute("slow").as_float();
	imp.pathfinding_normal_speed = actual_node.attribute("normal").as_float();
	imp.pathfinding_fast_speed = actual_node.attribute("fast").as_float();
	imp.min_distance_to_pathfind = general_node.child("min_distance_to_pathfind").attribute("value").as_int();
	imp.min_distance_to_shoot = general_node.child("min_distance_to_shoot").attribute("value").as_int();
	imp.distance_to_player = general_node.child("distance_to_player").attribute("value").as_int();
	imp.seconds_to_wait = general_node.child("seconds_to_wait").attribute("value").as_int();
	imp.scene1_pathfinding_start = general_node.child("scene1_pathfinding_start").attribute("x").as_int();
	imp.particle_speed = { general_node.child("particle_speed").attribute("x").as_float(), general_node.child("particle_speed").attribute("y").as_float() };
	imp.hurt_fx = general_node.child("hurt_fx").attribute("value").as_uint();

	// Load animations
	animations_node = config.child("types").child("imp").child("animations");

	//r_shield_idle
	node = animations_node.child("r_shield_idle");
	imp.r_shield_idle.speed = node.attribute("speed").as_float();
	imp.r_shield_idle.loops = node.attribute("loops").as_bool();
	imp.coll_size = { node.child("frame").attribute("w").as_int(), node.child("frame").attribute("h").as_int() };
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.r_shield_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_shield_idle
	node = animations_node.child("l_shield_idle");
	imp.l_shield_idle.speed = node.attribute("speed").as_float();
	imp.l_shield_idle.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.l_shield_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_shield_hurt
	node = animations_node.child("r_shield_hurt");
	imp.r_shield_hurt.speed = node.attribute("speed").as_float();
	imp.r_shield_hurt.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.r_shield_hurt.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_shield_hurt
	node = animations_node.child("l_shield_hurt");
	imp.l_shield_hurt.speed = node.attribute("speed").as_float();
	imp.l_shield_hurt.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.l_shield_hurt.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_jump
	node = animations_node.child("r_jump");
	imp.r_jump.speed = node.attribute("speed").as_float();
	imp.r_jump.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.r_jump.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_jump
	node = animations_node.child("l_jump");
	imp.l_jump.speed = node.attribute("speed").as_float();
	imp.l_jump.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.l_jump.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_throw_bomb
	node = animations_node.child("r_throw_bomb");
	imp.r_throw_bomb.speed = node.attribute("speed").as_float();
	imp.r_throw_bomb.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.r_throw_bomb.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_throw_bomb
	node = animations_node.child("l_throw_bomb");
	imp.l_throw_bomb.speed = node.attribute("speed").as_float();
	imp.l_throw_bomb.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.l_throw_bomb.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_shield_walk
	node = animations_node.child("r_shield_walk");
	imp.r_shield_walk.speed = node.attribute("speed").as_float();
	imp.r_shield_walk.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.r_shield_walk.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_shield_walk
	node = animations_node.child("l_shield_walk");
	imp.l_shield_walk.speed = node.attribute("speed").as_float();
	imp.l_shield_walk.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.l_shield_walk.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//invisible
	node = animations_node.child("invisible");
	imp.invisible.speed = node.attribute("speed").as_float();
	imp.invisible.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		imp.invisible.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}
	//_load_animations
	//_IMP

	// MONKEY
	general_node = config.child("types").child("monkey").child("general");
	
	actual_node = general_node.child("coll_offset");
	monkey.coll_offset = { actual_node.attribute("x").as_int(), actual_node.attribute("y").as_int(), actual_node.attribute("w").as_int(), actual_node.attribute("h").as_int() };
	monkey.lives = general_node.child("lives").attribute("value").as_int();
	actual_node = general_node.child("pathfinding_affect_area");
	monkey.enemy_pathfinding_affect_area = { actual_node.child("enemy").attribute("x").as_int(), actual_node.child("enemy").attribute("y").as_int(),actual_node.child("enemy").attribute("w").as_int(),actual_node.child("enemy").attribute("h").as_int() };
	monkey.player_pathfinding_affect_area = { actual_node.child("player").attribute("x").as_int(), actual_node.child("player").attribute("y").as_int(),actual_node.child("player").attribute("w").as_int(),actual_node.child("player").attribute("h").as_int() };
	actual_node = general_node.child("pathfinding_speed");
	monkey.pathfinding_slow_speed = actual_node.attribute("slow").as_float();
	monkey.pathfinding_normal_speed = actual_node.attribute("normal").as_float();
	monkey.min_distance_to_hit = general_node.child("min_distance_to_hit").attribute("value").as_int();
	monkey.distance_to_player = general_node.child("distance_to_player").attribute("value").as_int();
	monkey.particle_speed = { general_node.child("particle_speed").attribute("x").as_float(), general_node.child("particle_speed").attribute("y").as_float() };
	monkey.hurt_fx = general_node.child("hurt_fx").attribute("value").as_uint();

	// Load animations
	animations_node = config.child("types").child("monkey").child("animations");

	//r_idle
	node = animations_node.child("r_idle");
	monkey.r_idle.speed = node.attribute("speed").as_float();
	monkey.r_idle.loops = node.attribute("loops").as_bool();
	monkey.coll_size = { node.child("frame").attribute("w").as_int(), node.child("frame").attribute("h").as_int() };
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		monkey.r_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_idle
	node = animations_node.child("l_idle");
	monkey.l_idle.speed = node.attribute("speed").as_float();
	monkey.l_idle.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		monkey.l_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_hurt
	node = animations_node.child("r_hurt");
	monkey.r_hurt.speed = node.attribute("speed").as_float();
	monkey.r_hurt.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		monkey.r_hurt.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_hurt
	node = animations_node.child("l_hurt");
	monkey.l_hurt.speed = node.attribute("speed").as_float();
	monkey.l_hurt.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		monkey.l_hurt.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_hit
	node = animations_node.child("r_hit");
	monkey.r_hit.speed = node.attribute("speed").as_float();
	monkey.r_hit.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		monkey.r_hit.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_hit
	node = animations_node.child("l_hit");
	monkey.l_hit.speed = node.attribute("speed").as_float();
	monkey.l_hit.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		monkey.l_hit.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}
	//_load_animations
	//_MONKEY

	// CAT PEASANT
	general_node = config.child("types").child("cat_peasant").child("general");

	actual_node = general_node.child("coll_offset");
	catPeasant.coll_offset = { actual_node.attribute("x").as_int(), actual_node.attribute("y").as_int(), actual_node.attribute("w").as_int(), actual_node.attribute("h").as_int() };
	catPeasant.lives = general_node.child("lives").attribute("value").as_int();
	catPeasant.pathfinding_normal_speed = actual_node.child("pathfinding_speed").attribute("normal").as_float();
	catPeasant.min_distance_to_shoot = general_node.child("min_distance_to_shoot").attribute("value").as_int();
	catPeasant.distance_to_player = general_node.child("distance_to_player").attribute("value").as_int();
	catPeasant.seconds_to_wait = general_node.child("seconds_to_wait").attribute("value").as_int();
	catPeasant.particle_speed = { general_node.child("particle_speed").attribute("x").as_float(), general_node.child("particle_speed").attribute("y").as_float() };
	catPeasant.hurt_fx = general_node.child("hurt_fx").attribute("value").as_uint();

	// Load animations
	animations_node = config.child("types").child("cat_peasant").child("animations");

	//r_idle
	node = animations_node.child("r_idle");
	catPeasant.r_idle.speed = node.attribute("speed").as_float();
	catPeasant.r_idle.loops = node.attribute("loops").as_bool();
	catPeasant.coll_size = { node.child("frame").attribute("w").as_int(), node.child("frame").attribute("h").as_int() };
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_idle
	node = animations_node.child("l_idle");
	catPeasant.l_idle.speed = node.attribute("speed").as_float();
	catPeasant.l_idle.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_idle_no_staff
	node = animations_node.child("r_idle_no_staff");
	catPeasant.r_idle_no_staff.speed = node.attribute("speed").as_float();
	catPeasant.r_idle_no_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_idle_no_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_idle_no_staff
	node = animations_node.child("l_idle_no_staff");
	catPeasant.l_idle_no_staff.speed = node.attribute("speed").as_float();
	catPeasant.l_idle_no_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_idle_no_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_hurt
	node = animations_node.child("r_hurt");
	catPeasant.r_hurt.speed = node.attribute("speed").as_float();
	catPeasant.r_hurt.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_hurt.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_hurt
	node = animations_node.child("l_hurt");
	catPeasant.l_hurt.speed = node.attribute("speed").as_float();
	catPeasant.l_hurt.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_hurt.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_hurt_no_staff
	node = animations_node.child("r_hurt_no_staff");
	catPeasant.r_hurt_no_staff.speed = node.attribute("speed").as_float();
	catPeasant.r_hurt_no_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_hurt_no_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_hurt_no_staff
	node = animations_node.child("l_hurt_no_staff");
	catPeasant.l_hurt_no_staff.speed = node.attribute("speed").as_float();
	catPeasant.l_hurt_no_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_hurt_no_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_dead
	node = animations_node.child("r_dead");
	catPeasant.r_dead.speed = node.attribute("speed").as_float();
	catPeasant.r_dead.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_dead.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_dead
	node = animations_node.child("l_dead");
	catPeasant.l_dead.speed = node.attribute("speed").as_float();
	catPeasant.l_dead.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_dead.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_dead_no_staff
	node = animations_node.child("r_dead_no_staff");
	catPeasant.r_dead_no_staff.speed = node.attribute("speed").as_float();
	catPeasant.r_dead_no_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_dead_no_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_dead_no_staff
	node = animations_node.child("l_dead_no_staff");
	catPeasant.l_dead_no_staff.speed = node.attribute("speed").as_float();
	catPeasant.l_dead_no_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_dead_no_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_throw_staff
	node = animations_node.child("r_throw_staff");
	catPeasant.r_throw_staff.speed = node.attribute("speed").as_float();
	catPeasant.r_throw_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.r_throw_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_throw_staff
	node = animations_node.child("l_throw_staff");
	catPeasant.l_throw_staff.speed = node.attribute("speed").as_float();
	catPeasant.l_throw_staff.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		catPeasant.l_throw_staff.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}
	//_load_animations
	//_CAT_PEASANT

	// CAT
	general_node = config.child("types").child("cat").child("general");

	actual_node = general_node.child("coll_offset");
	cat.coll_offset = { actual_node.attribute("x").as_int(), actual_node.attribute("y").as_int(), actual_node.attribute("w").as_int(), actual_node.attribute("h").as_int() };

	// Load animations
	animations_node = config.child("types").child("cat").child("animations");

	//r_idle
	node = animations_node.child("r_idle");
	cat.r_idle.speed = node.attribute("speed").as_float();
	cat.r_idle.loops = node.attribute("loops").as_bool();
	cat.coll_size = { node.child("frame").attribute("w").as_int(), node.child("frame").attribute("h").as_int() };
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_idle
	node = animations_node.child("l_idle");
	cat.l_idle.speed = node.attribute("speed").as_float();
	cat.l_idle.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_idle.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_going_ZZZ
	node = animations_node.child("r_going_ZZZ");
	cat.r_going_ZZZ.speed = node.attribute("speed").as_float();
	cat.r_going_ZZZ.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_going_ZZZ.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_going_ZZZ
	node = animations_node.child("l_going_ZZZ");
	cat.l_going_ZZZ.speed = node.attribute("speed").as_float();
	cat.l_going_ZZZ.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_going_ZZZ.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_waking_up
	node = animations_node.child("r_waking_up");
	cat.r_waking_up.speed = node.attribute("speed").as_float();
	cat.r_waking_up.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_waking_up.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_waking_up
	node = animations_node.child("l_waking_up");
	cat.l_waking_up.speed = node.attribute("speed").as_float();
	cat.l_waking_up.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_waking_up.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_to_crouch
	node = animations_node.child("r_to_crouch");
	cat.r_to_crouch.speed = node.attribute("speed").as_float();
	cat.r_to_crouch.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_to_crouch.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_to_crouch
	node = animations_node.child("l_to_crouch");
	cat.l_to_crouch.speed = node.attribute("speed").as_float();
	cat.l_to_crouch.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_to_crouch.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_crouch
	node = animations_node.child("r_crouch");
	cat.r_crouch.speed = node.attribute("speed").as_float();
	cat.r_crouch.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_crouch.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_crouch
	node = animations_node.child("l_crouch");
	cat.l_crouch.speed = node.attribute("speed").as_float();
	cat.l_crouch.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_crouch.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_rise
	node = animations_node.child("r_rise");
	cat.r_rise.speed = node.attribute("speed").as_float();
	cat.r_rise.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_rise.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_rise
	node = animations_node.child("l_rise");
	cat.l_rise.speed = node.attribute("speed").as_float();
	cat.l_rise.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_rise.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_jump
	node = animations_node.child("r_jump");
	cat.r_jump.speed = node.attribute("speed").as_float();
	cat.r_jump.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_jump.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_jump
	node = animations_node.child("l_jump");
	cat.l_jump.speed = node.attribute("speed").as_float();
	cat.l_jump.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_jump.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_fall
	node = animations_node.child("r_fall");
	cat.r_fall.speed = node.attribute("speed").as_float();
	cat.r_fall.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_fall.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_fall
	node = animations_node.child("l_fall");
	cat.l_fall.speed = node.attribute("speed").as_float();
	cat.l_fall.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_fall.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_land_soft
	node = animations_node.child("r_land_soft");
	cat.r_land_soft.speed = node.attribute("speed").as_float();
	cat.r_land_soft.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_land_soft.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_land_soft
	node = animations_node.child("l_land_soft");
	cat.l_land_soft.speed = node.attribute("speed").as_float();
	cat.l_land_soft.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_land_soft.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_roll
	node = animations_node.child("r_roll");
	cat.r_roll.speed = node.attribute("speed").as_float();
	cat.r_roll.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_roll.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_roll
	node = animations_node.child("l_roll");
	cat.l_roll.speed = node.attribute("speed").as_float();
	cat.l_roll.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_roll.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_to_run
	node = animations_node.child("r_to_run");
	cat.r_to_run.speed = node.attribute("speed").as_float();
	cat.r_to_run.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_to_run.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_to_run
	node = animations_node.child("l_to_run");
	cat.l_to_run.speed = node.attribute("speed").as_float();
	cat.l_to_run.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_to_run.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_run
	node = animations_node.child("r_run");
	cat.r_run.speed = node.attribute("speed").as_float();
	cat.r_run.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_run.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_run
	node = animations_node.child("l_run");
	cat.l_run.speed = node.attribute("speed").as_float();
	cat.l_run.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_run.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_turn
	node = animations_node.child("r_turn");
	cat.r_turn.speed = node.attribute("speed").as_float();
	cat.r_turn.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_turn.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_turn
	node = animations_node.child("l_turn");
	cat.l_turn.speed = node.attribute("speed").as_float();
	cat.l_turn.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_turn.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//r_brake
	node = animations_node.child("r_brake");
	cat.r_brake.speed = node.attribute("speed").as_float();
	cat.r_brake.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.r_brake.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//l_brake
	node = animations_node.child("l_brake");
	cat.l_brake.speed = node.attribute("speed").as_float();
	cat.l_brake.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.l_brake.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}

	//attack
	node = animations_node.child("attack");
	cat.attack.speed = node.attribute("speed").as_float();
	cat.attack.loops = node.attribute("loops").as_bool();
	for (node = node.child("frame"); node; node = node.next_sibling("frame")) {
		cat.attack.PushBack({ node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() });
	}
	//_load_animations
	//_CAT

	return ret;
}

bool j1EntityFactory::Start()
{
	// Load player textures
	LOG("Loading entities textures");
	CatPeasantTex = App->tex->Load(CatPeasant_spritesheet.GetString());
	MonkeyTex = App->tex->Load(Monkey_spritesheet.GetString());
	ImpTex = App->tex->Load(Imp_spritesheet.GetString());
	PlayerTex = App->tex->Load(Player_spritesheet.GetString());
	CatTex = App->tex->Load(Cat_spritesheet.GetString());

	return true;
}

bool j1EntityFactory::PreUpdate()
{
	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (queue[i].type != ENTITY_TYPES::NO_TYPE)
		{
			SpawnEntity(queue[i]);
			queue[i].type = ENTITY_TYPES::NO_TYPE;
			LOG("Spawning entity at %d", queue[i].y * App->scene->scale);
		}
	}
	
	return true;
}

// Called before render is available
bool j1EntityFactory::Update(float dt)
{
	for (uint i = 0; i < MAX_ENTITIES; ++i) {
		if (entities[i] != nullptr) {
			entities[i]->Move(dt);
		}
	}

	App->scene->MoveCamera();

	// Draw Map
	App->map->Draw();

	for (uint i = 0; i < MAX_ENTITIES; ++i)
		if (entities[i] != nullptr) {
			if (entities[i]->type == ENTITY_TYPES::CAT_PEASANT_)
				entities[i]->Draw(CatPeasantTex);
			else if (entities[i]->type == ENTITY_TYPES::IMP_)
				entities[i]->Draw(ImpTex);
			else if (entities[i]->type == ENTITY_TYPES::MONKEY_)
				entities[i]->Draw(MonkeyTex);
			else if (entities[i]->type == ENTITY_TYPES::PLAYER_)
				entities[i]->Draw(PlayerTex);
		}

	// Draw Above layer
	App->map->DrawAboveLayer();

	App->gui->Blit(dt);

	return true;
}

bool j1EntityFactory::PostUpdate()
{
	// check camera position to decide what to spawn
	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (entities[i] != nullptr)
		{
			if (entities[i]->dead) {
				delete entities[i];
				entities[i] = nullptr;
			}
		}
	}
	
	return true;
}

// Called before quitting
bool j1EntityFactory::CleanUp()
{
	LOG("Freeing all entities");

	// Remove all paths
	p2List_item<PathInfo*>* item;
	item = paths.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	paths.clear();

	App->tex->UnLoad(CatPeasantTex);
	App->tex->UnLoad(MonkeyTex);
	App->tex->UnLoad(ImpTex);
	App->tex->UnLoad(PlayerTex);
	App->tex->UnLoad(CatTex);

	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (queue[i].type != ENTITY_TYPES::NO_TYPE)
		{
			queue[i].type = ENTITY_TYPES::NO_TYPE;
			queue[i].x = NULL;
			queue[i].y = NULL;
		}

		if (entities[i] != nullptr)
		{
			delete entities[i];
			entities[i] = nullptr;
		}
	}

	return true;
}

bool j1EntityFactory::AddEntity(ENTITY_TYPES type, uint path)
{
	bool ret = false;

	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (queue[i].type == ENTITY_TYPES::NO_TYPE)
		{
			queue[i].path = GetPathByIndex(path);
			queue[i].type = type;

			if (queue[i].path != nullptr) {
				queue[i].x = queue[i].path->start_pos.x;
				queue[i].y = queue[i].path->start_pos.y;
			}
			else {
				queue[i].x = 0;
				queue[i].y = 0;
			}

			ret = true;
			break;
		}
	}

	return ret;
}

void j1EntityFactory::SpawnEntity(const EntityInfo& info)
{
	// find room for the new entity
	uint i = 0;
	for (; entities[i] != nullptr && i < MAX_ENTITIES; ++i);

	if (i != MAX_ENTITIES)
	{
		switch (info.type)
		{	
		case ENTITY_TYPES::CAT_PEASANT_:
			entities[i] = new CatPeasant(info.x, info.y, info.path);
			entities[i]->type = ENTITY_TYPES::CAT_PEASANT_;
			break;

		case ENTITY_TYPES::IMP_:
			entities[i] = new Imp(info.x, info.y, info.path);
			entities[i]->type = ENTITY_TYPES::IMP_;
			break;

		case ENTITY_TYPES::MONKEY_:
			entities[i] = new Monkey(info.x, info.y, info.path);
			entities[i]->type = ENTITY_TYPES::MONKEY_;
			break;

		case ENTITY_TYPES::PLAYER_:
			playerData = new Player(info.x, info.y);
			entities[i] = playerData;
			entities[i]->type = ENTITY_TYPES::PLAYER_;
			break;

		case ENTITY_TYPES::CAT_:
			entities[i] = new Cat(info.x, info.y);
			entities[i]->type = ENTITY_TYPES::CAT_;
			break;
		}
	}
}

void j1EntityFactory::OnCollision(Collider* c1, Collider* c2)
{
	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (entities[i] != nullptr && (entities[i]->GetCollider() == c1)) {

			entities[i]->OnCollision(c1, c2);

			break;
		}
	}
}

bool j1EntityFactory::LoadPathsInfo()
{
	bool ret = true;

	uint index = 1;

	// Repetitive paths
	SaveRepetitivePaths(index);
	// Start-end paths
	SaveStartEndPaths(index);

	return ret;
}

bool j1EntityFactory::SaveRepetitivePaths(uint& index)
{
	bool ret = false;

	p2SString tmp("%s%d", "Enemy", index);
	Object* obj = App->map->data.GetObjectByName("Enemies", tmp);

	while (obj != nullptr) {

		// Save actual path
		PathInfo* path = new PathInfo();

		path->start_pos = { (int)obj->x, (int)obj->y };

		path->path = new iPoint[MAX_POLYLINE_POINTS];
		memset(path->path, 0, sizeof(iPoint) * MAX_POLYLINE_POINTS);

		// Create path
		if (obj->polyline != nullptr) {

			path->path[0].x = obj->polyline[0] + path->start_pos.x;
			path->path[0].y = obj->polyline[1] + path->start_pos.y;

			path->path_size = 1;
			int i = 1;
			while (obj->polyline[i * 2 + 0] != 0 && obj->polyline[i * 2 + 1] != 0 && i < MAX_POLYLINE_POINTS) {

				path->path[i].x = obj->polyline[i * 2 + 0] + path->start_pos.x;
				path->path[i].y = obj->polyline[i * 2 + 1] + path->start_pos.y;

				path->path_size++;
				++i;
			}
		}

		paths.add(path);
		ret = true;

		// Search next path
		index++;
		p2SString tmp("%s%d", "Enemy", index);
		obj = App->map->data.GetObjectByName("Enemies", tmp);
	}

	if (obj != nullptr)
		RELEASE(obj);

	return ret;
}

bool j1EntityFactory::SaveStartEndPaths(uint& index)
{
	bool ret = false;

	p2SString tmp("%s%d%s", "Enemy", index, "S");
	Object* obj = App->map->data.GetObjectByName("Enemies", tmp);

	while (obj != nullptr) {

		// Save actual path
		PathInfo* path = new PathInfo();

		path->start_pos = { (int)obj->x, (int)obj->y };

		p2SString tmp("%s%d%s", "Enemy", index, "E");
		obj = App->map->data.GetObjectByName("Enemies", tmp);

		if (obj != nullptr) {
			path->end_pos = { (int)obj->x, (int)obj->y };

			paths.add(path);
			ret = true;
		}

		// Search next path
		index++;
		p2SString tmp1("%s%d%s", "Enemy", index, "S");
		obj = App->map->data.GetObjectByName("Enemies", tmp1);
	}

	if (obj != nullptr)
		RELEASE(obj);

	return ret;
}

bool j1EntityFactory::AddEntities()
{
	bool ret = false;

	if (!App->scene->loading_state)
		AddEntity(PLAYER_, 0);

	int index = 1;
	p2SString tmp("%s%d", "Enemy", index);
	Object* obj = App->map->data.GetObjectByName("Enemies", tmp);

	while (obj != nullptr) {
		// Add entity
		AddEntity((ENTITY_TYPES)obj->type, index);
		ret = true;

		// Search next entity
		index++;
		p2SString tmp("%s%d", "Enemy", index);
		obj = App->map->data.GetObjectByName("Enemies", tmp);
	}

	p2SString tmp1("%s%d%s", "Enemy", index, "S");
	obj = App->map->data.GetObjectByName("Enemies", tmp1);

	while (obj != nullptr) {
		// Add entity
		AddEntity((ENTITY_TYPES)obj->type, index);
		ret = true;

		// Search next entity
		index++;
		p2SString tmp1("%s%d%s", "Enemy", index, "S");
		obj = App->map->data.GetObjectByName("Enemies", tmp1);
	}

	if (obj != nullptr)
		RELEASE(obj);

	return ret;
}

PathInfo* j1EntityFactory::GetPathByIndex(uint index) const {

	PathInfo* path = nullptr;
	p2List_item<PathInfo*>* iterator = paths.start;

	int i = 0;
	while (iterator != nullptr && i <= index - 1) {

		if (i == index - 1)
			path = iterator->data;

		iterator = iterator->next;
		++i;
	}

	return path;
}

// -------------------------------------------------------------
// -------------------------------------------------------------

bool j1EntityFactory::Load(pugi::xml_node& save) 
{
	bool ret = false;

	pugi::xml_node node;

	if (App->scene->last_index != App->scene->index) {
		LoadEntities();
	}

	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (playerData != nullptr) {

			if (entities[i] == playerData) {
				if (save.child("player") != NULL) {
					node = save.child("player");
				}
				else {
					return true;
				}
				if (node.child("position") != NULL) {
					playerData->position.x = node.child("position").attribute("position.x").as_float();
					playerData->position.y = node.child("position").attribute("position.y").as_float();
				}
				if (node.child("state") != NULL) {
					playerData->player.SetState((playerstates)(node.child("state").attribute("state").as_int()));
				}
				if (node.child("speed") != NULL) {
					playerData->speed.x = node.child("speed").attribute("speed.x").as_float();
					playerData->speed.y = node.child("speed").attribute("speed.y").as_float();
				}
			}
		}
	}

	ret = true;
	return ret;
}

bool j1EntityFactory::LoadEntities() 
{
	bool ret = false;

	// Remove all paths
	p2List_item<PathInfo*>* item;
	item = paths.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	paths.clear();

	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (entities[i] != nullptr)
		{
			delete entities[i];
			entities[i] = nullptr;
		}
	}

	AddEntity(PLAYER_, 0);
	if (PreUpdate())
		ret = true;

	return ret;
}

bool j1EntityFactory::Save(pugi::xml_node& save) const 
{
	bool ret = false;

	pugi::xml_node node;
	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (playerData != nullptr) {

			if (entities[i] == playerData) {

				if (save.child("player") == NULL) {
					node = save.append_child("player");
				}
				else {
					node = save.child("player");
				}
				if (node.child("position") == NULL) {
					node.append_child("position").append_attribute("position.x") = playerData->position.x;
					node.child("position").append_attribute("position.y") = playerData->position.y;
				}
				else {
					node.child("position").attribute("position.x") = playerData->position.x;
					node.child("position").attribute("position.y") = playerData->position.y;
				}
				if (node.child("state") == NULL) {
					node.append_child("state").append_attribute("state") = playerData->player.GetState();
				}
				else {
					node.child("state").attribute("state") = playerData->player.GetState();
				}
				if (node.child("speed") == NULL) {
					node.append_child("speed").append_attribute("speed.x") = playerData->speed.x;
					node.child("speed").append_attribute("speed.y") = playerData->speed.y;
				}
				else {
					node.child("speed").attribute("speed.x") = playerData->speed.x;
					node.child("speed").attribute("speed.y") = playerData->speed.y;
				}
			}
		}
	}

	ret = true;
	return ret;
}

PathInfo::PathInfo() {}

PathInfo::PathInfo(const PathInfo& i) :
	start_pos(i.start_pos), path(i.path), path_size(i.path_size)
{}