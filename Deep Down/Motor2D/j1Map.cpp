#include "p2Defs.h"
#include "p2Log.h"

#include "j1App.h"

#include "j1Input.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Collision.h"
#include "j1Scene.h"
#include "j1Audio.h"
#include "j1Player.h"
#include "j1Map.h"

#include <math.h>

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name.create("map");
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.create(config.child("folder").child_value());

	return ret;
}

void j1Map::Draw()
{
	if (map_loaded == false)
		return;

	// TODO 5: Prepare the loop to draw all tilesets + Blit
	p2List_item<TileSet*>* draw_tilesets = data.tilesets.start;
	while (draw_tilesets != NULL)
	{
		p2List_item<MapLayer*>* draw_layers = data.layers.start;
		while (draw_layers != NULL) {

			if (draw_layers->data->index != ABOVE) {

				for (int i = 0; i < draw_layers->data->width; i++) {
					for (int j = 0; j < draw_layers->data->height; j++) {

						if (draw_layers->data->Get(i, j) != 0) {

							SDL_Rect rect = draw_tilesets->data->GetTileRect(draw_layers->data->Get(i, j));
							SDL_Rect* section = &rect;

							iPoint world = MapToWorld(i, j);

							if (draw_layers->data->index == PARALLAX) {
								App->render->Blit(draw_tilesets->data->texture, world.x, world.y, section, draw_layers->data->speed);
							}
							else if (draw_layers->data->index == COLLISION) {
								if (App->collision->GetDebug())
									App->render->Blit(draw_tilesets->data->texture, world.x, world.y, section, draw_layers->data->speed);
							}
							else if (draw_layers->data->index == GATE) {
								if (App->scene->gate == false)
									App->render->Blit(draw_tilesets->data->texture, world.x, world.y, section, draw_layers->data->speed);
							}
							else {
								App->render->Blit(draw_tilesets->data->texture, world.x, world.y, section, draw_layers->data->speed);
							}
						}
					}
				}
			}
			draw_layers = draw_layers->next;
		}
		draw_tilesets = draw_tilesets->next;
	}
}

void j1Map::DrawAboveLayer()
{
	if (map_loaded == false)
		return;

	if (aboveLayer != nullptr) {

		p2List_item<TileSet*>* draw_tilesets = data.tilesets.start;
		while (draw_tilesets != NULL)
		{

			for (int i = 0; i < aboveLayer->width; i++) {
				for (int j = 0; j < aboveLayer->height; j++) {

					if (aboveLayer->Get(i, j) != 0) {

						SDL_Rect rect = draw_tilesets->data->GetTileRect(aboveLayer->Get(i, j));
						SDL_Rect* section = &rect;

						iPoint world = MapToWorld(i, j);

						App->render->Blit(draw_tilesets->data->texture, world.x, world.y, section, aboveLayer->speed);
					}
				}
			}
			draw_tilesets = draw_tilesets->next;
		}
	}
}

iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	ret.x = x * data.tile_width;
	ret.y = y * data.tile_height;

	return ret;
}

iPoint j1Map::WorldToMap(int x, int y) const
{
	iPoint ret;

	ret.x = x / data.tile_width;
	ret.y = y / data.tile_height;

	return ret;
}

iPoint j1Map::MouseTile(int x, int y) const
{
	iPoint ret;

	ret.x = x / data.tile_width;
	ret.y = y / data.tile_height;

	return ret;
}

SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all objectGroups
	p2List_item<ObjectGroup*>* objectGroup;
	objectGroup = data.objectGroups.start;

	while (objectGroup != NULL)
	{
		// Remove all objects
		p2List_item<Object*>* object;
		object = objectGroup->data->objects.start;

		while (object != NULL)
		{
			RELEASE(object->data);
			object = object->next;
		}

		objectGroup->data->objects.clear();

		RELEASE(objectGroup->data);
		objectGroup = objectGroup->next;
	}
	data.objectGroups.clear();

	// Remove all tilesets
	p2List_item<TileSet*>* item;
	item = data.tilesets.start;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// TODO 2: clean up all layer data
	// Remove all layers

	p2List_item<MapLayer*>* item1;
	item1 = data.layers.start;

	while (item1 != NULL)
	{
		RELEASE(item1->data);
		item1 = item1->next;
	}
	data.layers.clear();

	collisionLayer = nullptr;
	aboveLayer = nullptr;

	// Clean up the pugui tree
	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name)
{
	bool ret = true;
	p2SString tmp("%s%s", folder.GetString(), file_name);

	pugi::xml_parse_result result = map_file.load_file(tmp.GetString());

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	// Load general info ----------------------------------------------
	if(ret == true)
	{
		ret = LoadMap();
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for (tileset = map_file.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if (ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if (ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		data.tilesets.add(set);
	}

	// TODO 4: Iterate all layers and load each of them
	// Load layer info ----------------------------------------------
	pugi::xml_node layer;
	for (layer = map_file.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* set = new MapLayer();

		if (ret == true)
		{
			ret = LoadLayer(layer, set);
		}

		data.layers.add(set);
	}

	// Load ObjectGroups and GameObjects!!!!!
	
	pugi::xml_node objectGroup;
	pugi::xml_node object;

	for (objectGroup = map_file.child("map").child("objectgroup"); objectGroup && ret; objectGroup = objectGroup.next_sibling("objectgroup"))
	{
		ObjectGroup* set = new ObjectGroup();

		if (ret == true)
		{
			ret = LoadObjectGroupDetails(objectGroup, set);
		}

		for (object = objectGroup.child("object"); object && ret; object = object.next_sibling("object"))
		{
			Object* set1 = new Object();

			if (ret == true)
			{
				ret = LoadObject(object, set1);
			}

			set->objects.add(set1);
		}

		data.objectGroups.add(set);
	}
	

	if (ret == true)
	{
		LOG("Successfully parsed map XML file: %s", file_name);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tile_width, data.tile_height);

		p2List_item<TileSet*>* item = data.tilesets.start;
		while (item != NULL)
		{
			TileSet* s = item->data;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.GetString(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item = item->next;
		}

		// TODO 4: Add info here about your loaded layers
		// Adapt this vcode with your own variables

		p2List_item<MapLayer*>* item_layer = data.layers.start;
		while (item_layer != NULL)
		{
			MapLayer* l = item_layer->data;
			LOG("Layer ----");
			LOG("name: %s", l->name.GetString());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			item_layer = item_layer->next;
		}

		// Info about ObjectGroups and GameObjects!!!!!
		
		p2List_item<ObjectGroup*>* item_group = data.objectGroups.start;
		while (item_group != NULL)
		{
			ObjectGroup* s = item_group->data;
			LOG("Object Group ----");
			LOG("name: %s", s->name.GetString());

			p2List_item<Object*>* item_object = item_group->data->objects.start;
			while (item_object != NULL)
			{
				Object* s = item_object->data;
				LOG("Object ----");
				LOG("name: %s", s->name.GetString());
				LOG("id: %d", s->id);
				LOG("x: %d y: %d", s->x, s->y);
				LOG("width: %d height: %d", s->width, s->height);

				item_object = item_object->next;
			}

			item_group = item_group->next;
		}	
	}

	map_loaded = ret;

	return ret;
}

// Unload map
bool j1Map::UnLoad() {
	LOG("Unloading map");

	// Remove all objectGroups
	p2List_item<ObjectGroup*>* objectGroup;
	objectGroup = data.objectGroups.start;

	while (objectGroup != NULL)
	{
		// Remove all objects
		p2List_item<Object*>* object;
		object = objectGroup->data->objects.start;

		while (object != NULL)
		{
			RELEASE(object->data);
			object = object->next;
		}

		objectGroup->data->objects.clear();

		RELEASE(objectGroup->data);
		objectGroup = objectGroup->next;
	}
	data.objectGroups.clear();

	// Remove all tilesets
	p2List_item<TileSet*>* item;
	item = data.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// Remove all layers
	p2List_item<MapLayer*>* item1;
	item1 = data.layers.start;

	while (item1 != NULL)
	{
		RELEASE(item1->data);
		item1 = item1->next;
	}
	data.layers.clear();


	collisionLayer = nullptr;
	aboveLayer = nullptr;

	return true;
}

// Load map general properties
bool j1Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = map_file.child("map");

	if(map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tile_width = map.attribute("tilewidth").as_int();
		data.tile_height = map.attribute("tileheight").as_int();
		p2SString bg_color(map.attribute("backgroundcolor").as_string());

		data.background_color.r = 0;
		data.background_color.g = 0;
		data.background_color.b = 0;
		data.background_color.a = 0;

		if(bg_color.Length() > 0)
		{
			p2SString red, green, blue;
			bg_color.SubString(1, 2, red);
			bg_color.SubString(3, 4, green);
			bg_color.SubString(5, 6, blue);

			int v = 0;

			sscanf_s(red.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.r = v;

			sscanf_s(green.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.g = v;

			sscanf_s(blue.GetString(), "%x", &v);
			if(v >= 0 && v <= 255) data.background_color.b = v;
		}

		p2SString orientation(map.attribute("orientation").as_string());

		if(orientation == "orthogonal")
		{
			data.type = MAPTYPE_ORTHOGONAL;
		}
		else if(orientation == "isometric")
		{
			data.type = MAPTYPE_ISOMETRIC;
		}
		else if(orientation == "staggered")
		{
			data.type = MAPTYPE_STAGGERED;
		}
		else
		{
			data.type = MAPTYPE_UNKNOWN;
		}
	}

	return ret;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	set->name.create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if(offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if(image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->Load(PATH(folder.GetString(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->tex_width = image.attribute("width").as_int();

		if(set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if(set->tex_height <= 0)
		{
			set->tex_height = h;
		}

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;
	}

	return ret;
}

// TODO 3: Create the definition for a function that loads a single layer
bool j1Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name = node.attribute("name").as_string();

	// Set layer index
	if (layer->name == "Collision") {
		collisionLayer = layer;
		layer->index = COLLISION;
	}
	else if (layer->name == "Above") {
		aboveLayer = layer;
		layer->index = ABOVE;
	}
	else if (layer->name == "Parallax") {
		layer->index = PARALLAX;
	}
	else if (layer->name == "Gate") {
		layer->index = GATE;
	}

	layer->width = node.attribute("width").as_uint();
	layer->height = node.attribute("height").as_uint();

	layer->data = new uint[layer->width * layer->height];

	memset(layer->data, 0, layer->width * layer->height);
	
	int i = 0;

	for (pugi::xml_node tile_gid = node.child("data").child("tile"); tile_gid; tile_gid =  tile_gid.next_sibling("tile")) {
		layer->data[i] = tile_gid.attribute("gid").as_uint();
		i++;
	}

	// Read layer properties
	pugi::xml_node speed = node.child("properties").child("property");
	if (speed != nullptr) {		
		p2SString name = speed.attribute("name").as_string();
		if (name == "Speed")
			layer->speed = speed.attribute("value").as_float();
	}

	return ret;
}

inline uint MapLayer::Get(int x, int y) const {
	return data[width * y + x];
}

bool j1Map::LoadObjectGroupDetails(pugi::xml_node& objectGroup_node, ObjectGroup* objectGroup)
{
	bool ret = true;
	objectGroup->name.create(objectGroup_node.attribute("name").as_string());

	return ret;
}

bool j1Map::LoadObject(pugi::xml_node& object_node, Object* object)
{
	bool ret = true;

	object->name = object_node.attribute("name").as_string();
	object->id = object_node.attribute("id").as_uint();
	object->width = object_node.attribute("width").as_uint();
	object->height = object_node.attribute("height").as_uint();
	object->x = object_node.attribute("x").as_uint();
	object->y = object_node.attribute("y").as_uint();

	return ret;
}

fPoint MapData::GetObjectPosition(p2SString groupObject, p2SString object) {

	fPoint pos = { 0,0 };

	p2List_item<ObjectGroup*>* item;
	item = objectGroups.start;

	int ret = true;

	while (item != NULL && ret)
	{
		if (item->data->name == groupObject) {

			p2List_item<Object*>* item1;
			item1 = item->data->objects.start;

			while (item1 != NULL && ret)
			{
				if (item1->data->name == object) {
					pos.x = item1->data->x;
					pos.y = item1->data->y;

					ret = false;
				}

				item1 = item1->next;
			}
		}

		item = item->next;
	}

	return pos;
}

fPoint MapData::GetObjectSize(p2SString groupObject, p2SString object) {

	fPoint size = { 0,0 };

	p2List_item<ObjectGroup*>* item;
	item = objectGroups.start;

	int ret = true;

	while (item != NULL && ret)
	{
		if (item->data->name == groupObject) {

			p2List_item<Object*>* item1;
			item1 = item->data->objects.start;

			while (item1 != NULL && ret)
			{
				if (item1->data->name == object) {
					size.x = item1->data->width;
					size.y = item1->data->height;

					ret = false;
				}

				item1 = item1->next;
			}
		}

		item = item->next;
	}

	return size;
}

bool MapData::CheckIfEnter(p2SString groupObject, p2SString object, fPoint position) {

	fPoint objectPos = GetObjectPosition(groupObject, object);
	fPoint objectSize = GetObjectSize(groupObject, object);

	return (objectPos.x < position.x + 1 && objectPos.x + objectSize.x > position.x && objectPos.y < position.y + 1 && objectSize.y + objectPos.y > position.y);
}

void j1Map::CheckNextTile(iPoint position, iPoint size, int offset, int pixels, bool& left, bool& right, bool& up, bool& down) {

	if (collisionLayer != nullptr) {

		iPoint leftUp;
		iPoint leftDown;
		iPoint rightUp;
		iPoint rightDown;

		int collLeftUp;
		int collLeftDown;
		int collRightUp;
		int collRightDown;

		//Up
		leftUp = WorldToMap(position.x + offset, position.y - pixels);
		rightUp = WorldToMap(position.x + size.x - offset, position.y - pixels);

		collRightUp = collisionLayer->Get(rightUp.x, rightUp.y);
		collLeftUp = collisionLayer->Get(leftUp.x, leftUp.y);

		if (collRightUp == 1181 || (collRightUp == 1182 && App->scene->gate == false) || collLeftUp == 1181 || (collLeftUp == 1182 && App->scene->gate == false))
			up = false;
		else
			up = true;

		//Down
		leftDown = WorldToMap(position.x + offset, position.y + size.y + pixels);
		rightDown = WorldToMap(position.x + size.x - offset, position.y + size.y + pixels);

		collRightDown = collisionLayer->Get(rightDown.x, rightDown.y);
		collLeftDown = collisionLayer->Get(leftDown.x, leftDown.y);

		if (collRightDown == 1181 || (collRightDown == 1182 && App->scene->gate == false) || collLeftDown == 1181 || (collLeftDown == 1182 && App->scene->gate == false))
			down = false;
		else
			down = true;

		//Right
		rightUp = WorldToMap(position.x + size.x + pixels, position.y + offset);
		rightDown = WorldToMap(position.x + size.x + pixels, position.y + size.y - offset);

		collRightUp = collisionLayer->Get(rightUp.x, rightUp.y);
		collRightDown = collisionLayer->Get(rightDown.x, rightDown.y);

		if (collRightUp == 1181 || (collRightUp == 1182 && App->scene->gate == false) || collRightDown == 1181 || (collRightDown == 1182 && App->scene->gate == false))
			right = false;
		else
			right = true;

		//Left
		leftUp = WorldToMap(position.x - pixels, position.y + offset);
		leftDown = WorldToMap(position.x - pixels, position.y + size.y - offset);

		collLeftUp = collisionLayer->Get(leftUp.x, leftUp.y);
		collLeftDown = collisionLayer->Get(leftDown.x, leftDown.y);

		if (collLeftUp == 1181 || (collLeftUp == 1182 && App->scene->gate == false) || collLeftDown == 1181 || (collLeftDown == 1182 && App->scene->gate == false)) {
			left = false;
		}
		else {
			left = true;
		}
	}
}

void j1Map::CheckTile(iPoint position, iPoint size, int offset, bool& left, bool& right, bool& up, bool& down) {

	if (collisionLayer != nullptr) {

		iPoint leftUp;
		iPoint leftDown;
		iPoint rightUp;
		iPoint rightDown;

		int collLeftUp;
		int collLeftDown;
		int collRightUp;
		int collRightDown;

		//Up
		leftUp = WorldToMap(position.x + offset, position.y);
		rightUp = WorldToMap(position.x + size.x - offset, position.y);

		collRightUp = collisionLayer->Get(rightUp.x, rightUp.y);
		collLeftUp = collisionLayer->Get(leftUp.x, leftUp.y);

		if (collRightUp == 1183 || collLeftUp == 1183) {
			App->player->SetState(punished_);
			App->audio->PlayFx(5);
		}
		//Down
		leftDown = WorldToMap(position.x + offset, position.y + size.y);
		rightDown = WorldToMap(position.x + size.x - offset, position.y + size.y);

		collRightDown = collisionLayer->Get(rightDown.x, rightDown.y);
		collLeftDown = collisionLayer->Get(leftDown.x, leftDown.y);

		if (collRightUp == 1183 || collLeftUp == 1183) {
			App->player->SetState(punished_);
			App->audio->PlayFx(5);
		}

		//Right
		rightUp = WorldToMap(position.x + size.x, position.y + offset);
		rightDown = WorldToMap(position.x + size.x, position.y + size.y - offset);

		collRightUp = collisionLayer->Get(rightUp.x, rightUp.y);
		collRightDown = collisionLayer->Get(rightDown.x, rightDown.y);

		if (collRightUp == 1183 || collLeftUp == 1183) {
			App->player->SetState(punished_);
			App->audio->PlayFx(5);
		}

		//Left
		leftUp = WorldToMap(position.x, position.y + offset);
		leftDown = WorldToMap(position.x, position.y + size.y - offset);

		collLeftUp = collisionLayer->Get(leftUp.x, leftUp.y);
		collLeftDown = collisionLayer->Get(leftDown.x, leftDown.y);

		if (collRightUp == 1183 || collLeftUp == 1183) {
			App->player->SetState(punished_);
			App->audio->PlayFx(5);
		}
		
	}
}