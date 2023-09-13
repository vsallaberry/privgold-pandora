// rendertext.cpp: based on Don's gl_text.cpp
// Based on Aardarples rendertext 
// 2005 Console Rendering System by Rogue: 2005-08-a-few-days
// 2022-2023 Vincent Sallaberry - utf8,Readline-like, python, misc. improvements
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
#include "vs_log_modules.h"

// ****************
// Console Rendering System by Rogue
// 2005-08-a-few-days
// 2022-2023 Vincent Sallaberry - utf8,Readline-like, python, misc. improvements
// ****************

#if defined(_WIN32 ) && defined(GetCharWidth)
// Windows defines GetCharWidth to GetCharWidth[AW]: we want TextPlane::GetCharWidth()
# undef GetCharWidth // we don't use windows GetCharWidth in vegastrike
#endif

#ifdef HAVE_SDL
static SDL_mutex * _rtextSDLMutex()
{
	static SDL_mutex * rv = SDL_CreateMutex();
	return rv;
}
#endif


//Render Text (Console) Constructor {{{
RText::RText() : textplane(GFXColor(1, 1, 1, 1), GFXColor(0.05f, 0.05f, 0.2f, 0.5f)) {
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

	static void * font = getFontFromName(vs_config->getVariable("graphics", "interpreter_font",
										 vs_config->getVariable("graphics", "font", "helvetica12")));
	textplane.SetFlag(TextPlane::FLG_UNDERSCORE_AS_SPACE, false);
	textplane.SetFont(font);
	textplane.SetCharSize(.8, .12);

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
    int x = float_to_int(left);
    int y = float_to_int(top);
	
    textplane.SetPos(x, y);
    textplane.Draw(str);
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
    float tp_w, tp_h, txt_w = textplane.GetStringWidth(shorter);
    static const float txt_w_percents = 0.97;
    static const std::string shorter_header = "...";
    textplane.GetSize(tp_w, tp_h);
    if (shorter.size() && txt_w > tp_w * txt_w_percents) {
        txt_w += textplane.GetStringWidth(shorter_header);
        do {
            Utf8Iterator u8it = Utf8Iterator::begin(shorter);
            txt_w -= textplane.GetCharWidth(*u8it);
		    shorter.erase(0, (++u8it).pos());
		    if (shortpos > u8it.pos())
			    shortpos -= u8it.pos();
        } while (shorter.size() && txt_w > tp_w * txt_w_percents);
        shortpos += shorter_header.length();
        shorter = shorter_header + shorter;
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
    std::string::size_type len = s.find_last_not_of("\n\r");
    if (len != std::string::npos)
        ++len;
	CMD_LOG(logvs::VERBOSE, "%s", s.substr(0, len).c_str());
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
#if 1
    static const float txt_w_percents = 0.97;
    static const std::string shorter_header = "";
    static const std::string shorter_footer = "";
    float tp_w, tp_h;
    textplane.GetSize(tp_w, tp_h);
    for(Utf8Iterator it = Utf8Iterator::begin(s), itend = it.end(); it != itend; ) {
        Utf8Iterator itLine = it;
        while (itLine != itend && (*itLine != '\n' && *itLine != '\r'))
            ++itLine;

        std::string sub = s.substr(it.pos(), itLine.pos() - it.pos());
        float txt_w = textplane.GetStringWidth(sub);

        if (itLine.pos() > it.pos() + 1 && txt_w > tp_w * txt_w_percents) {
            txt_w += textplane.GetStringWidth(shorter_header) + textplane.GetStringWidth(shorter_footer);
            do {
                txt_w -= textplane.GetCharWidth(*itLine);
                --itLine;
            } while (itLine.pos() > it.pos() + 1 && txt_w > tp_w * txt_w_percents);

            Utf8Iterator itSpace = itLine;
            while (itSpace.pos() > it.pos() + 1 && *itSpace != ' ' && *itSpace != '\t'
                   && *itSpace != ',' && *itSpace != ';')
                --itSpace;
            if (itSpace.pos() > it.pos() + 1)
                itLine = itSpace + 1;

            sub = shorter_header + s.substr(it.pos(), itLine.pos() - it.pos());
            sub.append(shorter_footer);
        }

        CMD_DBG(logvs::DBG, "LINE: [%s]\n", sub.c_str());

        conline(sub, 1);

        for (char c; itLine != itend && ((c = *itLine) == '\n' || c == '\r') && *(++itLine) != c; )
            ; /* nothing */
        it = itLine;
    }
#else
    static const size_t maxChars = 2.0 / textplane.GetStringWidth("W");
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
#endif
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
				if ((*iter == '_' || *iter == '#') && iter.pos() > 0 && *(iter-1) == '#')
					--iter;
				CMD_DBG(logvs::DBG+1, "Backspace buf#%zu<%s> oldpos:%zu newpos:%zu curpos:%d",
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
				Utf8Iterator it = Utf8Iterator::begin(commandbuf, curpos);
				if (*it == '#' && *(++it) != '#' && *it != '_' && it.pos() + 6 <= it.end().pos()) {
					it += 6;
				}
				commandbuf.erase(curpos, (++it).pos());
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
				if ((mod & (WSK_MOD_LALT|WSK_MOD_RALT)) != 0) {
					while (iter.pos() > 0 && (*iter == ' ' || *iter == '\t'))
						--iter;
					while (iter.pos() > 0 && *iter != ' ' && *iter != '\t')
						--iter;
				} else --iter;
				if ((*iter == '_' || *iter == '#') && iter.pos() > 0 && *(iter-1) == '#')
					--iter;
				curpos = (iter).pos();
			}
			break;

		case WSK_RIGHT:
			if (curpos < commandbuf.size()) {
				Utf8Iterator it = Utf8Iterator::begin(commandbuf, curpos);
				if (*it == '#' && *(++it) != '#' && *it != '_' && it.pos() + 6 <= it.end().pos()) {
					it += 6;
				}
				if ((mod & (WSK_MOD_LALT|WSK_MOD_RALT)) != 0) {
					while (it != it.end() && (*it == ' ' || *it == '\t'))
						++it;
					while (it != it.end() && *it != ' ' && *it != '\t')
						++it;
				} else ++it;
				curpos += (it).pos();
			}
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
			if ((mod & (WSK_MOD_LSHIFT|WSK_MOD_RSHIFT)) != 0) {
				if (ndraw > 1) --ndraw;
			} else {
				int nscroll = 1;
				if ((mod & (WSK_MOD_LCTRL|WSK_MOD_RCTRL)) != 0)
					nscroll = ndraw - 2;
				if (nscroll > conlines.size() + 1 - scrollpos - ndraw)
					nscroll = conlines.size() + 1 - scrollpos - ndraw;
				if (scrollpos + ndraw + nscroll <= conlines.size() + 1)
					scrollpos += nscroll;
				CMD_DBG(logvs::DBG+1, "PAGEUP scollpos:%d", scrollpos);
			}
			break ;

		case WSK_PAGEDOWN:
			if ((mod & (WSK_MOD_LSHIFT|WSK_MOD_RSHIFT)) != 0) {
				if (ndraw < this->conlines.size()+1) ++ndraw;
			} else {
				int nscroll = 1;
				if ((mod & (WSK_MOD_LCTRL|WSK_MOD_RCTRL)) != 0)
					nscroll = ndraw - 2;
				scrollpos = (scrollpos > nscroll - 1) ? scrollpos - nscroll : 0;
				CMD_DBG(logvs::DBG+1, "PAGEDOWN scollpos:%d", scrollpos);
			}
			break ;

		default:
			//add it to the command buffer
			if (code>0&&(code<128||WSK_CODE_IS_UTF32(code))) {
				char utf8[MB_LEN_MAX+1];
				unsigned int wc = WSK_CODE_TO_UTF32(code);
				size_t u8sz;
				if (wc == '#' || wc == '_') {
					commandbuf.insert(commandbuf.begin() + curpos++, '#');
				} else if (textplane.GetCharWidth(wc) == 0) {
					break ; // don't add characters we cannot display.
				}
				u8sz = utf32_to_utf8(utf8, wc);
				commandbuf.insert(curpos, utf8);
				curpos += u8sz;
				if (vhistory.size()) {
					if (vhistory.back().empty())
						vhistory.pop_back();
					histpos = vhistory.size() - 1;
				}
			}
			break;
		}
	}

}
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

