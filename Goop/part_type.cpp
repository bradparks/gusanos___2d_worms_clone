#include "part_type.h"

#include "resource_list.h"

#include "sprite.h"
#include "text.h"
#include "parser.h"

#include <allegro.h>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

ResourceList<PartType> partTypeList("objects/");

TimerEvent::TimerEvent(int _delay, int _delayVariation)
{
	delay=_delay;
	delayVariation = _delayVariation;
	event = new PartEvent;
}

TimerEvent::~TimerEvent()
{
	delete event;
}

PartType::PartType()
{
	gravity = 0;
	damage = 0;
	bounceFactor = 1;
	color = 0;
	repeat = 1;
	alpha = 255;
	timeout = -1;
	timeoutVariation = 0;
	wormDetectRange = 0;
	radius = 0;
	delayBetweenFrames = 1;
	
	sprite = NULL;
	
	groundCollision = NULL;
	creation = NULL;
}

PartType::~PartType()
{
	if (groundCollision) delete groundCollision;
	if (creation) delete creation;
	for ( vector<TimerEvent*>::iterator i = timer.begin(); i != timer.end(); i++)
	{
		delete *i;
	}
}

bool PartType::load(const string &filename)
{

	ifstream fileStream;
	
	fileStream.open( filename.c_str() );

	if ( fileStream.is_open() )
	{
		string parseLine;
		PartEvent *currEvent = NULL;
		while ( !fileStream.eof() )
		{
			getline( fileStream, parseLine );
			if ( !parseLine.empty() )
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
					else if ( var == "bounce_factor" ) bounceFactor = cast<float>(val);
					else if ( var == "damage" ) damage = cast<float>(val);
					else if ( var == "worm_detect_range" ) wormDetectRange = cast<float>(val);
				}
				
				if ( lineID == Parser::EVENT_START )
				{
					iter++;
					string eventName = *iter;
					if ( eventName == "ground_collision" )
					{
						currEvent = new PartEvent;
						groundCollision = currEvent;
					}
					else if ( eventName == "creation" )
					{
						currEvent = new PartEvent;
						creation = currEvent;
					}
					else if ( eventName == "timer" )
					{
						int delay = 100;
						int delayVariation = 0;
						iter++;
						if( iter != tokens.end())
						{
							delay = cast<int>(*iter);
						}
						iter++;
						if( iter != tokens.end())
						{
							delayVariation = cast<int>(*iter);
						}
						timer.push_back(new TimerEvent(delay, delayVariation));
						currEvent = timer.back()->event;
					}
					else currEvent = NULL;
				}
				
				if ( lineID == Parser::ACTION && currEvent != NULL)
				{
					currEvent->addAction(*iter, Parser::getActionParams( tokens ));
				}
				
			}
		}
		fileStream.close();
		return true;
	} else
	{
		return false;
	}
}


