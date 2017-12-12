#include "UISlider.h"
#include "j1Render.h"

UISlider::UISlider(iPoint local_pos, UIElement* parent, UISlider_Info& info, j1Module* listener) : UIElement(local_pos, parent, listener), slider(info)
{
	type = UIElement_TYPE::SLIDER_;

	is_draggable = info.is_draggable;
	horizontal = info.horizontal_orientation;
	vertical = info.vertical_orientation;
	tex_area = info.tex_area;
	tex = App->gui->GetTexture(slider.tex_name);

	if (tex_area.w == 0)
		SDL_QueryTexture((SDL_Texture*)tex, NULL, NULL, &width, &height);
	else {
		width = tex_area.w;
		height = tex_area.h;
	}

	SetOrientation();
}

void UISlider::Draw() const
{
	iPoint blit_pos = { 0,0 };
	blit_pos.x = GetScreenPos().x - App->render->camera.x;
	blit_pos.y = GetScreenPos().y - App->render->camera.y;

	if (slider.quad) {
		SDL_SetRenderDrawColor(App->render->renderer, slider.color.r, slider.color.g, slider.color.b, slider.color.a);
		SDL_RenderFillRect(App->render->renderer, &slider.tex_area);
	}
	else {
		if (tex_area.w != 0)
			App->render->Blit(tex, blit_pos.x, blit_pos.y, &tex_area);
		else
			App->render->Blit(tex, blit_pos.x, blit_pos.y);
	}

	if (App->gui->debug_draw)
		DebugDraw(blit_pos);
}

void UISlider::DebugDraw(iPoint blit_pos) const
{
	Uint8 alpha = 80;

	SDL_Rect quad = { blit_pos.x, blit_pos.y, width, height };
	App->render->DrawQuad(quad, 255, 0, 40, alpha, false);
}