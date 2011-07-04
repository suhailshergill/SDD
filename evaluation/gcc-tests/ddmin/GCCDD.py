#!/s/python-2.6.2/bin/python
# -*- python -*-

###############################################################################
from __future__ import with_statement


# $Id: GCCDD.py,v 1.1 2001/11/05 19:53:33 zeller Exp $
# Using delta debugging on GCC input

import DD
import commands
import string


import optparse
import sys

numberOfUnresolvedTests = 0

class MyDD(DD.DD):
    def __init__(self):
        DD.DD.__init__(self)
        self.fileName = "input.c"
        self.commandName = "/s/gcc-3.4.4/bin/gcc -c -O3"
        
    def _test(self, deltas):
        # Build input
        input = ""
        for (index, character) in deltas:
            input = input + character

        # Write input to `input.c'
        out = open(self.fileName, 'w')
        out.write(input)
        out.close()

        print self.coerce(deltas)

        # Invoke GCC
        (status, output) = commands.getstatusoutput(
            "%s %s 2>&1" % (self.commandName, self.fileName))

        print output
        print "Exit code", status

        # Determine outcome
        if status == 0 and output.find("warning") <0:
            return self.PASS
        elif output.find("warning") >=0:
            global numberOfUnresolvedTests
            numberOfUnresolvedTests += 1
            return self.UNRESOLVED
        elif output.find("internal compiler error") >= 0:
            return self.FAIL
        global numberOfUnresolvedTests
        numberOfUnresolvedTests += 1
        return self.UNRESOLVED

    def coerce(self, deltas):
        # Pretty-print the configuration
        input = ""
        for (index, character) in deltas:
            input = input + character
        return input


def invokeDDmin(fileName):
    """main worker
    """
    # Load deltas from `bug.c'
    deltas = []
    index = 1
    for character in open(fileName).read():
        deltas.append((index, character))
        index = index + 1

    mydd = MyDD()
    
    print "Simplifying failure-inducing input..."
    c = mydd.ddmin(deltas)              # Invoke DDMIN
    print "The 1-minimal failure-inducing input is", mydd.coerce(c)
    with open(fileName + '_min', 'w') as ofile:
        ofile.write(mydd.coerce(c))
    print "Removing any element will make the failure go away."

    global numberOfUnresolvedTests
    print "NUMBEROFUNRESOLVEDTESTS", numberOfUnresolvedTests 



def main(argv=None):
    """do stuff
    """
    if argv is None:
        argv = sys.argv

    # import ipdb; ipdb.set_trace()

    parser = optparse.OptionParser(usage='%prog [options] <fileName>')
    parser.add_option('-v', '--version', action='store', default=1, type='int',
                      help = 'option placeholder')

    options, args = parser.parse_args(argv[1:])
    if len(args) !=1:
        parser.error('wrong number of positional arguments')

    invokeDDmin(args[0])



if __name__ == '__main__':
    main()

    # print
    
    # print "Isolating the failure-inducing difference..."
    # (c, c1, c2) = mydd.dd(deltas)	# Invoke DD
    # print "The 1-minimal failure-inducing difference is", c
    # print mydd.coerce(c1), "passes,", mydd.coerce(c2), "fails"



# Local Variables:
# mode: python
# End:
