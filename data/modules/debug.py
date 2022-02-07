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
_logwrite = _logfile.write
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
	global _logwrite
	(modlevel,_,_,_) = (0,0,0,0) if level==0 else log_levelModuleFileLine(stackdec+1)
	if modlevel >= level and _logwrite is not None:
		try:
			_logwrite(msg + eol)
		except Exception as e:
			print '[debug.py] !! LOG WRITE ERROR: ' + str(e)

class VSLogPrintStream:
	def __init__(self):
		pass
	def write(self, string):
		VS.LogPrint(string, '')

def _pprint(object, stream=None, level=NOTICE,stackdec=0):
	global _logfile
	(modlevel,_,_,_) = log_levelModuleFileLine(stackdec+1)
	if modlevel >= level:
		import pprint
		pprint.pprint(object, (VSLogPrintStream() if _logfile is None else _logfile) if stream is None else stream)

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

def initMsgCenter(forceMsgCenter=False):
	global _msgcenter_dispatch
	_msgcenter_dispatch = True if forceMsgCenter else _str2bool(VS.vsConfig("log","msgcenter","false")) 
	debug('MsgCenter dispatch '+ ('enabled' if _msgcenter_dispatch else 'disabled') + '.')

def __init():
	global _logfile, _logwrite, _logfile_istty, _log_colorize, _msgcenter_dispatch, _log_timestamp
	try:
		_file = VS.LogFile("")
		_logwrite = None
		if _file == "":
			_logfile = None
		elif _file.lower() == "stdout":
			_logfile = sys.stdout
		elif _file.lower() == "stderr" or _str2bool(VS.vsConfig("log","redirect","yes")):
			_logfile = sys.stderr
		else:
			try:
				_logfile = open(_file, 'a', buffering=1) # buffering = 1, -1 for full buffered, system default if omitted
				_logwrite = VS.LogPrint
				# We use VS.LogPrint() rather than a python file object, in order to have FILE streams sync in python/c++
				if os.name == 'posix':
					_logfile_istty=_logfile.isatty()
				_logfile.close()
				_logfile = None
			except:
				sys.stderr.write('debug.py: ERROR while opening logfile '+_file+', using stderr.\n')
				_logfile = sys.stderr

		if _logfile is not None and _logwrite is None:
			_logwrite = _logfile.write

		_msgcenter_dispatch = _str2bool(VS.vsConfig("log","msgcenter","false")) 
		str_colorize=VS.vsConfig("log","colorize","auto").lower()
		_log_timestamp=_str2bool(VS.vsConfig("log","timestamp","yes").lower())

		if _logfile is not None and os.name == 'posix':
			_logfile_istty=_logfile.isatty()
			_log_colorize = -1 if str_colorize == "" or str_colorize == 'auto' else (1 if _str2bool(str_colorize) else 0)
		else:
			# on windows: try 'reg add HKEY_CURRENT_USER\Console /v VirtualTerminalLevel /t REG_DWORD /d 0x00000001 /f'
			_logfile_istty = False
			_log_colorize = 0
		debug('debug initialized, os='+os.name+' file = ' + _file + ('(tty)' if _logfile_istty else '')+', msgcenter='+str(_msgcenter_dispatch)+'.')
	except:
		_logfile_istty=False
		_msgcenter_dispatch=False
		_logfile=sys.stderr
		_logwrite=_logfile.write
		_log_colorize=0
		_log_timestamp = False
		debug('debug initialize failed, os='+os.name+' using file = stderr.')

def log_terminate():
	global _logfile, _logwrite
	if _logfile is not None:
		_logfile.flush()
		if _logfile is not sys.stderr and _logfile is not sys.stdout:
			_logfile.close()
	_logfile = sys.stderr
	_logwrite = _logfile.write

__init()

