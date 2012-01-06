#!/usr/bin/env python
# encoding: utf-8

# LICENSE : GNU GENERAL PUBLIC LICENSE Version 2


APPNAME = 'freqt'
VERSION = '0.221'

#srcdir = 'src'
out = 'bin'

def options(opt):
    opt.tool_options('compiler_cxx')
    opt.tool_options('compiler_cc')
    opt.tool_options('daemon', tooldir='tool')
    opt.add_option( '--debug'
        , action='store_true'
        , default=False
        , help='Enable debug mode [Default: False]'
        , dest='debug'
    )
    opt.add_option( '--prof'
        , action='store_true'
        , default=False
        , help='Enable profiling mode [Default: False]'
        , dest='prof'
    )

def _setOS(conf):
    #check OS name
    import os
    if os.name == 'nt':
        conf.define('OS_WINDOWS', 1)
    elif os.name == 'posix':
        uname = os.uname()[0]
        if uname == 'Darwin':
            conf.define('OS_MACOSX', 1)
        elif uname == 'Linux':
            conf.define('OS_LINUX', 1)
        else:
            error("Unknown OS type.")
    else:
        error("Unknown OS type.")

def configure(conf):
    _setOS(conf)
    conf.check_tool('compiler_cxx')
    conf.check_tool('compiler_cc')
    _ccv = conf.env['CC_VERSION']
    if ( int(_ccv[0]) >= 4 and int(_ccv[1])>=4 ):
        conf.env["CCV0x"] = True
    else:
        print " CC_VERSION is %s (<4.4). Can't use c++0x" % ( ".".join(_ccv) )
        conf.env["CCV0x"] = False

    conf.check_tool('daemon', tooldir='tool')

    import Options
    conf.env["PROF_MODE"] = Options.options.prof
    if conf.env['PROF_MODE'] == True:
        conf.env.append_value('CPPFLAGS', '-pg')
        conf.env.append_value('LINKFLAGS', '-pg')
    conf.env["DEBUG_MODE"] = Options.options.debug

    import datetime
    d = datetime.datetime.today()
    DATETIME = d.strftime("%Y-%m-%d %H:%M:%S")
    conf.define("CONST_VERSION", VERSION)
    conf.define("CONST_COMF_DATETIME", DATETIME)

    conf.write_config_header('config.hpp')

def build(bld):
#    bld.recurse('lib')

    CONFDIR = 'bin'
    INCS  = [CONFDIR]
    if bld.env['PROF_MODE']:
        CXXFLAGS = ['-g', '-pg', '-Wall'] #,'-Wshadow']
    elif bld.env['DEBUG_MODE']:
        CXXFLAGS = ['-gstabs+', '-O0', '-Wall'] #,'-Wshadow']
    else:
        CXXFLAGS = ['-O3', '-Wall'] #,'-Wshadow']

    bld(features = 'cxx cprogram',
        source = 'freqt.cpp',
        target = 'freqt',
        includes = INCS,
        lib = [],
        cxxflags     = CXXFLAGS,
        use = [
         ],
        uselib = [
         ]
    )


def shutdown(ctx):
	pass

def dist_hook():
    import os

    if os.path.exists('.lock-wscript'):
        os.remove('.lock-wscript')

