#include "SharedPool.h"
#include <string>

template <typename T, typename RT> SharedPool<T,RT> * SharedPool<T,RT>::ms_singleton = NULL;

template<typename T, typename RT> SharedPool<T,RT>::SharedPool()
#ifdef __GLIBC__
	: referenceCounter(
#if defined(_WIN32)||__GNUC__!=2
						RT::min_buckets
#endif
										)
#endif
{
	if (ms_singleton == NULL)
		ms_singleton = this;
}

template<typename T, typename RT> SharedPool<T,RT>::~SharedPool()
{
	if (ms_singleton == this)
		ms_singleton = NULL;
}


class SharedPoolInitializer
{
	StringPool stringPool;
};

static SharedPoolInitializer sharedPoolInitializer;
