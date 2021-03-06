#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1BetweenTransitions.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Gui.h"
#include "j1Window.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UICursor.h"
#include "UIWindow.h"
#include "UILifeBar.h"
#include "UISlider.h"

#include "Brofiler\Brofiler.h"

j1Gui::j1Gui() : j1Module()
{
	name.create("gui");
}

// Destructor
j1Gui::~j1Gui()
{}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	atlas_file_name = conf.child("atlas").attribute("file").as_string("");

	pugi::xml_node rects = conf.child("UI_elements_rects");
	pugi::xml_node node;
	uint i = 0;
	UI_elements_rects[(UIElement_Rect)i] = { 0,0,0,0 };

	for (node = rects.child("rect"); node; node = node.next_sibling("rect")) {
		UI_elements_rects[(UIElement_Rect)++i] = { node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() };
	}
	//UI_elements_rects

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	bool ret = true;

	// Load textures
	atlas = App->tex->Load(atlas_file_name.GetString());

	// Load fonts
	map_fonts[Font_Names::MSMINCHO_] = App->font->Load("Assets/Fonts/MSMINCHO.TTF");
	map_fonts[Font_Names::ZELDA_] = App->font->Load("Assets/Fonts/Zelda.ttf", 60);

	UI_elements_tree = new nTree<UIElement*>((UIElement*)App->win->window);

	return ret;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	bool ret = true;

	UI_elements_list.clear();

	UI_elements_tree->recursivePreOrderList(UI_elements_tree->getRoot(), &UI_elements_list);

	if (drag_to_true) {
		SetUpDraggingNode(true);
		drag_to_true = false;
	}
	else if (drag_to_false) {
		SetUpDraggingNode(false);
		drag_to_false = false;
	}

	return ret;
}

bool j1Gui::Update(float dt)
{
	BROFILER_CATEGORY("UI_UpdateLogic", Profiler::Color::Azure);

	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
		debug_draw = !debug_draw;

	p2List_item<UIElement*>* UI_elem_it = UI_elements_list.start;

	while (UI_elem_it != nullptr) {
		UI_elem_it->data->Update(dt);

		if (UI_elem_it->data->drag && update_drag)
			UI_elem_it->data->UpdateDragging(dt);

		UI_elem_it = UI_elem_it->next;
	}

	UI_elem_it = UI_elements_list.start;

	return ret;
}

bool j1Gui::Blit(float dt) const
{
	BROFILER_CATEGORY("UI_BlitElements", Profiler::Color::Azure);

	bool ret = true;

	p2List_item<UIElement*>* UI_elem_it = UI_elements_list.start;

	while (UI_elem_it != nullptr) {
		UI_elem_it->data->Draw();
		UI_elem_it = UI_elem_it->next;
	}

	return ret;
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	bool ret = true;

	p2List_item<UIElement*>* UI_elem_it = UI_elements_list.start;

	while (UI_elem_it != nullptr) {

		if (UI_elem_it->data->HasToBeRemoved()) {
			RELEASE(UI_elem_it->data);
		}

		UI_elem_it = UI_elem_it->next;
	}

	return ret;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");
	bool ret = true;

	if (ClearAllUI()) {
		UI_elements_tree->clear();

		// Clear UI_elements list (active elements)
		p2List_item<UIElement*>* UI_elem_it = UI_elements_list.start;

		while (UI_elem_it != nullptr) {
			ret = true;
			RELEASE(UI_elem_it->data);
			UI_elem_it = UI_elem_it->next;
		}

		UI_elements_list.clear();
		LOG("All GUI removed");
	}

	// Remove textures
	if (ClearMapTextures())
		LOG("All GUI textures removed");

	return ret;
}

UIImage* j1Gui::CreateUIImage(iPoint local_pos, UIImage_Info& info, j1Module* listener, UIElement* parent)
{
	UIImage* image = new UIImage(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)image, parent);

	return image;
}

UILabel* j1Gui::CreateUILabel(iPoint local_pos, UILabel_Info& info, j1Module* listener, UIElement* parent)
{
	UILabel* label = new UILabel(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)label, parent);

	return label;
}

UIButton* j1Gui::CreateUIButton(iPoint local_pos, UIButton_Info& info, j1Module* listener, UIElement* parent)
{
	UIButton* button = new UIButton(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)button, parent);

	return button;
}

UICursor* j1Gui::CreateUICursor(UICursor_Info& info, j1Module* listener, UIElement* parent)
{
	iPoint local_pos = { 0,0 };

	UICursor* cursor = new UICursor(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)cursor, parent);

	return cursor;
}

UIWindow* j1Gui::CreateUIWindow(iPoint local_pos, UIWindow_Info& info, j1Module* listener, UIElement* parent)
{
	UIWindow* window = new UIWindow(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)window, parent);

	return window;
}

