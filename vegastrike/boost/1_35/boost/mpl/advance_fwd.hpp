
#ifndef BOOST_MPL_ADVANCE_FWD_HPP_INCLUDED
#define BOOST_MPL_ADVANCE_FWD_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source$
// $Date: 2004-09-02 08:41:37 -0700 (Thu, 02 Sep 2004) $
// $Revision: 24874 $

#include <boost/mpl/aux_/common_name_wknd.hpp>

namespace boost { namespace mpl {

BOOST_MPL_AUX_COMMON_NAME_WKND(advance)

template< typename Tag > struct advance_impl;
template< typename Iterator, typename N > struct advance;

}}

#endif // BOOST_MPL_ADVANCE_FWD_HPP_INCLUDED
