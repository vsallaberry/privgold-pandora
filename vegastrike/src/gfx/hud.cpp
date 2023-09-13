/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "gfxlib.h"
#include "cmd/unit_generic.h"
#include "hud.h"
#include "lin_time.h"
#include "file_main.h"
#include "gfx/aux_texture.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "xml_support.h"
#include "cmd/base.h"
//#include "glut.h"
#include "gnuhash.h"

#include "gldrv/gl_globals.h"
#include "unicode.h"
#include "log.h"

#if defined(HAVE_CWCTYPE)
# include <cwctype>
#elif defined(HAVE_WCTYPE_H)
# include <wctype.h>
#else
# include <ctype.h>
# define iswprint(c) isprint(c)
#endif

typedef vsUMap<void *, unsigned int *> DisplayLists;

static DisplayLists fonts_display_lists;

static bool isInside() {
  if (BaseInterface::CurrentBase) return true;
  return false;
}

void * getFontFromName(const std::string & fontName) {
    void * font;
    if (fontName == "helvetica18") {
        font = GLUT_BITMAP_HELVETICA_18;
    } else if (fontName == "helvetica12") {
        font = GLUT_BITMAP_HELVETICA_12;
    } else if (fontName == "fixed15") {
        font = GLUT_BITMAP_9_BY_15;
    } else if (fontName == "helvetica10") {
        font = GLUT_BITMAP_HELVETICA_10;
    } else if (fontName == "times24") {
        font = GLUT_BITMAP_TIMES_ROMAN_24;
    } else if (fontName == "times10") {
        font = GLUT_BITMAP_TIMES_ROMAN_10;
    } else if (fontName == "fixed13") {
        font = GLUT_BITMAP_8_BY_13;
    } else {
        font = GLUT_BITMAP_HELVETICA_12;
    }
    return font;
}

std::string getFontName(void * font) {
	if (font == GLUT_BITMAP_HELVETICA_18) {
		return "helvetica18";
	} else if (font == GLUT_BITMAP_HELVETICA_12) {
		return "helvetica12";
	} else if (font == GLUT_BITMAP_9_BY_15) {
		return "fixed15";
	} else if (font == GLUT_BITMAP_HELVETICA_10) {
		return "helvetica10";
	} else if (font == GLUT_BITMAP_TIMES_ROMAN_24) {
		return "times24";
	} else if (font == GLUT_BITMAP_TIMES_ROMAN_10) {
		return "times10";
	} else if (font == GLUT_BITMAP_8_BY_13) {
		return "fixed13";
	} else {
		return "";
	}
}

static float getFontPointSize(void * & font) {
    float point;
    if (font == GLUT_BITMAP_HELVETICA_18) {
        point = 40;
    } else if (font == GLUT_BITMAP_HELVETICA_12) {
        point = 26;
    } else if (font == GLUT_BITMAP_9_BY_15) {
        point = 34;
    } else if (font == GLUT_BITMAP_HELVETICA_10) {
        point = 22;
    } else if (font == GLUT_BITMAP_TIMES_ROMAN_24) {
        point = 50;
    } else if (font == GLUT_BITMAP_TIMES_ROMAN_10) {
        point = 22;
    } else if (font == GLUT_BITMAP_8_BY_13) {
        point = 30;
    } else {
        point = 26;
        font = GLUT_BITMAP_HELVETICA_12;
    }
    return point;
}

static bool getDefaultFont(void * & outfont, float & outpointsize, bool force_inside=false, bool whatinside=false) {
  static void * whichfont=getFontFromName(vs_config->getVariable("graphics","font","helvetica12"));
  static void * whichdockedfont=getFontFromName(vs_config->getVariable("graphics","basefont","helvetica12"));
  bool inside = isInside();
  if (force_inside)
	  inside=whatinside;
  static bool lastinside=inside;
  outfont = inside? whichdockedfont : whichfont;
  static float point = getFontPointSize(outfont);
  bool changed = (lastinside != inside && whichfont != whichdockedfont);
  if (changed) {
	  point = getFontPointSize(outfont);
      lastinside = inside;
      VS_LOG("gui", logvs::VERBOSE, "defaultFont changed to %s, point:%g, inside:%d", getFontName(outfont).c_str(), point, inside);
  }
  outpointsize = point;
  return changed;
}

