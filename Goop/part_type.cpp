#include "part_type.h"

#include "resource_list.h"

#include "game.h"
#ifndef DEDSERV
#include "sprite_set.h"
#include "sprite.h"
#include "gfx.h"
#include "distortion.h"
#include "animators.h"
#endif
#include "util/text.h"
#include "util/vec.h"
#include "util/angle.h"
#include "util/macros.h"
#include "parser.h"
#include "detect_event.h"
#include "timer_event.h"

#include "particle.h"
#include "simple_particle.h"
#include "game_actions.h"
#include "omfg_script.h"


#include <allegro.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
namespace fs = boost::filesystem;

using namespace std;

ResourceList<PartType> partTypeList;

void newParticle_Particle(PartType* type, Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), int dir = 1, BasePlayer* owner = NULL, Angle angle = Angle(0))
{
	BaseObject* particle = new Particle(type, pos_, spd_, dir, owner, angle);
	
#ifdef USE_GRID
	if(type->colLayer != Grid::NoColLayer)
		game.objects.insert( particle, type->colLayer, type->renderLayer);
	else
		game.objects.insert( particle, type->renderLayer);
#else
	game.objects.insert( type->colLayer, type->renderLayer, particle );	
#endif
}

template<class T>
void newParticle_SimpleParticle(PartType* type, Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), int dir = 1, BasePlayer* owner = NULL, Angle angle = Angle(0))
{
	int timeout = type->simpleParticle_timeout + rndInt(type->simpleParticle_timeoutVariation);
	
	BaseObject* particle = new T(pos_, spd_, owner, timeout, type->gravity, type->colour);
	
	if(type->creation)
		type->creation->run(particle);
	
	USE_GRID // If this errors out, USE_GRID isn't defined, so define it ffs! >:o
	game.objects.insert( particle, type->colLayer, type->renderLayer);
}

#ifdef DEDSERV
void newParticle_Dummy(PartType* type, Vec pos_ = Vec(0.f, 0.f), Vec spd_ = Vec(0.f, 0.f), int dir = 1, BasePlayer* owner = NULL, Angle angle = Angle(0))
{
	if(type->creation)
	{
		BaseObject particle(owner, pos_, spd_);
		type->creation->run(&particle);
	}
}
#endif

PartType::PartType()
: newParticle(0), wupixels(0)
, invisible(false)
{
	gravity			= 0;
	bounceFactor	= 1;
	groundFriction	= 1;
	colour			= -1;
	repeat			= 1;
	alpha			= 255;
	angularFriction = 0;
	animDuration	= 100;
	animType		= ANIM_LOOPRIGHT;
	animOnGround	= 1;
	damping			= 1;
	acceleration	= 0;
	maxSpeed		= -1;
	colLayer		= 0;
	health			= 100;
	radius			= 0;
	
	renderLayer = Grid::WormRenderLayer;
#ifndef DEDSERV
	sprite = NULL;
	distortion = NULL;
	distortMagnitude = 0.8;

	blender = BlitterContext::None;
	
	lightHax = NULL;
#endif
	
	line2Origin = false;
	culled = false;
	
	groundCollision = NULL;
	creation = NULL;
	death = NULL;
	
	for ( int i = 0; i < 16; ++i )
	{
		customEvents.push_back(NULL);
	}
}

PartType::~PartType()
{
	delete groundCollision;
	delete creation;
#ifndef DEDSERV
	delete distortion;
	delete lightHax;
#endif
	for ( vector<TimerEvent*>::iterator i = timer.begin(); i != timer.end(); i++)
	{
		delete *i;
	}
	for ( vector<DetectEvent*>::iterator i = detectRanges.begin(); i != detectRanges.end(); i++)
	{
		delete *i;
	}
}

