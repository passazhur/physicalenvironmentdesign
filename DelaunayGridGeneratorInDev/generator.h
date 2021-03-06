#ifndef DELAUNAYGRIDGENERATOR_H
#define DELAUNAYGRIDGENERATOR_H

#include <stdexcept>
#include <limits>

#include "containerdeclaration.h"

#include "datamanager.h"
#include "grid.h"
#include "gridelement.h"
#include "nodewrapper.h"
#include "piecewiselinearcomplex.h"
#include <MathUtils>

namespace DelaunayGridGenerator
{
    /// Before using this, make shure that
    /// sizeof(_FacetType_*) == sizeof(void*)) at your platform;
    /// Note, that QList is not the "real" list with O(1) insertion complexyty, when
    /// QLinkedList is. But index-search for QList is O(1) and for DefinedListType is O(N),
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
        private: _DimType_ _DiscretizationStep;

        private: DefinedVectorType<_WrappedNodeType_*>  _nodesList;

        // "real" lists of manipulated object
        private: DefinedListType<_WrappedNodeType_*>    _aliveNodesPtrs;
        private: DefinedListType<_WrappedNodeType_*>    _deadNodesPtrs;
        private: DefinedListType<_FacetType_*>          _aliveFacetsPtrs;
        private: DefinedListType<_FacetType_*>          _deadFacetsPtrs;
        private: DefinedListType<_WrappedElementType_*> _elementsPtrs;

        private: _ElementsTreeDataManagerType_ *_ptrToElementsDataManager = nullptr;

        private: const _PlcType_ *_ptrToPlc = nullptr;

        public : const DefinedVectorType<_WrappedNodeType_*> & getNodeList() const noexcept
        {
            return _nodesList;
        }
        public : const DefinedListType<_WrappedNodeType_*> & getAliveNodeList() const noexcept
        {
            return _aliveNodesPtrs;
        }
        public : const DefinedListType<_WrappedNodeType_*> & getDeadNodeList() const noexcept
        {
            return _deadNodesPtrs;
        }
        public : const DefinedListType<_FacetType_*> & getAliveFacetsList() const noexcept
        {
            return _aliveFacetsPtrs;
        }
        public : const DefinedListType<_FacetType_*> & getDeadFacetsList() const noexcept
        {
            return _deadFacetsPtrs;
        }
        public : const DefinedListType<_WrappedElementType_*> & getElementsList() const noexcept
        {
            return _elementsPtrs;
        }

        private: void _copyAndWrapPlcNodesToInternalStorage() noexcept
        {
            int _index = 0;
            for(auto i:_ptrToPlc->getNodeList())
            {
                // Don't forget to delete
                _nodesList.push_back(new _WrappedNodeType_(*i, _index));
                _nodesList.back()->appendToAliveList(_aliveNodesPtrs);
                ++_index;
            }

        }

        private: struct _NodeIndexIterator
        {
            const Generator &_ref;
            const int *ptrToIndexses;
            const _WrappedNodeType_ &operator [] (int index) const noexcept
            {
                return *_ref._nodesList[ptrToIndexses[index]];
            }
        };

        private: bool _isInSphereFacet(
                const _FacetType_ *targetFacet,
                const _WrappedNodeType_ &sphereCenter,
                const _DimType_ &sphereRadius ) const noexcept
        {
            for(int j=0, k=0; j<_nDimensions_; ++j, ++k)
            {
                _DimType_ _dist = MathUtils::trunc(
                            _nodesList[targetFacet->getNodeIndexes()[j]]->distance(sphereCenter),
                            _DiscretizationStep);
                if(_dist > sphereRadius)
                    return false;
            }
            return true;
        }

        private: bool _isAlreadyUsedNode(
                int targetIndex,
                const int *usedIdexes,
                int usedNodesCout) const noexcept
        {
            for(int j=0; j<usedNodesCout; ++j)
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
        /// Don't forget to delete facet later
        private: _FacetType_* _constructFirstFacet() throw(std::runtime_error)
        {
            if(_aliveNodesPtrs.size() < _nDimensions_)
                throw std::runtime_error("_constructFirstFacet(),  not enough nodes");

            int _facetNodesIndexes[_nDimensions_];
            _NodeIndexIterator _indexIterator = {*this, _facetNodesIndexes};

            // Get first alive node;
            /// \todo don't use indexes, remake all to pointers;
            _facetNodesIndexes[0] = _nodesList.front()->getGlobalIndex();

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
                                  (*_curAliveNode)->getGlobalIndex(),_facetNodesIndexes,i))
                            continue;

