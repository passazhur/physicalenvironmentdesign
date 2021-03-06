#ifndef GRIDRENDER_H
#define GRIDRENDER_H

// For usage of Qt native macro SIGNAL() and SLOT()
#ifndef WT_NO_SLOT_MACROS
#define WT_NO_SLOT_MACROS
#endif // WT_NO_SLOT_MACROS

// For disabling boost warnings
#ifndef BOOST_SIGNALS_NO_DEPRECATION_WARNING
#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#endif // BOOST_SIGNALS_NO_DEPRECATION_WARNING

#include "Wt/WGLWidget"
#include "Wt/WMatrix4x4"

#include "piecewiselinearcomplex.h"

using namespace Wt;

#define USER_SIDE_CONTROL

namespace Utilities
{
    /// Ui web widget for WebGl render
    /// Algorithm: just see
    /// http://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1WGLWidget.html
    /// Remember that WebGL works only with floats and short indexes
    class GridRender : public WGLWidget
    {       
        public : enum RENDERING_MODE {MODE_2D, MODE_3D};
        /// default MODE_3D, see constructor
        private: RENDERING_MODE _renderingMode;
        public : RENDERING_MODE getRenderingMode() const {return _renderingMode;}
        public : void setRenderingMode(RENDERING_MODE mode){_renderingMode = mode;}

        private: Program                _shaderProgram;

        private: AttribLocation         _attributeVertexPosition;
        private: UniformLocation        _uniformVertexColor;
        private: UniformLocation        _uniformSceneMatrix;

        private: Buffer                 _renderingNodesVertexPositionBuffer;
        private: Buffer                 _renderingSegmentsIndexBuffer;
        private: Buffer                 _renderingFacetsIndexBuffer;

        public: GridRender(WContainerWidget *parent);

        /// default [1.0, 1.0, 1.0, 1.0] - white, see constructor
        private: FEM::Vector4D _PlcNodesColor;
        public : void setRenderingPlcNodesColor(double R, double G, double B, double A)
        {
            _PlcNodesColor(R,G,B,A);
        }
        /// default [0.0, 1.0, 0.0, 1.0] - green, see constructor
        private: FEM::Vector4D _PlcSegmentsColor;
        public : void setRenderingPlcSegmentsColor(double R, double G, double B, double A)
        {
            _PlcSegmentsColor(R,G,B,A);
        }
        /// default [0.8, 0.8, 0.8, 1.0] - light gray, see constructor
        private: FEM::Vector4D _PlcFacetsColor;
        public : void setRenderingPlcFacetsColor(double R, double G, double B, double A)
        {
            _PlcFacetsColor(R,G,B,A);
        }

        private: DelaunayGridGenerator::CommonPlc2D *_refToRenderingPlc2D = nullptr;
        /// Remember that WebGL works only with floats and short indexes
        public: void setRenderingPiecewiseLinearComplex(
                DelaunayGridGenerator::CommonPlc2D *renderingPlc2D) throw (std::logic_error);
        private: void _initializePlc2DNodesBuffers();
        private: void _initializePlc2DFacetsBuffers();
        private: void _drawPlc2DNodes();
        private: void _drawPlc2DFacets();

        private: DelaunayGridGenerator::CommonPlc3D *_refToRenderingPlc3D = nullptr;
        /// Remember that WebGL works only with floats and short indexes
        public: void setRenderingPiecewiseLinearComplex(
                DelaunayGridGenerator::CommonPlc3D *renderingPlc3D) throw (std::logic_error);

        /// See WGLWidget::bufferDatafv() and WGLWidget::renderfv() for more info
        private: void _initializePlc3DNodesBuffers() throw(std::runtime_error);
        private: void _initializePlc3DSegmentsBuffers() throw(std::runtime_error);
        private: void _initializePlc3DFacetsBuffers() throw(std::runtime_error);
        private: void _drawPlc3DNodes() throw(std::runtime_error);
        private: void _drawPlc3DSegments() throw(std::runtime_error);
        private: void _drawPlc3DFacets() throw(std::runtime_error);

        private: void _preloadAllBuffers();

        public: void initializeGL() override;
        public: void paintGL() override;
        public: void resizeGL(int width, int height) override;
        public: void updateGL() override;

        // User JavaScript conrol
#ifdef USER_SIDE_CONTROL
        /// \todo there are too many matrices! combine them
        private: JavaScriptMatrix4x4    _userSideModelMatrix;
        private: JavaScriptMatrix4x4    _userSideUserControlMatrix;
        private: JavaScriptMatrix4x4    _userSideWorldViewMatrix;
        private: JavaScriptMatrix4x4    _userSideProjectionMatrix;
        private: JavaScriptMatrix4x4    _userSideSceneMatrix;
        ///Multiplies Projection, World-View and Model matrices at client side
        private: void _buildSceneMatrix();
        private: JSlot                  _onMouseWentDownJSlot;
        private: JSlot                  _onMouseDraggedJSlot;
        private: JSlot                  _onMouseWheelJSlot;
        /// See native WGLWidget::glObjJsRef()
        private: std::string _glObjJsRef();
        /// Makes user-side JavaScript callback functions for view mouse-control
        private: void _initializeUserSideMouseControl();

        // Temporaly objects, just for TEST, to delete later
#else
        /// \todo there are too many matrices! combine them
        private: WMatrix4x4             _mModel;
        private: WMatrix4x4             _mWorld;
        private: WMatrix4x4             _mControl;
        private: WMatrix4x4             _mProj;
        private: WMatrix4x4             _mScene;    // =Proj*World*Control*Model
        private: int _oldMouseCoors[2];
        private: void _onMouseWentDown(const WMouseEvent &event);
        private: void _onMouseDragged(const WMouseEvent &event);
        private: void _onMouseWheel(const WMouseEvent &event);
#endif //USER_SIDE_CONTROL
    };

    /// See WGLWidget::makeFloat() -> Utils::round_js_str() -> round_js_str()
    /// at src/web/WebUtils.c
    /// \todo replace this function
    static inline char *generic_double_to_str(double d, char *buf)
    {
        if (!boost::math::isnan(d)) {
            if (!boost::math::isinf(d)) {
                sprintf(buf, "%.7e", d);
            } else {
                if (d > 0) {
                    sprintf(buf, "Infinity");
                } else {
                    sprintf(buf, "-Infinity");
                }
            }
        } else {
            sprintf(buf, "NaN");
        }
        return buf;
    }
}

#endif // GRIDRENDER_H
