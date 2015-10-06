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

namespace vx {
	class Viewer {
  public:
    Viewer() {
      _scene = init();
      _viewer.setSceneData(_scene);
      _viewer.setCameraManipulator(new osgGA::TrackballManipulator());
      _viewer.realize();
    }
    void frame() {
      _viewer.frame();
    }
    bool done() const {
      return _viewer.done();
    }
	protected:
		osg::ref_ptr<osg::Geode> create_sqr(float width, float length);
		osg::ref_ptr<osg::Texture2D> load_texture(const std::string& fname);
    osg::ref_ptr<osg::PositionAttitudeTransform>  create_ground();
		osg::ref_ptr<osg::Node> init();
    //
    osg::ref_ptr<osg::Node> _scene;
    osgViewer::Viewer _viewer;

	};
}
#endif
