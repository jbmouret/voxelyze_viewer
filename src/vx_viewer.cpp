
#include "vx_viewer.hpp"
#include "vx_voxels.hpp"

using namespace osg;

namespace vx
{
	static constexpr int ReceivesShadowTraversalMask = 0x2;
	static constexpr int CastsShadowTraversalMask = 0x1;

	struct VoxelsUpdateCallback : public osg::Drawable::UpdateCallback
	{
		VoxelsUpdateCallback(Voxels* vx) : _vx(vx) {}
		virtual void update(osg::NodeVisitor *vs, osg::Drawable* drawable) {
			_vx->update();
		}
	private:
		Voxels* _vx;
	};

	struct VoxelsDrawCallback : public osg::Drawable::DrawCallback
	{
		VoxelsDrawCallback(CVX_MeshRender *renderer) : _renderer(renderer) {}
		virtual void drawImplementation(osg::RenderInfo &, const osg::Drawable *) const {
			_renderer->updateMesh();
			_renderer->glDraw();
		}
	private:
		CVX_MeshRender *_renderer;
	};


	ref_ptr<Geode> Viewer :: create_sqr(float width, float length)
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

	ref_ptr<Texture2D> Viewer::load_texture(const std::string& fname)
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


	// a big textured square
	ref_ptr<PositionAttitudeTransform> Viewer :: create_ground()
	{
		static const float ground_x = Voxels::voxel_size * 30;
		static const float ground_y = ground_x;
		static const float ground_z = 0;

		ref_ptr<Geode> geode_sqr = create_sqr(ground_x, ground_y);
		geode_sqr->setNodeMask(ReceivesShadowTraversalMask);
		ref_ptr<PositionAttitudeTransform>  pat(new PositionAttitudeTransform());
		pat->setPosition(Vec3(-0.5 * ground_x, -0.5 * ground_y, -Voxels::voxel_size / 2.0));
		pat->addChild(geode_sqr.get());
		ref_ptr<StateSet> ss_checker(new StateSet());
		ss_checker->setTextureAttributeAndModes(0, load_texture("data/checker.tga"));
		geode_sqr->setStateSet(ss_checker.get());

		return pat;

	}

	ref_ptr<Node> Viewer :: init() {
		// we need the scene's state set to enable the light for the entire scene
		ref_ptr<Group> scene = new Group();

		ref_ptr<osg::Geode> geode_voxels = new osg::Geode();
		ref_ptr<Voxels> voxels = new Voxels;
		geode_voxels->addDrawable(voxels);
		PositionAttitudeTransform *cubeTransform = new PositionAttitudeTransform();
		cubeTransform->addChild(geode_voxels);
		cubeTransform->setPosition(Vec3(0, 0, 0));
		geode_voxels->setNodeMask(CastsShadowTraversalMask);


		ref_ptr<VoxelsUpdateCallback> cb = new VoxelsUpdateCallback(voxels);
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

		// lights & shadows
		Vec3 center(0.0f, 0.0f, 0.0f);
		float radius = 500;
		Vec3 light_position(center + Vec3(0.0f, Voxels::voxel_size * 50, Voxels::voxel_size * 50));

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
}

int main() {
	vx::Viewer viewer;
	while (!viewer.done()) {
		viewer.frame();
	}
}
