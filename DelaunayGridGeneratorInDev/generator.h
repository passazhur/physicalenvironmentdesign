#ifndef DELAUNAYGRIDGENERATOR_H
#define DELAUNAYGRIDGENERATOR_H

#include <stdexcept>
#include <limits>
#include <QList>
#include <QLinkedList>

#include "datamanager.h"
#include "grid.h"
#include "gridelement.h"
#include "nodewrapper.h"
#include "piecewiselinearcomplex.h"
#include "mathutils.h"

namespace DelaunayGridGenerator
{
    /// Note, that QList is not the "real" list with O(1) insertion complexyty, when
    /// QLinkedList is. But index-search for QList is O(1) and for QLinkedList is O(N),
    /// that's why one need to use wrapped objects with self-pointers to achieve O(1) in
    /// both cases. See help references or
    /// http://qt-project.org/doc/qt-5.1/qtcore/containers.html#algorithmic-complexity
    /// \todo the <int> is used for node indexing, but what if there will be more nodes?
    template <
        typename _WrappedNodeType_,
        typename _FacetType_,
        typename _WrappedElementType_,
        typename _ElementsTreeDataManagerType_,
        typename _PlcType_,
        typename _GridType_,
        int _nDimensions_,
        typename _DimType_ = MathUtils::Real>
    class Generator
    {
        /// \todo it is only for tests, remove is later
        friend class Test_Generator;

        private: _DimType_ _DiscretizationStep;

        private: QList<_WrappedNodeType_>           _nodesList;

        // "real" lists of manipulated object
        private: QLinkedList<_WrappedNodeType_*>    _aliveNodesPtrs;
        private: QLinkedList<_WrappedNodeType_*>    _deadNodesPtrs;
        private: QLinkedList<_FacetType_*>          _aliveFacetsPtrs;
        private: QLinkedList<_FacetType_*>          _deadFacetsPtrs;
        private: QLinkedList<_WrappedElementType_*> _elementsPtrs;

        private: _ElementsTreeDataManagerType_ *_ptrToElementsDataManager = nullptr;

        private: const _PlcType_ *_ptrToPlc = nullptr;

        public : const QList<_WrappedNodeType_> & getNodeList() const noexcept
        {
            return _nodesList;
        }
        public : const QLinkedList<_FacetType_*> & getAliveFacetsList() const noexcept
        {
            return _aliveFacetsPtrs;
        }
        public : const QLinkedList<_FacetType_*> & getDeadFacetsList() const noexcept
        {
            return _deadFacetsPtrs;
        }

        private: void _copyAndWrapPlcNodesToInternalStorage() noexcept
        {
            int _index = 0;
            for(auto i:_ptrToPlc->getNodeList())
            {
                _nodesList.append(_WrappedNodeType_(i, _index));
                _nodesList.back().appendToAliveList(_aliveNodesPtrs);
                ++_index;
            }

        }

        private: struct _NodeIndexIterator
        {
            const Generator &_ref;
            const int *ptrToIndexses;
            const _WrappedNodeType_ &operator [] (int index) const noexcept
            {
                return _ref._nodesList[ptrToIndexses[index]];
            }
        };

        private: bool _isAlreadyUsedNode(
                int targetIndex,
                const int *usedIdexes,
                int usedNodesCout) const noexcept
        {
            for(int j=0; j<usedNodesCout; j++)
                if(targetIndex == usedIdexes[j])
                    return true;
            return false;
        }

        /// \todo perhaps it should be at differen class, FiniteElement for example
        /// \todo use std::is_permutation
        private: bool _isSameNodeIndexses(
                const int *nodeIndexesA,
                const int *nodeIndexesB) const noexcept
        {
            for(int i=0; i<_nDimensions_; ++i)
            {
                bool _isFound = false;
                for(int j=0; j<_nDimensions_; ++j)
                    if(nodeIndexesA[i] == nodeIndexesB[j])
                    {
                        _isFound = true;
                        break;
                    }
                if(!_isFound)
                    return false;
            }
            return true;
        }

