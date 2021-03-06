#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"

#include "Animation.h"

#define LABELS_POS_MOUSE_ENTER {3,0}
#define CAM_BLIT_INCREASE_OFFSET 7
#define FADE_LESS_SECONDS 6
#define FADE_MORE_SECONDS 8
#define LIFE 222

struct SDL_Texture;
struct UILifeBar;
struct UIWindow;
struct UILabel;
struct UIButton;
struct UIImage;
struct UISlider;

enum names_labels_pause
{
	RESUME_,
	SAVE_,
	OPTIONS_,
	QUIT_,
	BACK_,
	NO_NAME_
};

class j1Scene : public j1Module
{
public:
	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// On triggered
	void OnUIEvent(UIElement* UIelem, UIEvents UIevent);

	// Save
	bool Save(pugi::xml_node&) const;

	// Load
	bool Load(pugi::xml_node&);

	// Debug keys
	void DebugKeys();

	void OpeningPauseMenu(bool from_scratch = false);

	void OpeningSubMenuOptions();

	void MoveCamera();

public:
	// Camera
	float up = false, down = false, left = false, right = false;
	uint width = 0;
	uint height = 0;
	uint scale = 0;

	// Map
	uint index = 0;
	uint last_index = 0;
	bool gate = false, fx = false;

	// Player
	bool end_of_level_reached = false;
	bool god = false;

	// Cats score
	uint cats_first_map = 0;
	uint cats_second_map = 0;
	UILabel* cats_score = nullptr;
	bool activate_UI_anim = false;

	// Pause menu
	bool pause = false;
	bool menu_bouncing = false;
	iPoint menu_position = { 0,0 };

	// Save and load
	bool loading = true;
	bool loading_state = false;
	bool forced_cleanup = false;
	bool back_to_main_menu = false;
	bool check_continue = false;

private:
	p2SString map1 = nullptr;
	p2SString map2 = nullptr;
	p2SString song1 = nullptr;
	p2SString song2 = nullptr;
	p2SString fx_names[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	float countdown_to_die = 0.0f;
	float count_time = 0.0f;
	uint volume_adjustment = 0;

	iPoint mouse = { 0,0 };

	// In-game UI
	UILabel* countdown_time = nullptr;
	UIImage* cat_UI = nullptr;
	Animation catsUI_anim;

	// Pause_menu
	UIWindow* pause_menu = nullptr;
	UILabel* menu_pause_labels[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	UIButton* menu_pause_checkbox[2] = { nullptr, nullptr };
	UIButton* closeWindow = nullptr;
	UISlider* volume_slider = nullptr;
	UISlider* fx_slider = nullptr;
	bool swap_music = false;
	bool swap_fx = false;

public:
	UILifeBar* progress_bar = nullptr;
};

#endif // __j1SCENE1_H__