bool PartType::isSimpleParticleType()
{
	if(repeat != 1 || alpha != 255
#ifndef DEDSERV
	|| sprite || distortion || blender || lightHax
#endif
	|| damping != 1.f
	|| acceleration != 0.f || !groundCollision
	|| death || timer.size() > 1 || line2Origin
	|| detectRanges.size() > 0)
	{
		return false;
	}
		
	std::vector<BaseAction*>::const_iterator i = groundCollision->actions.begin();
	for(; i != groundCollision->actions.end(); ++i)
	{
		Remove* event = dynamic_cast<Remove *>(*i);
		if(!event)
			return false; // groundCollision contains non-remove actions
	}
	
	if(timer.size() == 1)
	{
		// triggerTimes is irrelevant since it will only trigger once anyway
		
		if(timer[0]->actions.size() != 1)
			return false;
			
		Remove* event = dynamic_cast<Remove *>(timer[0]->actions[0]);
		if(!event)
			return false; // timer event contains non-remove actions
		
		// One timer with one remove action

		simpleParticle_timeout = timer[0]->delay;
		simpleParticle_timeoutVariation = timer[0]->delayVariation;
	}
	else
	{
		simpleParticle_timeout = 0;
		simpleParticle_timeoutVariation = 0;
	}
		

	return true;
}