        /// If grid is empty, constructs the first facet,
        /// It should be done in O(N^(_nDimensions_-1)) steps.
        /// \todo use tree to make it in O(N^(_nDimensions_-2)log(N))
        /// Algorithm:
        /// 1) Take first node;
        /// 2) Take next node, which match Delaunay criteria (check all nodes);
        /// 3) repeat step 2) until one finds all needed nodes.
        /// It is the modification of Fleischmann's approach,
        /// for more details see "Fleischmann - Three-Dimensional Delaunay Mesh
        /// Generation Using a Modified Advancing Front Approach"
        private: _FacetType_* _constructFirstFacet() throw(std::runtime_error)
        {
            if(_aliveNodesPtrs.size() < _nDimensions_)
                throw std::runtime_error("_constructFirstFacet(),  not enough nodes");

            int _facetNodesIndexes[_nDimensions_];
            _NodeIndexIterator _indexIterator = {*this, _facetNodesIndexes};

            // Get first alive node;
            /// \todo don't use indexes, remake all to pointers;
            _facetNodesIndexes[0] = _nodesList.first().getGlobalIndex();

            // Get next correct alive nodes;
            for(int i=1; i<_nDimensions_; ++i)
            {
                auto _curAliveNode = _nodesList.begin();

                // they should be lineary independent,
                // i.e. not lie at same line in 3D, and so on...
                if(i>1)
                {
                    bool _isDependent = true;
                    do
                    {
                        ++_curAliveNode;
                        // ignore already used nodes
                        if(_isAlreadyUsedNode(
                                  (*_curAliveNode).getGlobalIndex(),_facetNodesIndexes,i))
                            continue;

                        _facetNodesIndexes[i] = (*_curAliveNode).getGlobalIndex();
                        _isDependent = MathUtils::calculateIsCoplanarStatusByMatrixRank<
                                _WrappedNodeType_,
                                _nDimensions_,
                                _NodeIndexIterator,
                                _DimType_>
                                (_indexIterator,i+1);
                    }while(_isDependent);
                }
                else // They can't be equal by default!
                {
                    ++_curAliveNode;    // It should be second node
                    _facetNodesIndexes[i] = (*_curAliveNode).getGlobalIndex();
                }

                // Test Delaunay criteria for rest of the nodes
                // Note, that Delaunay testing excludes lineary depended nodes
                // because they can't locate at circumscribed sphere
                /// \todo make it simple for 2D
                /// \todo try to save sphere center and radius at each step
                ++_curAliveNode;
                while(_curAliveNode!=_nodesList.end())
                {
                    // If new node is "not Delaunay" use it as last found node of facet
                    // do not mind about rest nodes on the sphere - it is first facet,
                    // so there have to not be any intersections, and one can use the firs node
                    /// \todo see MathUtils::calculateIsNotDelaunayStatus()
                    _DimType_ _sphereRadius;
                    _WrappedNodeType_ _sphereCenter =
                            MathUtils::calculateCircumSphereCenterByCayleyMengerDeterminant<
                                _WrappedNodeType_,
                                _nDimensions_,
                                _NodeIndexIterator,
                                _DimType_>(_indexIterator, i+1, &_sphereRadius);
                    _sphereRadius = MathUtils::round(_sphereRadius,_DiscretizationStep);
                    for( ; _curAliveNode!=_nodesList.end(); ++_curAliveNode)
                    {
                        // ignore already used nodes
                        if(_isAlreadyUsedNode(
                                (*_curAliveNode).getGlobalIndex(),_facetNodesIndexes,i))
                            continue;
                        _DimType_ _dist = MathUtils::round(
                                    (*_curAliveNode).distance(_sphereCenter),
                                    _DiscretizationStep);
                        if(_dist < _sphereRadius)
                        {
                            _facetNodesIndexes[i] = (*_curAliveNode).getGlobalIndex();
                            ++_curAliveNode;
                            break;
                        }
                    }
                }
            }
            // It will be GridFacet::DIRECTION_BOUTH by default, see constructor
            return new _FacetType_(&_nodesList, _facetNodesIndexes);
        }

