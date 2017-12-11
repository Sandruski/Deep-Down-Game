#ifndef __UIElement_H__
#define __UIElement_H__

#include "SDL\include\SDL.h"
#include "p2Defs.h"

#include "p2Point.h"

class j1Module;

enum UIElement_TYPE
{
	NO_TYPE_,
	IMAGE_,
	LABEL_,
	BUTTON_,
	WINDOW_,
	LIFEBAR_,
	MAX_TYPES_
};

enum UIElement_HORIZONTAL_POS {
	LEFT_,
	RIGHT_,
	CENTER_
};

enum UIElement_VERTICAL_POS {
	TOP_,
	BOTTOM_,
	MIDDLE_
};

enum UIEvents {
	NO_EVENT_,
	MOUSE_ENTER_,
	MOUSE_LEAVE_,
	MOUSE_RIGHT_CLICK_,
	MOUSE_LEFT_CLICK_,
	MOUSE_UP_,
	MAX_EVENTS_
};

// ---------------------------------------------------

class UIElement
{
protected:
	UIElement_TYPE type = UIElement_TYPE::NO_TYPE_;
	UIElement_HORIZONTAL_POS horizontal = UIElement_HORIZONTAL_POS::LEFT_;
	UIElement_VERTICAL_POS vertical = UIElement_VERTICAL_POS::TOP_;
	j1Module* listener = nullptr;

	bool to_remove = false;

	bool is_draggable = false;

	// Texture parameters
	const SDL_Texture* tex = nullptr;
	SDL_Rect tex_area = { 0,0,0,0 };
	int width = 0, height = 0;

private:
	iPoint local_pos = { 0,0 };
	UIElement* parent = nullptr;

private:
	SDL_Rect screen = { 0,0,0,0 };

public:
	UIElement(iPoint local_pos, UIElement* parent, j1Module* listener);

	virtual ~UIElement();

	virtual void Update(float dt);

	virtual void Draw() const;

	virtual void HandleInput();

	virtual bool HasToBeRemoved() const;

	virtual UIElement_TYPE GetType() const;

	virtual void DebugDraw(iPoint blit_pos) const;

	bool MouseHover() const;

	virtual void UpdateDragging(float dt);

	void SetOrientation();

	SDL_Rect GetScreenRect() const;
	SDL_Rect GetLocalRect() const;
	iPoint GetScreenPos() const;
	iPoint GetLocalPos() const;
	void SetLocalPos(iPoint local_pos);

public:
	bool drag = false;
	iPoint mouse_click_pos = { 0,0 };
};

#endif // __UIElement_H__