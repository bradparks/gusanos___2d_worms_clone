#include "explosion.h"

#include "vec.h"
#include "game.h"
#include "base_object.h"
#include "base_worm.h"
#include "base_player.h"
#include "exp_type.h"
#include "events.h"
#include "distortion.h"
#include "gfx.h"
#include "sprite.h"
#include "sprite_set.h"
#include "base_animator.h"
#include "animators.h"
#include "detect_event.h"

#include <vector>

using namespace std;

Explosion::Explosion(ExpType *type, const Vec& _pos, BasePlayer* owner) : BaseObject(owner)
{
	m_type = type;
	
	pos = _pos;
	
	m_alpha = m_type->alpha;
	
	m_timeout = m_type->timeout + (int)( rnd() * (m_type->timeoutVariation+1) ); 
	// I add +1 or variation will always be 0 if the value of timeout variation is 1
	
	if ( m_type->destAlpha >= 0 )
	{
		m_fadeSpeed = ( m_type->destAlpha - m_alpha ) / m_timeout;
	}else m_fadeSpeed = 0;
	
	
	if ( m_sprite = m_type->sprite )
	{
		m_animator = new AnimRightOnce( m_sprite, m_timeout+2);
	}
	else m_animator = 0;
	
	if ( m_type->creation )
	{
		m_type->creation->run(this);
	}

	for ( vector< DetectEvent* >::iterator t = m_type->detectRanges.begin(); t != m_type->detectRanges.end(); ++t )
	{
		(*t)->check(this);
	}
}

void Explosion::think()
{
	// Animation
	if ( m_animator ) m_animator->tick();
	
	// Alpha Fade
	if ( m_type->blender && m_fadeSpeed )
	{
		if ( fabs( m_type->destAlpha - m_alpha ) < fabs(m_fadeSpeed) )
		{
			m_fadeSpeed = 0;
			m_alpha = m_type->destAlpha;
		}
		else
			m_alpha += m_fadeSpeed;
	}
	
	if ( --m_timeout < 0)
	{
		deleteMe = true;
	}
}

void Explosion::draw(BITMAP* where,int xOff, int yOff)
{
	int x = static_cast<int>(pos.x - xOff);
	int y = static_cast<int>(pos.y - yOff);
	
	BlitterContext blitter(m_type->blender, (int)m_alpha);
	
	if (!m_sprite)
	{
		if(m_type->wupixels)
			blitter.putpixelwu(where, pos.x - xOff, pos.y - yOff, m_type->colour);
		else
			blitter.putpixel(where, x, y, m_type->colour);
	}
	else
	{	
		if ( false )
		{
			if ( !m_type->blender )
				m_sprite->getSprite(m_animator->getFrame(), Angle(0))->draw(where, x, y);
			else
				m_sprite->getSprite(m_animator->getFrame(), Angle(0))->drawBlended(where, x, y, blitter, false, 0);
		}
		else
		{
			//TODO: Blending
			game.level.specialDrawSprite( m_sprite->getSprite(m_animator->getFrame(), Angle(0)), where, pos - Vec(xOff, yOff), pos );
			
		}
	}
	if (m_type->distortion)
	{
		m_type->distortion->apply( where, x, y, m_type->distortMagnitude );
	}
}