        /// Construct element with given facet;
        /// It should be done in O(N) steps;
        /// \todo use tree to make it in O(log(N))
        /// If there is no matching node - given facet is in metastructure,
        /// so it can't be constructed an element, and method returns nullptr.
        private: _WrappedElementType_* _constructElement(_FacetType_ *curAliveFacet)
            throw(std::runtime_error)
        {
            // Note, if there are _nDimensions_ nodes, then it is last facet,
            // and it is in metastructure
            if(_aliveNodesPtrs.size() < _nDimensions_)
                throw std::runtime_error("_constructElement(),  not enough nodes");

            int _elementNodesIndexes[_nDimensions_+1];
            memcpy(_elementNodesIndexes,curAliveFacet->getNodeIndexes(),_nDimensions_*sizeof(int));
            _NodeIndexIterator _indexIterator = {*this,_elementNodesIndexes};
            QLinkedList<_WrappedNodeType_*> _sphereLocatedNodes;
            _WrappedNodeType_ _sphereCenter;
            _DimType_ _sphereRadius;
            bool _isMetastructure = true;

            for(auto _curAliveNode = _aliveNodesPtrs.begin();
                  _curAliveNode != _aliveNodesPtrs.end(); ++_curAliveNode)
            {
                // ignore already used nodes
                if(_isAlreadyUsedNode(
                            (*_curAliveNode)->getGlobalIndex(),
                            _elementNodesIndexes,
                            _nDimensions_))
                    continue;

                // Check nodes side relative to facet and their lineary dependence
                // Note, that one should do this check for every node, because
                // it can be located at wrong side (and still, for later nodes,
                // they can't be lineary dependent, because they can't locate at
                // circumscribed sphere, but what about their side?)
                /// \todo try to make it like classic CDT with node visibility
                /// (dont check Delaunay criteria for wrong side, i.e invisible nodes)
                _DimType_ _determinant = MathUtils::round(
                            MathUtils::calculateIsCoplanarStatusWithClippingCheck<
                                _WrappedNodeType_,
                                _nDimensions_,
                                _NodeIndexIterator,
                                _DimType_>
                                (**_curAliveNode,_indexIterator),
                            _DiscretizationStep);
                if(_determinant == _DimType_(0.0) ||            // Node is lineary dependent
                        (_determinant < _DimType_(0.0) &&       // GridFacet::DIRECTION_LEFT
                         curAliveFacet->getFrontConstructionDirection()
                         == _FacetType_::DIRECTION_RIGHT) ||
                        (_determinant > _DimType_(0.0) &&       // GridFacet::DIRECTION_RIGHT
                         curAliveFacet->getFrontConstructionDirection()
                         == _FacetType_::DIRECTION_LEFT) /*||
                        (_determinant != _DimType_(0.0) &&
                         curAliveFacet->getFrontConstructionDirection()
                         != _FacetType_::DIRECTION_BOUTH)*/)
                    continue;

                // Ok! one found at least one node
                _isMetastructure = false;

                // Check Delaunay criteria
                // If node is on sphere - push it to _sphereLocatedNodes
                _elementNodesIndexes[_nDimensions_] = (*_curAliveNode)->getGlobalIndex();
                /// \todo use MathUtils::calculateIsNotDelaunayStatus

                _sphereCenter = MathUtils::calculateCircumSphereCenter<
                        _WrappedNodeType_,
                        _nDimensions_,
                        _NodeIndexIterator,
                        _DimType_>(_indexIterator, &_sphereRadius);
                _sphereRadius = MathUtils::round(_sphereRadius,_DiscretizationStep);
                _sphereLocatedNodes.clear();
                _sphereLocatedNodes.append(*_curAliveNode);

                for( ++_curAliveNode; _curAliveNode != _aliveNodesPtrs.end(); ++_curAliveNode)
                {
                    // ignore already used nodes
                    if(_isAlreadyUsedNode(
                                (*_curAliveNode)->getGlobalIndex(),
                                _elementNodesIndexes,
                                _nDimensions_ /* +1 */))
                        // It can't be the last node, so don't check it
                        continue;
                    _DimType_ _dist = MathUtils::round(
                                (*_curAliveNode)->distance(_sphereCenter),
                                _DiscretizationStep);
                    if(_dist <= _sphereRadius)
                    {
                        if(_dist == _sphereRadius)
                        {
                            // One should check node's side again
                            _determinant = MathUtils::round(
                                        MathUtils::calculateIsCoplanarStatusWithClippingCheck<
                                            _WrappedNodeType_,
                                            _nDimensions_,
                                            _NodeIndexIterator,
                                            _DimType_>
                                            (**_curAliveNode,_indexIterator),
                                        _DiscretizationStep);
                            // It can't be lineary dependent
                            if((_determinant > _DimType_(0.0) &&
                                    curAliveFacet->getFrontConstructionDirection()
                                    == _FacetType_::DIRECTION_RIGHT) ||
                                    (_determinant < _DimType_(0.0) &&
                                     curAliveFacet->getFrontConstructionDirection()
                                     == _FacetType_::DIRECTION_LEFT) ||
                                    (curAliveFacet->getFrontConstructionDirection()
                                     == _FacetType_::DIRECTION_BOUTH))
                                _sphereLocatedNodes.append(*_curAliveNode);
                        }
                        else break;
                    }
                }
                // One should decrease current iterator, because given
                // loop will increase it at next step, and miss the target
                --_curAliveNode;
            }

            // No nodes were found, the facet is in metastructure
            if(_isMetastructure)
                return nullptr;

            // Check intersections on sphere located nodes
            if(_sphereLocatedNodes.size() > 1)
            {
                for(auto _curAliveNode = _sphereLocatedNodes.begin();
                    _curAliveNode != _sphereLocatedNodes.end(); ++_curAliveNode)
                {
                    bool _isIntersection = false;
                    for(int i=0; i<(*_curAliveNode)->getFacets().size(); ++i)
                    {
                        _FacetType_* _targetFacet = static_cast<_FacetType_*>(
                                    (*_curAliveNode)->getFacets()[i]);
                        // Exclude dead facets, if there is an intersection, then
                        // there exist at least one alive facet, which intersects
                        if(_targetFacet->getState() == _FacetType_::STATE_DEAD)
                            continue;

                        _NodeIndexIterator _indexIteratorTargetAliveFacet =
                            {*this, _targetFacet->getNodeIndexes()};

                        for(int i=0; i<_nDimensions_; ++i)
                        {
                            int _newFacetNodesIndexes[_nDimensions_];
                            // Need this to skip one by one base nodes and construct new facets
                            for(int j=0, k=0; j<_nDimensions_; ++j, ++k)
                            {
                                if(k == _nDimensions_-1-i)
                                    ++k;
                                _newFacetNodesIndexes[j] = _elementNodesIndexes[k];
                            }
                            _NodeIndexIterator _indexIteratorCurrentNewFacet =
                                {*this,_newFacetNodesIndexes};

                            if(
                                    MathUtils::calculateSubsimplexSubsimplexIntersectionRound<
                                    _WrappedNodeType_,
                                    _nDimensions_,
                                    _NodeIndexIterator,
                                    _DimType_>(
                                        _indexIteratorCurrentNewFacet,
                                        _indexIteratorTargetAliveFacet,
                                        _DiscretizationStep))
                            {
                                _isIntersection = true;
                                break;
                            }
                        }
                        if(_isIntersection)
                            break;
                    }
                    if(!_isIntersection)
                    {
                        _elementNodesIndexes[_nDimensions_] =
                                (*_curAliveNode)->getGlobalIndex();
                        break;
                    }
                }
            }

            // Create new element
            return new _WrappedElementType_(
                        &_nodesList,
                        _elementNodesIndexes,
                        _sphereCenter,
                        _sphereRadius);
        }

