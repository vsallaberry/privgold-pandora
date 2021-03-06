import VS
import traceback
import sys
import debug

procedures = {
	}

def add(name, proc):
	procedures[name.lower()] = proc

import weapons_lib #adds to procedures list
import guilds #adds to procedures list
import campaign_lib
import dialog_box
import net_computer

running_cmds = {}

maxid = 2
def generateID():
	global maxid;
	maxid=maxid+1
	return str(maxid)

def splitArgs(argstr):
	ret=[]
	while argstr:
		arg = ''
		qadd = ''
		empty = True
		while argstr and argstr[0]=='"':
			end=argstr.find('"', 1)
			if end!=-1:
				arg += qadd + argstr[1:end]
				qadd='"'
				argstr = argstr[end+1:]
				empty = False
			else:
				arg = argstr[1:]
				empty = False
				argstr = ''
		space = argstr.find(' ')
		if space != -1:
			arg += argstr[:space]
			argstr = argstr[space+1:]
		else:
			arg += argstr
			argstr = ''
		if arg or not empty:
			ret.append(arg)
	return ret

def joinArgs(arglist):
	ret = ''
	for arg in arglist:
		if ret:
			ret += ' '
		arg = str(arg).replace('\'','') # Remove all single-quotes
		space = arg.find(' ')
		quote = arg.find('"')
		newstr = arg.replace('"','""')
		if not newstr or space!=-1 or quote!=-1:
			ret += '"' + newstr + '"'
		else:
			ret += newstr
	return ret

def putFunction(continuation, id, cp):
	global running_cmds;
	if not id:
		id = generateID()
	key = str(cp)+","+id
	running_cmds[key] = continuation
	return id

def getFunction(id, cp):
	key = str(cp)+","+id
	if running_cmds.has_key(key):
		func = running_cmds[key]
		del running_cmds[key]
		return func
	return None

# custom.run should be the last thing that happens in a function/
# it might either be synchronous or asynchronous (this could be considered a bug)

def run(cmd, args, continuation, id=None, cp=-1):
	if -1==cp:
		cp = VS.getCurrentPlayer()
	if continuation:
		id = putFunction(continuation, id, cp)
	if not isinstance(id,str):
		id = "null"
	debug.debug("running: "+cmd+", "+str(args)+"; id: "+id)
	VS.sendCustom(cp, cmd, joinArgs(args), id)
	return id

def respond(args, continuation, id, cp=-1):
	run("response", args, continuation, id, cp)

class LineBufferWriter:
	def __init__(self,line='',level=debug.INFO):
		self.line=line
	def println(self,line):
		debug.dprint(repr(line), '\n', level)
	def write(self,text):
		lines = text.split('\n')
		self.line=lines[-1]
		lines = lines[:-1]
		for l in lines:
			self.println(l)

class IOmessageWriter(LineBufferWriter):
	def __init__(self,cpnum):
		LineBufferWriter.__init__(self)
		if cpnum<0:
			self.cpstr='all'
		else:
			self.cpstr = 'p'+str(cpnum)
	def println(self, l):
		VS.IOmessage(0,"game",self.cpstr,l)


def processMessage(local, cmd, argstr, id, writer=None):
	cp = VS.getCurrentPlayer();
	cmd = cmd.lower()
	if not writer:
		if id or cp<0:
			writer = sys.stderr
		else:
			writer = IOmessageWriter(cp)
	debug.debug("======= Processing message "+str(id)+" =======",debug.INFO)
	try:
		args = splitArgs(argstr)
		debug.debug("Command: "+cmd,debug.INFO)
		for arg in args:
			debug.debug(repr(arg),debug.INFO)
		if cmd=='reloadlib' and local and len(args)>=1:
			reload(__import__(args[0]))
			writer.write("Reloaded "+str(args[0])+"\n")
		elif cmd=='local':
			# simple way of bouncing back message to client....
			if id:
				def localresponse(args):
					respond(args, None, id, cp)
			else:
				localresponse = None
			run(args[0], args[1:], localresponse, id, cp)
		elif (cmd=='response'):
			func = getFunction(id, cp)
			if func:
				ret = func(args)
				if ret and isinstance(ret, tuple) and len(ret)==2:
					respond(ret[0], ret[1], id, cp)
				elif ret==True:
					putFunction(func, id, cp)
				elif ret:
					respond(ret, None, id, cp)
				return ret
		elif procedures.has_key(cmd):
			ret = procedures[cmd](local, cmd, args, id)
			if id and id != 'null':
				if ret and isinstance(ret, tuple) and len(ret)==2:
					respond(ret[0], ret[1], id, cp)
				elif ret:
					respond(ret, None, id, cp)
			else:
				return ret

		elif VS.isserver():
			import server
			server.processMessage(cp, local, cmd, args, id, writer)
		else:
			writer.write("Command "+repr(cmd)+" does not exist. Available functions:\n")
			writer.write(procedures.keys())
	except:
		writer.write("An error occurred when processing custom command: \n"
			+ str(cmd)+" "+argstr + "\n")
		traceback.print_exc(file=writer)
	debug.debug("-------------------------- " +str(id)+" -------",debug.INFO)

