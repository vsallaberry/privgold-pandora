/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
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

#include "textinputdisplay.h"
#include "lin_time.h"
#include "guidefs.h"
#include "universe_util.h"
#include "unicode.h"
#include "log.h"

using namespace std;

TextInputDisplay::TextInputDisplay(std::vector <unsigned int> *keyboard_input_queue, const char * disallowed) {
  isFocused = false;
  if (keyboard_input_queue) {
    this->keyboard_queue=keyboard_input_queue;
  }else {
    this->keyboard_queue=&local_keyboard_queue;
  }
  passwordChar = '\0';
  keyboard_input_queue->clear();
  this->disallowed= new char[strlen(disallowed)+1];
  strcpy(this->disallowed, disallowed);
}

bool TextInputDisplay::processMouseDown(const InputEvent&event) {
	if (event.code != WHEELUP_MOUSE_BUTTON && event.code!= WHEELDOWN_MOUSE_BUTTON) {
        bool was_focused = this->isFocused;
        // If click is on me, set me focused... otherwise, clear my focus.
		this->isFocused = (hitTest(event.loc));
        if (!was_focused && this->isFocused) {
            UniverseUtil::enableKeyRepeat();
        }
	}
	return StaticDisplay::processMouseDown(event);
}

void TextInputDisplay::processUnfocus(const InputEvent&event) {
	if (event.code != WHEELUP_MOUSE_BUTTON && event.code!= WHEELDOWN_MOUSE_BUTTON) {
        bool was_focused = this->isFocused;
        // If click is on me, set me focused... otherwise, clear my focus.
		this->isFocused = false;
        if (was_focused) {
            UniverseUtil::restoreKeyRepeat();
        }
	}
	StaticDisplay::processUnfocus(event);
}

bool TextInputDisplay::processKeypress(unsigned int c) {
	return true;
}

void TextInputDisplay::draw() {
  std::string text = this->text();
  if (!this->isFocused) {
    if (passwordChar) {
      std::string text1;
      text1.insert(0u, text.length(), passwordChar);
      this->setText(text1);
    }
    this->StaticDisplay::draw();
    if (passwordChar) {
      this->setText(text);
    }
    return;
  }
  size_t LN = keyboard_queue->size();
  for (size_t i=0;i<LN;++i) {
    unsigned int c=(*keyboard_queue)[i];
	if (!processKeypress(c)) continue;
	
    if (c==8||c==127) {
        Utf8Iterator end = Utf8Iterator::end(text) - 1;
        if (*end == '#' && end.pos() != 0 && *(end-1) == '#') {
            --end;
        }
        text = text.substr(0, end.pos());
    }else if ((c!='\0'&&c<128) || WSK_CODE_IS_UTF32(c)) {
        bool allowed=true;
        for (Utf8Iterator it = Utf8Iterator::begin(disallowed); it != it.end(); ++it) {
            if (c == *it) {
                allowed=false;
                break;
            }
        }
        if (allowed) {
            if (c < 128) {
                char tmp[2]={0,0};
                tmp[0]=(char)c;
                text+=tmp;
                if (c == '#') // '#' is an escape character in PaintText
                    text += '#';
            } else if (WSK_CODE_IS_UTF32(c)) {
                c = WSK_CODE_TO_UTF32(c);
                char tmp[MB_CUR_MAX+1];
                size_t n = utf32_to_utf8(tmp, c);
                text += tmp;
            }
            VS_DBG("gui", logvs::DBG, "TextInputDisplay: adding char '%c' %x", c, c);
        }
    }
  }
  keyboard_queue->clear();
  unsigned int x= (unsigned int)getNewTime();
  string text1;
  if (passwordChar) {
	text1.insert(0u, text.length(), passwordChar);
  } else {
    text1=text;
  }
  if (x%2) {
    text1+="|";
  }
  this->setText(text1);
  this->StaticDisplay::draw();
  this->setText(text);
}

TextInputDisplay::~TextInputDisplay() {
  UniverseUtil::restoreKeyRepeat();
  delete []this->disallowed;
}