        /// After construction of the new element,
        /// one should update cross-references,
        /// alive/dead lists and grid elements state.
        /// Also, new Facets should be created and registered.
        /// If given element is nullptr (was not constructed) then
        /// given facet is in metastructure, and will be killed.
        private: void _updateListsAndStates(
                _WrappedElementType_ *newElement,
                _FacetType_ *curAliveFacet) noexcept
        {
            // If no element was constructed, the
            // facet is in metastructure, kill it!
            if(newElement == nullptr)
            {
                curAliveFacet->setMetastructure();
                curAliveFacet->kill(_aliveFacetsPtrs, _deadFacetsPtrs);
                curAliveFacet->tryToKillNodes(_aliveNodesPtrs, _deadNodesPtrs);
                return;
            }

            // Check if new facets already exist at neighbor elements.
            // Try all alive facets adjoined to last element's node.
            // If facet is really new  - create it, and register at lisits.
            _FacetType_ *_newFacets[_nDimensions_+1];
            _newFacets[0] = curAliveFacet;
            for(int i=0; i<_nDimensions_; ++i)  // for all new facets
            {
                int _newFacetNodesIndexes[_nDimensions_];
                // Need this to skip one by one base nodes and construct new facets
                for(int j=0, k=0; j<_nDimensions_; ++j, ++k)
                {
                    // skip from the end, it is importatnt!
                    // see next search directions updates
                    if(k == _nDimensions_-1-i)
                        ++k;
                    _newFacetNodesIndexes[j] = newElement->getNodeIndexes()[k];
                }

                // Check adjoined facets
                bool _alreadyExist = false;
                for(int f=0; f<(*newElement)[_nDimensions_].getFacets().size(); ++f)
                {
                    _FacetType_* _targetFacet = static_cast<_FacetType_*>(
                                (*newElement)[_nDimensions_].getFacets()[f]);

                    // Exclude dead facets
                    if(_targetFacet->getState() == _FacetType_::STATE_DEAD)
                        continue;

                    // If facet already exist, update it, and it's nodes, if needed
                    if(_isSameNodeIndexses(
                                _targetFacet->getNodeIndexes(),
                                _newFacetNodesIndexes))
                    {
                        _newFacets[i+1] = _targetFacet;
                        _targetFacet->kill(_aliveFacetsPtrs, _deadFacetsPtrs);
                        _targetFacet->tryToKillNodes(_aliveNodesPtrs, _deadNodesPtrs);
                        _alreadyExist = true;
                        break;
                    }
                }

                // Create new facet and register it
                if(!_alreadyExist)
                {
                    // It will be GridFacet::DIRECTION_BOUTH by default, see constructor
                    _newFacets[i+1] = new _FacetType_(&_nodesList, _newFacetNodesIndexes);
                    _newFacets[i+1]->registerAtNodes();
                    _newFacets[i+1]->appendToAliveList(_aliveFacetsPtrs);
                }
            }

            // Update search directions
            for(int i=0; i<_nDimensions_+1; ++i)  // for all new facets
            {
                // Exclude dead facets
                if(_newFacets[i]->getState() == _FacetType_::STATE_DEAD)
                    continue;

                if(_newFacets[i]->getFrontConstructionDirection()
                        == _FacetType_::DIRECTION_BOUTH)
                {
                    _DimType_ _determinant = MathUtils::round(
                                MathUtils::calculateIsCoplanarStatusWithClippingCheck<
                                _WrappedNodeType_,
                                _nDimensions_,
                                _FacetType_,
                                _DimType_>
                                ((*newElement)[_nDimensions_-i], *_newFacets[i]),
                            _DiscretizationStep);
                    // It can't be lineary dependent,
                    // it can be only left or right
                    if(_determinant > _DimType_(0.0)) // found at right, so next search at left
                        _newFacets[i]->setFrontConstructionDirection(
                                    _FacetType_::DIRECTION_LEFT);
                    else
                        _newFacets[i]->setFrontConstructionDirection(
                                    _FacetType_::DIRECTION_RIGHT);
                }
                else if((_newFacets[i]->getFrontConstructionDirection()
                         == _FacetType_::DIRECTION_LEFT) ||
                        (_newFacets[i]->getFrontConstructionDirection()
                         == _FacetType_::DIRECTION_RIGHT) ||
                        _newFacets[i]->isMetastructure())
                {
                    _newFacets[i]->kill(_aliveFacetsPtrs, _deadFacetsPtrs);
                    _newFacets[i]->tryToKillNodes(_aliveNodesPtrs, _deadNodesPtrs);
                }
            }

            newElement->setFacets(_newFacets);
            _elementsPtrs.append(newElement);
        }

