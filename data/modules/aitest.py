import VS
import sys
""" src/python/init.cpp: #if def OLD_PYTHON_TEST
class MyAI(VS.CommAI):
    def Execute(self):
        sys.stdout.write('MyAI\\n')
        return ''
"""
import debug
class MyAI(VS.PythonAI):
    def Execute(self):
        debug.debug('MyAI: PyhtonAI')
        
        