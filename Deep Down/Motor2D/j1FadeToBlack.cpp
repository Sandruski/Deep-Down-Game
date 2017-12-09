#include "p2Defs.h"
#include "p2Log.h"

#include "j1App.h"

#include "j1Render.h"
#include "j1Window.h"
#include "j1FadeToBlack.h"
#include "j1Textures.h"

#include <math.h>

#include "SDL/include/SDL_render.h"
#include "SDL/include/SDL_timer.h"

j1FadeToBlack::j1FadeToBlack()
{
	name.create("fade");
}

j1FadeToBlack::~j1FadeToBlack()
{}

bool j1FadeToBlack::Awake()
{
	BonFireAnim.PushBack({ 0, 0, 172, 236});
	BonFireAnim.PushBack({ 172, 0, 172, 236 });
	BonFireAnim.PushBack({ 344, 0, 172, 236 });
	BonFireAnim.PushBack({ 0, 236, 172, 236 });
	BonFireAnim.PushBack({ 172, 236, 172, 236 });
	BonFireAnim.PushBack({ 344, 236, 172, 236 });
	BonFireAnim.PushBack({ 0, 472, 172, 236 });
	BonFireAnim.PushBack({ 172, 472, 172, 236 });
	BonFireAnim.PushBack({ 344, 472, 172, 236 });
	BonFireAnim.speed = 2.0f;

}
// Load assets
bool j1FadeToBlack::Start()
{
	LOG("Preparing Fade Screen");
	SDL_SetRenderDrawBlendMode(App->render->renderer, SDL_BLENDMODE_BLEND);

	LOG("Loading Textures");
	bonfire = App->tex->Load("Assets/Sprites/UI/BonFire.png");
	// Get screen size
	uint width = 0, height = 0, scale = 0;

	App->win->GetWindowSize(width, height);
	scale = App->win->GetScale();

	screen = { 0, 0, static_cast<int>(width * scale), static_cast<int>(height * scale) };
	//
	Slider_rect = screen;
	Slider_rect.w = 0;
	return true;
}
bool j1FadeToBlack::CleanUp()
{
	LOG("UnLoading Textures");
	App->tex->UnLoad(bonfire);

	return true;
}
// Update: draw background
bool j1FadeToBlack::Update(float dt)
{
	this->dt = dt;

	if (current_step == fade_step::none)
		return true;

	switch (thisFade)
	{
	case normal_fade:
		NormalFade();
		break;
	case slider_fade:
		SliderFade();
		break;
	case total_black_fade:
		BlackFade();
		break;
	}
	
	
	return true;
}

// Fade to black. At mid point deactivate one module, then activate the other
bool j1FadeToBlack::FadeToBlack(j1Module* module_off, j1Module* module_on, float time, fades kind_of_fade)
{
	bool ret = false;

	if (current_step == fade_step::none)
	{
		
		thisFade = kind_of_fade;
		
		current_step = fade_step::fade_to_black;
		start_time = SDL_GetTicks();
		total_time = (Uint32)(time * 0.5f * 1000.0f);

		off = module_off;
		on = module_on;

		ret = true;
	}

	return ret;
}

bool j1FadeToBlack::IsFading() const
{
	return current_step != fade_step::none;
}

void j1FadeToBlack::NormalFade()
{
	Uint32 now = SDL_GetTicks() - start_time;
	float normalized = MIN(1.0f, (float)now / (float)total_time);

	switch (current_step)
	{
	case fade_step::fade_to_black:
	{

		if (now >= total_time)
		{
			off->CleanUp();
			on->Start();

			total_time += total_time;
			start_time = SDL_GetTicks();
			current_step = fade_step::fade_from_black;
		}
	} break;

	case fade_step::fade_from_black:
	{
		normalized = 1.0f - normalized;

		if (now >= total_time)
			current_step = fade_step::none;

	} break;
	}

	// Finally render the black square with alpha on the screen
	SDL_SetRenderDrawColor(App->render->renderer, 0, 0, 0, (Uint8)(normalized * 255.0f));
	SDL_RenderFillRect(App->render->renderer, &screen);

}

void j1FadeToBlack::SliderFade()
{

	Uint32 now = SDL_GetTicks() - start_time;
	float normalized = MIN(1.0f, (float)now / (float)total_time);

	switch (current_step)
	{
	case fade_step::fade_to_black:

		if (now >= total_time) {

			off->CleanUp();
			on->Start();

			total_time += total_time;
			start_time = SDL_GetTicks();

			current_step = fade_from_black;

		}
		break;

	case fade_step::fade_from_black:

		normalized = 1.0f - normalized;
		if (now >= total_time)
			current_step = fade_step::none;

		break;

	}
	Slider_rect.w = normalized*screen.w;

	SDL_SetRenderDrawColor(App->render->renderer, 0, 0, 0, 255.0f);
	SDL_RenderFillRect(App->render->renderer, &Slider_rect);

}

void j1FadeToBlack::BlackFade()
{

	Uint32 now = SDL_GetTicks() - start_time;

	switch (current_step)
	{
	case fade_step::fade_to_black:

		if (now >= total_time) {

			off->CleanUp();
			on->Start();

			total_time += total_time;
			start_time = SDL_GetTicks();

			current_step = fade_from_black;

		}
		break;

	case fade_step::fade_from_black:

		if (now >= total_time)
			current_step = fade_step::none;

		break;

	}

	SDL_SetRenderDrawColor(App->render->renderer, 0, 0, 0, 255.0f);
	SDL_RenderFillRect(App->render->renderer, &screen);

	r = &BonFireAnim.GetCurrentFrame();
	
	SDL_Rect a = { 0, 0, 172, 236 };
	App->render->Blit(bonfire, 672, 208, &a);


}