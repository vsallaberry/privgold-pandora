#ifndef IN_H
#define IN_H
enum KBSTATE {
	UP,
	DOWN,
	PRESS,
	RELEASE,
	RESET
};

class KBData;
typedef void (*KBHandler)(const KBData&, KBSTATE);

typedef void (*MouseHandler)(KBSTATE,int x, int y, int delx, int dely, int mod);

enum IN_SCOPE_ENUM {
	INSC_COCKPIT 		= 0,
	INSC_BASE,
	INSC_NB, INSC_ALL 	= INSC_NB // Last Line
};

unsigned int inGetCurrentScope(); // implemented in in_kb.cpp

#endif

