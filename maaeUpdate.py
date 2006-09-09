#!/usr/bin/python

PATH_TO_GPG = "/usr/bin/gpg"

import os
import sys

if os.system(PATH_TO_GPG + ' --verify ' + sys.argv[1]) != 0:
    sys.stderr.write(sys.argv[1] + ' does not verify!')
