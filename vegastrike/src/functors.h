/***************************************************************************
 *   Copyright (C) 2005 by Matthew Adams                                   *
 *   roguestar191 at comcast dot net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*
 * Copyright (C) 2022 Vincent Sallaberry
 *   New Functor class (#define FUNCTORS_NEW_FUNCTOR_CLASS)
 *   Only one function pointer is stored, easier to add new prototypes.
 */

#include <cstdlib>
#include <string>
#include <vector>

#ifdef FUNCTORS_INC
#else
#define FUNCTORS_INC 1

#define FUNCTORS_NEW_FUNCTOR_CLASS

class Attributes {
        public:
        Attributes() : hidden(false), webbcmd(false), immcmd(false), type(0), escape_chars(true) {}
        bool hidden; //hidden
        bool webbcmd; //web command
        bool immcmd; //immortal command
        int type;
        bool escape_chars;
	//nothing returns yet anyway, strings may be the most useful?
        class returnType {
                public:
                returnType(){;};
                returnType(const returnType &in) {
                        if(in.s.size() > 0)
                        s.append(in.s);

                }
                std::string s;
        };
        returnType m_return;

};

class TFunctor {
	public:
		Attributes attribs;
		virtual ~TFunctor(){};
		virtual void *Call(std::vector<std::string>&d, int &sock_in, bool *isDown)=0;
};

template <class TClass> class FunctorFun {
    public:
        typedef void (TClass::*void_fun)();                                                                 //fpt1
		typedef void (TClass::*strRef_fun)(std::string &);                                                  //fpt2
		typedef void (TClass::*cpChar_fun)(const char *);                                                   //fpt3
		typedef void (TClass::*cpCharA_fun)(const char *array[]);                                           //ftp4
		typedef void (TClass::*_2pChar_fun)(const char *, const char *);                                    //fpt5
		typedef void (TClass::*pBool_fun)(bool *);                                                          //fpt6
		typedef void (TClass::*int_fun)(int);                                                               //fpt7
		typedef void (TClass::*char_fun)(char);                                                             //fpt8
		typedef void (TClass::*pStrVecP_fun)(std::vector<std::string *> *d);                                //fpt9
		typedef void (TClass::*pStrVecP_intRef_fun)(std::vector<std::string *> *d, int &sock_in);           //fpt10
		typedef void (TClass::*strRef_intRef_fun)(std::string &, int&);                                     //fpt11
		typedef void (TClass::*pStrVecP_intRef_bool_fun)(std::vector<std::string *> *, int &, bool);        //fpt12
        typedef void (TClass::*strVecRef_fun)(const std::vector<std::string> &d);                           //fpt13 - new
};

#ifdef FUNCTORS_NEW_FUNCTOR_CLASS
// New 2022 method (only one function pointer is stored, easier to add new prototypes)
/*
 * Example:
 *   class Run {
 *   public:
 *     void run_str(std::string & args) {
 *         fprintf(stderr, "calling, args: %s\n", args.c_str());
 *     }
 *   };
 *   int main() {
 *     std::vector<std::string> v; int a;
 *     make_functor(new Run(), &Run::run_str)->Call(v, a, NULL);
 *     return 0;
 *   }
 *
 * *** Adding a new call prototype:
 * 1) (optional) add your new type 'typedef void (TClass::*newtype_fun)(ClassX clsx);' in FunctorFun class
 * 2) add the protected polyCall() method for new type in Functor class:
 *    virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::newtype_fun _fun) {
 *        (*this->where.*_fun)(ClassX());
 *        return NULL;
 *    }
 */

