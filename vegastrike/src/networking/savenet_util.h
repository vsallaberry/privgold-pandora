#ifndef __SAVENET_UTIL_H
#define __SAVENET_UTIL_H

#include <string>
#include "networking/clientptr.h"

namespace SaveNetUtil
{
	//void	SaveFiles( Cockpit * cp, string savestr, string xmlstr, string path);
	void	GetSaveBuffer( const std::string & savestr, const std::string & xmlstr, char * buffer);
	void	GetSaveStrings( int numplayer, std::string & savestr, std::string & xmlstr, bool savevars);
	void	GetSaveStrings( ClientPtr clt, std::string & savestr, std::string & xmlstr, bool savevars);
}

#endif

