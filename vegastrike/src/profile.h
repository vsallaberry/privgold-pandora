#ifndef VS_PROFILE_H
#define VS_PROFILE_H

#if !defined(VS_PROFILE_TIME_ENABLE) && defined(NDEBUG)
# define RESETTIME()
# define REPORTTIME(comment)
#else

# define RESETTIME() startTime()
# define REPORTTIME(comment) endTime(comment,__FILE__,__LINE__)

# include "lin_time.h"
# include "log.h"

static double vsprofile_start;
static inline void startTime() {
  vsprofile_start = queryTime();
}

static inline void endTime(const char* comment, const char* file, int lineno) {
  double time = queryTime() - vsprofile_start;
  //std::clog << file << "(" << comment << "):" << lineno << ": " << time << std::endl;
  VS_LOG("profile", logvs::NOTICE, "%s(%s):%d: %.03lf", file, comment, lineno, time);
}

#endif

#endif /* ! VS_PROFILE_H */
