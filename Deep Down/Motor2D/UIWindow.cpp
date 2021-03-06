#include "UIWindow.h"
#include "j1Input.h"
#include "p2Log.h"
#include "j1Window.h"
#include "j1Render.h"

UIWindow::UIWindow(iPoint local_pos, UIElement* parent, UIWindow_Info& info, j1Module* listener) : UIElement(local_pos, parent, listener), window(info)
{
	type = UIElement_TYPE::WINDOW_;

	tex_area = App->gui->GetRectFromAtlas(window.tex_area);

	draggable = window.draggable;
	interactive = window.interactive;
	horizontal = window.horizontal_orientation;
	vertical = window.vertical_orientation;
	width = tex_area.w;
	height = tex_area.h;

	SetOrientation();
}

void UIWindow::Update(float dt)
{
	if (listener != nullptr && interactive)
		HandleInput();
}

void UIWindow::DebugDraw(iPoint blit_pos) const
{
	Uint8 alpha = 80;

	SDL_Rect quad = { blit_pos.x, blit_pos.y, width, height };
	App->render->DrawQuad(quad, 0, 150, 255, alpha, false);
}

void UIWindow::HandleInput()
{
	iPoint mouse_pos;
	App->input->GetMousePosition(mouse_pos.x, mouse_pos.y);

	switch (UIevent) {

	case UIEvents::NO_EVENT_:
		if (MouseHover()) {
			next_event = false;
			UIevent = UIEvents::MOUSE_ENTER_;
			break;
		}
		break;
	case UIEvents::MOUSE_ENTER_:

		if (!MouseHover()) {
			LOG("MOUSE LEAVE");
			next_event = false;
			UIevent = UIEvents::MOUSE_LEAVE_;
			break;
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == SDL_PRESSED) {
			next_event = false;
			LOG("MOUSE L CLICK START");
			UIevent = UIEvents::MOUSE_LEFT_CLICK_;
			listener->OnUIEvent((UIElement*)this, UIevent);
			break;
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == SDL_PRESSED) {
			next_event = false;
			LOG("MOUSE R CLICK START");

			mouse_click_pos.x = mouse_pos.x * App->win->GetScale() - GetLocalPos().x;
			mouse_click_pos.y = mouse_pos.y * App->win->GetScale() - GetLocalPos().y;

			if (draggable) {
				drag = true;
				App->gui->drag_to_true = true;
			}

			UIevent = UIEvents::MOUSE_RIGHT_CLICK_;
			listener->OnUIEvent((UIElement*)this, UIevent);
			break;
		}

		if (!next_event) {
			LOG("MOUSE ENTER");
			listener->OnUIEvent((UIElement*)this, UIevent);
			next_event = true;
		}

		break;
	case UIEvents::MOUSE_RIGHT_CLICK_:

		if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == SDL_RELEASED) {
			LOG("MOUSE R CLICK FINISH");

			if (draggable) {
				drag = false;
				App->gui->drag_to_false = true;
			}

			listener->OnUIEvent((UIElement*)this, UIevent);
			UIevent = UIEvents::MOUSE_ENTER_;
			break;
		}

		break;
	case UIEvents::MOUSE_LEFT_CLICK_:

		if (!MouseHover()) {
			LOG("MOUSE LEAVE");
			UIevent = UIEvents::MOUSE_LEAVE_;
			break;
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == SDL_RELEASED) {
			LOG("MOUSE L CLICK FINISH");
			// Uncomment next line and create a new UIEvent if you want to execute a function when mouse button is released
			//listener->OnUIEvent((UIElement*)this, UIevent);
			UIevent = UIEvents::MOUSE_ENTER_;
			break;
		}

		break;
	case UIEvents::MOUSE_LEAVE_:
		listener->OnUIEvent((UIElement*)this, UIevent);
		UIevent = UIEvents::NO_EVENT_;
		break;
	}
}

//---------------------------------------------------------------

bool UIWindow::SlideTransition(float dt, int end_pos_y, float speed, bool bounce, float bounce_interval, float bounce_speed, bool down)
{
	bool ret = false;

	iPoint pos = GetLocalPos();

	if (down) {
		if (pos.y + height >= (int)end_pos_y - height / 2) {
			if (bounce && !start_bouncing)
				start_bouncing = true;
			else if (!bounce)
				ret = true;
		}
		else if (!start_bouncing) {
			if ((int)(speed * dt) >= 1)
				IncreasePos({ 0,(int)(speed * dt) });
			else
				IncreasePos({ 0, 1 });
		}
	}
	else {
		if (pos.y + height <= (int)end_pos_y - height / 2) {
			if (bounce && !start_bouncing)
				start_bouncing = true;
			else if (!bounce)
				ret = true;
		}
		else if (!start_bouncing) {
			if ((int)(speed * dt) >= 1)
				DecreasePos({ 0,(int)(speed * dt) });
			else
				DecreasePos({ 0, 1 });
		}
	}

	if (start_bouncing) {
		if (Bounce(dt, bounce_interval, bounce_speed, down)) {
			ret = true;
		}
	}

	return ret;
}

bool UIWindow::Bounce(float dt, float bounce_interval, float bounce_speed, bool down) 
{
	bool ret = false;

	iPoint pos = GetLocalPos();

	if (reset) {
		InitializeBounce(bounce_interval, down);
		reset = false;
	}

	if (bounce_value <= bounce_speed)
		ret = true;

	if (first_bounce) {		
		if (pos.y >= start_pos.y + bounce_value) {
			bounce_value -= bounce_speed;
			first_bounce = false;
		}
		else {
			if ((int)(bounce_value * 10.0f * dt) >= 1)
				IncreasePos({ 0, (int)(bounce_value * 10.0f * dt) });
			else
				IncreasePos({ 0, 1 });
		}
	}
	else {		
		if (pos.y <= start_pos.y - bounce_value) {
			bounce_value -= bounce_speed;
			first_bounce = true;
		}
		else {
			if ((int)(bounce_value * 10.0f * dt) >= 1)
				DecreasePos({ 0, (int)(bounce_value * 10.0f * dt) });
			else
				DecreasePos({ 0, 1 });
		}
	}

	return ret;
}

void UIWindow::InitializeBounce(float bounce_interval, bool down)
{
	bounce_value = bounce_interval;
	start_pos = GetLocalPos();

	if (!down)
		first_bounce = false;
}