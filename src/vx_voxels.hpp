#ifndef VX_VOXELS_HPP_
#define VX_VOXELS_HPP_


#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include "Voxelyze.h"
#include "VX_MeshRender.h"

namespace vx {
	class Voxels : public osg::Drawable
	{
	public:
		static constexpr float voxel_size = 0.005;
		Voxels() : _voxelyze(voxel_size), _renderer(&_voxelyze) {
			_voxelyze.enableFloor(true);
			CVX_Material* pMaterial = _voxelyze.addMaterial(1000000, 1000); //A material with stiffness E=1MPa and density 1000Kg/m^3
			pMaterial->setColor(0, 128, 0, 255);
			CVX_Material* pMaterial2 = _voxelyze.addMaterial(1000000, 1000); //A material with stiffness E=1MPa and density 1000Kg/m^3
			pMaterial2->setColor(256, 0, 0, 255);

			CVX_Voxel* Voxel1 = _voxelyze.setVoxel(pMaterial, 0, 0, 1); //Voxel at index x=0, y=0. z=0
			CVX_Voxel* Voxel2 = _voxelyze.setVoxel(pMaterial2, 1, 0, 1);
			CVX_Voxel* Voxel3 = _voxelyze.setVoxel(pMaterial, 2, 0, 1); //Beam extends in the +X direction

			_voxelyze.setGravity(1.0);

			this->setUseDisplayList(false);
			_renderer.generateMesh();
		}

		// we need to set up the bounding box of the data too, so that the scene graph knows where this
		// objects is, for both positioning the camera at start up, and most importantly for culling.
		virtual osg::BoundingBox computeBoundingBox() const
		{
			double x = voxel_size * 10;
			osg::BoundingBox bbox(-x, -x, -x, x, x, x);
			return bbox;
		}
		CVoxelyze* get_voxelyze() { return &_voxelyze; }
		CVX_MeshRender* get_renderer() { return &_renderer; }
	protected:

		virtual ~Voxels() {}
		CVoxelyze _voxelyze;
		CVX_MeshRender _renderer;
	};
}
#endif
