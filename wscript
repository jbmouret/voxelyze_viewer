#! /usr/bin/env python
VERSION='0.0.1'
APPNAME='voxelyze_viewer'

import copy
import os, sys

def options(opt):
        opt.add_option('--voxelyze', action='store', default=os.getcwd() + "/Voxelyze/", help='full path to voxelyze')
        opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    conf.env['LIB_OSG'] = [ 'osg', 'osgDB', 'osgUtil',
                            'osgGA', #GLU GL
                            'osgViewer', 'OpenThreads',
                            'osgFX', 'osgShadow']
    # for OSX
    conf.env.FRAMEWORK_OSG='OpenGL'


    ### VOXELYZE
    conf.env.INCLUDES_VOXELYZE = conf.options.voxelyze + '/include'
    conf.env.LIBPATH_VOXELYZE = conf.options.voxelyze + '/lib'
    conf.env.LIB_VOXELYZE = 'voxelyze.0.9'

    print "Voxelyze include: " +  str(conf.env['INCLUDES_VOXELYZE'])
    print "Voxelyze libs: " +  conf.env['LIBPATH_VOXELYZE']

def build(bld):
      bld.program(source='src/vx_viewer.cpp', target='vx_viewer',
                  uselib='OSG VOXELYZE',
                  cxxflags = ['-std=c++11', '-fdiagnostics-color'])
#         bld.stlib(source='a.c', target='mystlib') 3
