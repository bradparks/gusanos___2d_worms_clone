#ifndef GFX_H
#define GFX_H

#include <allegro.h>

#include <string>
#include <list>

class Gfx
{
	public:
		
	Gfx();
	~Gfx();
	
	void init();
	void shutDown();
	void registerInConsole();
	
	BITMAP* loadBitmap(const std::string &filename, RGB* palette = NULL);
	bool saveBitmap(const std::string &filename, BITMAP* image, RGB* palette = NULL);
	
	inline void updateScreen()
	{
		if ( m_vsync == 1 ) vsync();
		blit(buffer,screen,0,0,0,0,320,240);
		if (m_clearBuffer == 1 ) clear_bitmap(buffer);
	}
	
	BITMAP* buffer;
	
	private:
		
	int m_vsync;
	int m_clearBuffer;

};

std::string screenShot(const std::list<std::string> &args);

extern Gfx gfx;

#endif // _GFX_H_