#include "vx_voxels.hpp"

namespace vx {
    Voxels::Voxels(const char* file) : _voxelyze(file), _renderer(&_voxelyze)
    {
      _voxelyze.setGravity(2);
      _voxelyze.enableFloor(true);

        this->setUseDisplayList(false);
        _renderer.generateMesh();

        std::cout << "JSON loaded" << std::endl
                  << "\tmaterials: " << _voxelyze.materialCount() << std::endl
                  << "\tvoxels: " << _voxelyze.voxelCount() << std::endl
                  << "\tgravity:" << _voxelyze.gravity() << std::endl;
    }

    void Voxels::update()
    {
      _voxelyze.doTimeStep();//0.001);
        CVX_Material *m_plus = 0x0, *m_minus = 0x0;
        double s = sin(100*_t) * 0.15 + 1.0;
        double s2 = -sin(100*_t) * 0.15 + 1.0;
        for (size_t i = 0; i < _voxelyze.materialCount(); i++) {
            if (strcmp(_voxelyze.material(i)->name(), "active+") == 0)
                m_plus = _voxelyze.material(i);
            else if (strcmp(_voxelyze.material(i)->name(), "active-") == 0)
                m_minus = _voxelyze.material(i);
        }
        if (m_plus != 0)
            m_plus->setExternalScaleFactor(Vec3D<double>(s, s, s));
        else
            std::cerr << "Warning: active+ not found!" << std::endl;

        if (m_minus != 0)
            m_minus->setExternalScaleFactor(Vec3D<double>(s2, s2, s2));
        else
            std::cerr << "Warning: active- not found!" << std::endl;

        _t += _voxelyze.recommendedTimeStep () ;
	//	std::cout<<"_t:"<<_t<<" ";
	_fit = 0;
	for (size_t i = 0; i < _voxelyze.voxelCount(); ++i)
	  _fit += _voxelyze.voxel(i)->displacement().getX();
	_fit /= _voxelyze.voxelCount();
	//	std::cout<<_t<<" -> fit:"<<fit << " "<<s<<" "<< s2 << std::endl;
    }
}