template <class TClass, typename TFun> class Functor : public TFunctor
{
    public:
        Functor(TClass * _where, TFun _fun) : where(_where), fun(_fun) {};
        virtual ~Functor() {}
        void * Call(std::vector<std::string>&d, int &sock_in, bool *isDown) {
            void * ret = polyCall(d, sock_in, isDown, fun);
            return ret == NULL ? &(attribs.m_return) : ret;
        }
    protected:
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::void_fun _fun) { //1
            (*this->where.*_fun)();
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::strRef_fun _fun) { //2
        	std::string a;
        	unsigned int x;
        	for(x = 0; x < d.size(); x++) {
        		a.append(d[x]);
        		a.append(" ");
        	}
        	(*this->where.*_fun)(a);
        	return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::cpChar_fun _fun) { //3
        	if(d.size() >= 2) {
        		(*this->where.*_fun)(d[1].c_str());
        	} else (*this->where.*_fun)((const char *)NULL);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::cpCharA_fun _fun) { //4
        	std::vector<const char *> buf;
        	for(unsigned int c = 0; c < d.size(); ) {
        		buf.push_back(d[c].c_str());
        		c++;
        		if( !(c < d.size() ) ) {
        			buf.push_back(" ");
        		}
        	}
        	(*this->where.*_fun)(&buf[0]);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::_2pChar_fun _fun) { //5
        	if(d.size() < 2) {
        		(*this->where.*_fun)((const char *)NULL, (const char *)NULL);
        	} else if(d.size() < 3) {
        		(*this->where.*_fun)(d[1].c_str(), (const char *)NULL);
        	} else {
        		(*this->where.*_fun)(d[1].c_str(), d[2].c_str());
        	}
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::pBool_fun _fun) { //6
        	(*this->where.*_fun)(isDown);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::int_fun _fun) { //7
        	if(d.size() < 2) {
        		(*this->where.*_fun)(0);
        	} else {
        		(*this->where.*_fun)(atoi(d[1].c_str()));
        	}
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::char_fun _fun) { //8
        	if(d.size() < 2) {
        		char err = 0;
        		(*this->where.*_fun)(err);
        	}  else {
        		(*this->where.*_fun)(d[1][0]);
        	}
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::pStrVecP_fun _fun) { //9
        	std::vector<std::string *> dup;
        	std::vector<std::string>::iterator ptr = d.begin();
        	while(ptr < d.end()) {
        		dup.push_back(&(*(ptr)));
        		ptr++;
        	}
        	(*this->where.*_fun)(&dup);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::pStrVecP_intRef_fun _fun) { //10
        	std::vector<std::string *> dup;
        	std::vector<std::string>::iterator ptr = d.begin();
        	while(ptr < d.end()) {
        		dup.push_back(&(*(ptr)));
        		ptr++;
        	}
        	(*this->where.*_fun)(&dup, sock_in);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::strRef_intRef_fun _fun) { //11
        	std::string a;
        	unsigned int x;
        	for(x = 0; x < d.size(); x++) {
        		a.append(d[x]);
        		a.append(" ");
        	}
        	(*this->where.*_fun)(a, sock_in);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::pStrVecP_intRef_bool_fun _fun) {//12
        	std::vector<std::string *> dup;
        	std::vector<std::string>::iterator ptr = d.begin();
        	while(ptr < d.end()) {
        		dup.push_back(&(*(ptr)));
        		ptr++;
        	}
        	(*this->where.*_fun)(&dup, sock_in, false);
            return NULL;
        }
        virtual void * polyCall(std::vector<std::string>&d, int &sock_in, bool *isDown, typename FunctorFun<TClass>::strVecRef_fun _fun) { //13
            (*this->where.*_fun)(d);
            return NULL;
        }

        TClass * where;
        TFun fun;
};

// Easy way to create a functor without need of specifyuiing template parameters (like std::make_pair)
template <class TClass, typename TFun> Functor<TClass, TFun> * make_functor(TClass * where, TFun fun) {
    return new Functor<TClass, TFun>(where, fun);
}

