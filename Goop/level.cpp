#include "level.h"

#include "gfx.h"
#include "material.h"
#include <allegro.h>
#include <string>
#include <vector>

using namespace std;

Level::Level()
{
	loaded = false;
	
	image = NULL;
	material = NULL;
	background = NULL;
	
	m_materialList.insert( m_materialList.begin(), 257, Material() );

	m_materialList[0].worm_pass = false;
	m_materialList[0].particle_pass = false;
	
	m_materialList[1].worm_pass = false;
	m_materialList[1].particle_pass = false;
}

Level::~Level()
{
}

bool Level::load(const string &name)
{
	path = name;
	int vdepth=get_color_depth();
	set_color_depth(8);
	material = gfx.loadBitmap((name + "/material").c_str(),0);
	set_color_depth(vdepth);
	if (material)
	{
		image = gfx.loadBitmap((name + "/level").c_str(),0);
		if (image)
		{
			background = gfx.loadBitmap((name + "/background").c_str(),0);
			if(!background)
			{
				background = create_bitmap(material->w, material->h);
				for (int x = 0; x < background->w; x++)
				for (int y = 0; y < background->h; y++)
				{
					putpixel(background, x, y, getpixel(image, x, y));
				}
			}
			loaded = true;
			return true;
		}
	}
	unload();
	return false;
}

void Level::unload()
{
	loaded = false;
	path = "";
	if (image) destroy_bitmap(image);
	if (material) destroy_bitmap(material);
	if (background) destroy_bitmap(background);
	image = NULL;
	material = NULL;
	background = NULL;
}

bool Level::isLoaded()
{
	return loaded;
}

void Level::draw(BITMAP* where, int x, int y)
{
	if (image)
	{
		blit(image,where,x,y,0,0,where->w,where->h);
	}
}

const Material& Level::getMaterial(int x, int y)
{
	return m_materialList[getpixel(material,x,y)+1];
}

int Level::width()
{
	if ( material )
		return material->w;
	else
		return 0;
}

int Level::height()
{
	if ( material )
		return material->h;
	else
		return 0;
}

void Level::setName(const std::string& _name)
{
	name = _name;
}

const string& Level::getPath()
{
	return path;
}

const string& Level::getName()
{
	return name;
}
