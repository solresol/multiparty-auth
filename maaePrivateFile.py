import string
import time
import md5

def md5text(str):
    return ('%.2x' * 16) %  tuple(map(ord,md5.md5(str).digest()))
    

class maaePrivateFile:
    def __init__(self,fd):
        self.__filename = None
        if type(fd) == type(''):
            self.__filename = fd
            data = open(fd).readlines()
        elif type(fd) == type([]):
            pass
        else:
            data = fd.readlines()
            
        try:
            firstline = data[0]
            self.timestamp = string.atoi(firstline)
        except:
            self.timestamp = 0
            
        self.vars = {}
        
        try:
            data = data[1:]
        except:
            return

        for line in data:
            try:
                [l,r] = string.split(line,'=')
                l = string.strip(l)
                r = string.strip(r)
                self.vars[l] = r
            except:
                continue
    def add(self,username,cleartext):
        if self.vars.has_key(username): raise IndexError
        self.vars[username] = md5text(cleartext)
    def delete(self,username):
        del self.vars[username]
    def modify(self,username,cleartext):
        if not(self.vars.has_key(username)): raise IndexError
        self.vars[username] = md5text(cleartext)
    def confirm(self,username,cleartext):
        return self.vars[username] == md5text(cleartext)
    def set_timestamp(self,t=None):
        if t is None:
            t = int(time.time())
        self.timestamp = t
    def save(self,fd=None):
        if fd is None:
            fd = self.__filename
        if type(fd) == type(''):
            fd = open(fd,'w')
        fd.write(`self.timestamp`+'\n')
        for (name,checksum) in self.vars.items():
            fd.write(name + '=' + checksum + '\n')
        fd.close()
