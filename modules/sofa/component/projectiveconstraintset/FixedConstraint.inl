/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*                (c) 2006-2011 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_PROJECTIVECONSTRAINTSET_FIXEDCONSTRAINT_INL
#define SOFA_COMPONENT_PROJECTIVECONSTRAINTSET_FIXEDCONSTRAINT_INL

#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/core/behavior/ProjectiveConstraintSet.inl>
#include <sofa/component/projectiveconstraintset/FixedConstraint.h>
#include <sofa/component/linearsolver/SparseMatrix.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/common/Simulation.h>
#include <sofa/helper/gl/template.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <iostream>
using std::cerr;
using std::endl;
#include <sofa/component/topology/TopologySubsetData.inl>


#include <sofa/helper/gl/BasicShapes.h>




namespace sofa
{

namespace component
{

namespace projectiveconstraintset
{

using namespace core::topology;

using namespace sofa::defaulttype;
using namespace sofa::helper;
using namespace sofa::core::behavior;


// Define TestNewPointFunction
template< class DataTypes>
bool FixedConstraint<DataTypes>::FCPointHandler::applyTestCreateFunction(unsigned int, const sofa::helper::vector<unsigned int> &, const sofa::helper::vector<double> &)
{
    if (fc)
    {
        return false;
    }
    else
    {
        return false;
    }
}

// Define RemovalFunction
template< class DataTypes>
void FixedConstraint<DataTypes>::FCPointHandler::applyDestroyFunction(unsigned int pointIndex, value_type &)
{
    if (fc)
    {
        fc->removeConstraint((unsigned int) pointIndex);
    }
}

template <class DataTypes>
FixedConstraint<DataTypes>::FixedConstraint()
    : core::behavior::ProjectiveConstraintSet<DataTypes>(NULL)
    , f_indices( initData(&f_indices,"indices","Indices of the fixed points") )
    , f_fixAll( initData(&f_fixAll,false,"fixAll","filter all the DOF to implement a fixed object") )
    , _drawSize( initData(&_drawSize,0.0,"drawSize","0 -> point based rendering, >0 -> radius of spheres") )
{
    // default to indice 0
    f_indices.beginEdit()->push_back(0);
    f_indices.endEdit();

    pointHandler = new FCPointHandler(this, &f_indices);
}


template <class DataTypes>
FixedConstraint<DataTypes>::~FixedConstraint()
{
    if (pointHandler)
        delete pointHandler;
}

template <class DataTypes>
void FixedConstraint<DataTypes>::clearConstraints()
{
    f_indices.beginEdit()->clear();
    f_indices.endEdit();
}

template <class DataTypes>
void FixedConstraint<DataTypes>::addConstraint(unsigned int index)
{
    f_indices.beginEdit()->push_back(index);
    f_indices.endEdit();
}

template <class DataTypes>
void FixedConstraint<DataTypes>::removeConstraint(unsigned int index)
{
    removeValue(*f_indices.beginEdit(),index);
    f_indices.endEdit();
}

// -- Constraint interface


template <class DataTypes>
void FixedConstraint<DataTypes>::init()
{
    this->core::behavior::ProjectiveConstraintSet<DataTypes>::init();

    topology = this->getContext()->getMeshTopology();

    //  if (!topology)
    //    serr << "Can not find the topology." << sendl;

    // Initialize functions and parameters
    f_indices.createTopologicalEngine(topology, pointHandler);
    f_indices.registerTopologicalData();

    const SetIndexArray & indices = f_indices.getValue();

    unsigned int maxIndex=this->mstate->getSize();
    for (unsigned int i=0; i<indices.size(); ++i)
    {
        const unsigned int index=indices[i];
        if (index >= maxIndex)
        {
            serr << "Index " << index << " not valid!" << sendl;
            removeConstraint(index);
        }
    }

//  cerr<<"FixedConstraint<DataTypes>::init(), getJ = " << *getJ(0) << endl;

}

/// Update and return the jacobian. @todo update it when needed using topological engines instead of recomputing it at each call.
template <class DataTypes>
const sofa::defaulttype::BaseMatrix*  FixedConstraint<DataTypes>::getJ(const core::MechanicalParams* )
{
    unsigned numBlocks = this->mstate->getSize();
    unsigned blockSize = DataTypes::deriv_total_size;
    jacobian.resizeBloc( numBlocks*blockSize,numBlocks*blockSize );
//    cerr<<"FixedConstraint<DataTypes>::getJ, numblocs = "<< numBlocks << ", block size = " << blockSize << endl;

    for(unsigned i=0; i<numBlocks*blockSize; i++ )
    {
        jacobian.set(i,i,1);
    }
    for(unsigned i=0; i<f_indices.getValue().size(); i++ )
    {
//        cerr<<"FixedConstraint<DataTypes>::getJ, , set null block in " << f_indices.getValue()[i] << ", matrix before = " << jacobian << endl;
        for(unsigned j=0; j<blockSize; j++)
            jacobian.set( f_indices.getValue()[i]*blockSize+j, f_indices.getValue()[i]*blockSize+j, 0);
//        cerr<<"FixedConstraint<DataTypes>::getJ, , set null block in " << f_indices.getValue()[i] << ", matrix after = " << jacobian << endl;
    }

    return &jacobian;
}

///// Update and return the jacobian. @todo update it when needed using topological engines instead of recomputing it at each call.
//template <class DataTypes>
//const sofa::defaulttype::BaseMatrix*  FixedConstraint<DataTypes>::getJ(const core::MechanicalParams* )
//{
//    unsigned numBlocks = this->mstate->getSize();
//    unsigned blockSize = DataTypes::deriv_total_size;
//    jacobian.resizeBloc( numBlocks,numBlocks );
//    cerr<<"FixedConstraint<DataTypes>::getJ, numblocs = "<< numBlocks << ", block size = " << blockSize << endl;

//    Block ident, zero;
//    for( unsigned i=0; i<blockSize; i++ )
//        ident(i,i)= (SReal)1;

//    for(unsigned i=0; i<numBlocks; i++ )
//    {
//        *jacobian.wbloc(i,i,true) = ident;
//    }
//    for(unsigned i=0; i<f_indices.getValue().size(); i++ )
//    {
//        cerr<<"FixedConstraint<DataTypes>::getJ, , set null block in " << f_indices.getValue()[i] << ", matrix before = " << jacobian << endl;
//        *jacobian.wbloc( f_indices.getValue()[i],f_indices.getValue()[i], true) = zero;
//        cerr<<"FixedConstraint<DataTypes>::getJ, , set null block in " << f_indices.getValue()[i] << ", matrix after = " << jacobian << endl;
//    }

//    return &jacobian;
//}

template <class DataTypes>
void FixedConstraint<DataTypes>::projectResponse(const core::MechanicalParams* mparams /* PARAMS FIRST */, DataVecDeriv& resData)
{
//    cerr<<"FixedConstraint<DataTypes>::projectResponse is called "<<endl;
//    assert(false);
    helper::WriteAccessor<DataVecDeriv> res ( mparams, resData );
    const SetIndexArray & indices = f_indices.getValue(mparams);
    //serr<<"FixedConstraint<DataTypes>::projectResponse, dx.size()="<<res.size()<<sendl;
    if( f_fixAll.getValue(mparams) )
    {
        // fix everything
        typename VecDeriv::iterator it;
        for( it = res.begin(); it != res.end(); ++it )
        {
            *it = Deriv();
        }
    }
    else
    {
        for (SetIndexArray::const_iterator it = indices.begin();
                it != indices.end();
                ++it)
        {
            res[*it] = Deriv();
        }
    }
    //cerr<<"FixedConstraint<DataTypes>::projectResponse is called  res = "<<endl<<res<<endl;
}

template <class DataTypes>
void FixedConstraint<DataTypes>::projectJacobianMatrix(const core::MechanicalParams* mparams /* PARAMS FIRST */, DataMatrixDeriv& cData)
{
    helper::WriteAccessor<DataMatrixDeriv> c ( mparams, cData );
    const SetIndexArray & indices = f_indices.getValue(mparams);

    MatrixDerivRowIterator rowIt = c->begin();
    MatrixDerivRowIterator rowItEnd = c->end();

    if( f_fixAll.getValue(mparams) )
    {
        // fix everything
        while (rowIt != rowItEnd)
        {
            rowIt.row().clear();
            ++rowIt;
        }
    }
    else
    {
        while (rowIt != rowItEnd)
        {
            for (SetIndexArray::const_iterator it = indices.begin();
                    it != indices.end();
                    ++it)
            {
                rowIt.row().erase(*it);
            }
            ++rowIt;
        }
    }
    //cerr<<"FixedConstraint<DataTypes>::projectJacobianMatrix : helper::WriteAccessor<DataMatrixDeriv> c =  "<<endl<< c<<endl;
}

// projectVelocity applies the same changes on velocity vector as projectResponse on position vector :
// Each fixed point received a null velocity vector.
// When a new fixed point is added while its velocity vector is already null, projectVelocity is not usefull.
// But when a new fixed point is added while its velocity vector is not null, it's necessary to fix it to null. If not, the fixed point is going to drift.
template <class DataTypes>
void FixedConstraint<DataTypes>::projectVelocity(const core::MechanicalParams* /*mparams*/ /* PARAMS FIRST */, DataVecDeriv& /*vData*/)
{
#if 0 /// @TODO ADD A FLAG FOR THIS
    const SetIndexArray & indices = f_indices.getValue();
    //serr<<"FixedConstraint<DataTypes>::projectVelocity, res.size()="<<res.size()<<sendl;
    if( f_fixAll.getValue()==true )    // fix everyting
    {
        for( unsigned i=0; i<res.size(); i++ )
            res[i] = Deriv();
    }
    else
    {
        for (SetIndexArray::const_iterator it = indices.begin();
                it != indices.end();
                ++it)
        {
            res[*it] = Deriv();
        }
    }
#endif
}

template <class DataTypes>
void FixedConstraint<DataTypes>::projectPosition(const core::MechanicalParams* /*mparams*/ /* PARAMS FIRST */, DataVecCoord& /*xData*/)
{

}

// Matrix Integration interface
template <class DataTypes>
void FixedConstraint<DataTypes>::applyConstraint(defaulttype::BaseMatrix *mat, unsigned int offset)
{
    //sout << "applyConstraint in Matrix with offset = " << offset << sendl;
    //cerr<<"FixedConstraint<DataTypes>::applyConstraint(defaulttype::BaseMatrix *mat, unsigned int offset) is called "<<endl;
    const unsigned int N = Deriv::size();
    const SetIndexArray & indices = f_indices.getValue();

    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        // Reset Fixed Row and Col
        for (unsigned int c=0; c<N; ++c)
            mat->clearRowCol(offset + N * (*it) + c);
        // Set Fixed Vertex
        for (unsigned int c=0; c<N; ++c)
            mat->set(offset + N * (*it) + c, offset + N * (*it) + c, 1.0);
    }
}

template <class DataTypes>
void FixedConstraint<DataTypes>::applyConstraint(defaulttype::BaseVector *vect, unsigned int offset)
{
    //cerr<<"FixedConstraint<DataTypes>::applyConstraint(defaulttype::BaseVector *vect, unsigned int offset) is called "<<endl;
    //sout << "applyConstraint in Vector with offset = " << offset << sendl;
    const unsigned int N = Deriv::size();

    const SetIndexArray & indices = f_indices.getValue();
    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        for (unsigned int c=0; c<N; ++c)
            vect->clear(offset + N * (*it) + c);
    }
}




template <class DataTypes>
void FixedConstraint<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowBehaviorModels()) return;
    if (!this->isActive()) return;
    const VecCoord& x = *this->mstate->getX();
    //serr<<"FixedConstraint<DataTypes>::draw(), x.size() = "<<x.size()<<sendl;




    const SetIndexArray & indices = f_indices.getValue();

    if( _drawSize.getValue() == 0) // old classical drawing by points
    {
        std::vector< Vector3 > points;
        Vector3 point;
        //serr<<"FixedConstraint<DataTypes>::draw(), indices = "<<indices<<sendl;
        if( f_fixAll.getValue() )
            for (unsigned i=0; i<x.size(); i++ )
            {
                point = DataTypes::getCPos(x[i]);
                points.push_back(point);
            }
        else
            for (SetIndexArray::const_iterator it = indices.begin();
                    it != indices.end();
                    ++it)
            {
                point = DataTypes::getCPos(x[*it]);
                points.push_back(point);
            }
        vparams->drawTool()->drawPoints(points, 10, Vec<4,float>(1,0.5,0.5,1));
    }
    else // new drawing by spheres
    {
        std::vector< Vector3 > points;
        Vector3 point;
        glColor4f (1.0f,0.35f,0.35f,1.0f);
        if( f_fixAll.getValue()==true )
            for (unsigned i=0; i<x.size(); i++ )
            {
                point = DataTypes::getCPos(x[i]);
                points.push_back(point);
            }
        else
            for (SetIndexArray::const_iterator it = indices.begin();
                    it != indices.end();
                    ++it)
            {
                point = DataTypes::getCPos(x[*it]);
                points.push_back(point);
            }
        vparams->drawTool()->drawSpheres(points, (float)_drawSize.getValue(), Vec<4,float>(1.0f,0.35f,0.35f,1.0f));
    }
}

// Specialization for rigids
#ifndef SOFA_FLOAT
template <>
void FixedConstraint<Rigid3dTypes >::draw(const core::visual::VisualParams* vparams);
template <>
void FixedConstraint<Rigid2dTypes >::draw(const core::visual::VisualParams* vparams);
#endif
#ifndef SOFA_DOUBLE
template <>
void FixedConstraint<Rigid3fTypes >::draw(const core::visual::VisualParams* vparams);
template <>
void FixedConstraint<Rigid2fTypes >::draw(const core::visual::VisualParams* vparams);
#endif



} // namespace constraint

} // namespace component

} // namespace sofa

#endif


