#!/usr/bin/python

import maaeConfig
import getopt
import sys
import maaePrivateFile
import getpass
import os

(lhs,remainder) = getopt.getopt(sys.argv[1:],'a:d:m:f:c:',
                                ['add=','delete=','modify=','file=','confirm='])

def usage():
    w = sys.stderr.write
    w('Usage: ' + sys.argv[0] + ' --add username --file filename\n')
    w('Usage: ' + sys.argv[0] + ' --delete username --file filename\n')
    w('Usage: ' + sys.argv[0] + ' --modify username --file filename\n')
    w('Usage: ' + sys.argv[0] + ' --confirm username --file filename\n')
    
if remainder != []:
    usage()
    sys.exit()


filename = None
for (option,value) in lhs:
    if option == '-f' or option == '--file':
        filename = value


if filename is None:
    usage()
    sys.exit()

if not(os.path.exists(filename)):
    open(filename,'w').close()
    
privatefile = maaePrivateFile.maaePrivateFile(filename)
privatefile.set_timestamp()


for (option,value) in lhs:
    if option == '-a' or option == '--add':
        privatefile.add(value,
                        getpass.getpass('[User ' + value + "] Password / Swipe Card: "
                                        )
                        )
    if option == '-d' or option == '--delete':
        privatefile.delete(value)
    if option == '-m' or option == '--modify':
        privatefile.modify(value,
                           getpass.getpass('[User ' + value + "] Password / Swipe Card: "
                                           )
                           )
    if option == '-c' or option == '--confirm':
        if privatefile.confirm(value,
                               getpass.getpass('[User ' + value + "] Password / Swipe Card: "
                                               )
                               ):
            print "OK"
        else:
            print "Does not match"
        
        

privatefile.save(filename)
    