        /// \todo use just for inDev testing, delete later
        public : void _TEST_constructGrid(const _PlcType_ *ptrToPlc) throw(std::runtime_error)
        {
            if(!ptrToPlc)
                throw std::runtime_error("constructGrid(), bad pointer to PLC");

            _ptrToPlc = ptrToPlc;

            _copyAndWrapPlcNodesToInternalStorage();

            _FacetType_ *_firstAliveFacet = _constructFirstFacet();
            _firstAliveFacet->registerAtNodes();
            _firstAliveFacet->appendToAliveList(_aliveFacetsPtrs);

            while(!_aliveFacetsPtrs.empty())
            {
                _WrappedElementType_ *_newElement = _constructElement(_aliveFacetsPtrs.first());
                _updateListsAndStates(_newElement,_aliveFacetsPtrs.first());
            }
        }

        /// \todo use just for inDev testing, delete later
        public : void _TEST_iteration(const _PlcType_ *ptrToPlc) throw(std::runtime_error)
        {
            if(_aliveNodesPtrs.empty())
            {
                clear();
                _ptrToPlc = ptrToPlc;
                _copyAndWrapPlcNodesToInternalStorage();
            }
            else
            {
                if(_aliveFacetsPtrs.empty())
                {
                    _FacetType_ *_firstAliveFacet = _constructFirstFacet();
                    _firstAliveFacet->registerAtNodes();
                    _firstAliveFacet->appendToAliveList(_aliveFacetsPtrs);
                }
                else
                {
                    _WrappedElementType_ *_newElement = _constructElement(_aliveFacetsPtrs.first());
                    _updateListsAndStates(_newElement,_aliveFacetsPtrs.first());
                }
            }
        }

