#include "menu.h"
#include "keyboard.h"
#include "gfx.h"
#include "font.h"
#include "gconsole.h"
#include <boost/bind.hpp>
#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
using std::cout;
using std::endl;

namespace OmfgGUI
{

//GusanosFont defaultMenuFont;
GContext menu;

std::string cmdLoadXML(std::list<std::string> const& args)
{
	if(args.size() > 0)
	{
		std::string ret;

		std::list<std::string>::const_iterator i = args.begin();
		std::string path;
		Wnd* loadTo = menu.getRoot();
		
		path = *i++;
		if(i != args.end())
			loadTo = menu.findNamedWindow(*i++);
			
		if(!loadTo)
			return "DESTINATION WINDOW NOT FOUND";
			
		std::ifstream f(path.c_str());
		if(f)
		{
			menu.buildFromXML(f, loadTo);
			return "";
		}
		else
			return "ERROR LOADING \"" + path + '"';
	}
	
	return "GUI_LOADXML <FILE> [<DEST>] : LOADS AN XML FILE INTO A WINDOW (ROOT BY DEFAULT)";
}

std::string cmdLoadGSS(std::list<std::string> const& args)
{
	if(args.size() > 0)
	{
		std::string ret;
				
		std::list<std::string>::const_iterator i = args.begin();
		std::string path = *i++;
		
		bool passive = false;
		for(; i != args.end(); ++i)
		{
			if(*i == "passive")
				passive = true;
		}
		

		std::ifstream f(path.c_str());
		if(f)
		{
			menu.loadGSS(f);
			if(!passive) menu.updateGSS();
			return "";
		}
		else
			return "ERROR LOADING \"" + path + '"';
	}
	
	return "GUI_LOADGSS <FILE> : LOADS A GSS FILE";
}

std::string cmdGSS(std::list<std::string> const& args)
{
	if(args.size() > 0)
	{
		std::string ret;
				
		std::list<std::string>::const_iterator i = args.begin();
		std::string gss = *i++;
		
		bool passive = false;
		for(; i != args.end(); ++i)
		{
			if(*i == "passive")
				passive = true;
		}
		

		std::istringstream f(gss);

		menu.loadGSS(f);
		if(!passive) menu.updateGSS();
		
		return "";
	}
	
	return "GUI_GSS <GSS> : LOADS INLINED GSS";
}

std::string cmdFocus(std::list<std::string> const& args)
{
	if(args.size() > 0)
	{
		std::string ret;
				
		std::list<std::string>::const_iterator i = args.begin();
		std::string const& name = *i++;

		Wnd* newFocus = menu.findNamedWindow(name);

		if(newFocus)
		{
			menu.setFocus(newFocus);
			return "";
		}
		else
			return "NO WINDOW WITH ID \"" + name + '"';
	}
	
	return "GUI_FOCUS <WINDOW ID> : FOCUSES A WINDOW";
}

void GContext::init()
{
	keyHandler.keyDown.connect(boost::bind(&GContext::eventKeyDown, this, _1));
	keyHandler.keyUp.connect(boost::bind(&GContext::eventKeyUp, this, _1));
	
	console.registerCommands()
		("GUI_LOADXML", cmdLoadXML)
		("GUI_LOADGSS", cmdLoadGSS)
		("GUI_GSS", cmdGSS)
		("GUI_FOCUS", cmdFocus)
	;
}

bool GContext::eventKeyDown(int k)
{
	Wnd* focus = getFocus();
	if(focus)
	{
		Wnd* parent = focus->getParent();
		if(parent)
		{
			switch(k)
			{
				case KEY_UP:
				{
					if(Wnd* next = parent->findClosestChild(
						focus,
						Wnd::Up))
					{
						setFocus(next);
					}
						
				}
				break;
				
				case KEY_DOWN:
				{
					if(Wnd* next = parent->findClosestChild(
						focus,
						Wnd::Down))
					{
						setFocus(next);
					}
						
				}
				break;
				
				case KEY_LEFT:
				{
					if(Wnd* next = parent->findClosestChild(
						focus,
						Wnd::Left))
					{
						setFocus(next);
					}
						
				}
				break;
				
				case KEY_RIGHT:
				{
					if(Wnd* next = parent->findClosestChild(
						focus,
						Wnd::Right))
					{
						setFocus(next);
					}
						
				}
				break;
				
				
			}
		}
		
		switch(k)
		{
			case KEY_ENTER:
			{
				std::string cmd;
				if(focus->getAttrib("command", cmd))
				{
					console.parseLine(cmd);
				}
			}
			break;
		}
	}

	return true;
}

bool GContext::eventKeyUp(int k)
{
	/*Wnd* focus = getFocus();
	if(focus)
	{
		
	}*/
	
	return true;
}

void GContext::clear()
{
	delete m_rootWnd;
	m_rootWnd = 0;
	
	std::istringstream rootGSS(
		"#root { background: #000080 ; left: 0 ; top: 0 ; bottom : -1 ; right: -1; padding: 29; spacing: 20 }");
		
	std::istringstream rootXML("<window id=\"root\" />");
	
	loadGSS(rootGSS);
	buildFromXML(rootXML, 0);
}

BaseFont* GContext::loadFont(std::string const& name)
{
	Font* f = fontLocator.load(name);
	if(!f)
		return 0;
	return new GusanosFont(f);
}

int allegroColor(RGB const& rgb)
{
	return makecol(rgb.r, rgb.g, rgb.b);
}

// Draws a box
void AllegroRenderer::drawBox(
	Rect const& rect,
	RGB const& color,
	RGB const& borderLeftColor,
	RGB const& borderTopColor,
	RGB const& borderRightColor,
	RGB const& borderBottomColor)
{
	rectfill(gfx.buffer, rect.x1, rect.y1, rect.x2, rect.y2, allegroColor(color));
	hline(gfx.buffer, rect.x1, rect.y2, rect.x2, allegroColor(borderBottomColor));
	vline(gfx.buffer, rect.x2, rect.y1, rect.y2, allegroColor(borderRightColor));
	vline(gfx.buffer, rect.x1, rect.y1, rect.y2, allegroColor(borderLeftColor));
	hline(gfx.buffer, rect.x1, rect.y1, rect.x2, allegroColor(borderTopColor));
	
	//TODO: Render borders
}

// Draws text
void AllegroRenderer::drawText(BaseFont const& font, std::string const& str, ulong flags, ulong x, ulong y, RGB const& aColor)
{
	const int spacing = 0;

	if(GusanosFont const* f = dynamic_cast<GusanosFont const*>(&font))
	{
		if(flags & (BaseFont::CenterH | BaseFont::CenterV))
		{
			std::pair<int, int> dim = f->font->getDimensions(str);
			
			if(flags & BaseFont::CenterH)
				x -= dim.first / 2;
			if(flags & BaseFont::CenterV)
				y -= dim.second / 2;
		}
		
		f->font->draw(gfx.buffer, str, x, y, spacing);
	}
}

void AllegroRenderer::drawSprite(BaseSpriteSet const& spriteSet, int frame, ulong x, ulong y)
{
	//TODO: Render sprite
}

void AllegroRenderer::setClip(Rect const& rect)
{
	set_clip_rect(gfx.buffer, rect.x1, rect.y1, rect.x2, rect.y2);
}

Rect const& AllegroRenderer::getClip()
{
	get_clip_rect(gfx.buffer, &clipRect.x1, &clipRect.y1, &clipRect.x2, &clipRect.y2);
	return clipRect;
}

Rect const& AllegroRenderer::getViewportRect()
{
	screenRect = Rect(0, 0, SCREEN_W - 1, SCREEN_H - 1);
	return screenRect;
}


}