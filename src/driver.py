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
import timeit

from split import *
from listsets import *

prolog = Prolog()
################################################################################
sources = ['load.pl']

currentMinimalFileName = 'alpha.c'
tentativeMinimalFileName = 'beta.c'

constraintGenerator = "../bin/GenerateConstraints"
commandName = "/s/gcc-3.4.4/bin/gcc -c -O3"

numberOfUnresolvedTests = 0
numberOfTotalTests = 0
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
    # print "QR:", L
    return None if len(L)==0 else L[0]


def isVariableNone(v):
    return v == None or v == '' or v == '[]' or v == []


def runTest(commandName, fileName, logTest=True):
    # Invoke GCC
    (status, output) = commands.getstatusoutput(
        "%s %s 2>&1" % (commandName, fileName))

    # print output
    # print "Exit code", status

    # Determine outcome
    global numberOfUnresolvedTests
    global numberOfTotalTests
    numberOfTotalTests += 1
    if status == 0 and output.find("warning") <0:
        return 'PASS'
    elif output.find("warning") >=0:
        if logTest:
            numberOfUnresolvedTests += 1
        return 'UNRESOLVED'
    elif output.find("internal compiler error") >= 0:
        return 'FAIL'
    if logTest:
        numberOfUnresolvedTests += 1
    return 'UNRESOLVED'


def applyChanges(fileName, actionList):
    with open(fileName, 'r+') as fileHandle:
        for action in actionList:
            seekPos, replacement = getReplacementTuple(action)
            fileHandle.seek(seekPos)
            fileHandle.write(replacement)


def markNodes(result, node):
    command = None
    if result == 'FAIL':
        # print "NONESSENTIAL:", node
        command = "permanentlyDelete(%s)" % node
    elif result == 'PASS':
        # print "ESSENTIAL:", node
        command = "recursivelyMarkEssential(%s)" % node
    else:
        # print "UNRESOLVED:", node
        # import ipdb; ipdb.set_trace()
        command = "assertHasUntrackedDependency(%s)" % node
    return getQueryResult(command)

# def markNodeList(result, nodeList):
#     for node in nodeList:
#         command = None
#         if result == 'FAIL':
#             print "NONESSENTIAL:", node
#             command = "permanentlyDeleteList(%s)" % node
#         elif result == 'PASS':
#             print "ESSENTIAL:", node
#             command = "recursivelyMarkEssential(%s)" % node
#         else:
#             print "UNRESOLVED:", node
#         # import ipdb; ipdb.set_trace()
#             command = "assertHasUntrackedDependency(%s)" % node
#         getQueryResult(command)

        


def removeNodeTransitively(fileName, symbolToRemove):
    # print "TRYING:", symbolToRemove
    QR = getQueryResult("recursivelyComputeDeletionAction(%s, L1, L2)" %
                        symbolToRemove)
    currentDeletionSet = None
    if not(QR is None or isVariableNone(QR['L1'])):
        currentDeletionSet = map(getValueFromAtom, QR['L1'])
        applyChanges(fileName, QR['L2'])
        getQueryResult("delete(%s)" % symbolToRemove)
    return currentDeletionSet

def removeNodeList(fileName, symbols):
    # import ipdb; ipdb.set_trace()
    symbolsStr = "[%s]" % ', '.join(symbols)
    QR = getQueryResult("computeDeletionActionForList(%s, L)" %
                        symbolsStr)
    applyChanges(fileName, QR['L'])

# def recursivelyDescend2(symbolRemoved, currentDeletionSet, result):
#     testActuallyRun = True
#     deletionSet = None
#     symbolToRemove = None

#     while (result != 'FAIL'):
#         if testActuallyRun:
#             copy(currentMinimalFileName, tentativeMinimalFileName)

#         markNodes(result, symbolRemoved)
#         # if symbolRemoved == 'sym2':
#         #     import ipdb; ipdb.set_trace()
        
