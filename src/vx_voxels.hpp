#ifndef VX_VOXELS_HPP_
#define VX_VOXELS_HPP_

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include "Voxelyze.h"
#include "VX_MeshRender.h"

namespace vx {
    class Voxels : public osg::Drawable {
    public:
        static constexpr float voxel_size = 0.005;
        // load a json file
        Voxels(const char* file);

        // we need to set up the bounding box of the data too, so that the scene graph
        // knows where this
        // objects is, for both positioning the camera at start up, and most
        // importantly for culling.
        virtual osg::BoundingBox computeBoundingBox() const
        {
            double x = voxel_size * 10;
            osg::BoundingBox bbox(-x, -x, -x, x, x, x);
            return bbox;
        }
        void update();
        CVoxelyze* get_voxelyze() { return &_voxelyze; }
        CVX_MeshRender* get_renderer() { return &_renderer; }

    protected:
        virtual ~Voxels() {}
        CVoxelyze _voxelyze;
        CVX_MeshRender _renderer;
        double _t;
    };
}
#endif
