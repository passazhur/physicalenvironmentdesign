#include "test_gridelement.h"

#include <iostream>

#include "nodewrapper.h"

using namespace DelaunayGridGenerator;

void Test_GridElement::test()
{
    int _n[] = {0,1,2};
    DefinedVectorType<WrappedNode2D*> _nodesList;
    _nodesList.push_back(new WrappedNode2D(MathUtils::Node2D(0,0)));
    _nodesList.push_back(new WrappedNode2D(MathUtils::Node2D(1,0)));
    _nodesList.push_back(new WrappedNode2D(MathUtils::Node2D(0,1)));
    GridElement<WrappedNode2D,Edge,2> _myElement1(
                &_nodesList, _n);
    QVERIFY((_myElement1.getCircumSphereCenter()-MathUtils::Node2D(0.5,0.5))[0] < 1e-4);
    QVERIFY(std::fabs(_myElement1.getCircumSphereRadius() - std::sqrt(2)/2) < 1e-4);

    GridElement<WrappedNode2D,Edge,2> _myElement2(_myElement1);
    QVERIFY(_myElement2[0] == *_nodesList[0] &&
            _myElement2[1] == *_nodesList[1] &&
            _myElement2[2] == *_nodesList[2] );
    QVERIFY((_myElement2.getCircumSphereCenter()-_myElement1.getCircumSphereCenter()).isZero());
    QVERIFY(_myElement2.getCircumSphereRadius() == _myElement1.getCircumSphereRadius());

    Triangle _myTriangle(&_nodesList, _n);
    QVERIFY((_myTriangle.getCircumSphereCenter()-MathUtils::Node2D(0.5,0.5))[0] < 1e-4);
    QVERIFY(std::fabs(_myTriangle.getCircumSphereRadius() - std::sqrt(2)/2) < 1e-4);

    delete(_nodesList[0]);
    delete(_nodesList[1]);
    delete(_nodesList[2]);
    _nodesList.clear();

    DefinedVectorType<WrappedNode3D*> _nodesList2;
    _nodesList2.push_back(new WrappedNode3D(MathUtils::Node3D(1,0,0)));
    _nodesList2.push_back(new WrappedNode3D(MathUtils::Node3D(0,1,0)));
    _nodesList2.push_back(new WrappedNode3D(MathUtils::Node3D(0,0,0)));
    Facet _myFacet(&_nodesList2, _n);
    /// \todo
    //QVERIFY((_myFacet.calculateCircumSphereCenter()-MathUtils::Node3D(0.5,0.5,0.0))[0] < 1e-4);
    QVERIFY(sizeof(Facet*) == sizeof(void*));
    QVERIFY(sizeof(Edge*) == sizeof(void*));

    delete(_nodesList2[0]);
    delete(_nodesList2[1]);
    delete(_nodesList2[2]);
    _nodesList2.clear();
}