#         QR = getQueryResult("topScoringRemovableDeletedWUD(X)")
#         if QR is None or isVariableNone(QR['X']):
#             getQueryResult("undoAllDelete(L)")
#             break
#         symbolToRemove = getValueFromAtom(QR['X'])
#         # FIXME: for certain cases pyswip returns this, not really sure why. i
#         # *think* it is for unbound results, but not sure
#         if not isinstance(symbolToRemove, str):
#             testActuallyRun = False
#             break

#         QR = getQueryResult("transitiveRemovalList(%s, L)" %
#                             symbolToRemove)

#         if QR is None or isVariableNone(QR['L']):            
#             testActuallyRun = False
#             break
#             # print "ERROR!!!"
#             # import ipdb; ipdb.set_trace()
#         else:
#             deletionSet = map(getValueFromAtom, QR['L'])
            
#         if set(deletionSet) == set(currentDeletionSet):
#             testActuallyRun = False
#             continue
#         else:
#             getQueryResult("undoDelete(%s)" % symbolRemoved)
#             symbolRemoved = symbolToRemove
#             currentDeletionSet = removeNodeTransitively(tentativeMinimalFileName, symbolToRemove)

#             result = runTest(commandName, tentativeMinimalFileName)
#             testActuallyRun = True
#             continue
            
#     if result == 'FAIL':
#         markNodes(result, symbolRemoved)
#         copy(tentativeMinimalFileName, currentMinimalFileName)
    
        
## first ad-hoc version
# def recursivelyDescend(symbolRemoved, currentDeletionSet, result):
#     while (result != 'FAIL'):
#         # if symbolRemoved == 'sym13':
#         #     import ipdb; ipdb.set_trace()
#         copy(currentMinimalFileName, tentativeMinimalFileName)
#         getQueryResult("undoDelete(%s)" % symbolRemoved)

#         if result == 'PASS':
#             getQueryResult("recursivelyMarkEssential(%s)" % symbolRemoved)
#         else:
#             getQueryResult("assertHasUntrackedDependency(%s)" % symbolRemoved)
        
#         deletionSet = None
#         QR = getQueryResult("transitiveRemovalListWUDSorted(%s, L)" %
#                             symbolRemoved)
#         if QR is None or isVariableNone(QR['L']):            
#             deletionSet = currentDeletionSet
#         else:
#             deletionSet = map(getValueFromAtom, QR['L'])
                
#         symbolRemoved = None

#         newSymIndex = int(math.ceil(len(deletionSet)/2))
#         if len(deletionSet) == 1:
#             symbolRemoved = None
#             result = runTest(commandName, tentativeMinimalFileName)
#             break
#         else:
#             symbolRemoved = deletionSet[newSymIndex]
#             QR = getQueryResult("transitiveRemovalListWUDSorted(%s, L)" %
#                                 symbolRemoved)
#             if QR is None or isVariableNone(QR['L']):
#                 symbolRemoved = None
#                 result = runTest(commandName, tentativeMinimalFileName)
#                 break
#             else:
#                 newDeletionSet = map(getValueFromAtom, QR['L'])
#             if set(newDeletionSet) == set(currentDeletionSet):
#                 continue
                
#             print "TRYING::", symbolRemoved
#             QR = getQueryResult("recursivelyComputeDeletionAction(%s, L1, L2)" %
#                                 symbolRemoved)
#             applyChanges(tentativeMinimalFileName, QR['L2'])
#             getQueryResult("delete(%s)" % symbolRemoved)

#             result = runTest(commandName, tentativeMinimalFileName)
            
#     if result == 'FAIL':
#         if symbolRemoved:
#             print "HAHA:", symbolRemoved
#             getQueryResult("permanentlyDelete(%s)" % symbolToRemove)
#         copy(tentativeMinimalFileName, currentMinimalFileName)



