#include "j1App.h"
#include "j1EntityFactory.h"
#include "Monkey.h"

#include "p2Defs.h"
#include "p2Log.h"

#include "j1Collision.h"
#include "j1Particles.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1Audio.h"

#include "j1Input.h"

#include "SDL/include/SDL_timer.h"

/*
MONKEY (enemy):
� Type: air.
� Lives: 1.

- He follows a continuous path drawn in Tiled.
- He skips his path when he sees the player, and pathfinds him.
- When he reaches the player, he attacks him.
- If he stops seeing the player, he goes back to his initial path.
*/

Monkey::Monkey(float x, float y, PathInfo* path) : Entity(x, y), path_info(path), mlast_pathfinding(DEFAULT_PATH_LENGTH)
{
	last_pathfinding = nullptr;
	mlast_pathfinding.Clear();

	monkey = App->entities->GetMonkeyInfo();

	///
	LoadAnimationsSpeed();
	animation = &monkey.r_idle;
	monkeyState = MonkeyState::mr_idle;

	lives = monkey.lives;

	collider = App->collision->AddCollider({ 0, 0, monkey.coll_size.x + monkey.coll_offset.w, monkey.coll_size.y + monkey.coll_offset.h }, COLLIDER_TYPE::COLLIDER_MONKEY, App->entities);
	collider_size = monkey.coll_size;
}

void Monkey::LoadAnimationsSpeed()
{
	r_idle_speed = monkey.r_idle.speed;
	l_idle_speed = monkey.l_idle.speed;
	r_hurt_speed = monkey.r_hurt.speed;
	l_hurt_speed = monkey.l_hurt.speed;
	r_hit_speed = monkey.r_hit.speed;
	l_hit_speed = monkey.l_hit.speed;
}

void Monkey::Move(const float dt)
{
	deltaTime = dt;

	i_pos.x = (int)position.x;
	i_pos.y = (int)position.y;

	// Update animations speed
	UpdateAnimations(dt);

	// Update movement
	up = false;
	down = false;
	left = false;
	right = false;
	UpdateDirection();

	// Update hurt and death
	Wounded();

	// Update path/pathfinding
	if (!stop) {
		DoHit();

		if (!pathfinding)
			UpdatePath();

		UpdatePathfinding();
	}
	//_update_path/pathfinding

	// Update state
	GeneralStatesMachine();

	// Update collider
	collider_pos = { i_pos.x + monkey.coll_offset.x, i_pos.y + monkey.coll_offset.y };
	collider->SetPos(collider_pos.x, collider_pos.y);
}

void Monkey::Wounded()
{
	if (right_hurt) {
		if (monkey.r_hurt.Finished())
			dead = true;
	}
	else if (left_hurt) {
		if (monkey.l_hurt.Finished())
			dead = true;
	}
}

void Monkey::GeneralStatesMachine() 
{
	switch (monkeyState) {

	case MonkeyState::mr_idle:
		if (right_hurt) {
			monkeyState = MonkeyState::mr_hurt;
			break;
		}
		if (left_hurt) {
			monkeyState = MonkeyState::ml_hurt;
			break;
		}
		if (right_hit) {
			monkeyState = MonkeyState::mr_hit;
			break;
		}
		if (left) {
			monkeyState = MonkeyState::ml_idle;
			break;
		}
		animation = &monkey.r_idle;
		break;

	case MonkeyState::ml_idle:
		if (right_hurt) {
			monkeyState = MonkeyState::mr_hurt;
			break;
		}
		if (left_hurt) {
			monkeyState = MonkeyState::ml_hurt;
			break;
		}
		if (left_hit) {
			monkeyState = MonkeyState::ml_hit;
			break;
		}
		if (right) {
			monkeyState = MonkeyState::mr_idle;
			break;
		}
		animation = &monkey.l_idle;
		break;

	case MonkeyState::mr_hit:
		if (left) {
			monkeyState = MonkeyState::ml_hit;
			break;
		}
		animation = &monkey.r_hit;

			monkeyState = MonkeyState::mr_idle;
		break;

	case MonkeyState::ml_hit:
		if (right) {
			monkeyState = MonkeyState::mr_hit;
			break;
		}
		animation = &monkey.l_hit;

			monkeyState = MonkeyState::ml_idle;
		break;

	case MonkeyState::mr_hurt:
		if (left) {
			monkeyState = MonkeyState::ml_hurt;
			break;
		}
		animation = &monkey.r_hurt;

			monkeyState = MonkeyState::mr_idle;
		break;

	case MonkeyState::ml_hurt:
		if (right) {
			monkeyState = MonkeyState::mr_hurt;
			break;
		}
		animation = &monkey.l_hurt;

			monkeyState = MonkeyState::ml_idle;
		break;
	}
}