float getFontHeight(void * font) {
    float point;
	if (font == NULL) {
        getDefaultFont(font, point);
    } else {
        point = getFontPointSize(font);
    }
	return point / g_game.y_resolution;
}

/* *************************************************************
 * TextPlane
 *************************************************************** */
TextPlane::TextPlane(const GFXColor & c, const GFXColor & bgcol) {
  flags = FLG_UNDERSCORE_AS_SPACE;
  col=c;
  this->bgcol=bgcol;
  myDims.i = 2;  myDims.j=-2;
  myFont = NULL;
  myFontHeight = 0.0;
  myFontMetrics.Set(.06,.08,0);
  SetPos (0,0);
}

TextPlane::~TextPlane () {
}

int TextPlane::Draw (int offset) {
  return Draw (myText,offset,true,false,true);
}

void * TextPlane::SetFont(const std::string & fontName) {
    void * font = getFontFromName(fontName);
    return SetFont(font);
}

void * TextPlane::SetFont(void * font) {
	if (font == myFont)
        return font;
	VS_DBG("gui", logvs::DBG+1, "TextPlane(%p)::SetFont(%s)", this, getFontName(font).c_str());
    if (font == NULL) {
        myFontHeight = 0.0;
    } else {
        myFontHeight = getFontPointSize(font); // will set 'font' to default if NULL or not supported
    }
    return (myFont = font);
}

void * TextPlane::GetFont(bool forceinside, bool whichinside) const {
    if (myFont != NULL) {
        return myFont;
    }
    float point;
    void * font;
    getDefaultFont(font, point, forceinside, whichinside);
    return font;
}

unsigned int TextPlane::SetFlag(unsigned int flag, bool enable) {
	unsigned int ret = this->flags;
	if (enable) this->flags |= flag; else this->flags &= ~(flag);
	return ret;
}

unsigned int TextPlane::SetFlags(unsigned int flags) {
	unsigned int ret = this->flags;
	this->flags = flags;
	return ret;
}

float TextPlane::GetFontHeight() const {
    float point;
	if (myFont != NULL) {
        point = myFontHeight;
    } else {
    	void * font;
    	getDefaultFont(font, point);
    }
	return point / g_game.y_resolution;
}

static unsigned int * GetDisplayLists(void * font) {
#   define VS_HUD_DISPLAYLISTS_NCHARS (128)
    static bool use_bit = XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
    static bool use_display_lists = XMLSupport::parse_bool (vs_config->getVariable ("graphics","text_display_lists","true"));
    if (use_display_lists) {
        DisplayLists::iterator it = fonts_display_lists.find(font);
        if (it != fonts_display_lists.end()) {
            return it->second;
        }
        VS_LOG("gui", logvs::INFO, "creating Text DisplayList for font %p", font);
        unsigned int * lists = new unsigned int[VS_HUD_DISPLAYLISTS_NCHARS+1];
        memset(lists, 0, (VS_HUD_DISPLAYLISTS_NCHARS+1) * sizeof(*lists));
        for (unsigned int i = 32; i < VS_HUD_DISPLAYLISTS_NCHARS; i++){
            if (!iswprint(i)) {
                continue ;
            }
            lists[i]= GFXCreateList();
            if (use_bit) {
                glutBitmapCharacter (font, i);
            } else {
                glutStrokeCharacter (GLUT_STROKE_ROMAN, i);
            }
            if (!GFXEndList ()) {
                lists[i]=0;
                VS_LOG("gui", logvs::VERBOSE, "Text DisplayList creation error at char 0x%x", i);
            }
        }
        fonts_display_lists.insert(std::make_pair(font, lists));
        return lists;
    }
    return NULL;
}

