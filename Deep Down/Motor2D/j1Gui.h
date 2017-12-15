#ifndef __j1GUI_H__
#define __j1GUI_H__

#include "j1Module.h"
#include "UIElement.h"
#include "p2nTree.h"

#include <map>

#define CURSOR_WIDTH 2

#define Black_ { 0,0,0,255 }
#define White_ { 255,255,255,255 }
#define LightGrey_ { 231,231,231,255 }
#define Purple_ { 47,33,56,255 }
#define Pink_ { 183,116,141,255 }
#define LightPink_ { 255,247,226,255 }

using namespace std;

struct _TTF_Font;

enum Tex_Names {
	NO_TEX_,
	ATLAS_,
	CURSORTEX_,
	CHECKBOX_,
	DSUI_,
	MENU_PAUSE_,
	MAIN_MENU_,
	GENERAL_,
	SLIDER_,
	POINTER_SLIDER_,
	BUTTON_SLIDER_,
	BUTTON_SLIDER2_,
	CAT_SCORE_,
	MAX_TEXS_
};

enum Font_Names {
	DEFAULT_,
	MSMINCHO_,
	ZELDA_,
	SOBAD_,
	SOBAD_8_,
	DIGITAL7_,
	BLACK_BLOC_,
	MAX_FONTS_
};

struct UIImage_Info;
struct UILabel_Info;
struct UIButton_Info;
struct UICursor_Info;
struct UIWindow_Info;
struct UILifeBar_Info;
struct UISlider_Info;
struct UIImage;
struct UILabel;
struct UIButton;
struct UICursor;
struct UIWindow;
struct UILifeBar;
struct UISlider;

// ---------------------------------------------------

class j1Gui : public j1Module
{
public:

	j1Gui();

	// Destructor
	virtual ~j1Gui();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool Blit(float dt) const;

	// TODO 2: Create the factory methods
	// Gui creation functions
	UIImage* CreateUIImage(iPoint local_pos, UIImage_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);
	UILabel* CreateUILabel(iPoint local_pos, UILabel_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);
	UIButton* CreateUIButton(iPoint local_pos, UIButton_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);
	UICursor* CreateUICursor(UICursor_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);
	UIWindow* CreateUIWindow(iPoint local_pos, UIWindow_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);
	UILifeBar* CreateUILifeBar(iPoint local_pos, UILifeBar_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);
	UISlider* CreateUISlider(iPoint local_pos, UISlider_Info& info, j1Module* listener = nullptr, UIElement* parent = nullptr);

	bool DestroyElement(UIElement* elem);
	bool ClearAllUI();
	bool ClearMapTextures();

	void SetUpDraggingChildren(UIElement* elem, bool dragging);
	void SetUpDraggingNode(bool drag);

	_TTF_Font* GetFont(Font_Names font_name);
	const SDL_Texture* GetTexture(Tex_Names tex_name);
	void SetTextureAlphaMod(Tex_Names tex_name, float alpha);
	float IncreaseDecreaseAlpha(float from, float to, float seconds);
	void ResetAlpha();

	void CreateTextStartLevel(uint level);

private:
	p2SString atlas_file_name;

	p2List<UIElement*> UI_elements_list;

	p2List<UIElement*> to_spawn_UI_elements;
	p2List<UIElement*> to_spawn_UI_parents;

	map<Tex_Names, const SDL_Texture*> map_textures;
	map<Font_Names, _TTF_Font*> map_fonts;

	// Alpha parameters
	float total_time = 0.0f;
	float start_time = 0.0f;
	bool reset = true;

public:
	nTree<UIElement*>* UI_elements_tree;
	bool debug_draw = false;

	bool drag_to_true = false;
	bool drag_to_false = false;
	bool update_drag = false;
};

#endif // __j1GUI_H__