UILifeBar* j1Gui::CreateUILifeBar(iPoint local_pos, UILifeBar_Info& info, j1Module* listener, UIElement* parent)
{
	UILifeBar* life_bar = new UILifeBar(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)life_bar, parent);

	return life_bar;
}

UISlider* j1Gui::CreateUISlider(iPoint local_pos, UISlider_Info& info, j1Module* listener, UIElement* parent)
{
	UISlider* slider = new UISlider(local_pos, parent, info, listener);

	if (parent == nullptr)
		parent = (UIElement*)App->win->window;

	UI_elements_tree->insertChild((UIElement*)slider, parent);

	return slider;
}

bool j1Gui::DestroyElement(UIElement* elem)
{
	bool ret = false;

	UI_elements_tree->remove(elem);
	elem->to_remove = true;

	return ret;
}

bool j1Gui::ClearAllUI()
{
	bool ret = false;

	// Clear UI_elements tree
	UI_elements_tree->getRoot()->removeChildren();
	
	// Delete trans pointer to cursor?
	App->trans->CreateCursor();

	p2List_item<UIElement*>* UI_elem_it = UI_elements_list.start;

	while (UI_elem_it != nullptr) {

		if (UI_elem_it->data != (UIElement*)App->trans->l_level_name && UI_elem_it->data != (UIElement*)App->trans->l_cats_picked
			&& UI_elem_it->data != (UIElement*)App->trans->l_level_cats_picked && UI_elem_it->data != (UIElement*)App->trans->l_level_cats_picked2
			&& UI_elem_it->data != (UIElement*)App->trans->l_score_text && UI_elem_it->data != (UIElement*)App->trans->l_you
			&& UI_elem_it->data != (UIElement*)App->trans->l_died && UI_elem_it->data != (UIElement*)App->trans->black_screen_image)
			UI_elem_it->data->to_remove = true;

		UI_elem_it = UI_elem_it->next;
	}

	return ret;
}

bool j1Gui::ClearMapTextures() 
{
	bool ret = true;

	App->tex->UnLoad((SDL_Texture*)atlas);

	return ret;
}

_TTF_Font* j1Gui::GetFont(Font_Names font_name)
{
	return map_fonts[font_name];
}

const SDL_Texture* j1Gui::GetAtlas() const
{
	return atlas;
}

SDL_Rect j1Gui::GetRectFromAtlas(UIElement_Rect rect)
{
	return UI_elements_rects[rect];
}

void j1Gui::SetTextureAlphaMod(float alpha) 
{
	SDL_SetTextureAlphaMod((SDL_Texture*)atlas, (Uint8)alpha);
}

void j1Gui::ResetAlpha() 
{
	reset = true;
}

float j1Gui::IncreaseDecreaseAlpha(float from, float to, float seconds)
{
	float calculated_alpha = 0.0f;

	if (reset) {
		start_time = SDL_GetTicks();
		reset = false;
	}

	// Math operations
	total_time = (Uint32)(seconds * 0.5f * 1000.0f);

	Uint32 now = (SDL_GetTicks() - start_time);
	float normalized = MIN(1.0f, (float)now / (float)total_time);
	float normalized2 = MIN(1.0f, (float)now / (float)total_time);
	normalized2 = 1 - normalized2;

	float alpha = (to - from) * normalized;
	float alpha2 = (from - to) * normalized2;

	// Color change
	if (from > to)
		calculated_alpha = alpha2;
	else
		calculated_alpha = alpha;

	return calculated_alpha;
}

void j1Gui::SetUpDraggingChildren(UIElement* elem, bool drag)
{
	// List including to_drag element and all of its children
	p2List<UIElement*> to_drag;
	UI_elements_tree->recursivePreOrderList(UI_elements_tree->search(elem), &to_drag);

	// Don't drag elements which are not in the previous list
	p2List_item<UIElement*>* UI_elem_it = to_drag.start;
	UI_elem_it = UI_elements_list.start;
	
	while (UI_elem_it != nullptr && UI_elem_it->data != to_drag.start->data) {
		UI_elem_it->data->drag = false;
		UI_elem_it = UI_elem_it->next;
	}
	
	update_drag = drag;
}

void j1Gui::SetUpDraggingNode(bool drag) 
{
	UIElement* dragging_node = nullptr;
	int lowest_level = UI_elements_tree->getNumLevels(UI_elements_tree->getRoot()) + 1;

	// If several elements are clicked at once, select the element in the lowest level in the tree
	p2List_item<UIElement*>* UI_elem_it = UI_elements_list.start;

	while (UI_elem_it != nullptr) {

		if (UI_elem_it->data->drag == drag) {
			int level = UI_elements_tree->getNumLevels(UI_elements_tree->search(UI_elem_it->data));

			if (level <= lowest_level) {
				dragging_node = UI_elem_it->data;
			}
		}

		UI_elem_it = UI_elem_it->next;
	}

	SetUpDraggingChildren(dragging_node, drag);
}

// class Gui ---------------------------------------------------

