///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains misc. useful macros & defines.
 *	\file		IceUtils.cpp
 *	\author		Pierre Terdiman (collected from various sources)
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"


using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Returns the alignment of the input address.
 *	\fn			Alignment()
 *	\param		address	[in] address to check
 *	\return		the best alignment (e.g. 1 for odd addresses, etc)
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword Alignment(udword address)
{
	// Returns 0 for null addresses
	if(!address) return 0;

	// Test all bits
	udword Align = 1;
	for(udword i=1;i<32;i++)
	{
		// Returns as soon as the alignment is broken
		if(address&Align)	return Align;
		Align<<=1;
	}
	// Here all bits are null, except the highest one (else the address would be null)
	return Align;
}

