#ifndef VS_NET_VSNET_HEADERS_H
#define VS_NET_VSNET_HEADERS_H

#if defined(_WIN32) && !(defined(__CYGWIN__)) // || defined(__MINGW32__))
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	//#ifdef USE_WINSOCK2
	//#define _WIN32_WINNT 0x0400
	//#endif
	#include <windows.h>
	#ifdef HAVE_CONFIG_H
	# include "config.h"
	#endif
	#if !defined(SDL_WINDOWING) && defined(_WIN32) && defined(__MINGW32__)
	# include <winsock.h>
	#endif
#else
	#include <netdb.h>
 	#include <string.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#define SOCKET_ERROR -1
#endif
#ifdef __APPLE__
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;
using std::cerr;

#endif
