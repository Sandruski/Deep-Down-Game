#ifndef __j1MENU_PAUSE_PAUSE_H__
#define __j1MENU_PAUSE_PAUSE_H__

#include "j1Module.h"

#include "Animation.h"

struct SDL_Texture;
struct Cat;
struct UILabel;
struct UIImage;

enum MenuState {
	NO_MENU_,
	TITLE_ANIMATION_,
	PRESS_ANY_BUTTON_,
	TRANSITION_TO_MAIN_MENU_,
	MAIN_MENU_OPTIONS_ANIMATION_,
	AT_MAIN_MENU_,
	AT_SETTINGS_,
	AT_CREDITS_
};

enum MenuCatState {
	NO_CAT_,
	MC_APPEAR_RUNNING_,
	MC_JUMP_TITLE_,
	MC_RUN_FIRST_LETTER_,
	MC_ROLL_SECOND_LETTER_,
	MC_RUN_WORD_,
	MC_ROLL_END_WORD_,
	MC_STOP_JUMPING_,
	MC_AT_GROUND_,
	MC_START_,
	MC_CONTINUE_
};

enum MenuOptions {
	MM_START_,
	MM_CONTINUE_,
	MM_SETTINGS_,
	MM_CREDITS_,
	MM_EXIT_,
};

class j1Menu : public j1Module
{
public:
	j1Menu();

	// Destructor
	virtual ~j1Menu();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	void OnUIEvent(UIElement* UIelem, UIEvents UIevent);

private:
	MenuState menuState = MenuState::NO_MENU_;
	MenuCatState menuCatState = MenuCatState::NO_CAT_;

	Cat* cat = nullptr;
	float cat_position_increment[8];

	// UI Elements
	UILabel* title_letters[8];
	UIButton* main_menu_buttons[5];
	UILabel* main_menu_options[5];

	UILabel* press_any_button = nullptr;
	UILabel* skip = nullptr;
	UIImage* black_screen_image = nullptr;

	uint i = 0;
	float timer = 0;

	bool print_title = false;
	bool blit_cat = true;
	bool is_invisible = true;
	bool visible_again = false;

	// Screen parameters
	uint width = 0, height = 0;
	int scale = 0;

	iPoint camera_start_position = { 0,0 };
	bool camera_blit = false;
};

#endif // __j1MENU_PAUSE_PAUSE_H__