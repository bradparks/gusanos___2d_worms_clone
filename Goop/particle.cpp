#include "particle.h"

#include "vec.h"
#include "game.h"
#include "base_object.h"
#include "part_type.h"
#include "sprite.h"
#include "base_animator.h"
#include "animators.h"
#include "vec.h"

#include <vector>

using namespace std;

Particle::Particle(PartType *type, Vec _pos, Vec _spd)
{
	justCreated = true;
	m_type = type;
	
	pos = _pos;
	spd = _spd;
	m_angle = spd.getAngle();
	m_angleSpeed = 0;
	m_animator = NULL;
	
	m_sprite = m_type->sprite;
	if ( m_sprite )
	{
		switch ( m_type->animType )
		{
			case PartType::ANIM_PINGPONG : 
				m_animator = new AnimPingPong(m_sprite,m_type->animDuration); break;
			
			case PartType::ANIM_LOOPRIGHT : 
				m_animator = new AnimLoopRight(m_sprite,m_type->animDuration); break;
		}
	}
	
	for ( vector< TimerEvent* >::iterator i = m_type->timer.begin(); i != m_type->timer.end(); i++)
	{
		timer.push_back( PartTimer(*i) );
	}
}

void Particle::think()
{
	spd.y+=m_type->gravity;
	if ( !game.level.getMaterial( (int)(pos+spd).x, (int)pos.y ).particle_pass)
		spd.x*=-m_type->bounceFactor;
	if ( !game.level.getMaterial( (int)pos.x, (int)(pos+spd).y ).particle_pass)
	{
		spd.y*=-m_type->bounceFactor;
		if ( m_type->groundCollision != NULL )
			m_type->groundCollision->run(this);
	}
	
	for ( vector< PartTimer >::iterator t = timer.begin(); t != timer.end(); t++)
	{
		(*t).count--;
		if ( (*t).count < 0 )
		{
			(*t).m_tEvent->event->run(this);
			(*t).reset();
		}
	}
	
	if ( justCreated && m_type->creation )
	{
		m_type->creation->run(this);
		justCreated = false;
	}
	
	pos = pos + spd;
	if ( m_animator ) m_animator->tick();
}

float Particle::getAngle()
{
	return spd.getAngle();
}

void Particle::draw(BITMAP* where,int xOff, int yOff)
{
	if (!m_sprite)
		putpixel(where,(int)(pos.x)-xOff,(int)(pos.y)-yOff,makecol(255,255,255));
	else
	{
		m_sprite->draw(where, m_animator->getFrame(), (int)pos.x-xOff, pos.y-yOff);
	}
	if (m_type->distortion)
	{
		m_type->distortion->apply( where, pos.x-xOff, pos.y-yOff, m_type->distortMagnitud );
	}
}