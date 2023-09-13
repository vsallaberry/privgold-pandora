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

#ifndef __HUD_H
#define __HUD_H

#include <string>
#include "vec.h"
#include "gfxlib_struct.h"

#if defined(_WIN32 ) && defined(GetCharWidth)
// Windows defines GetCharWidth to GetCharWidth[AW]: we want TextPlane::GetCharWidth()
# undef GetCharWidth // we don't use windows GetCharWidth in vegastrike
#endif

class Texture;

class TextPlane {
	std::string myText;
    void * myFont;
    float myFontHeight;
    unsigned int flags;
    
	//Texture *myFont;
	Vector myFontMetrics; // i = width, j = height
	Vector myDims;
	//int numlet;
	/*
	struct GlyphPosition {
		float left, right, top, bottom;
	} myGlyphPos[256];
	*/
public:
	enum { FLG_NONE = 0, FLG_UNDERSCORE_AS_SPACE = 1 << 0 };
	GFXColor col,bgcol;
	TextPlane(const struct GFXColor &col=GFXColor(1,1,1,1),const struct GFXColor &bgcol=GFXColor(0,0,0,0));
	~TextPlane();
	void SetPos (float x, float y) {
	  myFontMetrics.k = y;
	  myDims.k=x;
	}
	void SetCharSize (float x, float y) {
	  myFontMetrics.i = x;
	  myFontMetrics.j = y;
	}
	void GetCharSize (float &x, float &y) const {
	  x = myFontMetrics.i;
	  y = myFontMetrics.j;
	}
	void GetPos (float &y, float &x) const {
	  y = myFontMetrics.k;
	  x = myDims.k;
	}
	void SetSize (float x, float y) {
	  myDims.i = x;
	  myDims.j = y;
	}
	void GetSize (float &x, float &y) const {
	  x = myDims.i;
	  y = myDims.j;
	}
	int Draw(int offset=0);//returns number of lines
	int Draw (const std::string &text, int offset=0, bool start_one_line_lower=false, bool force_highquality=false, bool automatte=false);
	void SetText(const std::string &newText) {
		myText = newText;
	}
	const std::string & GetText()const {
		return myText;
	}
    
    void * SetFont(const std::string & fontName);
    void * SetFont(void * font);
    void * GetFont(bool forceinside=false, bool whichinside=false) const;
    unsigned int SetFlag(unsigned int flag, bool enable);
    unsigned int SetFlags(unsigned int flags);

    float GetFontHeight() const;
    float GetCharWidth(int c) const {
    	return this->GetCharWidth(c, myFontMetrics.i);
    }
    float GetStringWidth(const std::string & str) const;
    std::string GetUserString(const std::string & str) const;

private:
    template<class IT>
    bool doNewLine(IT begin, IT end,
                   float cur_pos, float end_pos,
                   float metrics,
                   bool last_row);
    float GetCharWidth(int c, float myFontMetrics) const;
};

float getFontHeight(void * font);
void * getFontFromName(const std::string & fontName);
std::string getFontName(void * font);

#endif
