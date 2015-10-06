#! /usr/bin/env python
VERSION='0.0.1'
APPNAME='voxelyze_viewer'

import copy
import os, sys

def options(opt):
        opt.load('compiler_cxx')

def configure(conf):
        conf.load('compiler_cxx')

def set_options(opt):
    opt.tool_options('compiler_cxx')
    
def configure(conf):
    # log configure options
    fname = 'build/configure.options'
    args = open(fname, 'a')
    for i in sys.argv:
        args.write(i + ' ')
    args.write("\n")
    args.close()

    conf.load('compiler_cxx')
    conf.env['LIB_OSG'] = [ 'osg', 'osgDB', 'osgUtil', 'osgGA', #GLU GL
                           'osgViewer', 'OpenThreads', 
                            'osgFX', 'osgShadow']

def build(bld):
      bld.program(source='src/vx_viewer.cpp', target='app', use='OSG') 
