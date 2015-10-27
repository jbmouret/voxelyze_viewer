#ifndef VX_VIEWER_HPP_
#define VX_VIEWER_HPP_

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <iostream>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowTexture>
#include <osgShadow/SoftShadowMap>

#include "vx_voxels.hpp"

namespace vx {
    class Viewer {
    public:
      Viewer(const char* json) : _voxels(new Voxels(json))
        {
	    _scene = _init(_voxels);
            std::cout << "scene created" << std::endl;
            _init_view();
            std::cout << "view initialized" << std::endl;
        }
        void frame() { _viewer.frame(); }
        bool done() const { return _viewer.done(); }
        float t() const { return _voxels->t(); }
        float fit() const { return _voxels->fit(); }
    protected:
        osg::ref_ptr<osg::Geode> _create_sqr(float width, float length);
        osg::ref_ptr<osg::Texture2D> _load_texture(const std::string& fname);
        osg::ref_ptr<osg::Node> _create_ground();
        osg::ref_ptr<osg::Node> _init(osg::ref_ptr<Voxels> voxels);
        osg::ref_ptr<Voxels> _voxels;
        void _init_view();
        //
        osg::ref_ptr<osg::Node> _scene;
        osgViewer::Viewer _viewer;
        
    };
}
#endif
