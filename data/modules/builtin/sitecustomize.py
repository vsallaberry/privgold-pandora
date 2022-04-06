# Allows having relative paths in sys.path
# import sys; sys.meta_path.append(RelativeImporter('some/reference/path'))
import sys, os, imp
def decode(s):
    try:
        import encodings
        return s.decode('utf-8')
    except:
        #import sys; sys.stderr.write('decode error\\n')
        return s
class RelativeImporter:
    def __init__(self, path=None):
        #sys.stderr.write('IMPORT_FIXER:__init__()\\n')
        #if path is not None and not os.path.isdir(path):
        #    raise ImportError
        self.path = path
    def find_module(self, fullname, path=None):
        #sys.stderr.write('IMPORT_FIXER:find_module('+str(fullname)+')\\n')
        subname = fullname.split('.')[-1]
        if subname != fullname and self.path is None:
            return None
        path = '.' if self.path is None else self.path
        try:
            oldpath = os.getcwdu()
            os.chdir(decode(self.path))
            #sys.stderr.write('CHDIR DONE '+str(os.getcwd())+'\\n')
            file, filename, stuff = imp.find_module(subname, sys.path)
            loader = RelativeLoader(path,file,filename,stuff)
            #sys.stderr.write('LOADER DONE'+'\\n')
            os.chdir(decode(oldpath))
            #sys.stderr.write('OLD CHDIR DONE '+str(os.getcwd())+'\\n')
        except ImportError:
            return None
        return loader
class RelativeLoader:
    def __init__(self, path, file, filename, stuff):
        self.path = path
        self.file = file
        self.filename = filename
        self.stuff = stuff
    def load_module(self, fullname):
        #sys.stderr.write('LOAD_MODULE : '+(str(fullname))+'\\n')
        oldpath = os.getcwdu()
        os.chdir(decode(self.path))
        #sys.stderr.write('CHDIR DONE '+str(os.getcwd())+'\\n')
        mod = imp.load_module(fullname, self.file, self.filename, self.stuff)
        os.chdir(oldpath)
        #sys.stderr.write('OLD CHDIR DONE '+str(os.getcwd())+'\\n')
        if self.file:
            self.file.close()
        mod.__loader__ = self  # for introspection
        return mod
