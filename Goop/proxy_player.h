#ifndef PROXY_PLAYER_H
#define PROXY_PLAYER_H

#include "base_player.h"
#include <string>

class Worm;
class PlayerOptions;

class ProxyPlayer : public BasePlayer
{
public:

	ProxyPlayer();
	~ProxyPlayer();
	
	void subThink();
	void render();
	
private:

};

#endif  // _WORM_H_