#else
// Old 2005 method (TFun parameter is not used, it is here for compatibility with new 2022 method)
template <class TClass, typename TFun=typename FunctorFun<TClass>::void_fun> class Functor : public TFunctor
{
	// To add a new callback method, add a new fpt type here,
	//set it to NULL in nullify, then add it to the list
	// of if/else in the main Call method.
	private:
		void (TClass::*fpt1)();
		void (TClass::*fpt2)(std::string &);
		void (TClass::*fpt3)(const char *);
		void (TClass::*fpt4)(const char *array[]);
		void (TClass::*fpt5)(const char *, const char *);
		void (TClass::*fpt6)(bool *);
		void (TClass::*fpt7)(int);
		void (TClass::*fpt8)(char);
		void (TClass::*fpt9)(std::vector<std::string *> *d);
		void (TClass::*fpt10)(std::vector<std::string *> *d, int &sock_in);
		void (TClass::*fpt11)(std::string &, int&);		
		void (TClass::*fpt12)(std::vector<std::string *> *, int &, bool);
		void (TClass::*fpt13)(const std::vector<std::string> &);
		TClass* pt2Object; // pointer to object
        public:
		// New singularlized call method {{{:
		virtual void *Call(std::vector<std::string>&d, int &sock_in, bool *isDown) {
			//Comments {{{
			//ok, d[0] == command typed
			//    d[1] == arg1
			//    d[2] == arg2, etc. 
			//    sometimes socket can be ignored
			// }}}
			if(fpt1 != NULL) { // fpt1() no args {{{
				(*pt2Object.*fpt1)();
			// }}}
			} else if(fpt2 != NULL) { // fpt2(std::string &)  {{{
				std::string a;
				unsigned int x;
				for(x = 0; x < d.size(); x++) {
					a.append(d[x]);
					a.append(" ");
				}
				(*pt2Object.*fpt2)(a);
			// }}}
			} else if(fpt3 != NULL) { //fpt3(const char *); {{{
				if(d.size() >= 2) {
					(*pt2Object.*fpt3)(d[1].c_str());
				} else (*pt2Object.*fpt3)((const char *)NULL);
			// }}}
			} else if(fpt4 != NULL) { // (const char *array[]); {{{
				std::vector<const char *> buf;
				for(unsigned int c = 0; c < d.size(); ) {
					buf.push_back(d[c].c_str());
					c++;
					if( !(c < d.size() ) ) {
						buf.push_back(" ");
					}
				}
				(*pt2Object.*fpt4)(&buf[0]);
			// }}}
			} else if(fpt5 != NULL) { //(const char *, const char *); {{{
				if(d.size() < 2) {
					(*pt2Object.*fpt5)((const char *)NULL, (const char *)NULL);
				} else if(d.size() < 3) {
					(*pt2Object.*fpt5)(d[1].c_str(), (const char *)NULL);
				} else {
					(*pt2Object.*fpt5)(d[1].c_str(), d[2].c_str());
				} 
			//}}}
			} else if(fpt6 != NULL) { //(bool *); {{{
				(*pt2Object.*fpt6)(isDown);
			// }}}
			}else if(fpt7 != NULL) { // (int) {{{
				if(d.size() < 2) {
					(*pt2Object.*fpt7)(0);
				} else {
					(*pt2Object.*fpt7)(atoi(d[1].c_str()));
			
				}
			// }}}
			} else if(fpt8 != NULL) { // (char) {{{
				if(d.size() < 2) {
					char err = 0;
					(*pt2Object.*fpt8)(err);
				}  else { 
					(*pt2Object.*fpt8)(d[1][0]);
				} 
			// }}}
			} else if(fpt9 != NULL) { // (std::vector<std::string *> *d) {{{
				std::vector<std::string *> dup;
				std::vector<std::string>::iterator ptr = d.begin();
				while(ptr < d.end()) {
					dup.push_back(&(*(ptr)));
					ptr++;
				}
				(*pt2Object.*fpt9)(&dup);
			// }}}
			} else if(fpt10 != NULL) { // (std::vector<std::string *> *d, int) {{{
                std::vector<std::string *> dup;
				std::vector<std::string>::iterator ptr = d.begin();
                while(ptr < d.end()) {
                    dup.push_back(&(*(ptr)));
                    ptr++;
                }
                (*pt2Object.*fpt10)(&dup, sock_in);
			// }}}
			} else if(fpt11 != NULL) { //(std::string &, int&); {{{
				std::string a;
				unsigned int x;
				for(x = 0; x < d.size(); x++) {
					a.append(d[x]);
					a.append(" ");
				}
				(*pt2Object.*fpt11)(a, sock_in);
			// }}}
			} else if(fpt12 != NULL) { //(std::vector<std::string *> *, int &, bool); // {{{
				std::vector<std::string *> dup;
				std::vector<std::string>::iterator ptr = d.begin();
				while(ptr < d.end()) {
					dup.push_back(&(*(ptr)));
					ptr++;
				}
				(*pt2Object.*fpt12)(&dup, sock_in, false);
			} else if(fpt13 != NULL) { // (const std::vector<std::string> &d) {{{
				(*pt2Object.*fpt13)(d);
			} // }}}
			return &(attribs.m_return);
			return NULL;
		} // }}}
		void nullify() { // Set all the fpt's to null {{{
			fpt1 = NULL;
			fpt2 = NULL;
			fpt3 = NULL;
			fpt4 = NULL;
			fpt5 = NULL;
			fpt6 = NULL;
			fpt7 = NULL;
			fpt8 = NULL;
			fpt9 = NULL;
			fpt10 = NULL;
			fpt11 = NULL;
			fpt12 = NULL;
			fpt13 = NULL;
		}; // Nullify }}}
		// Constructors, call nullify, set pt2object and function pointer {{{
		Functor(TClass* _pt2Object, void(TClass::*_fpt)())
		{ nullify();pt2Object = _pt2Object;  fpt1=_fpt;};
//1 std::string
		Functor(TClass* _pt2Object, void(TClass::*_fpt)(std::string &))
		{ nullify();pt2Object = _pt2Object;  fpt2=_fpt; };
//1 c string
		Functor(TClass* _pt2Object, void(TClass::*_fpt)(const char *))
		{ nullify();pt2Object = _pt2Object;  fpt3=_fpt;};
		Functor(TClass* _pt2Object, void(TClass::*_fpt)(const char *array[]))
		{ nullify();pt2Object = _pt2Object; fpt4=_fpt;};
//2 c strings
		Functor(TClass* _Obj, void(TClass::*_fpt)(const char *, const char *))
		{ nullify();pt2Object = _Obj; fpt5=_fpt; };
//1 bool
		Functor(TClass* _Obj, void(TClass::*_fpt)(bool *))
		{ nullify();pt2Object = _Obj; fpt6=_fpt; };

		Functor(TClass* _Obj, void(TClass::*_fpt)(int))
		{ nullify();pt2Object = _Obj; fpt7=_fpt; };


		Functor(TClass* _Obj, void(TClass::*_fpt)(char))
		{ nullify();pt2Object = _Obj; fpt8=_fpt; };

		Functor(TClass* _Obj, void(TClass::*_fpt)(std::vector<std::string *> *d))
		{ nullify();pt2Object = _Obj, fpt9=_fpt; };		

		Functor(TClass* _Obj, void(TClass::*_fpt)(std::vector<std::string *> *d, int &))
		{ nullify();pt2Object = _Obj, fpt10=_fpt; }

		Functor(TClass* _pt2Object, void(TClass::*_fpt)(std::string &, int&))
		{ nullify();pt2Object = _pt2Object;  fpt11=_fpt; };
		
		Functor(TClass* _Obj, void(TClass::*_fpt)(std::vector<std::string *> *d, int &, bool))
		{ nullify();pt2Object = _Obj, fpt12=_fpt; }

		Functor(TClass* _Obj, void(TClass::*_fpt)(const std::vector<std::string> &d))
		{ nullify();pt2Object = _Obj, fpt13=_fpt; }
 // }}}

		virtual ~Functor(){};


};