        /// Constructs the grid;
        /// Input - Piecewise Linear Complex;
        /// Output - FEM::Grid;
        /// \todo
        public : void constructGrid(const _PlcType_ *ptrToPlc) throw(std::runtime_error)
        {

            if(!ptrToPlc)
                throw std::runtime_error("constructGrid(), bad pointer to PLC");
            /*_ptrToPlc = ptrToPlc; // need this for internal functions

            // initialize tree data manager
            _ptrToElementsDataManager = new _ElementsTreeDataManagerType_(
                        _ptrToPlc->getMinCoords(),
                        _ptrToPlc->getMaxCoords(),
                        _ptrToElementsDataManager);

            _copyAndWrapPlcNodesToInternalStorage();

            _constructFirstFacet();

            delete _ptrToElementsDataManager;
            _ptrToElementsDataManager = nullptr;*/
        }

        /// Clear all inner storages and prepare to next calculations
        public : void clear()
        {
            /// \todo
        }

        public : Generator() noexcept :
            _DiscretizationStep(std::numeric_limits<_DimType_>::epsilon() * 10){}
        public : ~Generator() noexcept
        {
            if(_ptrToElementsDataManager)
                delete _ptrToElementsDataManager;
        }
    };

    typedef Generator<
        WrappedNode2D,
        Edge,
        Triangle,
        Element2DTreeDataManager,
        CommonPlc2D,
        FEM::TriangularGrid<FEM::Node2D>,
        2> DelaunayGridGenerator2D;

    typedef Generator<
        WrappedNode3D,
        Facet,
        Tetrahedron,
        Element3DTreeDataManager,
        CommonPlc3D,
        FEM::TetrahedralGrid<FEM::Node3D>,
        3> DelaunayGridGenerator3D;
}

#endif // DELAUNAYGRIDGENERATOR_H