def invokeSDD(testFile, topPreferred= False, ddmin=False):
    # import ipdb; ipdb.set_trace()

    global numberOfUnresolvedTests
    global numberOfTotalTests
    numberOfUnresolvedTests = 0
    numberOfTotalTests = 0

    getQueryResult("clearAllLabels(L)")
    copy(testFile, currentMinimalFileName)

    searchHeuristic = None
    if topPreferred:
        searchHeuristic = "topScoringRemovableWUD(X)"
    else:
        searchHeuristic = "topScoringRemovableWUD2(X)"

    if ddmin:
        getQueryResult("markAllUntrackedDependencies(L)")

    while (getQueryResult("allRemovableWUD(L)")):
        copy(currentMinimalFileName, tentativeMinimalFileName)

        QR = getQueryResult(searchHeuristic)
        if QR is None or isVariableNone(QR['X']):
            break
        symbolToRemove = getValueFromAtom(QR['X'])
        # if symbolToRemove == 'sym0':
        #     import ipdb; ipdb.set_trace()

        currentDeletionSet = removeNodeTransitively(tentativeMinimalFileName,
                                                    symbolToRemove)
        result = runTest(commandName, tentativeMinimalFileName)
        markNodes(result, symbolToRemove)

        # import ipdb; ipdb.set_trace()
        if result == 'FAIL':
            copy(tentativeMinimalFileName, currentMinimalFileName)
        # else:
            # recursivelyDescend2(symbolToRemove, currentDeletionSet, result)

    # Now run ddmin on nodes with untracked dependencies
    # QR = getQueryResult("allUntrackedDependencies(L)")
    QR = getQueryResult("allNotPermanentlyDeleted(L)")
    n = 2
    L = []
    # import ipdb; ipdb.set_trace()
    if not(QR is None or isVariableNone(QR['L'])):
        L = map(getValueFromAtom, QR['L'])

    print L
    # if not ddmin:
    #     n = len(L)
    copy(currentMinimalFileName, tentativeMinimalFileName)
    while len(L) >= 2:
        # print L
        subsets = split(L, n)
        
        some_complement_is_failing = False
        for subset in subsets:
            complement = listminus(L, subset)
            removeNodeList(tentativeMinimalFileName, subset)
            result = runTest(commandName, tentativeMinimalFileName)
            if result == 'FAIL':
                copy(tentativeMinimalFileName, currentMinimalFileName)
                L = complement
                n = max(n-1, 2)
                some_complement_is_failing = True
                break
            else:
                copy(currentMinimalFileName, tentativeMinimalFileName)

        if not some_complement_is_failing:
            if n == len(L):
                break
            n = min(n * 2, len(L))
        
    # FIXME:HACK
    # import ipdb; ipdb.set_trace()
    print "NUMBEROFUNRESOLVEDTESTS", numberOfUnresolvedTests
    print "TOTALTESTS", numberOfTotalTests
    print L
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
    parser.add_option('-d', '--ddmin', action='store_true', default=False,
                      help = 'run vanilla ddmin')
    parser.add_option('-t', '--topPreferred', action='store_true', default=False,
                      help = 'give higher preference to top-level constructs')


    options, args = parser.parse_args(argv[1:])
    if len(args) !=1:
        parser.error('wrong number of positional arguments')

    testFile = args[0]
    if not (os.path.exists(testFile) and os.path.isfile(testFile)):
        parser.error('make sure input file "%s" exists' % testFile)

    result = runTest(commandName, testFile, False)
    if result != 'FAIL':
        return
        
    # stderr = None
    # if not options.verbose:
    #     stderr = open('/dev/null')

    s = 'call(["%s", "-plugin", "gen-constraints","%s"],stderr=open("/dev/null"))' % (constraintGenerator, testFile)
    setup = "from subprocess import call"
    t = timeit.Timer(stmt=s, setup=setup)
    print "CONSTRAINT GENERATION: %s" % str(t.timeit(5)/5)

    for item in sources:
        prolog.consult(item)

    # import ipdb; ipdb.set_trace()
    s = 'invokeSDD("%s", %s, %s)' % (testFile, str(options.topPreferred), str(options.ddmin))
    setup = "from __main__ import invokeSDD"
    t = timeit.Timer(stmt=s, setup=setup)
    # invokeSDD(testFile, options.topPreferred, options.ddmin)
    print "TIME TAKEN: %s" % str(t.timeit(5)/5)

###############################################################################
if __name__ == '__main__':
    sys.exit(main())