// Easy way to create a functor without need of specifyuiing template parameters (like std::make_pair)
template <class TClass, typename TFun> Functor<TClass, TFun> * make_functor(TClass * where, TFun fun) {
    return new Functor<TClass,TFun>(where, fun);
}

#endif // ! FUNCTORS_NEW_FUNCTOR_CLASS



/* *******************************************
 * FUNCTOR UNITARY TESTS
 * *******************************************/


#if defined(VS_FUNCTORS_TESTS)
# include <stdio.h>

namespace vs_functors_tests {

# define STR(x) #x
# define PTEST(hdr,cond,res,...) fprintf(stderr, hdr " %s [%s] - %d" "\n", __VA_ARGS__, res, STR(cond), __LINE__)
# define TESTV(_hdr, cond, ...) ((cond) ? (0) \
                                        : (PTEST(_hdr,cond,"FAILED",__VA_ARGS__)*0+1))
# define TEST(hdr,cond) TESTV("%s" hdr, cond, "")

class Run {
public:
    void run_str(std::string & args) {
        fprintf(stderr, "calling, args: %s\n", args.c_str());
    }
    void run_vec(const std::vector<std::string> & vec) {
        fprintf(stderr, "calling, args:");
        for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
            fprintf(stderr, " %s", it->c_str());
        }
        fprintf(stderr, "\n");
    }
};

class Run2 {
public:
    void run(const char * arg1, const char * arg2) {
        fprintf(stderr, "calling, arg1: %s, arg2: %s\n", arg1, arg2);
    }
    void run_strRef_intRef(std::string & s, int & i) {
        fprintf(stderr, "calling, s:%s, i:%d, setting to 'ohoh', 912\n", s.c_str(), i);
        s = "hoho"; i = 912;
    }
};


int test_functor() {
	unsigned int nerrors = 0;
	fprintf(stderr, "\n+ Functors Tests...\n");

    Run * run = new Run();
    Functor<Run, FunctorFun<Run>::strRef_fun> * functor = new Functor<Run, FunctorFun<Run>::strRef_fun>(run, &Run::run_str);
    Functor<Run, FunctorFun<Run>::strVecRef_fun> * functor2 = new Functor<Run, FunctorFun<Run>::strVecRef_fun>(run, &Run::run_vec);
    std::vector<std::string> v;
    int a = 0;
    v.push_back("hello");
    v.push_back("new");
    v.push_back("world");
    v.push_back("!");
    functor->Call(v, a, NULL);
    functor2->Call(v, a, NULL);
    make_functor(run, &Run::run_str)->Call(v, a, NULL);
    make_functor(run, &Run::run_vec)->Call(v, a, NULL);

    Run2 *run2 = new Run2;
    make_functor(run2, &Run2::run)->Call(v, a, NULL);
    std::string s("hello"); a = 2;
    make_functor(run2, &Run2::run_strRef_intRef)->Call(v, a, NULL); fprintf(stderr, "  strRef: %s intRef: %d\n", s.c_str(), a);
    nerrors += TEST("Run2==run_strRef_intRef", a == 912);

    fprintf(stderr, "Functor tests: %u error(s)\n", nerrors);
    return nerrors;
}

} // ! namespace vs_functor_tests

#endif // !# if defined(VS_FUNCTORS_TESTS)
/* ***********************************************************/


#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */


