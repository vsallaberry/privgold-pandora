// rendertext.cpp: based on Don's gl_text.cpp
// Based on Aardarples rendertext 
#include "config.h"
#include "command.h"

#include "vegastrike.h"
#include "cg_global.h"
#ifdef HAVE_SDL
#include <SDL.h>
#endif
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#include <sstream>
#include "unicode.h"
#include "log.h"
#include "configxml.h"

// ****************
// Console Rendering System by Rogue
// 2005-08-a-few-days
//
// ****************

#ifdef HAVE_SDL
static SDL_mutex * _rtextSDLMutex()
{
	static SDL_mutex * rv = SDL_CreateMutex();
	return rv;
}
#endif


//Render Text (Console) Constructor {{{
RText::RText() {
#ifdef HAVE_SDL
	//Initialize shared mutex
	// (creation is always single-threaded, since no aliases are possible yet)
	_rtextSDLMutex();
#endif

	ndraw = 15;
	WORDWRAP = 85;
	conskip = 0;
	histpos = 0;
	curpos = 0;
	scrollpos = 0;
	saycommand("");
}
// }}}
// Render Text (Console) Destructor {{{
RText::~RText() {

}
// }}}
// Set the text width, not used  .. yet{{{
int RText::text_width(const char *str)
{
return 0;
};
// }}}
// Should be unused defines {{{
#define VIRTW 2400
#define VIRTH 1800
#define PIXELTAB (VIRTW/12)
#define FONTH 64
// }}}
// Draw text, used by the console, should be private, use conoutf to print to the console {{{
void RText::draw_text(const std::string &str, float left, float top, int gl_num)
{
	static void * font = getFontFromName(vs_config->getVariable("graphics", "interpreter_font",
										 vs_config->getVariable("graphics", "font", "helvetica12")));
    int x = float_to_int(left);
    int y = float_to_int(top);
	
    std::string::const_iterator iter = str.begin();
    GFXColor foreground(1, 1, 1, 1);
    GFXColor background(0.05f, 0.05f, 0.2f, 0.5f);
    TextPlane newTextPlane(foreground,background);
    newTextPlane.SetFont(font);
    newTextPlane.SetPos(x, y);
    newTextPlane.SetCharSize(.8, .12);
    newTextPlane.Draw(str);
};
// }}}

// {{{ RText::get_prompt()
std::string RText::get_prompt(const std::string & beforeCursor, const std::string & afterCursor) {
	return "#FF1100" "> " "#FF1100" + beforeCursor + "#000000" + "|" + "#FF1100" + afterCursor;
}
// }}}

// render the console, only call if bool console == true {{{
void RText::renderconsole()// render buffer
{

    int nd = 0;
    std::vector<std::string> refs;
	bool breaker = false;
//	int i = 0;
//	int lastmillis = 0;
//	int length = 0;
//	for(std::vector<cline>::iterator iter = conlines.begin();
//	iter < conlines.end(); iter++) length++;
	{
	if (scrollpos > 0) {
		++nd;
		refs.push_back("...");
	}
	for(std::vector<cline>::iterator iter = conlines.begin() + scrollpos; iter < conlines.end(); iter++)  {
		if(nd < ndraw)
			refs.push_back((*(iter)).cref);
		else iter = conlines.end();
		nd++;
	}
	if (scrollpos + ndraw <= conlines.size()) {
		refs.back() = "...";
	}
	}
    size_t j = 0;
    float x = -1;
    float y = -0.5;
    std::string workIt;
    workIt.append("\n");
    bool breakout = true;
	std::vector<std::string>::iterator iter = refs.end();
	if(iter == refs.begin()) breakout = false;
    for(; breakout;) {
		iter--;
		if(iter == refs.begin()) breakout = false;
		workIt.append((*(iter)));
		workIt.append("\n");
    };
    y = 1;
    std::ostringstream drawCommand;
	std::string shorter;
	shorter.append(getcurcommand());
	int shortpos = curpos;
	while (shorter.size() > 80) {
		Utf8Iterator u8it = ++Utf8Iterator::begin(shorter);
		shorter.erase(0, u8it.pos());
		if (shortpos > u8it.pos())
			shortpos -= u8it.pos();
	} //erase the front of the current command while it's larger than 80
	// charactors, as to not draw off the screen
    drawCommand << workIt << get_prompt(shorter.substr(0,shortpos), shorter.substr(shortpos)) << "#000000";
    std::string Acdraw; //passing .str() straight to draw_text produces an 
		//error with gcc 4, because it's constant I believe
    Acdraw.append(drawCommand.str());
    draw_text(Acdraw, x, y, 2);

};
// }}}
//append a line to the console, optional "highlight" method , untested {{{
void RText::conline(const std::string &csf, bool highlight)        // add a line to the console buffer
{
	std::string sf(csf);
	{
		// std::string::npos could be negative, and comparing to unsigned int wont fly
		int search =0;
		unsigned int lastsearch = 0;
		for(; (search = sf.find("/r"))!=std::string::npos ; ) {
			sf.replace(lastsearch, search-lastsearch, "");
			lastsearch = search;
		}
	}
    cline cl;
    int lastmillis = 0;
    cl.outtime = lastmillis;                        // for how long to keep line on screen
    if(highlight)        // show line in a different colour, for chat etc.
    {
        cl.cref.append("\f");
        cl.cref.append(sf);
    }
    else
    {
        cl.cref.append(sf);
    };
    conlines.insert(conlines.begin(), cl);
//    puts(cl.cref.c_str());

};
// }}}
// print a line to the console, broken at \n's {{{
void RText::conoutf(const char *in) {
	std::string foobar(in);
	conoutf(foobar);
	return;
}