void Monkey::UpdateDirection()
{
	if (position.x < last_pos.x) {
		left = true;
	}
	else if (position.x > last_pos.x) {
		right = true;
	}

	if (position.y < last_pos.y) {
		up = true;
	}
	else if (position.y > last_pos.y) {
		down = true;
	}

	last_pos = position;
}

void Monkey::UpdateAnimations(const float dt) 
{
	monkey.r_idle.speed = r_idle_speed * dt;
	monkey.l_idle.speed = l_idle_speed * dt;
	monkey.r_hurt.speed = r_hurt_speed * dt;
	monkey.l_hurt.speed = l_hurt_speed * dt;
	monkey.r_hit.speed = r_hit_speed * dt;
	monkey.l_hit.speed = l_hit_speed * dt;
}

void Monkey::OnCollision(Collider* c1, Collider* c2) 
{
	if (c2->type == COLLIDER_ARROW) {
		lives--;
		stop = true;
		App->audio->PlayFx(7);
		if (App->entities->playerData->position.x < position.x)
			left_hurt = true;
		else
			right_hurt = true;
	}
}

void Monkey::UpdatePath()
{
	if (path_info->path != nullptr) {

		// If enemy was pathfinding towards the player and doesn't see it anymore, create a pathfinding back home
		if (pathfinding_stop) {
			if (ResetPathfindingVariables()) {
				normal_path_index = last_normal_path_index;
				create_pathfinding_back = true;
			}
		}

		// Create pathfinding back
		if (create_pathfinding_back) {
			home = path_info->path[normal_path_index];

			if (CreatePathfinding(home)) {
				going_back_home = true;
				pathfinding_finished = true;
				create_pathfinding_back = false;
			}
		}

		// If it has to go back home and hasn't reached home yet, update the pathfinding back
		if (going_back_home) {
			if (!Pathfind(monkey.pathfinding_normal_speed))
				going_back_home = false;
		}
		// If it is home, update the normal path
		else {
			if (!DoNormalPath(monkey.pathfinding_slow_speed))
				RecalculatePath();
		}
	}
}

