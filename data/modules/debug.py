import traceback
import sys
import os
import VS
import site

debugnum=0

NONE=0
ERROR=1
WARN=2
NOTICE=3
INFO=4
VERBOSE=5
DEBUG=6

# Cache of Vegastrike engine log levels
_levels=dict()
# Per file log levels (True), global log level (False)
_perfilelog = True
# logfile
_logfile = sys.stderr
_logfile_istty = False
_log_colorize = -1
_msgcenter_dispatch = False
_log_timestamp = True

_level_color_reset = "\x1b[00m"
_level_color = [
	"",
	"\x1b[01;31m",
	"\x1b[01;33m",
	"\x1b[00;32m",
	"\x1b[00;34m",
	"\x1b[00;36m",
	"\x1b[00;35m"
]

class VSException(Exception):
	pass

def prettyfile(fil):
	lasttwo=str(fil).replace('\\', '/').split('/')[-2:]
	if len(lasttwo)<2: return fil
	return lasttwo[0][0]+'/'+lasttwo[1]

def log_levelModuleFileLine(stackdec=0):
	global _levels, _perfilelog
	laststack = traceback.extract_stack()[-2-stackdec]
	pfile = prettyfile(laststack[0])
	module = pfile if _perfilelog else 'pyModules'
	if module == '<string>':
		module = 'python<string>'
	if not module in _levels:
		level = VS.LogLevel(module, False)
		_levels[module] = level
	else:
		level = _levels[module]
	return (level,module,pfile,laststack[1])

def _dprint(msg, eol='\n', level=0, stackdec=0):
	global _logfile
	(modlevel,_,_,_) = (0,0,0,0) if level==0 else log_levelModuleFileLine(stackdec+1)
	if modlevel >= level and _logfile is not None:
		try:
			_logfile.write(msg + eol)
		except:
			print '!! LOG ERRORS: ' + str(_logfile.errors)
	
def _pprint(object, stream=None, level=NOTICE,stackdec=0):
	global _logfile
	(modlevel,_,_,_) = log_levelModuleFileLine(stackdec+1)
	if modlevel >= level:
		import pprint
		pprint.pprint(object, _logfile if stream is None else stream)

def _debug_noengine(msg, level=NOTICE, stackdec=0): # Simple line number
	laststack = traceback.extract_stack()[-2-stackdec]
	dprint(' +++ '+prettyfile(laststack[0])+':'+str(laststack[1])+': '+str(msg))
	return 1

def _debug(msg, level=NOTICE, stackdec=0): # Simple line number
	global _perfilelog, _level_colors, _logfile_istty, _log_colorize, _msgcenter_dispatch, _log_timestamp
	(modlevel,module,pfile,line) = log_levelModuleFileLine(stackdec+1)
	if modlevel >= level:
		logmsg = (pfile+':' if not _perfilelog else '')+str(line)+': '+str(msg)
		color = (_level_color[level if level >= 0 and level <= DEBUG else DEBUG] if (_log_colorize and (_logfile_istty or _log_colorize==1)) else '')
		timestamp = '%.03f ' % (VS.timeofday()) if _log_timestamp else ''
		dprint(timestamp + '[' + color + module + (_level_color_reset if len(color) else '') + '] ' + logmsg)
		if _msgcenter_dispatch:
			VS.IOmessage(0, "log/"+module, "all", logmsg)
		return 1
	return 0

def _info(msg, level=NOTICE, stackdec=0): # == /dev/null
	return debug(msg,level,stackdec+1)

def _warn(msg, level=WARN,stackdec=0): # Traceback without killing the script
	global debugnum
	debugnum+=1
	#print " *** Python Warning "+str(debugnum)+"!"
	if debug(' *** Python Warning '+str(debugnum)+'!  -->  '+str(msg),level,stackdec+1) <= 0:
		return 0
	dprint('Warning Traceback '+str(debugnum)+':')
	for frame in traceback.extract_stack()[:-1]:
		dprint('  File "'+prettyfile(frame[0])+'", line '+str(frame[1])
			+', in '+str(frame[2])+'\n	'+str(frame[3]))
	dprint('Message: '+str(msg)+'\n\n')
	return 1

def _fatal(msg, level=ERROR, stackdec=0): # Kill the script!
	global debugnum
	debugnum+=1
	#print "Python VSException "+str(debugnum)+"!"
	debug('Python VSException '+str(debugnum)+'!  ->  '+str(msg), level, stackdec+1)
	raise VSException(msg)

fatal = _fatal # Really bad error... Kill the script.  Same as a call to raise()

dprint = _dprint # 'print' like function used by debug modules
pprint = _pprint

warn = _warn   # Anything that shouldn't happen, but shouldn't cause a crash either.
error = _warn  # Different name for the same thing.

# Less important messages that happen a lot.
debug = _debug # Useful messages for hunting down bugs, or loading status.
info = _info   # I don't think this is useful, but why not?

# For release, we can disable unimportant messages:
# debug = _info

def _str2bool(s):
	return s == '1' or s.lower() == 'yes' or s.lower() == 'true' or s.lower() == 'enabled' or s.lower() == 'on'

def init(forceMsgCenter=False):
	global _logfile, _logfile_istty, _log_colorize, _msgcenter_dispatch, _log_timestamp
	_file = VS.LogFile("")
	if _file == "":
		_logfile = None
	elif _file.lower() == "stdout":
		_logfile = sys.stdout
	elif _file.lower() == "stderr":
		_logfile = sys.stderr
	else:
		try:
			_logfile = os.open(_file, "a", 1) # 1 for line-buffered
		except:
			_logfile = sys.stderr
	try:
		str_msgcenter=VS.vsConfig("log","msgcenter","false").lower() if not forceMsgCenter else "yes"
		_msgcenter_dispatch = _str2bool(str_msgcenter)
		str_colorize=VS.vsConfig("log","colorize","auto").lower()
		_log_timestamp=_str2bool(VS.vsConfig("log","timestamp","yes").lower())

		if _logfile is not None and os.name == 'posix':
			_logfile_istty=_logfile.isatty()
			_log_colorize = -1 if str_colorize == "" or str_colorize == 'auto' else (1 if _str2bool(str_colorize) else 0)
		else:
			_logfile_istty = False
			_log_colorize = 0
		debug('debug initialized, os='+os.name+' file = ' + _file + ('(tty)' if _logfile_istty else '')+'.')
	except:
		_logfile_istty=False
		_msgcenter_dispatch=False
		_logfile=sys.stderr
		_log_colorize=0
		_log_timestamp = False
		debug('debug initialize failed, os='+os.name+' using file = stderr.')

init()