void RText::conoutf(const std::string &s, int a, int b, int c)
{
#ifdef HAVE_SDL
	// NOTE: first call must be single-threaded!
	SDL_mutex * mymutex = _rtextSDLMutex();
	SDL_LockMutex(mymutex);
#endif
	VS_LOG("interpreter", logvs::NOTICE, "%s", s.substr(0,s.find_last_not_of("\r\n")+1).c_str());
// Old {{{
//	{
//		for(int x = WORDWRAP; x < s.size(); x = x+WORDWRAP) {
//			s.insert(x, "\n");
//		}
//	}

//	size_t x = s.find("\n");
//	if(x < std::string::npos) {
//		size_t xlast = 0;
//		for(; x < std::string::npos; x = s.find("\n", x+1)) {
//			std::string newone;
//			newone.append(s.substr(xlast, x-xlast));
//			conline(newone, 1);
//			xlast = x+1;
//		}
//		
//	} else {
//		conline(s, 1);
//	}
// }}}
	unsigned int fries = s.size();
	std::string customer;
	for(unsigned int burger = 0; burger < fries; burger++) {
		if(s[burger] == '\n' || burger == fries-1) {
			if(burger == fries-1) 
				if(s[fries-1] != '\n' && s[fries-1] != '\r')
					customer += s[burger];
			conline(customer, 1);
			customer.erase();
		} else if( customer.size() >= WORDWRAP) {
			customer += s[burger];
			std::string fliptheburger;
			while( customer.size() && customer[customer.size()-1] != ' ') {
				fliptheburger += customer[customer.size()-1];
				std::string::iterator oldfloormeat = customer.end();
				oldfloormeat--; 
				customer.erase(oldfloormeat);
			}
			conline(customer, 1);
			customer.erase();
			{
				std::string spatchula;
				for(int salt = fliptheburger.size()-1; salt >= 0; salt--) {
					spatchula += fliptheburger[salt];
				}
				fliptheburger.erase();
				fliptheburger.append(spatchula);
			}
			customer.append(fliptheburger);
		} else if( s[burger] != '\r') { 
			customer += s[burger]; // get fat
		}
	}
#ifdef HAVE_SDL
	SDL_UnlockMutex(mymutex);
#endif
};
// }}}
//same as above, but I think it works better {{{
void RText::conoutn(const std::string &cs, int a, int b, int c) {
	std::string s(cs);
	size_t x = s.find("\n");
	size_t xlast = 0;
	if(x >= std::string::npos) {
		conoutf(s);
	}
	std::string::iterator iter = s.end();
	if(iter != s.begin() ) { 
		iter--;
		if(strcmp(&(*(iter)),"\n") != 0) { s.append("\n"); };
	}
	while(x < std::string::npos) {
		std::string part;
		part.append(s.substr(xlast, x-xlast));
		xlast = x+1;
		x = s.find("\n", x+1);
		conoutf(part, a, b, c);
	}
}
// }}}
// saycommand(char *), should "say" something, will be useful only with network enabled {{{
//does nothing now
void RText::saycommand(const char *init)///
{ //actually, it appends "init" to commandbuf
//  SDL_EnableUNICODE((init!=NULL));
//    if(!editmode) keyrepeat(saycommandon);
    if(init && *init)
    	commandbuf.append(init);
};
// }}}
// Console Keyboard Input {{{
void RText::ConsoleKeyboardI(int code, int mod, bool released)
{
	if(!released) {
		if (mod == WSK_MOD_LCTRL || mod == WSK_MOD_RCTRL) {
			// Special line edit modes (begin/end/cut) and shortcuts (ctrl+a=HOME, ...)
			switch (code) {
			case 'a':
				code = WSK_HOME;
				mod = 0;
				break ;
			case 'e':
				code = WSK_END;
				mod = 0;
				break ;
			case 'd':
				code = WSK_DELETE;
				mod = 0;
				break ;
			case WSK_UP:
				code = WSK_PAGEUP;
				mod = 0;
				break ;
			case WSK_DOWN:
				code = WSK_PAGEDOWN;
				mod = 0;
				break ;
			case 'k':
				commandbuf.erase(curpos);
				curpos = commandbuf.size();
				return ;
			case 'u':
				commandbuf.erase(0, curpos);
				curpos = 0;
				return ;
			}
		}
		switch(code){
//pop teh back of commandbuf
	        case WSK_BACKSPACE:
				if (curpos > 0 && commandbuf.size()) {
					Utf8Iterator iter = Utf8Iterator::end(commandbuf);
					while (iter.pos() > curpos)
						--iter;
					size_t oldpos = iter.pos();
					--iter;
					VS_DBG("interpreter", logvs::DBG+1, "Backspace buf#%zu<%s> oldpos:%zu newpos:%zu curpos:%d",
						   commandbuf.size(), commandbuf.c_str(), oldpos, iter.pos(), curpos);
					commandbuf.erase(iter.pos(), oldpos - iter.pos());
					curpos = iter.pos();
					if (vhistory.size()) {
						if (vhistory.back().empty())
							vhistory.pop_back();
						histpos = vhistory.size() - 1;
					}
				}
				break;

	        case WSK_DELETE:
	        	if (curpos < commandbuf.size()) {
	        		commandbuf.erase(curpos, (++Utf8Iterator::begin(commandbuf, curpos)).pos());
	        		if (vhistory.size()) {
	        			if (vhistory.back().empty())
	        				vhistory.pop_back();
	        			histpos = vhistory.size() - 1;
	        		}
	        	}
	        	break;

	        case WSK_LEFT:
	        	if (curpos > 0) {
	        		Utf8Iterator iter = Utf8Iterator::end(commandbuf);
	        		while (iter.pos() > curpos)
	        			--iter;
	        		curpos = (--iter).pos();
	        	}
		        break;

	        case WSK_RIGHT:
	        	if (curpos < commandbuf.size())
	        		curpos += (++Utf8Iterator::begin(commandbuf, curpos)).pos();
	        	break;

	        case WSK_UP:
	        	if (!vhistory.size())
	        		break ;
	        	if (histpos >= vhistory.size() - 1 && vhistory.back() != commandbuf) {
	        		vhistory.push_back(commandbuf);
	        	} else if (histpos > 0){
	        		--histpos;
	        	}
	        	commandbuf.assign(vhistory[histpos]);
	        	curpos = commandbuf.size();
	        	break ;

	        case WSK_DOWN:
	        	if (!vhistory.size() || histpos + 1 >= vhistory.size())
	        		break ;
	        	++histpos;
	        	commandbuf.assign(vhistory[histpos]);
	        	curpos = commandbuf.size();
	        	break ;

	        case WSK_RETURN:
				if(commandbuf[0])
				{
					std::vector<std::string>::iterator iter = vhistory.end();
					bool noSize = false;
					if(iter <=vhistory.begin() && iter >= vhistory.end()) noSize = true;
					if(!noSize) {
						iter--;
						if(commandbuf.compare((*(iter))) != 0 && !noSize)
						{
//store what was typed into a vector for a command history 
//to scroll up and down through what was typed
//This "feature" isn't finished
							vhistory.push_back(commandbuf);  // cap this?
						}
					} else if(noSize)vhistory.push_back(commandbuf);
					
					histpos = vhistory.size() - 1;
				//commands beginning with / are executed
				//in localPlayer.cpp just before this is called
				};
				if(commandbuf.size() > 0) {
//print what was typed - Now done in the command processor
//clear the buffer
					commandbuf.erase();
				}
				curpos = 0;
				break;

	        case WSK_HOME:
	        	curpos = 0;
	        	break ;

	        case WSK_END:
	        	curpos = commandbuf.size();
	        	break ;

	        case WSK_PAGEUP:
	        	if (scrollpos + ndraw <= conlines.size()) ++scrollpos;
	        	VS_DBG("interpreter", logvs::DBG+1, "PAGEUP scollpos:%d", scrollpos);
	        	break ;

	        case WSK_PAGEDOWN:
	        	if (scrollpos > 0) --scrollpos;
	        	VS_DBG("interpreter", logvs::DBG+1, "PAGEDOWN scollpos:%d", scrollpos);
	        	break ;

			default:
//add it to the command buffer
				if (code>0&&(code<128||WSK_CODE_IS_UTF32(code))) {
					char utf8[MB_LEN_MAX+1];
					size_t u8sz = utf32_to_utf8(utf8, WSK_CODE_TO_UTF32(code));
					commandbuf.insert(curpos, utf8);
					curpos += u8sz;
					if (vhistory.size()) {
						if (vhistory.back().empty())
							vhistory.pop_back();
						histpos = vhistory.size() - 1;
					}
				};
				break;
		}
	}

};
// }}}
// get the current command buffer, to execute at enter {{{
std::string RText::getcurcommand()
{
    return commandbuf;
};
// }}}




//footer, leave at bottom
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

