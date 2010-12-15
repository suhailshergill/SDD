#!/s/python-2.6.2/bin/python
# -*- python -*-

from __future__ import with_statement

import optparse
import sys
import os
import os.path

from os import symlink
from shutil import copy, move
from subprocess import call
from pyswip import Prolog

import commands
import math

prolog = Prolog()
################################################################################
sources = ['load.pl']

currentMinimalFileName = 'alpha.c'
tentativeMinimalFileName = 'beta.c'

constraintGenerator = "../bin/GenerateConstraints"
commandName = "/s/gcc-3.4.4/bin/gcc -c -O3"
################################################################################
def getValueFromAtom(a):
    if isinstance(a, str):
        return a
    return a.value

def getReplacementTuple(f):
    """Used to unpack source range and replacement from pyswip functor.
    """
    beginning = f.args[0]
    end = f.args[1].args[0]
    replaceWith = getValueFromAtom(f.args[1].args[1])
    padding = ''.join([' ' for i in xrange(end - beginning - len(replaceWith))])
    return beginning, replaceWith + padding

def getQueryResult(q):
    L = []
    G = prolog.query(q)
    while True:
        try:
            item = G.next()
            L.append(item)
        except StopIteration:
            break
        except Exception:
            continue
    return None if len(L)==0 else L[0]


def isVariableNone(v):
    return v == None or v == '' or v == '[]' or v == []


def runTest(commandName, fileName):
    # Invoke GCC
    (status, output) = commands.getstatusoutput(
        "%s %s 2>&1" % (commandName, fileName))

    print output
    print "Exit code", status

    # Determine outcome
    if status == 0 and output.find("warning") <0:
        return 'PASS'
    elif output.find("warning") >=0:
        return 'UNRESOLVED'
    elif output.find("internal compiler error") >= 0:
        return 'FAIL'
    return 'UNRESOLVED'


def applyChanges(fileName, actionList):
    with open(fileName, 'r+') as fileHandle:
        for action in actionList:
            seekPos, replacement = getReplacementTuple(action)
            fileHandle.seek(seekPos)
            fileHandle.write(replacement)

def recursivelyDescend(symbolRemoved, currentDeletionSet, result):
    while (result != 'FAIL'):
        # if symbolRemoved == 'sym13':
        #     import ipdb; ipdb.set_trace()
        copy(currentMinimalFileName, tentativeMinimalFileName)
        getQueryResult("undoDelete(%s)" % symbolRemoved)

        if result == 'PASS':
            getQueryResult("recursivelyMarkEssential(%s)" % symbolRemoved)
        else:
            getQueryResult("assertHasUntrackedDependency(%s)" % symbolRemoved)
        
        deletionSet = None
        QR = getQueryResult("transitiveRemovalListWUDSorted(%s, L)" %
                            symbolRemoved)
        if QR is None or isVariableNone(QR['L']):            
            deletionSet = currentDeletionSet
        else:
            deletionSet = map(getValueFromAtom, QR['L'])
                
        symbolRemoved = None

        newSymIndex = int(math.ceil(len(deletionSet)/2))
        if len(deletionSet) == 1:
            symbolRemoved = None
            result = runTest(commandName, tentativeMinimalFileName)
            break
        else:
            symbolRemoved = deletionSet[newSymIndex]
            QR = getQueryResult("transitiveRemovalListWUDSorted(%s, L)" %
                                symbolRemoved)
            if QR is None or isVariableNone(QR['L']):
                symbolRemoved = None
                result = runTest(commandName, tentativeMinimalFileName)
                break
            else:
                newDeletionSet = map(getValueFromAtom, QR['L'])
            if set(newDeletionSet) == set(currentDeletionSet):
                continue
                
            print "TRYING::", symbolRemoved
            QR = getQueryResult("recursivelyComputeDeletionAction(%s, L1, L2)" %
                                symbolRemoved)
            applyChanges(tentativeMinimalFileName, QR['L2'])
            getQueryResult("delete(%s)" % symbolRemoved)

            result = runTest(commandName, tentativeMinimalFileName)
            
    if result == 'FAIL':
        if symbolRemoved:
            print "HAHA:", symbolRemoved
            getQueryResult("permanentlyDelete(%s)" % symbolToRemove)
        copy(tentativeMinimalFileName, currentMinimalFileName)



def invokeSDD(currentMinimalFileName):
    result = runTest(commandName, currentMinimalFileName)
    if result != 'FAIL':
        return
    # import ipdb; ipdb.set_trace()
    while (getQueryResult("allRemovableWUD(L)")):
        copy(currentMinimalFileName, tentativeMinimalFileName)

        QR = getQueryResult("topScoringRemovableWUD(X)")
        if QR is None or isVariableNone(QR['X']):
            break
        symbolToRemove = getValueFromAtom(QR['X'])
        print "TRYING:", symbolToRemove
        # if symbolToRemove == 'sym51':
        #     import ipdb; ipdb.set_trace()
        QR = getQueryResult("transitiveRemovalListSorted(%s, L)" %
                            symbolToRemove)
        currentDeletionSet = map(getValueFromAtom, QR['L'])
        QR = getQueryResult("recursivelyComputeDeletionAction(%s, L1, L2)" %
                            symbolToRemove)
        applyChanges(tentativeMinimalFileName, QR['L2'])
        getQueryResult("delete(%s)" % symbolToRemove)

        result = runTest(commandName, tentativeMinimalFileName)
        # import ipdb; ipdb.set_trace()
        if result == 'FAIL':
            print "HAHA:", symbolToRemove 
            getQueryResult("permanentlyDelete(%s)" % symbolToRemove)
            copy(tentativeMinimalFileName, currentMinimalFileName)
        else:
            recursivelyDescend(symbolToRemove, currentDeletionSet, result)
        
    # FIXME:HACK
    # import ipdb; ipdb.set_trace()
    move(currentMinimalFileName, tentativeMinimalFileName)
    with open(tentativeMinimalFileName) as ifile:
        with open(currentMinimalFileName, 'w') as ofile:
            for line in ifile:
                lineStrip = line.strip()
                if lineStrip == '' or lineStrip == ';':
                    continue
                ofile.write(line)


def main(argv=None):
    """ Do Something.
    """

    if argv is None:
        argv = sys.argv

    parser = optparse.OptionParser(usage='%prog [options] fileName')
    parser.add_option('-v', '--verbose', action='store_true', default=False,
                      help = 'turn on debugging messages')

    options, args = parser.parse_args(argv[1:])
    if len(args) !=1:
        parser.error('wrong number of positional arguments')

    testFile = args[0]
    if not (os.path.exists(testFile) and os.path.isfile(testFile)):
        parser.error('make sure input file "%s" exists' % testFile)

    stderr = None
    if not options.verbose:
        stderr = open('/dev/null')
    call([constraintGenerator, "-plugin", "gen-constraints", testFile],
         stderr=stderr)

    for item in sources:
        prolog.consult(item)

    # import ipdb; ipdb.set_trace()
    copy(testFile, currentMinimalFileName)
    invokeSDD(currentMinimalFileName)

###############################################################################
if __name__ == '__main__':
    sys.exit(main())
