#include "liero.h"
#include "../gfx.h"
#include "../vec.h"
#include "zlib.h"
#include <string>
#include <cstring>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
namespace fs = boost::filesystem;

LieroLevelLoader LieroLevelLoader::instance;

bool LieroLevelLoader::canLoad(fs::path const& path, std::string& name)
{
	if(fs::extension(path) == ".lev")
	{
		name = basename(path);
		return true;
	}
	return false;
}

static unsigned char const lieroPalette[] = 
{0x0,0x0,0x0,0x6c,0x38,0x0,0x6c,0x50,0x0,0xa4,0x94,0x80,0x0,0x90,0x0,0x3c,0xac,0x3c,0xfc,0x54,0x54,0xa8,0xa8,0xa8,0x54,
0x54,0x54,0x54,0x54,0xfc,0x54,0xd8,0x54,0x54,0xfc,0xfc,0x78,0x40,0x8,0x80,0x44,0x8,0x88,0x48,0xc,0x90,0x50,0x10,0x98,
0x54,0x14,0xa0,0x58,0x18,0xac,0x60,0x1c,0x4c,0x4c,0x4c,0x54,0x54,0x54,0x5c,0x5c,0x5c,0x64,0x64,0x64,0x6c,0x6c,0x6c,
0x74,0x74,0x74,0x7c,0x7c,0x7c,0x84,0x84,0x84,0x8c,0x8c,0x8c,0x94,0x94,0x94,0x9c,0x9c,0x9c,0x38,0x38,0x88,0x50,0x50,
0xc0,0x68,0x68,0xf8,0x90,0x90,0xf4,0xb8,0xb8,0xf4,0x6c,0x6c,0x6c,0x90,0x90,0x90,0xb4,0xb4,0xb4,0xd8,0xd8,0xd8,0x20,
0x60,0x20,0x2c,0x84,0x2c,0x3c,0xac,0x3c,0x70,0xbc,0x70,0xa4,0xd4,0xa4,0x6c,0x6c,0x6c,0x90,0x90,0x90,0xb4,0xb4,0xb4,
0xd8,0xd8,0xd8,0xa8,0xa8,0xf8,0xd0,0xd0,0xf4,0xfc,0xfc,0xf4,0x3c,0x50,0x0,0x58,0x70,0x0,0x74,0x90,0x0,0x94,0xb0,0x0,
0x78,0x48,0x34,0x9c,0x78,0x58,0xc4,0xa8,0x7c,0xec,0xd8,0xa0,0x9c,0x78,0x58,0xc4,0xa8,0x7c,0xec,0xd8,0xa0,0xc8,0x64,
0x0,0xa0,0x50,0x0,0x48,0x48,0x48,0x6c,0x6c,0x6c,0x90,0x90,0x90,0xb4,0xb4,0xb4,0xd8,0xd8,0xd8,0xfc,0xfc,0xfc,0xc4,0xc4,
0xc4,0x90,0x90,0x90,0x98,0x3c,0x0,0xb4,0x64,0x0,0xd0,0x8c,0x0,0xec,0xb4,0x0,0xa8,0x54,0x0,0xd8,0x0,0x0,0xbc,0x0,0x0,
0xa4,0x0,0x0,0xc8,0x0,0x0,0xac,0x0,0x0,0xd8,0x0,0x0,0xbc,0x0,0x0,0xa4,0x0,0x0,0xd8,0x0,0x0,0xbc,0x0,0x0,0xa4,0x0,0x0,
0x50,0x50,0xc0,0x68,0x68,0xf8,0x90,0x90,0xf4,0x50,0x50,0xc0,0x68,0x68,0xf8,0x90,0x90,0xf4,0x94,0x88,0x0,0x88,0x7c,0x0,
0x7c,0x70,0x0,0x74,0x64,0x0,0x84,0x5c,0x28,0xa0,0x84,0x48,0xbc,0xb0,0x68,0xd8,0xdc,0x88,0xf8,0xf8,0xbc,0xf4,0xf4,0xfc,
0xfc,0x0,0x0,0xf8,0x18,0x4,0xf8,0x34,0x8,0xf8,0x50,0x10,0xf8,0x6c,0x14,0xf8,0x88,0x18,0xf8,0xa4,0x20,0xf8,0xc0,0x24,
0xf8,0xdc,0x28,0xf4,0xe8,0x3c,0xf4,0xf4,0x50,0xf4,0xf4,0x70,0xf4,0xf4,0x94,0xf0,0xf0,0xb4,0xf0,0xf0,0xd4,0xf0,0xf0,
0xf8,0x2c,0x84,0x2c,0x3c,0xac,0x3c,0x70,0xbc,0x70,0x2c,0x84,0x2c,0x3c,0xac,0x3c,0x70,0xbc,0x70,0xf8,0x3c,0x3c,0xf4,
0x7c,0x7c,0xf4,0xbc,0xbc,0x68,0x68,0xf8,0x90,0x90,0xf4,0xb8,0xb8,0xf4,0x90,0x90,0xf4,0x3c,0xac,0x3c,0x70,0xbc,0x70,
0xa4,0xd4,0xa4,0x70,0xbc,0x70,0x94,0x88,0x0,0x88,0x74,0x0,0x7c,0x60,0x0,0x70,0x4c,0x0,0x64,0x38,0x0,0x58,0x28,0x0,
0x68,0x68,0x88,0x90,0x90,0xc0,0xbc,0xbc,0xf8,0xc8,0xc8,0xf4,0xdc,0xdc,0xf4,0x28,0x70,0x28,0x2c,0x84,0x2c,0x34,0x98,
0x34,0x3c,0xac,0x3c,0xfc,0xc8,0xc8,0xf4,0xa4,0xa4,0xf8,0x5c,0x5c,0xf4,0x4c,0x4c,0xf4,0x3c,0x3c,0xf4,0x4c,0x4c,0xf4,
0x5c,0x5c,0xf4,0xa4,0xa4,0x54,0x28,0x0,0x58,0x28,0x0,0x5c,0x2c,0x0,0x60,0x30,0x0,0x3c,0x1c,0x0,0x40,0x1c,0x0,0x44,
0x20,0x0,0x48,0x24,0x0,0xfc,0xfc,0xfc,0xdc,0xdc,0xdc,0xbc,0xbc,0xbc,0x9c,0x9c,0x9c,0x7c,0x7c,0x7c,0x9c,0x9c,0x9c,
0xbc,0xbc,0xbc,0xdc,0xdc,0xdc,0x6c,0x4c,0x2c,0x7c,0x54,0x30,0x8c,0x60,0x38,0x9c,0x6c,0x40,0xac,0x78,0x48,0x0,0x0,
0x0,0x28,0x24,0x8,0x50,0x4c,0x14,0x78,0x74,0x1c,0xa0,0x98,0x28,0xc8,0xc0,0x30,0xf4,0xe8,0x3c,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfc,0x0,0x0,0xfc,0x24,0x0,
0xfc,0x48,0x0,0xfc,0x6c,0x0,0xfc,0x90,0x0,0xfc,0xb4,0x0,0xfc,0xd8,0x0,0xfc,0xfc,0x0,0xa8,0xf0,0x0,0x54,0xe8,0x0,
0x0,0xe0,0x0,0xfc,0x0,0x0,0xe8,0x4,0x14,0xd8,0xc,0x2c,0xc4,0x14,0x44,0xb4,0x18,0x58,0xa0,0x20,0x70,0x90,0x28,0x88,
0x7c,0x2c,0x9c,0x6c,0x34,0xb4,0x58,0x3c,0xcc,0x48,0x44,0xe4};

