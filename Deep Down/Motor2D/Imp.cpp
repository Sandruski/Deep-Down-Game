#include "j1App.h"
#include "j1Enemies.h"
#include "Imp.h"

#include "p2Defs.h"
#include "p2Log.h"

#include "j1Collision.h"
#include "j1Player.h"
#include "j1Particles.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1Player.h"

#include "j1Input.h"

#include "SDL/include/SDL_timer.h"

Imp::Imp(float x, float y, PathInfo* path) : Enemy(x, y, path) 
{
	imp = App->enemies->GetImpInfo();

	///
	animation = &imp.r_shield_idle;
	impState = ImpState::r_shield_idle;

	collider = App->collision->AddCollider({ 0, 0, imp.coll_size.x + imp.coll_offset.w, imp.coll_size.y + imp.coll_offset.h }, COLLIDER_TYPE::COLLIDER_IMP, App->enemies);
	collider_size = imp.coll_size;

	follow_pathfinding1 = App->collision->AddCollider({ i_pos.x - 50, i_pos.y, 100, 100 }, COLLIDER_TYPE::COLLIDER_NONE, App->enemies);
	follow_pathfinding2 = App->collision->AddCollider({ (int)App->player->position.x - 50, (int)App->player->position.y - 10, 100, 200 }, COLLIDER_TYPE::COLLIDER_NONE, App->enemies);

	speed = { 0.8f, 1 };
}

void Imp::Move(float dt)
{
	i_pos.x = (int)position.x;
	i_pos.y = (int)position.y;

	// Update path/pathfinding
	if (!pathfinding)
		UpdatePath();

	UpdatePathfinding();

	// Update state
	GeneralStatesMachine();

	// Update movement
	throw_bomb = false;
	UpdateDirection();

	// Update collider
	//collider_pos = { i_pos.x + imp.coll_offset.x, i_pos.y + imp.coll_offset.y };
	//collider->SetPos(collider_pos.x, collider_pos.y);

	// Debug
	collider->SetPos(i_pos.x, i_pos.y);
}

void Imp::GeneralStatesMachine() 
{
	switch (impState) {

	case r_shield_idle:
		if (throw_bomb) {
			impState = ImpState::r_throw_bomb;
			break;
		}
		if (stop_x && left) {
			impState = ImpState::l_shield_idle;
			break;
		}
		if (right) {
			impState = ImpState::r_shield_walk;
			break;
		}
		if (left) {
			impState = ImpState::l_shield_walk;
			break;
		}
		animation = &imp.r_shield_idle;
		break;

	case l_shield_idle:
		if (throw_bomb) {
			impState = ImpState::l_throw_bomb;
			break;
		}
		if (stop_x && right) {
			impState = ImpState::r_shield_idle;
			break;
		}
		if (right) {
			impState = ImpState::r_shield_walk;
			break;
		}
		if (left) {
			impState = ImpState::l_shield_walk;
			break;
		}
		animation = &imp.l_shield_idle;
		break;

	case r_shield_walk:
		if (left) {
			impState = ImpState::l_shield_walk;
			break;
		}
		animation = &imp.r_shield_walk;
		impState = ImpState::r_shield_idle;
		break;

	case l_shield_walk:
		if (right) {
			impState = ImpState::r_shield_walk;
			break;
		}
		animation = &imp.l_shield_walk;
		impState = ImpState::l_shield_idle;
		break;

	case r_jump:
		if (left) {
			impState = ImpState::l_jump;
			break;
		}
		animation = &imp.r_jump;
		impState = ImpState::r_shield_idle;
		break;

	case l_jump:
		if (right) {
			impState = ImpState::r_jump;
			break;
		}
		animation = &imp.l_jump;
		impState = ImpState::l_shield_idle;
		break;

	case r_throw_bomb:
		if (left) {
			impState = ImpState::l_throw_bomb;
			break;
		}
		App->particles->AddParticle(App->particles->Imp_r_bomb, position.x, position.y, COLLIDER_IMP_BOMB, NULL, { 0,0 });
		animation = &imp.r_throw_bomb;
		impState = ImpState::r_shield_idle;
		break;

	case l_throw_bomb:
		if (right) {
			impState = ImpState::r_throw_bomb;
			break;
		}
		App->particles->AddParticle(App->particles->Imp_l_bomb, position.x, position.y, COLLIDER_IMP_BOMB, NULL, { 0,0 });
		animation = &imp.l_throw_bomb;
		impState = ImpState::l_shield_idle;
		break;

	case r_shield_hurt:
		if (left) {
			impState = ImpState::l_shield_hurt;
			break;
		}
		animation = &imp.r_shield_hurt;
		impState = ImpState::r_shield_idle;
		break;

	case l_shield_hurt:
		if (right) {
			impState = ImpState::r_shield_hurt;
			break;
		}
		animation = &imp.l_shield_hurt;
		impState = ImpState::l_shield_idle;
		break;
	}
}