bool PartType::load2(fs::path const& filename)
{
	//cerr << "Loading particle: " << filename.native_file_string() << endl;
	
	fs::ifstream fileStream(filename);
	
	if ( fileStream )
	{	
		string parseLine;
		Event *currEvent = NULL;
		while ( portable_getline( fileStream, parseLine ) )
		{
			//if ( !parseLine.empty() ) //Unnecessary
			{
				string var;
				string val;

				vector<string> tokens;
				tokens = Parser::tokenize ( parseLine );
				int lineID = Parser::identifyLine( tokens );
				
				vector<string>::iterator iter = tokens.begin();
				
				if ( lineID == Parser::PROP_ASSIGMENT )
				{
					var = *iter;
					iter++;
					if ( iter != tokens.end() && *iter == "=")
					{
						iter++;
						if ( iter != tokens.end() )
							val = *iter;
					}
					
					if ( var == "gravity" ) gravity = cast<float>(val);
					else if ( var == "repeat" ) repeat = cast<int>(val);	
					else if ( var == "bounce_factor" ) bounceFactor = cast<float>(val);
					else if ( var == "ground_friction" ) groundFriction = cast<float>(val);
					else if ( var == "damping" ) damping = cast<float>(val);
					else if ( var == "acceleration" ) acceleration = cast<float>(val);
					else if ( var == "max_speed" ) maxSpeed = cast<float>(val);

					else if ( var == "angular_friction" ) angularFriction = cast<Angle>(val);
					else if ( var == "health" ) health = cast<float>(val);
					else if ( var == "radius" ) radius = cast<float>(val);

					else if ( var == "line_to_origin" ) 
						line2Origin = ( cast<int>(val) == 1 );
					else if ( var == "col_layer" ) colLayer = cast<int>(val);
#ifndef DEDSERV
					else if ( var == "sprite" ) sprite = spriteList.load(val);
#else
					else if ( var == "sprite" ) /* ignore */;
#endif
					else if ( var == "invisible" ) invisible = (cast<int>(val) != 0);
					else if ( var == "occluded" ) culled = (cast<int>(val) != 0);
					else if ( var == "anim_duration" ) animDuration = cast<int>(val);
#ifndef DEDSERV
					else if ( var == "light_radius" ) lightHax = genLight(cast<int>(val));
#else
					else if ( var == "light_radius" ) /* ignore */;
#endif
					else if ( var == "anim_on_ground" ) animOnGround = cast<int>(val);
					else if ( var == "anim_type" )
					{
						if ( val == "ping_pong" ) animType = ANIM_PINGPONG;
						else if ( val == "loop_right" ) animType = ANIM_LOOPRIGHT;
						else if ( val == "right_once" ) animType = ANIM_RIGHTONCE;
					}
					else if ( var == "render_layer" ) renderLayer = cast<int>(val);
					else if ( var == "alpha" ) alpha = cast<int>(val);
#ifndef DEDSERV
					else if ( var == "blender" )
					{
						if ( val == "add" ) blender = BlitterContext::Add;
						else if ( val == "alpha" ) blender = BlitterContext::Alpha;
						else if ( val == "alphach" ) blender = BlitterContext::AlphaChannel;
					}
#else
					else if ( var == "blender" ) /* ignore */;
#endif
					else if ( var == "wu_pixels" ) 
						wupixels = ( cast<int>(val) != 0 );
					else if ( var == "colour" || var == "color" )
					{
						if ( tokens.size() >= 5 )
						colour = makecol( cast<int>(tokens[2]), cast<int>(tokens[3]), cast<int>(tokens[4]) );
					}
#ifndef DEDSERV
					else if ( var == "distortion" && !distortion )
					{
						if ( val == "lens" && tokens.size() >= 4)
							distortion = new Distortion( lensMap( cast<int>(tokens[3]) ) );
						else if ( val == "swirl" && tokens.size() >= 4)
							distortion = new Distortion( swirlMap( cast<int>(tokens[3]) ) );
						else if ( val == "ripple" && tokens.size() >= 4)
							distortion = new Distortion( rippleMap( cast<int>(tokens[3]) ) );
						else if ( val == "random" && tokens.size() >= 4)
							distortion = new Distortion( randomMap( cast<int>(tokens[3]) ) );
						else if ( val == "spin" && tokens.size() >= 4)
							distortion = new Distortion( spinMap( cast<int>(tokens[3]) ) );
						else if ( val == "bitmap" && tokens.size() >= 4)
							distortion = new Distortion( bitmapMap( tokens[3] ) );
					}
					else if ( var == "distort_magnitude" ) distortMagnitude = cast<float>(val);
#else
					else if ( var == "distortion" ) /* ignore */;
					else if ( var == "distort_magnitude" ) /* ignore */;
#endif
					else
					{
						std::cout << "Unknown variable on following line:" << std::endl;
						std::cout << "\t" << parseLine << std::endl;
					}
				}
				
				if ( lineID == Parser::EVENT_START )
				{
					iter++;
					string eventName = *iter;
					if ( eventName == "ground_collision" )
					{
						currEvent = new Event();
						groundCollision = currEvent;
					}
					else if ( eventName == "creation" )
					{
						currEvent = new Event();
						creation = currEvent;
					}
					else if ( eventName == "death" )
					{
						currEvent = new Event();
						death = currEvent;
					}
					else if ( eventName == "timer" )
					{
						int delay = 100;
						int delayVariation = 0;
						int triggerTimes = 0;
						iter++;
						if( iter != tokens.end())
						{
							delay = cast<int>(*iter);
							++iter;
						}
						if( iter != tokens.end())
						{
							delayVariation = cast<int>(*iter);
							++iter;
						}
						if( iter != tokens.end())
						{
							triggerTimes = cast<int>(*iter);
						}
						timer.push_back(new TimerEvent(delay, delayVariation, triggerTimes));
						currEvent = timer.back();
					}
					else if ( eventName == "detect_range" )
					{
						float range = 10;
						bool detectOwner = true;
						int detectFilter = 0;
						iter++;
						if( iter != tokens.end())
						{
							range = cast<float>(*iter);
							++iter;
						}
						if( iter != tokens.end())
						{
							detectOwner = (bool)cast<int>(*iter);
							++iter;
						}
						while ( iter != tokens.end() )
						{
							if ( *iter == "worms" ) detectFilter |= 1;
							else
							{
								detectFilter |= 1 << ( cast<int>(*iter)+1 );
							}
							++iter;
						}
						if ( !detectFilter ) detectFilter = 1;
						detectRanges.push_back( new DetectEvent(range, detectOwner, detectFilter));
						currEvent = detectRanges.back();
					}
					else if ( eventName == "custom_event" && iter!= tokens.end() )
					{
						iter++;
						size_t eventIndex = cast<size_t>(*iter);
						if ( eventIndex < customEvents.size() )
						{
							currEvent = new Event();
							customEvents[eventIndex] = currEvent;
						}
					}
					else
					{
						std::cout << "Unknown event on following line:" << std::endl;
						std::cout << "\t" << parseLine << std::endl;
						std::cout << "Event name given: \"" << eventName << "\"" << std::endl;
						//std::cout << "----------------" << std::endl;
						std::cout << filename.native_file_string() << std::endl;
						currEvent = 0;
					}

				}
				
				if ( lineID == Parser::ACTION && currEvent )
				{
					if(!currEvent->addAction(*iter, Parser::getActionParams( tokens )))
					{
						//TODO: Add more info here
						cerr << "Couldn't add action to event" << endl;
					}
				}
				
			}
		}
		//fileStream.close(); //RAII ffs
		
		// Do optimizations
				
		if(isSimpleParticleType())
		{
#ifndef DEDSERV
			if(wupixels)
			{
				switch(bitmap_color_depth(screen))
				{
					default: newParticle = newParticle_SimpleParticle<SimpleParticle>; break;
					case 32: newParticle = newParticle_SimpleParticle<SimpleParticle32wu>; break;
					case 16: newParticle = newParticle_SimpleParticle<SimpleParticle16wu>; break;
				}
			}
			else
			{
				switch(bitmap_color_depth(screen))
				{
					default: newParticle = newParticle_SimpleParticle<SimpleParticle>; break;
					case 32: newParticle = newParticle_SimpleParticle<SimpleParticle32>; break;
					case 16: newParticle = newParticle_SimpleParticle<SimpleParticle16>; break;
				}
			}
#else
			newParticle = newParticle_Dummy;
#endif
			cerr << filename.native_file_string() << ": blood" << endl;
		}
		else
			newParticle = newParticle_Particle;
			
		if( colLayer >= 0 )
			colLayer = Grid::CustomColLayerStart + colLayer;
		else
			colLayer = Grid::NoColLayer;
		
		return true;

	} else
	{
		return false;
	}
}

