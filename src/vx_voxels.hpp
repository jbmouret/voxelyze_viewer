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
		// load a json file
		Voxels(const char* file) : _voxelyze(file), _renderer(&_voxelyze){
			this->setUseDisplayList(false);
			_renderer.generateMesh();
			std::cout << "JSON loaded" << std::endl
							  << "\tmaterials: " << _voxelyze.materialCount () << std::endl
								<< "\tvoxels: "<< _voxelyze.voxelCount()
						  	<< std::endl;
		}

		// a default constructor "just to test"
		Voxels() : _voxelyze(voxel_size), _renderer(&_voxelyze), _t(0) {
			_voxelyze.enableFloor(true);
			CVX_Material* pMaterial = _voxelyze.addMaterial(1e7, 1e6); //A material with stiffness E=1MPa and density 1e6Kg/m^3
			pMaterial->setColor(0, 128, 0, 255);

			CVX_Material* pMaterial2 = _voxelyze.addMaterial(1e7, 1e6); //A material with stiffness E=1MPa and density 1000Kg/m^3
			pMaterial2->setColor(256, 0, 0, 255);

			pMaterial->setPoissonsRatio(0.35);
			pMaterial2->setPoissonsRatio(0.35);

			pMaterial->setCollisionDamping (0.5);
			pMaterial2->setCollisionDamping (0.5);

			pMaterial->setStaticFriction (1000.0);
			pMaterial2->setStaticFriction (1000.0);
			pMaterial->setKineticFriction (0.5);
			pMaterial2->setKineticFriction (0.5);

			pMaterial->setModelBilinear (1e+007, 0, 0, 0);
			pMaterial2->setModelBilinear (1e+007, 0, 0, 0);




			std::cout<<"static friction:"<<pMaterial2->staticFriction() << std::endl;
			std::cout<<"dyn friction:"<<pMaterial2->kineticFriction() << std::endl;

			_m1 = pMaterial2;
			_m2 = pMaterial;

			CVX_Voxel* Voxel1 = _voxelyze.setVoxel(pMaterial, 0, 0, 0); //Voxel at index x=0, y=0. z=0
			CVX_Voxel* Voxel2 = _voxelyze.setVoxel(pMaterial, 1, 0, 0);
			CVX_Voxel* Voxel3 = _voxelyze.setVoxel(pMaterial2, 1, 0, 1); //Beam extends in the +X direction
			CVX_Voxel* Voxel4 = _voxelyze.setVoxel(pMaterial2, 2, 0, 1); //Beam extends in the +X direction
			CVX_Voxel* Voxel5 = _voxelyze.setVoxel(pMaterial, 3, 0, 1); //Beam extends in the +X direction

			_voxelyze.setGravity(3);
			_voxelyze.saveJSON("test.json");
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
		void update(){
			_voxelyze.doTimeStep();
			double s = cos(_t) / 10.0 + 1.0;
			double s2 = sin(_t) / 10.0 + 1.0;

		//	_m1->setExternalScaleFactor(Vec3D<double>(s, s, s));
		//	_m2->setExternalScaleFactor(Vec3D<double>(s2, s2, s2));

			_t += 0.05;

		}
		CVoxelyze* get_voxelyze() { return &_voxelyze; }
		CVX_MeshRender* get_renderer() { return &_renderer; }
	protected:
		virtual ~Voxels() {}
		CVoxelyze _voxelyze;
		CVX_MeshRender _renderer;
		CVX_Material *_m1;
		CVX_Material *_m2;

		double _t;
	};
}
#endif
