#ifndef VS_RENDERTEXT_H
#define VS_RENDERTEXT_H

class RText {
	protected:
		//text rendering
		struct cline { 
			cline() { outtime = 0; };
			cline(const cline &in) {
			if(in.cref.size() > 0)
				cref.append(in.cref);
			outtime = in.outtime;
			}
			std::string cref;
			int outtime; 
		};
		std::vector<cline> conlines;
		std::vector<std::string> vhistory;
		int ndraw;
		int WORDWRAP;
		int conskip;
		int histpos;
		int curpos;
		int scrollpos;
		std::string commandbuf;
	public:
		RText();
		virtual ~RText();
		std::string getcurcommand();
		int text_width(const char *str); //set the text width?
		virtual std::string get_prompt(const std::string & beforeCursor, const std::string & afterCursor);
		void draw_text(const std::string &str, float left, float top, int gl_num); //creates textplane object
		void renderconsole(); //renders the text in the console
		void conline(const std::string &sf, bool highlight); //add a line to
			//the console
		void conoutfconst (const char *);
		virtual void conoutf(const std::string &s, int a = 0, int b = 0, int c = 0);
		virtual void conoutf(const char * s);
		//add a line to the console(Use this one.)
		void saycommand(const char *init); //actually does the appending of
			//the string to the commandbuf, and seperates entries
		void ConsoleKeyboardI(int code, int mod, bool release);
			//interpret keyboard input to the console
		void conoutn(const std::string &in, int a, int b, int c);
};

#endif // ! VS_RENDERTEXT_H