                        _facetNodesIndexes[i] = (*_curAliveNode)->getGlobalIndex();
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
                    _facetNodesIndexes[i] = (*_curAliveNode)->getGlobalIndex();
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
                    _sphereRadius = MathUtils::trunc(_sphereRadius,_DiscretizationStep);
                    for( ; _curAliveNode!=_nodesList.end(); ++_curAliveNode)
                    {
                        // ignore already used nodes
                        if(_isAlreadyUsedNode(
                                (*_curAliveNode)->getGlobalIndex(),_facetNodesIndexes,i))
                            continue;
                        _DimType_ _dist = MathUtils::trunc(
                                    (*_curAliveNode)->distance(_sphereCenter),
                                    _DiscretizationStep);
                        if(_dist < _sphereRadius)
                        {
                            _facetNodesIndexes[i] = (*_curAliveNode)->getGlobalIndex();
                            ++_curAliveNode;
                            break;
                        }
                    }
                }
            }
            // It will be GridFacet::DIRECTION_BOUTH by default, see constructor
            // Don't forget to delete!
            return new _FacetType_(&_nodesList, _facetNodesIndexes);
        }

        /// Construct element with given facet;
        /// It should be done in O(N) steps;
        /// \todo use tree to make it in O(log(N))
        /// If there is no matching node - given facet is in metastructure,
        /// so it can't be constructed an element, and method returns nullptr.
        /// Don't forget to delete element later
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
            DefinedListType<_WrappedNodeType_*> _sphereLocatedNodes;
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
                _DimType_ _determinant = MathUtils::trunc(
                            MathUtils::calculateIsCoplanarStatusWithClippingCheck<
//                            MathUtils::calculateIsCoplanarStatusWithClippingCheckNormalized<
                                _WrappedNodeType_,
                                _nDimensions_,
                                _NodeIndexIterator,
                                _DimType_>
                                (**_curAliveNode,_indexIterator),
                            _DiscretizationStep);
                /// \todo when det > 0.0 it is actually "Left", fix it, change signs!
                if(_determinant == _DimType_(0.0) ||            // Node is lineary dependent
                        (_determinant < _DimType_(0.0) &&       // DIRECTION_LEFT
                         curAliveFacet->getFrontConstructionDirection()
                         == DIRECTION_RIGHT) ||
                        (_determinant > _DimType_(0.0) &&       // DIRECTION_RIGHT
                         curAliveFacet->getFrontConstructionDirection()
                         == DIRECTION_LEFT) /*||
                        (_determinant != _DimType_(0.0) &&
                         curAliveFacet->getFrontConstructionDirection()
                         != DIRECTION_BOUTH)*/)
                    continue;

                // Ok! found at least one node
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
                _sphereRadius = MathUtils::trunc(_sphereRadius,_DiscretizationStep);
                _sphereLocatedNodes.clear();
                _sphereLocatedNodes.push_back(*_curAliveNode);

                for( ++_curAliveNode; _curAliveNode != _aliveNodesPtrs.end(); ++_curAliveNode)
                {
                    // ignore already used nodes
                    if(_isAlreadyUsedNode(
                                (*_curAliveNode)->getGlobalIndex(),
                                _elementNodesIndexes,
                                _nDimensions_ /* +1 */))
                        // It can't be the last node, so don't check it
                        continue;
                    _DimType_ _dist = MathUtils::trunc(
                                (*_curAliveNode)->distance(_sphereCenter),
                                _DiscretizationStep);
                    if(_dist <= _sphereRadius)
                    {
                        if(_dist == _sphereRadius)
                        {
                            // One should check node's side again
                            _determinant = MathUtils::trunc(
                                        MathUtils::calculateIsCoplanarStatusWithClippingCheck<
//                                      MathUtils::calculateIsCoplanarStatusWithClippingCheckNormalized<
                                            _WrappedNodeType_,
                                            _nDimensions_,
                                            _NodeIndexIterator,
                                            _DimType_>
                                            (**_curAliveNode,_indexIterator),
                                        _DiscretizationStep);
                            // It can't be lineary dependent
                            if((_determinant > _DimType_(0.0) &&
                                    curAliveFacet->getFrontConstructionDirection()
                                    == DIRECTION_RIGHT) ||
                                    (_determinant < _DimType_(0.0) &&
                                     curAliveFacet->getFrontConstructionDirection()
                                     == DIRECTION_LEFT) ||
                                    (curAliveFacet->getFrontConstructionDirection()
                                     == DIRECTION_BOUTH))
                                _sphereLocatedNodes.push_back(*_curAliveNode);
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
            /// \todo refactor, move in separated method
            if(_sphereLocatedNodes.size() > 1)
            {
                for(auto _curCheckedNode = _sphereLocatedNodes.begin();
                    _curCheckedNode != _sphereLocatedNodes.end(); ++_curCheckedNode)
                {
                    _elementNodesIndexes[_nDimensions_] =(*_curCheckedNode)->getGlobalIndex();
                    bool _isIntersection = false;
                    for(auto _curAliveNode = _curCheckedNode; //_sphereLocatedNodes.begin();
                        _curAliveNode != _sphereLocatedNodes.end(); ++_curAliveNode)
                    {
                        for(auto _f = (*_curAliveNode)->getFacets().begin();
                            _f != (*_curAliveNode)->getFacets().end(); ++_f)
                        {
                            _FacetType_* _targetFacet = static_cast<_FacetType_*>(*_f);
                            // Exclude dead facets, if there is an intersection, then
                            // there exist at least one alive facet, which intersects
                            if(_targetFacet->getState() == _FacetType_::STATE_DEAD)
                                continue;

                            // Exclude facets, which are outside sphere, there can be
                            // an intersection only with insphere facets
                            if(!_isInSphereFacet(_targetFacet, _sphereCenter, _sphereRadius))
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
                                        MathUtils::calculateSubsimplexSubsimplexIntersectionBarycentric<
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
                        if(_isIntersection)
                            break;
                    }
                    if(!_isIntersection)
                        break;
                }
            }

            // Create new element
            // Don't forget to delete!
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
        /// Don't forget to delete facets later
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
                for(auto _f = (*newElement)[_nDimensions_].getFacets().begin();
                    _f != (*newElement)[_nDimensions_].getFacets().end(); ++_f)
                {
                    _FacetType_* _targetFacet = static_cast<_FacetType_*>(*_f);

                    // Exclude just created facets
                    for(int p=0; p<i; ++p)
                        if(_targetFacet == _newFacets[p+1])
                            continue;

                    // Exclude dead facets
                    if(_targetFacet->getState() == _FacetType_::STATE_DEAD)
                        continue;

                    // If facet already exist, update it
                    if(_isSameNodeIndexses(
                                _targetFacet->getNodeIndexes(),
                                _newFacetNodesIndexes))
                    {
                        _newFacets[i+1] = _targetFacet;
                        _targetFacet->kill(_aliveFacetsPtrs, _deadFacetsPtrs);
                        // Don't kill nodes here, while all facets isn't created!
                        _alreadyExist = true;
                        break;
                    }
                }

                // Create new facet and register it
                if(!_alreadyExist)
                {
                    // It will be GridFacet::DIRECTION_BOUTH by default, see constructor
                    // Don't forget to delete!
                    _newFacets[i+1] = new _FacetType_(&_nodesList, _newFacetNodesIndexes);
                    _newFacets[i+1]->registerAtNodes();
                    _newFacets[i+1]->appendToAliveList(_aliveFacetsPtrs);
                }
            }

            // Update search directions
            for(int i=0; i<_nDimensions_+1; ++i)  // for all new facets
            {
                newElement->saveConstructionDirections( // save what was
                            i,_newFacets[i]->getFrontConstructionDirection());

                // Exclude dead facets
                if(_newFacets[i]->getState() == _FacetType_::STATE_DEAD)
                {
                    // Now nodes can be killed, if need
                    _newFacets[i]->tryToKillNodes(_aliveNodesPtrs, _deadNodesPtrs);
                    continue;
                }

                // For just created facets, or for first facet!
                if(_newFacets[i]->getFrontConstructionDirection()
                        == DIRECTION_BOUTH)
                {
                    _DimType_ _determinant = MathUtils::trunc(
                                MathUtils::calculateIsCoplanarStatusWithClippingCheck<
//                              MathUtils::calculateIsCoplanarStatusWithClippingCheckNormalized<
                                _WrappedNodeType_,
                                _nDimensions_,
                                _FacetType_,
                                _DimType_>
                                ((*newElement)[_nDimensions_-i], *_newFacets[i]),
                            _DiscretizationStep);
                    // It can't be lineary dependent,
                    // it can be only left or right
                    if(_determinant > _DimType_(0.0)) // found at right, so next search at left
                    {
                        // save what you found
                        newElement->saveConstructionDirections(i, DIRECTION_RIGHT);
                        // update with opposite
                        _newFacets[i]->setFrontConstructionDirection(DIRECTION_LEFT);
                    }
                    else
                    {
                        // save what you found
                        newElement->saveConstructionDirections(i, DIRECTION_LEFT);
                        // update with opposite
                        _newFacets[i]->setFrontConstructionDirection(DIRECTION_RIGHT);
                    }
                }
                // For base facet, if it isn't first at triangulation
                else if((_newFacets[i]->getFrontConstructionDirection() == DIRECTION_LEFT) ||
                        (_newFacets[i]->getFrontConstructionDirection() == DIRECTION_RIGHT) ||
                        _newFacets[i]->isMetastructure())
                {
                    _newFacets[i]->kill(_aliveFacetsPtrs, _deadFacetsPtrs);
                    _newFacets[i]->tryToKillNodes(_aliveNodesPtrs, _deadNodesPtrs);
                }
            }

            newElement->setFacets(_newFacets);
            _elementsPtrs.push_back(newElement);
        }

        /// \todo use just for inDev testing, delete later
        public : int _iteration;
        public : void _TEST_iteration(const _PlcType_ *ptrToPlc) throw(std::runtime_error)
        {
            if(_aliveNodesPtrs.empty())
            {
                _iteration = 0;
                clear();
                _ptrToPlc = ptrToPlc;
                _copyAndWrapPlcNodesToInternalStorage();
            }
            else
            {
                ++_iteration;
                if(_aliveFacetsPtrs.empty())
                {
                    _FacetType_ *_firstAliveFacet = _constructFirstFacet();
                    _firstAliveFacet->registerAtNodes();
                    _firstAliveFacet->appendToAliveList(_aliveFacetsPtrs);
                }
                else
                {
                    _WrappedElementType_ *_newElement = _constructElement(_aliveFacetsPtrs.front());
                    _updateListsAndStates(_newElement,_aliveFacetsPtrs.front());
                }
            }
        }

        /// \todo use just for inDev testing, delete later
        public : void _TEST_undo_iteration() throw(std::runtime_error)
        {
            if(!_elementsPtrs.empty())
            {
                auto _element = _elementsPtrs.end();
                --_element;
                // Get base facet
                _FacetType_ *_baseFacet = (*_element)->getFacets()[0];
                // Unregister last created element (update facets and nodes)
                (*_element)->unRegister(
                            _aliveNodesPtrs,
                            _deadNodesPtrs,
                            _aliveFacetsPtrs,
                            _deadFacetsPtrs);
                // Destroy last created element
                delete(*_element);
                _elementsPtrs.erase(_element);
                --_iteration;
                if(!_aliveFacetsPtrs.empty())
                {
                    // Move base facet to top of alive facets
                    _aliveFacetsPtrs.erase(_baseFacet->getPointerToMyself());
                    _baseFacet->prependToAliveList(_aliveFacetsPtrs);
                }
            }
        }

        /// Constructs the grid;
        /// Input - Piecewise Linear Complex;
        /// Output - new FEM::Grid, dont forget to delete later;
        /// Environment characteristics of grid elements will be set to nullptr;
        /// This function clears all previous data of generator;
        /// Note, if clearWhenDone == false (default is true),
        /// all internal data will be saved until new call of this function or
        /// until new clear() call. Data will be copied to new grid object,
        /// so make shure, that you have enough memory to ceep it.
        /// \todo
        public : _GridType_* constructGrid(
                const _PlcType_ *ptrToPlc,
                bool clearWhenDone = true)
            throw(std::runtime_error)
        {
            if(!ptrToPlc)
                throw std::runtime_error("constructGrid(), bad pointer to PLC");

            // Prepare to new grid construction
            clear();
            _ptrToPlc = ptrToPlc;
            // initialize tree data manager
//            _ptrToElementsDataManager = new _ElementsTreeDataManagerType_(
//                        _ptrToPlc->getMinCoords(),
//                        _ptrToPlc->getMaxCoords(),
//                        _ptrToElementsDataManager);
            if(_ptrToPlc->getNodeList().size()<_nDimensions_+1)
                throw std::runtime_error("constructGrid(), not enough nodes at input PLC");
            _copyAndWrapPlcNodesToInternalStorage();

            // Create first facet
            _FacetType_ *_firstAliveFacet = _constructFirstFacet();
            _firstAliveFacet->registerAtNodes();
            _firstAliveFacet->appendToAliveList(_aliveFacetsPtrs);

            // while exist alive facets, create new elements
            while(!_aliveFacetsPtrs.empty())
            {
                _WrappedElementType_ *_newElement = _constructElement(_aliveFacetsPtrs.front());
                _updateListsAndStates(_newElement,_aliveFacetsPtrs.front());
            }

            // Create new Grid object
            // Don't forget to delete!
            _GridType_ *_newGrid = new _GridType_();

            // Copy nodes to new grid
            auto _node = _nodesList.begin();
            while(_node != _nodesList.end())
            {
                _newGrid->createNode(**_node);
                if(clearWhenDone)
                {
                    delete (*_node);
                    _node = _nodesList.erase(_node);
                }
                else ++_node;
            }

            // Copy elements to new grid
            /// \todo check volume orientation
            auto _element = _elementsPtrs.begin();
            while(_element != _elementsPtrs.end())
            {
                // Note, that environment characteristics is set to nullptr
                auto _newElement = &_newGrid->createFiniteElement(
                            (*_element)->getNodeIndexes(), nullptr);
                _newElement->permuteOnNegativeVolume();
                if(clearWhenDone)
                {
                    delete(*_element);  // see destructor
                    _element = _elementsPtrs.erase(_element);
                }
                else ++_element;
            }

            if(clearWhenDone)
                clear();
            return _newGrid;
        }

        /// Clear all inner storages and prepare to next calculations
        /// It delets all created elements, facets and clear all lists.
        public : void clear() noexcept
        {
            auto _element = _elementsPtrs.begin();
            while(_element != _elementsPtrs.end())
            {
                delete(*_element);
                _element = _elementsPtrs.erase(_element);
            }

            auto _aliveFacet = _aliveFacetsPtrs.begin();
            while(_aliveFacet != _aliveFacetsPtrs.end())
            {
                delete(*_aliveFacet);
                _aliveFacet = _aliveFacetsPtrs.erase(_aliveFacet);
            }

            auto _deadFacet = _deadFacetsPtrs.begin();
            while(_deadFacet != _deadFacetsPtrs.end())
            {
                delete(*_deadFacet);
                _deadFacet = _deadFacetsPtrs.erase(_deadFacet);
            }

            _aliveNodesPtrs.clear();
            _deadNodesPtrs.clear();

            for(auto _node : _nodesList)
                delete(_node);
            _nodesList.clear();

            if(_ptrToElementsDataManager)
            {
                delete _ptrToElementsDataManager;
                _ptrToElementsDataManager = nullptr;
            }
        }
        public : friend std::ostream & operator << (
                std::ostream &textStream,
                const Generator &object) noexcept
        {
            textStream << "DelaunayGridGenerator " << &object << ":"          << std::endl;
            textStream << " Nodes       : " << object._nodesList.size()       << std::endl;
            textStream << " Alive nodes : " << object._aliveNodesPtrs.size()  << std::endl;
            textStream << " Dead nodes  : " << object._deadNodesPtrs.size()   << std::endl;
            textStream << " Alive facets: " << object._aliveFacetsPtrs.size() << std::endl;
            textStream << " Dead facets : " << object._deadFacetsPtrs.size()  << std::endl;
            textStream << " Elements    : " << object._elementsPtrs.size()    << std::endl;
            textStream << " EPS         : " << object._DiscretizationStep     << std::endl;
            return textStream;
        }
        public : Generator() noexcept :
            _DiscretizationStep(std::sqrt(std::numeric_limits<_DimType_>::epsilon())){}
//            _DiscretizationStep(std::numeric_limits<_DimType_>::epsilon()){}
//            _DiscretizationStep(1e-6){}
        public : ~Generator() noexcept
        {
            clear();
        }
    };

    typedef Generator<
        WrappedNode2D,
        Edge,
        Triangle,
        Element2DTreeDataManager,
        Plc2D,
        FEM::TriangularGrid,
        2> DelaunayGridGenerator2D;

    typedef Generator<
        WrappedNode3D,
        Facet,
        Tetrahedron,
        Element3DTreeDataManager,
        Plc3D,
        FEM::TetrahedralGrid,
        3> DelaunayGridGenerator3D;
}

#endif // DELAUNAYGRIDGENERATOR_H