bool LieroLevelLoader::load(Level* level, fs::path const& path)
{
	fs::ifstream f(path);
	if(!f)
		return false;
		
	f.seekg(0, std::ios::end);
	std::streamoff fileSize = f.tellg();
	f.seekg(0, std::ios::beg);
	
	const int width = 504;
	const int height = 350;
	const std::streamoff regularFileSize = width*height;
	if(fileSize < regularFileSize)
		return false;
	
	level->material = create_bitmap_ex(8, width, height);
	level->image = create_bitmap(width, height);
	level->background = create_bitmap(width, height);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int c = f.get();
			
			unsigned char const* entry = &lieroPalette[c * 3];
			int imagec = makecol(entry[0], entry[1], entry[2]);
			putpixel(level->image, x, y, imagec);
			
			entry = &lieroPalette[(160 + (rndgen() & 3)) * 3];
			int backgroundc = makecol(entry[0], entry[1], entry[2]);
			putpixel(level->background, x, y, backgroundc);
			
			putpixel(level->material, x, y, 1); //TODO
		}
	}
	
	//TODO
	level->m_materialList[2].worm_pass = true;
	level->m_materialList[2].particle_pass = true;

	level->loaded = true;
	return true;
}

const char* LieroLevelLoader::getName()
{
	return "Liero level loader";
}