static unsigned char HexToChar (char a) {
  if (a>='0'&&a<='9') 
    return a-'0';
  else if (a>='a'&&a<='f') {
    return 10+a-'a';
  }else if (a>='A'&&a<='F') {
    return 10+a-'A';
  }
  return 0;
}

static unsigned char TwoCharToByte (char a, char b) {
  return 16*HexToChar(a)+HexToChar(b);
}

static float TwoCharToFloat(char a, char b) {
  return (TwoCharToByte(a,b)/255.);
}

template <class IT>
static float TwoCharToFloat(IT & it) {
  char a = *(it++) & 0xff;
  char b = *(it++) & 0xff;
  return (TwoCharToFloat(a,b));
}

void DrawSquare(float left,float right, float top, float bot) {
	GFXBegin (GFXQUAD);
	GFXVertex3f(left,top,0);
	GFXVertex3f(left,bot,0);
	GFXVertex3f(right,bot,0);
	GFXVertex3f(right,top,0);
	GFXVertex3f(right,top,0);
	GFXVertex3f(right,bot,0);
	GFXVertex3f(left,bot,0);
	GFXVertex3f(left,top,0);

	GFXEnd ();
}
                                   
float TextPlane::GetCharWidth(int c, float myFontMetrics) const {
  static bool use_bit = XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
  void * fnt = use_bit ? this->GetFont() : GLUT_STROKE_ROMAN;
  float charwid = use_bit?glutBitmapWidth(fnt,c):glutStrokeWidth(fnt,c);
  float dubyawid = use_bit?glutBitmapWidth(fnt,'W'):glutStrokeWidth(fnt,'W');
  return charwid*myFontMetrics/dubyawid;
}

float TextPlane::GetStringWidth(const std::string & str) const {
	static bool use_bit = XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
	float width = 0.0;

	for (Utf8Iterator it = Utf8Iterator::begin(str), itend = it.end(); it != itend; ++it) {
		if (*it == '#' && *(++it) != '#' && *it != '_') { //allows escape character '##', '#_' to print a '#', or '_'
			//Considering nexts are chars(not wchars).Could do 'if (*(text_it+5) != 0)' but it has cost.
			if (it.pos() + 6 <= itend.pos()) {
				it += 6 - 1;
			}
			continue ;
		}
		width += use_bit ? glutBitmapWidth(this->GetFont(), *it) /(float)(.5*g_game.x_resolution)
				         : this->GetCharWidth(*it);
	}
	return width;
}

std::string TextPlane::GetUserString(const std::string & str) const {
	std::string ret;
	for (Utf8Iterator it = Utf8Iterator::begin(str), itend = it.end(); it != itend; ) {
		if (*it == '#' && *(++it) != '#' && *it != '_') { //allows escape character '##', '#_' to print a '#', or '_'
			//Considering nexts are chars(not wchars).Could do 'if (*(text_it+5) != 0)' but it has cost.
			if (it.pos() + 6 <= itend.pos()) {
				it += 6;
			}
			continue ;
		}
		size_t pos = (it++).pos();
		ret.append(str.c_str() + pos, it.pos() - pos);
	}
	return ret;
}

template<class IT>
bool TextPlane::doNewLine(IT begin, IT end,
                          float cur_pos, float end_pos,
                          float metrics,
                          bool last_row) {
  if (*begin=='\n')
    return true;
  if (*begin==' '&&!last_row) {
    cur_pos+=this->GetCharWidth(*begin,metrics);
    begin++;
    for ( ; begin != end && cur_pos <= end_pos && !isspace(*begin); ++begin) {
      cur_pos += this->GetCharWidth(*begin,metrics);
    }
    return cur_pos>end_pos;
  }
  return cur_pos+((begin+1!=end)?this->GetCharWidth(*begin,metrics):0)>=end_pos;
}
                                   
