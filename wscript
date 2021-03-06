#!/usr/bin/env python
# encoding: utf-8

import Options

VERSION='0.1'
APPNAME='constraint-generator'

srcdir='.'
blddir='.build'

def init(): pass

def set_options(opt):
    opt.tool_options('compiler_cxx compiler_cc')
    opt.add_option('--llvm-prefix', action='store', dest='llvmprefix',
                   help='The prefix for an installation of llvm+clang')

def configure(conf):
    conf.check_tool('compiler_cxx compiler_cc')
    conf.check_tool('tex')

    llvm_inc = '%s/include' % Options.options.llvmprefix
    llvm_lib = '%s/lib' % Options.options.llvmprefix

    conf.env['CXXFLAGS'] = [ '-Wall', '-O0', '-g', '-D__STDC_LIMIT_MACROS', '-D__STDC_CONSTANT_MACROS', '-fno-rtti']
    conf.env.append_value('LIBPATH', llvm_lib)
    conf.env.append_value('CPPPATH', llvm_inc)
    conf.env['LLVMLIBDIR'] = llvm_lib

def build(bld):
    #bld.recurse('doc')
    cgen = bld.new_task_gen(
        features = 'cxx cprogram',
        source = [ 'src/frontend/Driver.cpp',
                   'src/frontend/GenerateConstraints.cpp',
                   'src/frontend/RealSourceRanges.cpp',
                   ],
        rpath = bld.get_env()['LLVMLIBDIR'],
        target = 'GenerateConstraints',
        libs = [
            'clangFrontendTool',
            'clangFrontend',
            'clangDriver',
            'clangSerialization',
            'clangCodeGen',
            'clangParse',
            'clangSema',
            'clangStaticAnalyzerFrontend',
            'clangStaticAnalyzerCheckers',
            'clangStaticAnalyzerCore',
            'clangAnalysis',
            'clangIndex',
            'clangRewrite',
            'clangAST',
            'clangLex',
            'clangBasic',
            'LLVM-2.9',
            'pthread',
            'dl'
            ],
        install_path = '${PREFIX}/bin')