namespace EventID
{
enum type
{
	Creation,
	DetectRange,
	GroundCollision,
	Death,
	Timer,
	CustomEvent,
};
}

bool PartType::load(fs::path const& filename)
{
	fs::ifstream fileStream(filename);

	if (!fileStream )
		return false;
	
	OmfgScript::Parser parser(fileStream, gameActions, filename.native_file_string());
	
	namespace af = OmfgScript::ActionParamFlags;
		
	parser.addEvent("creation", EventID::Creation, af::Object);
	
	parser.addEvent("death", EventID::Death, af::Object);
	
	parser.addEvent("ground_collision", EventID::GroundCollision, af::Object);
	
	parser.addEvent("timer", EventID::Timer, af::Object)
		("delay")
		("delay_var")
		("max_trigger")
	;
	
	parser.addEvent("custom_event", EventID::CustomEvent, af::Object | af::Object2)
		("index", false)
	;
	
	parser.addEvent("detect_range", EventID::DetectRange, af::Object | af::Object2)
		("range")
		("detect_owner")
		("layers")
	;
		
	if(!parser.run())
	{
		if(parser.incomplete())
			parser.error("Trailing garbage");
		return false;
	}

#ifndef DEDSERV
	{
		OmfgScript::TokenBase* v = parser.getProperty("sprite");
		if(!v->isDefault())
			sprite = spriteList.load(v->toString());
	}
#endif
	invisible = parser.getBool("invisible", false);
	culled = parser.getBool("occluded", false);
	animOnGround = parser.getBool("anim_on_ground");
	renderLayer = parser.getInt("render_layer", Grid::WormRenderLayer);
	colLayer = parser.getInt("col_layer", 0);
	
	animDuration = parser.getInt("anim_duration", 100);
	gravity = parser.getDouble("gravity", 0.0);
	repeat = parser.getInt("repeat", 1);
	bounceFactor = parser.getDouble("bounce_factor", 1.0);
	groundFriction = parser.getDouble("ground_friction", 1.0);
	damping = parser.getDouble("damping", 1.0);
	acceleration = parser.getDouble("acceleration", 0.0);
	maxSpeed = parser.getDouble("max_speed", -1.0);
	angularFriction = Angle(parser.getDouble("angular_friction", 0.0));
	health = parser.getDouble("health", 100.0);
	radius = parser.getDouble("radius", 0.0);
	line2Origin = parser.getBool("line_to_origin", false);
	
	std::string const& animtypestr = parser.getString("anim_type", "loop_right");
	if(animtypestr == "ping_pong") animType = ANIM_PINGPONG;
	else if(animtypestr == "loop_right") animType = ANIM_LOOPRIGHT;
	else if(animtypestr == "right_once") animType = ANIM_RIGHTONCE;
	
	
/*
#ifndef DEDSERV
	else if ( var == "light_radius" ) lightHax = genLight(cast<int>(val));
#else
	else if ( var == "light_radius" ) ;
#endif
*/

	if(OmfgScript::Function const* f = parser.getFunction("distortion"))
	{
		if ( f->name == "lens" )
			distortion = new Distortion( lensMap( (*f)[0]->toInt() ));
		else if ( f->name == "swirl" )
			distortion = new Distortion( swirlMap( (*f)[0]->toInt() ));
		else if ( f->name == "ripple" )
			distortion = new Distortion( rippleMap( (*f)[0]->toInt() ));
		else if ( f->name == "random" )
			distortion = new Distortion( randomMap( (*f)[0]->toInt() ) );
		else if ( f->name == "spin" )
			distortion = new Distortion( spinMap( (*f)[0]->toInt() ) );
		else if ( f->name == "bitmap" )
			distortion = new Distortion( bitmapMap( (*f)[0]->toString() ) );
	}
		
	alpha = parser.getInt("alpha", 255);
	wupixels = parser.getBool("wu_pixels", false);
	distortMagnitude = parser.getDouble("distort_magnitude", 0.8);
	
	std::string const& blenderstr = parser.getString("blender", "none");
	if(blenderstr == "add") blender = BlitterContext::Add;
	else if(blenderstr == "alpha") blender = BlitterContext::Alpha;
	else if(blenderstr == "alphach") blender = BlitterContext::AlphaChannel;
	else blender = BlitterContext::None;
	
	colour = parser.getProperty("color", "colour")->toColor(255, 255, 255);
		
	OmfgScript::Parser::EventIter i(parser);
	for(; i; ++i)
	{
		std::vector<OmfgScript::TokenBase*> const& p = i.params();
		switch(i.type())
		{
			case EventID::Creation:
				creation = new Event(i.actions());
			break;
			
			case EventID::GroundCollision:
				groundCollision = new Event(i.actions());
			break;
			
			case EventID::Death:
				death = new Event(i.actions());
			break;
			
			case EventID::Timer:
				timer.push_back(new TimerEvent(i.actions(), p[0]->toInt(100), p[1]->toInt(0), p[2]->toInt(0)));
			break;
			
			case EventID::DetectRange:
			{
				int detectFilter = 0;
				if(p[2]->isList())
				{
					const_foreach(i, p[2]->toList())
					{
						OmfgScript::TokenBase& v = **i;
						if ( v.isString() )
						{
							if( v.toString() == "worms" ) detectFilter |= 1;
						}
						else if ( v.isInt() )
							detectFilter |= 1 << (v.toInt() + 1);
					}
				}
				else
				{
					detectFilter = 1;
				}
				detectRanges.push_back( new DetectEvent(i.actions(), p[0]->toDouble(10.0), p[1]->toBool(true), detectFilter));
			}
			break;
			
			case EventID::CustomEvent:
			{
				size_t eventIndex = p[0]->toInt();
				if ( eventIndex < customEvents.size() )
				{
					customEvents[eventIndex] = new Event(i.actions());
				}
			}
			break;
		}
	}
	
	if(isSimpleParticleType())
	{
#ifndef DEDSERV
		if(wupixels)
		{
			switch(bitmap_color_depth(screen))
			{
				default: newParticle = newParticle_SimpleParticle<SimpleParticle>; break;
				case 32: newParticle = newParticle_SimpleParticle<SimpleParticle32wu>; break;
				case 16: newParticle = newParticle_SimpleParticle<SimpleParticle16wu>; break;
			}
		}
		else
		{
			switch(bitmap_color_depth(screen))
			{
				default: newParticle = newParticle_SimpleParticle<SimpleParticle>; break;
				case 32: newParticle = newParticle_SimpleParticle<SimpleParticle32>; break;
				case 16: newParticle = newParticle_SimpleParticle<SimpleParticle16>; break;
			}
		}
#else
		newParticle = newParticle_Dummy;
#endif
		cerr << filename.native_file_string() << ": blood" << endl;
	}
	else
		newParticle = newParticle_Particle;
		
	if( colLayer >= 0 )
		colLayer = Grid::CustomColLayerStart + colLayer;
	else
		colLayer = Grid::NoColLayer;
			
/*
#ifndef DEDSERV
	else if ( var == "light_radius" ) lightHax = genLight( cast<int>(val) );
#else
	else if ( var == "light_radius" ) ;
#endif
*/
	return true;
}

#ifndef DEDSERV
BaseAnimator* PartType::allocateAnimator()
{
	switch ( animType )
	{
		case PartType::ANIM_PINGPONG : 
			return new AnimPingPong(sprite, animDuration);
		break;
		
		case PartType::ANIM_LOOPRIGHT : 
			return new AnimLoopRight(sprite, animDuration);
		break;
			
		case PartType::ANIM_RIGHTONCE : 
			return new AnimRightOnce(sprite, animDuration);
		break;
	}
	
	return 0;
}
#endif