void Imp::UpdateDirection() {

	if (i_pos.x != last_pos.x) {
		stop_x = false;

		if (i_pos.x < last_pos.x) {
			left = true;
			right = false;
		}
		else if (i_pos.x > last_pos.x) {
			right = true;
			left = false;
		}
	}
	else
		stop_x = true;

	if (i_pos.y != last_pos.y) {
		stop_y = false;

		if (i_pos.y < last_pos.y) {
			up = true;
			down = false;
		}
		else if (i_pos.y > last_pos.y) {
			down = true;
			up = false;
		}
	}
	else
		stop_y = true;

	last_pos.x = i_pos.x;
	last_pos.y = i_pos.y;
}

void Imp::OnCollision(Collider* c1, Collider* c2) 
{
}

void Imp::UpdatePath()
{
	if (path_info->end_pos.x != NULL && path_info->end_pos.y != NULL) {

		// If enemy was pathfinding towards the player and doesn't see it anymore, create a pathfinding back home
		if (pathfinding_stop) {
			if (ResetNormalPathVariables()) {
				create_pathfinding_back = true;
				pathfinding_stop = false;
			}
		}

		// Create pathfinding back
		if (create_pathfinding_back) {
			iPoint to_go;

			FindDestination(to_go);

			if (CreatePathfinding(to_go)) {
				going_back_home = true;
				do_normal_path = true;
				create_pathfinding_back = false;
			}
		}

		// If the enemy doesn't see the player, create its normal path
		if (normal_path_finished && !pathfinding) {
			if (ResetNormalPathVariables()) {
				create_normal_path = true;
			}
		}

		// Create normal path
		if (create_normal_path) {
			iPoint to_go;

			FindDestination(to_go);

			if (CreatePathfinding(to_go)) {
				do_normal_path = true;
				create_normal_path = false;
			}
		}

		// If it has to go back home and hasn't reached home yet, update the pathfinding back
		if (going_back_home) {
			if (!Pathfind()) {
				RecalculatePath();
				going_back_home = false;
				pathfinding_finished = true;
			}
		}
		// If it is home, update the normal path
		else if (do_normal_path) {
			if (!Pathfind()) {
				RecalculatePath();
				normal_path_finished = true;
			}
		}
	}
}

void Imp::FindDestination(iPoint& to_go)
{
	switch (normal_path_index) {
	case StartEndPath::start:
		to_go = path_info->start_pos;
		break;
	case StartEndPath::end:
		to_go = path_info->end_pos;
		break;
	default:
		to_go = { 0,0 };
	}
}

void Imp::UpdatePathfinding()
{
	SDL_Rect enemy_pos;
	SDL_Rect player_pos;
	UpdatePathfindingAffectArea(enemy_pos, player_pos);

	// If player is near the enemy... Create a pathfinding towards it
	if (pathfinding_finished && SDL_HasIntersection(&enemy_pos, &player_pos)) {
		if (ResetPathfindingVariables()) {
			create_pathfinding = true;
		}
	}

	// Create pathfinding
	if (create_pathfinding) {
		iPoint dest;
		dest.x = (int)App->player->position.x;
		dest.y = (int)App->player->position.y;

		if (CreatePathfinding(dest)) {
			pathfinding = true;
			create_pathfinding = false;
			go = true;
		}
		else {
			pathfinding_stop = true;
			create_pathfinding = false;
		}
	}

	if (go && SDL_HasIntersection(&enemy_pos, &player_pos)) {
		pathfind = true;
		go = false;
	}

	if (pathfinding) {
		// If player is near the enemy and the enemy hasn't reached the end of the path yet, update the path
		if (pathfind) {
			if (!Pathfind())
				pathfind = false;
		}
		else if (!pathfind) {
			// If the enemy reaches the end of the path and doesn't see the player anymore, stop pathfinding and go back home
			if (!SDL_HasIntersection(&enemy_pos, &player_pos)) {
				pathfinding = false;
				pathfinding_stop = true;
			}
			// If the enemy reaches the end of the path and continues seeing the player, create a new pathfinding towards him
			else {
				pathfinding = false;
				pathfinding_finished = true;
			}
		}
	}
}

void Imp::UpdatePathfindingAffectArea(SDL_Rect& enemy, SDL_Rect& player)
{
	follow_pathfinding1->SetPos(i_pos.x - 30, i_pos.y - 30);
	follow_pathfinding2->SetPos((int)App->player->position.x - 25, (int)App->player->position.y - 50);

	enemy = { i_pos.x - 30, i_pos.y - 30, 100, 100 };
	player = { (int)App->player->position.x - 25, (int)App->player->position.y - 50, 100, 200 };
}

bool Imp::ResetPathfindingVariables()
{
	bool ret = true;

	pathfinding_finished = false;
	pathfinding_stop = false;
	pathfinding = false;
	pathfind = true;
	pathfinding_size = 0;

	return ret;
}

bool Imp::ResetNormalPathVariables()
{
	bool ret = true;

	do_normal_path = false;
	normal_path_finished = false;
	pathfinding_size = 0;

	return ret;
}

bool Imp::CreatePathfinding(iPoint destination)
{
	bool ret = false;

	if (App->pathfinding->CreatePath(App->map->WorldToMap(i_pos.x, i_pos.y), App->map->WorldToMap(destination.x, destination.y), Distance::DISTANCE_TO, true) > -1) {
		last_pathfinding = App->pathfinding->GetLastPath();

		pathfinding_size = last_pathfinding->Count();

		if (pathfinding_size > 1)
			pathfinding_index = 1;
		else
			pathfinding_index = 0;

		mlast_pathfinding.Clear();

		for (int i = 0; i < pathfinding_size; ++i) {
			mlast_pathfinding.PushBack(*last_pathfinding->At(i));
			ret = true;
		}
	}

	return ret;
}

void Imp::UpdateMovement(iPoint to_go)
{
	if (i_pos.x < to_go.x)
		position.x += speed.x;
	else if (i_pos.x > to_go.x)
		position.x -= speed.x;
	if (i_pos.y < to_go.y)
		position.y += speed.x;
	else if (i_pos.y > to_go.y)
		position.y -= speed.x;
}

bool Imp::Pathfind()
{
	bool ret = true;

	iPoint to_go = App->map->MapToWorld(mlast_pathfinding[pathfinding_index].x, mlast_pathfinding[pathfinding_index].y);

	UpdateMovement(to_go);

	if (i_pos == to_go) {
		if (pathfinding_index < pathfinding_size - 1)
			pathfinding_index++;
	}

	if (i_pos == App->map->MapToWorld(mlast_pathfinding[pathfinding_size - 1].x, mlast_pathfinding[pathfinding_size - 1].y))
		ret = false;

	return ret;
}

void Imp::RecalculatePath()
{
	switch (normal_path_index) {
	case StartEndPath::start:
		normal_path_index = StartEndPath::end;
		break;
	case StartEndPath::end:
		normal_path_index = StartEndPath::start;
		break;
	default:
		break;
	}
}