int TextPlane::Draw(const std::string & newText, int offset,bool startlower, bool force_highquality, bool automatte)
{
  int retval=1;
  bool drawbg = (bgcol.a!=0);
  // some stuff to draw the text stuff
  //   and any other stuff related to the text stuff. Or other stuff.
  static bool use_bit = force_highquality
                     || XMLSupport::parse_bool(vs_config->getVariable ("graphics","high_quality_font","false"));
  static float font_point = XMLSupport::parse_float (vs_config->getVariable ("graphics","font_point","16"));
  static bool font_antialias = XMLSupport::parse_bool (vs_config->getVariable ("graphics","font_antialias","true"));
  static float std_wid=glutStrokeWidth (GLUT_STROKE_ROMAN,'W');
  void * fnt = this->GetFont();
  unsigned int * display_lists = GetDisplayLists(fnt);
  float rowheight;
  if (use_bit) {
	  myFontMetrics.i = glutBitmapWidth(fnt,'W');
      rowheight = this->GetFontHeight();
      myFontMetrics.j = rowheight;
  } else {
      myFontMetrics.i = font_point*std_wid / (119.05+33.33);
      myFontMetrics.j = font_point;
      myFontMetrics.j /= .5 * g_game.y_resolution;
      rowheight = myFontMetrics.j;
  }
  myFontMetrics.i /= .5 * g_game.x_resolution;
  float tmp,row, col;
  float origcol,origrow;
  GetPos(row,col);
  GetPos(row,origcol);
  Utf8Iterator text_it = Utf8Iterator::begin(newText);
  Utf8Iterator text_itend = text_it.end();
	  
  if (startlower) {
      row -= rowheight;

  }
  GFXPushBlendMode();
  glLineWidth (1);
  if (!use_bit&&font_antialias) {
    GFXBlendMode (SRCALPHA,INVSRCALPHA);
	if(gl_options.smooth_lines)
	{
		glEnable(GL_LINE_SMOOTH);
	}
  }else {
	GFXBlendMode (SRCALPHA,INVSRCALPHA);
	if(gl_options.smooth_lines)
	{
		glDisable(GL_LINE_SMOOTH);
	}
  }
  GFXColorf(this->col);

  GFXDisable (DEPTHTEST);
  GFXDisable (CULLFACE);

  GFXDisable (LIGHTING);

  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);

  glPushMatrix();
  glLoadIdentity();
  if (!automatte&&drawbg) {
	GFXColorf(this->bgcol);
	DrawSquare(col,this->myDims.i,row-rowheight*.25,row+rowheight);
  }
  GFXColorf(this->col);

  int entercount=0;
  for ( ; entercount < offset && text_it != text_itend; ++text_it) {
    if (*text_it == '\n')
      entercount++;
  }
  glTranslatef(col,row,0);  
  //  glRasterPos2f (g_game.x_resolution*(1-(col+1)/2),g_game.y_resolution*(row+1)/2);
  glRasterPos2f (0,0);
  float scalex=1;
  float scaley=1;
  int potentialincrease=0;
  if (!use_bit) {
    int numplayers=1;
    if (_Universe) // _Universe can be NULL during bootstrap.
      numplayers = (_Universe->numPlayers()>3?_Universe->numPlayers()/2:
                    _Universe->numPlayers());
    scalex=numplayers*myFontMetrics.i/std_wid;
    scaley=myFontMetrics.j/(119.05+33.33);
  }
  glScalef (scalex,scaley,1);
  bool firstThroughLoop=true;
  GFXColor currentCol (this->col);
  while(text_it != text_itend && (firstThroughLoop||row>myDims.j-rowheight*.25)) {
    unsigned int myc = *text_it;
    float shadowlen = 0;
    if (myc == '_' && (flags & FLG_UNDERSCORE_AS_SPACE) != 0) {
      myc = ' ';
    }
    
    if (myc == '#' && (myc = *(++text_it)) != '#' && myc != '_') { //allows escape character '##', '#_' to print a '#', or '_'
      //Considering nexts are chars(not wchars).Could do 'if (*(text_it+5) != 0)' but it has cost.
      if (text_it.pos() + 6 <= text_itend.pos()) {
        VS_DBG("gui", logvs::DBG+3, "ESCAPE # itpos=%zu itendpos=%zu", text_it.pos(), text_itend.pos());
        float r,g,b;
        r = TwoCharToFloat (text_it);
        g = TwoCharToFloat (text_it);
        b = TwoCharToFloat (text_it);
        if (r==0 && g==0 && b==0) {
		  currentCol = this->col;
        }else {
		  currentCol = GFXColor(r, g, b, this->col.a);
        }
        GFXColorf(currentCol);
        static bool setRasterPos= XMLSupport::parse_bool(vs_config->getVariable("graphics","set_raster_text_color","true"));
        if (use_bit&&setRasterPos)
          glRasterPos2f(col-origcol,0);
        VS_DBG("gui", logvs::DBG+3, "ESCAPE # END itpos=%zu", text_it.pos());
      } else {
        break;
      }
      continue;
    }
    if(myc == '\t') {
      shadowlen=glutBitmapWidth(fnt,' ')*5./(.5*g_game.x_resolution);
    } else {
      if (use_bit) {
        shadowlen = glutBitmapWidth (fnt,myc)/(float)(.5*g_game.x_resolution); // need to use myc -- could have transformed '_' to ' '
      } else {
        shadowlen = myFontMetrics.i*glutStrokeWidth(GLUT_STROKE_ROMAN,myc)/std_wid;
      }
    }
    if (myc >= 32) {//always true
	  if(automatte){
		GFXColorf(this->bgcol);
		DrawSquare(col-origcol,col-origcol+shadowlen/scalex,-rowheight*.25/scaley,rowheight*.75/scaley);
		GFXColorf(currentCol);
	  }
      //glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
      retval+=potentialincrease;
      potentialincrease=0;
      int lists;
      if (myc < VS_HUD_DISPLAYLISTS_NCHARS && display_lists != NULL
      &&  (lists = display_lists[myc])) {
	    GFXCallList(lists);
	  }else{
		 if (use_bit){
	        glutBitmapCharacter (fnt,myc);
		  }
		 else{
           glutStrokeCharacter (GLUT_STROKE_ROMAN,myc);
		 }
      }
	}
    if (myc == '\t') {
	  if(automatte){
		GFXColorf(this->bgcol);
		DrawSquare(col-origcol,col-origcol+shadowlen*5/(.5*g_game.x_resolution),-rowheight*.25/scaley,rowheight*.75/scaley);
		GFXColorf(currentCol);
	  }
      col+=shadowlen;
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
      glutBitmapCharacter (fnt,' ');
    } else {
      col+=shadowlen;
    }
    if(doNewLine(text_it,text_itend,col,myDims.i, myFontMetrics.i,row-rowheight<=myDims.j)){
      GetPos (tmp,col);
      firstThroughLoop=false;
      row -= rowheight;
      glPopMatrix();
      glPushMatrix ();
      glLoadIdentity();
	  if (!automatte&&drawbg) {
		GFXColorf(this->bgcol);
		DrawSquare(col,this->myDims.i,row-rowheight*.25,row+rowheight*.75);
	  }
      if (*text_it=='\n') {
	    currentCol = this->col;
      }
	  GFXColorf(currentCol);
      glTranslatef (col,row,0);
      glScalef(scalex,scaley,1);
      glRasterPos2f(0,0);
      potentialincrease++;
	}
    ++text_it;
  }
  if(gl_options.smooth_lines)
  {
	  glDisable(GL_LINE_SMOOTH);
  }
  glPopMatrix();

  
  GFXPopBlendMode();
  GFXColorf(this->col);
  return retval;
}