void Monkey::UpdatePathfinding()
{
	SDL_Rect enemy_pos;
	SDL_Rect player_pos;
	UpdatePathfindingAffectArea(enemy_pos, player_pos);

	// If player is near the enemy... Create a pathfinding towards it
	if (pathfinding_finished && SDL_HasIntersection(&enemy_pos, &player_pos)) {
		if (ResetPathfindingVariables()) {
			last_normal_path_index = normal_path_index;
			create_pathfinding = true;
		}
	}

	// Create pathfinding
	if (create_pathfinding) {
		iPoint dest;
		dest.x = (int)App->entities->playerData->position.x;
		dest.y = (int)App->entities->playerData->position.y;

		if (CreatePathfinding(dest)) {
			pathfinding = true;
			create_pathfinding = false;
		}
	}

	if (pathfinding) {
		// If player is near the enemy and the enemy hasn't reached the end of the path yet, update the path
		if (SDL_HasIntersection(&enemy_pos, &player_pos) && pathfind) {
			if (!Pathfind(monkey.pathfinding_normal_speed)) {
				if (position.DistanceTo(App->entities->playerData->position) < monkey.min_distance_to_hit) {
					Hit();
					wait = true;
				}
				else if (!wait)
					pathfind = false;
			}
		}
		else {
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

void Monkey::Hit()
{
	if (do_hit) {
		if (right)
			right_hit = true;
		else if (left)
			left_hit = true;
		else
			right_hit = true;

		do_hit = false;
	}
}

void Monkey::DoHit() 
{
	if (right_hit) {
		if (monkey.r_hit.Finished()) {
			App->particles->AddParticle(App->particles->monkeyAttack, i_pos.x + monkey.distance_to_player, i_pos.y, COLLIDER_MONKEY_HIT, NULL, monkey.particle_speed);

			pathfind = false;
			right_hit = false;
			monkey.r_hit.Reset();
			do_hit = true;
			wait = false;
		}
	}
	else if (left_hit) {
		if (monkey.l_hit.Finished()) {
			App->particles->AddParticle(App->particles->monkeyAttack, i_pos.x - monkey.distance_to_player, i_pos.y, COLLIDER_MONKEY_HIT, NULL, monkey.particle_speed);

			pathfind = false;
			left_hit = false;
			monkey.l_hit.Reset();
			do_hit = true;
			wait = false;
		}
	}
}

void Monkey::UpdatePathfindingAffectArea(SDL_Rect& enemy, SDL_Rect& player)
{
	enemy = { i_pos.x - monkey.enemy_pathfinding_affect_area.x, i_pos.y - monkey.enemy_pathfinding_affect_area.y, monkey.enemy_pathfinding_affect_area.w, monkey.enemy_pathfinding_affect_area.h };
	player = { (int)App->entities->playerData->position.x - monkey.enemy_pathfinding_affect_area.x, (int)App->entities->playerData->position.y - monkey.enemy_pathfinding_affect_area.y, monkey.enemy_pathfinding_affect_area.w, monkey.enemy_pathfinding_affect_area.h };
}

bool Monkey::ResetPathfindingVariables()
{
	bool ret = true;

	pathfinding_finished = false;
	pathfinding_stop = false;
	pathfinding = false;
	pathfind = true;
	pathfinding_size = 0;

	return ret;
}

bool Monkey::CreatePathfinding(const iPoint destination)
{
	bool ret = false;

	if (App->pathfinding->CreatePath(App->map->WorldToMap(i_pos.x, i_pos.y), App->map->WorldToMap(destination.x, destination.y), Distance::DISTANCE_TO)) {
		last_pathfinding = App->pathfinding->GetLastPath();

		pathfinding_size = last_pathfinding->Count();

		pathfinding_index = 1;

		mlast_pathfinding.Clear();

		for (int i = 0; i < pathfinding_size; ++i) {
			mlast_pathfinding.PushBack(*last_pathfinding->At(i));
			ret = true;
		}
	}

	return ret;
}

void Monkey::UpdateMovement(const iPoint to_go, float velocity)
{
	speed.x = mlast_pathfinding[pathfinding_index].x - App->map->WorldToMap(position.x, position.y).x;
	speed.y = mlast_pathfinding[pathfinding_index].y - App->map->WorldToMap(position.x, position.y).y;

	speed.x *= velocity * deltaTime;
	speed.y *= velocity * deltaTime;

	position.x += speed.x;
	position.y += speed.y;
}

void Monkey::UpdateNormalPathMovement(const iPoint to_go, float velocity)
{
	iPoint map = App->map->WorldToMap(to_go.x, to_go.y);

	speed.x = map.x - App->map->WorldToMap(position.x, position.y).x;
	speed.y = map.y - App->map->WorldToMap(position.x, position.y).y;

	float m = sqrtf(pow(speed.x, 2.0f) + pow(speed.y, 2.0f));

	if (m != 0)
		speed.x *= velocity * deltaTime / m;
	if (m != 0)
		speed.y *= velocity * deltaTime / m;

	position.x += speed.x;
	position.y += speed.y;
}

bool Monkey::Pathfind(float velocity)
{
	bool ret = true;

	if (pathfinding_size > 1) {
		iPoint to_go = App->map->MapToWorld(mlast_pathfinding[pathfinding_index].x, mlast_pathfinding[pathfinding_index].y);

		UpdateMovement(to_go, velocity);

		if (App->map->WorldToMap(i_pos.x, i_pos.y) == App->map->WorldToMap(to_go.x, to_go.y)) {
			if (pathfinding_index < pathfinding_size - 1)
				pathfinding_index++;
		}

		if (App->map->WorldToMap(i_pos.x, i_pos.y) == mlast_pathfinding[pathfinding_size - 1] || App->entities->playerData->player.GetState() == playerstates::punished_)
			ret = false;
	}
	else
		ret = false;

	return ret;
}

bool Monkey::DoNormalPath(float velocity)
{
	bool ret = true;

	iPoint to_go = path_info->path[normal_path_index];

	if (to_go.x != 0 && to_go.y != 0) {

		UpdateNormalPathMovement(to_go, velocity);

		ret = true;

		if (App->map->WorldToMap(i_pos.x, i_pos.y) == App->map->WorldToMap(to_go.x, to_go.y)) {
			if (normal_path_index < path_info->path_size - 1)
				normal_path_index++;
			else
				ret = false;
		}
	}

	return ret;
}

void Monkey::RecalculatePath()
{
	normal_path_index = 0;

	// Flip path
	FlipPath(path_info);
}

void Monkey::FlipPath(const PathInfo* path_info) 
{
	iPoint* start = &path_info->path[0];
	iPoint* end = &path_info->path[path_info->path_size - 1];

	while (start < end)
		SWAP(*start++, *end--);
}