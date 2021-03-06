#ifndef BOOST_INTERPROCESS_CONFIG_INCLUDED
#define BOOST_INTERPROCESS_CONFIG_INCLUDED
#include <boost/config.hpp>
#endif

#ifdef BOOST_MSVC
   #ifndef _CRT_SECURE_NO_DEPRECATE
   #define  BOOST_INTERPROCESS_CRT_SECURE_NO_DEPRECATE
   #define _CRT_SECURE_NO_DEPRECATE
   #endif
   #pragma warning (push)
   #pragma warning (disable : 4702) // unreachable code
   #pragma warning (disable : 4706) // assignment within conditional expression
   #pragma warning (disable : 4127) // conditional expression is constant
   #pragma warning (disable : 4146) // unary minus operator applied to unsigned type, result still unsigned
   #pragma warning (disable : 4284) // odd return type for operator->
   #pragma warning (disable : 4244) // possible loss of data
   #pragma warning (disable : 4251) // 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
   #pragma warning (disable : 4267) // conversion from 'X' to 'Y', possible loss of data
   #pragma warning (disable : 4275) // non ? DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
   #pragma warning (disable : 4355) // 'this' : used in base member initializer list
   #pragma warning (disable : 4503) // 'identifier' : decorated name length exceeded, name was truncated
   #pragma warning (disable : 4511) // copy constructor could not be generated
   #pragma warning (disable : 4512) // assignment operator could not be generated
   #pragma warning (disable : 4514) // unreferenced inline removed
   #pragma warning (disable : 4521) // Disable "multiple copy constructors specified"
   #pragma warning (disable : 4522) // 'class' : multiple assignment operators specified
   #pragma warning (disable : 4675) // 'method' should be declared 'static' and have exactly one parameter
   #pragma warning (disable : 4710) // function not inlined
   #pragma warning (disable : 4711) // function selected for automatic inline expansion
   #pragma warning (disable : 4786) // identifier truncated in debug info
   #pragma warning (disable : 4996) // 'function': was declared deprecated
#endif
