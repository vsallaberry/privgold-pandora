import debug

TRACE_BIGDEBUG = 10
TRACE_DEBUG = 5
TRACE_VERBOSE = 3
TRACE_INFO = 1
TRACE_NORMAL = 0
TRACE_WARNING = -1
TRACE_ERROR = -5

#__xgui_trace_level = TRACE_NORMAL
__xgui_trace_level = TRACE_VERBOSE
#__xgui_trace_level = TRACE_DEBUG
#__xgui_trace_level = TRACE_BIGDEBUG

def _trace_noengine(level,msg):
	global __xgui_trace_level
	if __xgui_trace_level>=level:
		print msg

def _trace(level,msg):
	global TRACE_NORMAL, __xgui_trace_level
	if __xgui_trace_level>=level:
		debug.debug(msg, max(debug.ERROR,level-TRACE_NORMAL+debug.NOTICE), 1)

trace = _trace
