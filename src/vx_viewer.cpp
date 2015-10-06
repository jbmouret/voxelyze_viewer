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

#include "Voxelyze.h"
#include "VX_MeshRender.h"

using namespace osg;

struct VoxelsUpdateCallback : public osg::Drawable::UpdateCallback
{
	VoxelsUpdateCallback(CVoxelyze* vx) : _vx(vx) {}
	virtual void update(osg::NodeVisitor *vs, osg::Drawable* drawable) {
		_vx->doTimeStep();
	}
private:
	CVoxelyze* _vx;
};

struct VoxelsDrawCallback : public osg::Drawable::DrawCallback
{
	VoxelsDrawCallback(CVX_MeshRender *renderer) : _renderer(renderer) {}
	virtual void drawImplementation(osg::RenderInfo &, const osg::Drawable *) const {
		//_renderer->generateMesh();
 		_renderer->updateMesh();
		_renderer->glDraw();
	}
private:
	CVX_MeshRender *_renderer;
};



// Now the OSG wrapper for the above OpenGL code, the most complicated bit is computing
// the bounding box for the above example, normally you'll find this the easy bit.
class Voxels : public osg::Drawable
{
    public:
        Voxels() : _voxelyze(0.05), _renderer(&_voxelyze) {
					_voxelyze.enableFloor(true);
					CVX_Material* pMaterial = _voxelyze.addMaterial(1000000, 1000); //A material with stiffness E=1MPa and density 1000Kg/m^3
					pMaterial->setColor(0, 128, 0, 255);
					CVX_Material* pMaterial2 = _voxelyze.addMaterial(1000000, 1000); //A material with stiffness E=1MPa and density 1000Kg/m^3
					pMaterial2->setColor(256, 0, 0, 255);

					CVX_Voxel* Voxel1 = _voxelyze.setVoxel(pMaterial, 0, 0, 1); //Voxel at index x=0, y=0. z=0
					CVX_Voxel* Voxel2 = _voxelyze.setVoxel(pMaterial2, 1, 0, 1);
					CVX_Voxel* Voxel3 = _voxelyze.setVoxel(pMaterial, 2, 0, 1); //Beam extends in the +X direction

					//Voxel1->external()->setFixedAll(); //Fixes all 6 degrees of freedom with an external condition on Voxel 1
				//	Voxel3->external()->setForce(0, 0, 0); //pulls Voxel 3 downward with 1 Newton of force.
					_voxelyze.setGravity(1.0);

					this->setUseDisplayList(false);
					_renderer.generateMesh();
				} //5mm voxels{}

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        Voxels(const Voxels& Voxels,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            osg::Drawable(Voxels,copyop), _voxelyze(0.05), _renderer(&_voxelyze) {}

        META_Object(myVoxelsApp, Voxels)

        // we need to set up the bounding box of the data too, so that the scene graph knows where this
        // objects is, for both positioning the camera at start up, and most importantly for culling.
        virtual osg::BoundingBox computeBoundingBox() const
        {
            // value_type xmin, value_type ymin, value_type zmin, value_type xmax, value_type ymax, value_type zmax)
            osg::BoundingBox bbox(-0.1, -0.1, -0.1, 0.1, 0.1, 0.1);
            return bbox;
        }
				CVoxelyze* get_voxelyze() { return &_voxelyze; }
				CVX_MeshRender* get_renderer() { return &_renderer; }
    protected:

        virtual ~Voxels() {}
				CVoxelyze _voxelyze;
				CVX_MeshRender _renderer;
};


ref_ptr<Geode> create_sqr(float width, float length)
{
	ref_ptr<Geometry> sqr(new Geometry);
	ref_ptr<Geode> geode_sqr(new Geode);
	ref_ptr<Vec3Array> sqr_v(new Vec3Array);

	geode_sqr->addDrawable(sqr.get());
	float x = width;
	float y = length;
	sqr_v->push_back(Vec3(0, 0, 0));
	sqr_v->push_back(Vec3(0, y, 0));
	sqr_v->push_back(Vec3(x, y, 0));
	sqr_v->push_back(Vec3(x, 0, 0));
	sqr->setVertexArray(sqr_v.get());

	ref_ptr<DrawElementsUInt>
	quad(new DrawElementsUInt(PrimitiveSet::QUADS, 0));
	quad->push_back(3);
	quad->push_back(2);
	quad->push_back(1);
	quad->push_back(0);

	sqr->addPrimitiveSet(quad.get());

	ref_ptr<Vec2Array> texcoords(new Vec2Array(4));
	float rep = 1;
	(*texcoords)[0].set(0.0f, 0.0f);
	(*texcoords)[1].set(0.0f, rep);
	(*texcoords)[2].set(rep, rep);
	(*texcoords)[3].set(rep, 0.0f);
	sqr->setTexCoordArray(0, texcoords.get());

	ref_ptr<Vec3Array> normals(new Vec3Array);
	normals->push_back(osg::Vec3(0, 0, 1));
	sqr->setNormalArray(normals.get());
	sqr->setNormalBinding(Geometry::BIND_OVERALL);

	return geode_sqr;
}

ref_ptr<Texture2D> load_texture(const std::string& fname)
{
	ref_ptr<Texture2D> texture = new Texture2D();
	ref_ptr<Image> image = osgDB::readImageFile(fname);
	assert(image);
	texture->setImage(image);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	texture->setDataVariance(osg::Object::DYNAMIC);
	return texture;
}

const int ReceivesShadowTraversalMask = 0x2;
const int CastsShadowTraversalMask = 0x1;

// a big textured square
ref_ptr<PositionAttitudeTransform>  create_ground()
{
	static const float ground_x = 1;
	static const float ground_y = ground_x;
	static const float ground_z = 0;

	ref_ptr<Geode> geode_sqr = create_sqr(ground_x, ground_y);
	geode_sqr->setNodeMask(ReceivesShadowTraversalMask);
	ref_ptr<PositionAttitudeTransform>  pat(new PositionAttitudeTransform());
	pat->setPosition(Vec3(-0.5 * ground_x, -0.5 * ground_y, -0.025));
	pat->addChild(geode_sqr.get());
	ref_ptr<StateSet> ss_checker(new StateSet());
	ss_checker->setTextureAttributeAndModes(0, load_texture("data/checker.tga"));
	geode_sqr->setStateSet(ss_checker.get());

	return pat;

}

ref_ptr<Node> startup() {
	// we need the scene's state set to enable the light for the entire scene
	ref_ptr<Group> scene = new Group();

	ref_ptr<osg::Geode> geode_voxels = new osg::Geode();
	ref_ptr<Voxels> voxels = new Voxels;
	geode_voxels->addDrawable(voxels);
	PositionAttitudeTransform *cubeTransform = new PositionAttitudeTransform();
	cubeTransform->addChild(geode_voxels);
	cubeTransform->setPosition(Vec3(0, 0, 0));
	geode_voxels->setNodeMask(CastsShadowTraversalMask);


	ref_ptr<VoxelsUpdateCallback> cb = new VoxelsUpdateCallback(voxels->get_voxelyze());
	voxels->setUpdateCallback(cb);
	ref_ptr<VoxelsDrawCallback> cb2 = new VoxelsDrawCallback(voxels->get_renderer());
	voxels->setDrawCallback(cb2);

	ref_ptr<PositionAttitudeTransform> ground = create_ground();
	scene->addChild(ground);

	// create white material
	ref_ptr<Material> material = new Material();
	material->setDiffuse(Material::FRONT,  Vec4(1.0, 1.0, 1.0, 1.0));
	material->setSpecular(Material::FRONT, Vec4(0.0, 0.0, 0.0, 1.0));
	material->setAmbient(Material::FRONT,  Vec4(0.1, 0.1, 0.1, 1.0));
	material->setEmission(Material::FRONT, Vec4(0.0, 0.0, 0.0, 1.0));
	material->setShininess(Material::FRONT, 25.0);

	// assign the material to the sphere and cube
	//sphere->getOrCreateStateSet()->setAttribute(material);
	ground->getOrCreateStateSet()->setAttribute(material);
	scene->addChild(cubeTransform);

	std::cout<<"starting shadows" << std::endl;

	// lights & shadows
	Vec3 center(0.0f, 0.0f, 0.0f);
	float radius = 500;
	Vec3 light_position(center + Vec3(0.0f, -0.5f, 1));

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);

	osg::ref_ptr<osgShadow::SoftShadowMap> sm = new osgShadow::SoftShadowMap;
	shadowedScene->setShadowTechnique(sm.get());

	ref_ptr<LightSource> ls = new LightSource;
	ls->getLight()->setPosition(osg::Vec4(light_position, 1));
	ls->getLight()->setAmbient(Vec4(1, 1, 1, 1.0));
	ls->getLight()->setDiffuse(Vec4(0.9, 0.9, 0.9, 1.0));

	shadowedScene->addChild(scene);
	shadowedScene->addChild(ls);

	return shadowedScene;
}


int main() {
	ref_ptr<Node> scene = startup();

	osgViewer::Viewer viewer;
	viewer.setSceneData(scene);
	viewer.setCameraManipulator(new osgGA::TrackballManipulator());
	viewer.realize();

	while (!viewer.done()) {
		viewer.frame();
	}
}
