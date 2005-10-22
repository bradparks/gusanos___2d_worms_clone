#ifndef DEDSERV

#include "viewport.h"

#include "game.h"
#include "sfx.h"
#include <allegro.h>
#include "base_worm.h"
#include "base_player.h"
#include "player.h"
#include "glua.h"
#include "lua/bindings-gfx.h"
#include "blitters/blitters.h"
#include "culling.h"
#include <list>


#include <iostream>

using namespace std;

Viewport::Viewport()
: m_dest(0)
{
	lua.pushFullReference(*this, LuaBindings::viewportMetaTable);
	luaReference = lua.createReference();
}

Viewport::~Viewport()
{
	lua.destroyReference(luaReference);
	destroy_bitmap(m_dest);
	sfx.freeListener(m_listener);
}

struct TestCuller
{
	TestCuller(BITMAP* dest_, BITMAP* src_, int scrOffX_, int scrOffY_, int destOffX_, int destOffY_)
	: dest(dest_), src(src_), scrOffX(scrOffX_), scrOffY(scrOffY_), destOffX(destOffX_), destOffY(destOffY_)
	{
		
	}
	
	bool block(int x, int y)
	{
		return !game.level.unsafeGetMaterial(x, y).worm_pass;
	}
	
	void line(int y, int x1, int x2)
	{
		//hline_add(dest, x1 + scrOffX, y + scrOffY, x2 + scrOffX + 1, makecol(50, 50, 50), 255);
		
	
		drawSpriteLine_add(
			dest,
			src,
			x1 + scrOffX,
			y + scrOffY,
			x1 + destOffX,
			y + destOffY,
			x2 + destOffX + 1,
			255);
	}
	
	BITMAP* dest;
	BITMAP* src;

	int scrOffX;
	int scrOffY;
	int destOffX;
	int destOffY;
};

//static BITMAP* testLight = 0;

void Viewport::setDestination(BITMAP* where, int x, int y, int width, int height)
{
	if(width > where->w
	|| height > where->h)
		return;
	
	destroy_bitmap(m_dest);
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( x + width > where->w ) x = where->w - width;
	if ( y + height > where->h ) y = where->h - height;
	m_dest = create_sub_bitmap(where,x,y,width,height);
	
	m_listener = sfx.newListener();
	
/*
	if(!testLight)
	{
		static int s = 200;
		testLight = create_bitmap(s, s);
		
		for(int y = 0; y < s; ++y)
		for(int x = 0; x < s; ++x)
		{
			double v = double(s)/2.0 - (IVec(x, y) - IVec(s/2, s/2)).length();
			if(v < 0.0)
				v = 0.0;
			int iv = int(v);
			putpixel_solid(testLight, x, y, makecol(iv, iv, iv));
		}
	}*/
}

void Viewport::render(BasePlayer* player)
{
	int offX = static_cast<int>(m_pos.x);
	int offY = static_cast<int>(m_pos.y);
	game.level.draw(m_dest, offX, offY);
	
	

#ifdef USE_GRID
	for ( Grid::iterator iter = game.objects.beginAll(); iter; ++iter)
	{
		iter->draw(m_dest, offX, offY);
	}
#else
	for ( int i = 0; i < RENDER_LAYERS_AMMOUNT ; ++i)
	{
		ObjectsList::RenderLayerIterator iter;
		for ( iter = game.objects.renderLayerBegin(i); (bool)iter; ++iter)
		{
			(*iter)->draw(m_dest, offX, offY);
		}
	}
#endif

#if 0
	{
		BasePlayer* player = game.localPlayers[0];
	
		BaseWorm* worm = player->getWorm();
		if(worm->isActive())
		{
			IVec off(m_pos);
			IVec v(worm->pos);
			IVec loff(v - IVec(testLight->w/2, testLight->h/2));
			
			Rect r(0, 0, game.level.width() - 1, game.level.height() - 1);
			r &= Rect(testLight) + loff;
			
			Culler<TestCuller> testCuller(TestCuller(m_dest, testLight, -off.x, -off.y, -loff.x, -loff.y), r);
	
			testCuller.cullOmni(v.x, v.y);
		}
	}
#endif

	EACH_CALLBACK(i, wormRender)
	{
		for(list<BasePlayer*>::iterator playerIter = game.players.begin(); playerIter != game.players.end(); ++playerIter)
		{
			BaseWorm* worm = (*playerIter)->getWorm();
			if( worm && worm->isActive() )
			{
				Vec renderPos = worm->getRenderPos();
				int x = (int)renderPos.x - offX;
				int y = (int)renderPos.y - offY;
				bool ownViewport = (*playerIter == player);
				lua.callReference(0, *i, (lua_Number)x, (lua_Number)y, worm->luaReference, luaReference, ownViewport);
			}
		}
		
	}
	
	if(BaseWorm* worm = player->getWorm())
	{
		EACH_CALLBACK(i, viewportRender)
		{
			lua.callReference(0, *i, luaReference, worm->luaReference);
		}
	}
}

void Viewport::setPos(float x, float y)
{
	m_pos.x=x;
	m_pos.y=y;
	
	if (m_listener) m_listener->pos = m_pos + Vec(m_dest->w/2,m_dest->h/2);
	
	if ( m_pos.x + m_dest->w > game.level.width() ) m_pos.x = game.level.width() - m_dest->w;
	else if ( m_pos.x < 0 ) m_pos.x = 0;
	if ( m_pos.y + m_dest->h > game.level.height() ) m_pos.y = game.level.height() - m_dest->h;
	else if ( m_pos.y < 0 ) m_pos.y = 0;
	
}

void Viewport::interpolateTo(float x, float y, float factor)
{
	Vec dest(x-m_dest->w/2,y-m_dest->h/2);

	m_pos = m_pos + (dest-m_pos)*factor;
	

	if (m_listener) m_listener->pos = Vec(x,y);

	if ( m_pos.x + m_dest->w > game.level.width() ) m_pos.x = game.level.width() - m_dest->w;
	else if ( m_pos.x < 0 ) m_pos.x = 0;
	if ( m_pos.y + m_dest->h > game.level.height() ) m_pos.y = game.level.height() - m_dest->h;
	else if ( m_pos.y < 0 ) m_pos.y = 0;
}

void Viewport::interpolateTo(Vec dest, float factor)
{
	m_pos = m_pos + (dest-Vec(m_dest->w/2,m_dest->h/2)-m_pos)*factor;
	
	if (m_listener) m_listener->pos = dest;
	
	if ( m_pos.x + m_dest->w > game.level.width() ) m_pos.x = game.level.width() - m_dest->w;
	else if ( m_pos.x < 0 ) m_pos.x = 0;
	if ( m_pos.y + m_dest->h > game.level.height() ) m_pos.y = game.level.height() - m_dest->h;
	else if ( m_pos.y < 0 ) m_pos.y = 0;
}

#endif
