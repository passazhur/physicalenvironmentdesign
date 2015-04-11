#ifndef REPRESENTATIVEVOLUMEELEMENT_H
#define REPRESENTATIVEVOLUMEELEMENT_H

#include <stdexcept>
#include <cmath>
#include <iostream>

#include <FUNCTIONS/rand.h>
#include "CLMANAGER/clmanager.h"

#include "node.h"

/// It is discretized RVE
/// \warning for correct usage of OpenCL functionality, the _size
/// should be equal to some power of two.
/// \warning it uses 3 * _size * _size * _size * sizeof(float) bytes of RAM memory
class RepresentativeVolumeElement
{
    private: int _size;
    public : int getSize() const noexcept {return _size;}
    /// mask is _data < 0;
    private: float * _data = nullptr;
    private: float * _cuttedData = nullptr; /// \todo
    private: int _discreteRadius = 1; /// \todo logic usage?
    public : float * getData() noexcept {return _data;}
    public : const float * getData() const noexcept {return _data;}
    public : float * getCuttedData() noexcept {return _cuttedData;}
    public : const float * getCuttedData() const noexcept {return _cuttedData;}
    public : int getDiscreteRadius() const noexcept {return _discreteRadius;}

    private: float _representationSize = 1.0f;
    public : float getRepresentationSize() const noexcept {return _representationSize;}
    public : void setRepresentationSize(const float newRepresentationSize) noexcept {
        _representationSize = newRepresentationSize;}

    /// OpenCL pointers, should be created oly once
    private: static cl::Program *_programPtr;
    private: static cl::Kernel *_kernelXPtr;
    private: static cl::Kernel *_kernelYPtr;
    private: static cl::Kernel *_kernelZPtr;

    /// Constructor
    /// \todo all OpenCL uses only first system defined platform
    public : RepresentativeVolumeElement(
            const int discreteSize,
            const float representationSize = 1.0) throw (std::runtime_error);

    /// Clean _data storage, by set it elements to 0
    public : inline void cleanData() noexcept {
        memset(_data, 0, sizeof(float) * _size * _size * _size);}

    /// Clean masked _data storage, by set it elements to 0
    /// (i.e _data elements >= 0)
    public : void cleanUnMaskedData() noexcept;

    /// Clean mask
    /// (set all _data elements < 0 equal to -(_data elements)
    public : void cleanMask() noexcept;

    private : inline void _copyDataToCuttedData() noexcept {
        memcpy(_cuttedData, _data, sizeof(float) * _size * _size * _size);}

    private : void _copyMaskedCuttedDataToData() noexcept;

    /// Add random noise to _data elements that are not masked by_mask
    /// (i.e. to _data elements >= 0),
    /// note that _data can be unnormalized after this call, if _data wasn't cleaned before
    /// \todo move to device
    public : void addRandomNoise() noexcept;

    /// Apply relative to unmasked _data random noise
    /// (i.e. relative to _data elements >= 0),
    /// Note, that _data shold be normalized before this call, to get the correct rezult
    /// Relative means that random deviations will be near current _data value,
    /// high and small values reduce deviations. (value power function is used).
    /// deviationCoefficient - smaller coefficient increases deviations.
    /// Coefficient should be inside [0.0f, 1.0f].
    /// \todo move to device
    public : void applyRelativeRandomNoise(
            const float deviationCoefficient = 1.0f) throw (std::logic_error);

    /// Find min and max unmasked _data elements values (unmasked i.e >=0)
    public : void findUnMaskedMinAndMax(float &min, float &max) noexcept;

    /// Scale unmasked _data to range levelA <= _data[i,j,k] <= levelB;
    /// 0 <= levelA < levelB <= 1.0f
    public : void scaleUnMasked(
            const float levelA,
            const float levelB) throw (std::logic_error);

    /// Normalize all data
    /// \todo move to device
    public : void normalize() noexcept;

    /// Normalize unmsked data (i.e. elements, where _mask[i,j,k] == 0)
    /// \todo move to device
    public : void normalizeUnMasked() noexcept;

    /// Gaussian blur filter function, for external usage only
    /// Note, r, fx, fy, fz should be > 0;
    public: static inline float GaussianBlurFilter(
            float r,
            float x, float y, float z,
            float fx, float fy, float fz) noexcept {
        return std::exp(-(x*x/fx/fx + y*y/fy/fy + z*z/fz/fz) / ((r/2.0) * (r/2.0)));}

    /// Apply Gaussian filter to previously generated random filed
    /// see (2002) Torguato - Random Heterogeneous Materials Microstructure
    ///                       and Macroscopic Properties
    /// data will hold normalized GRF after this call
    /// \todo it simple CPU usage for now, use GPU!
    /// \todo add ellipsoid rotation
    /// \todo fix borders calculations
    /// \todo X and Z are replaced
    /// \todo cout
    public : void applyGaussianFilter(
            int discreteRadius,
            float ellipsoidScaleFactorX = 1.0,
            float ellipsoidScaleFactorY = 1.0,
            float ellipsoidScaleFactorZ = 1.0) throw (std::logic_error);

    private: inline void _CLGaussianBlurFilterPhase(
            cl::Buffer &_dataBuffer,
            cl::Buffer &_cuttedDataBuffer,
            cl::CommandQueue &_queue,
            cl::NDRange &_localThreads,
            cl::Event &_event,
            cl::Kernel &_phaseKernel);

    /// Apply Gaussian blur filter to _data
    /// _data will hold normalized GRF after this call
    ///
    /// see (2002) Torguato - Random Heterogeneous Materials Microstructure
    ///                       and Macroscopic Properties
    ///
    /// For RVE128 R=8, i.e for 1 073 741 824 operation (12.03.2015, full dim filter)
    ///
    ///                                                             United filter       Separated filter
    ///
    /// 1)  Single thread CPU -                                     354.751 seconds     6.83422 seconds
    ///
    /// 2)  OpenCL GPU "Cedar (AMD Radeon HD 6300M Series)"
    ///     (AMD OpenCL SDK 2.9.1 driver 1445.5 (VM))
    ///     2 units 0.750 GHz, WorkGroupSize: 4x4x4 -               42.5417 seconds     1.04376 seconds
    ///
    /// 3)  OpenCL CPU "Intel(R) Core(TM) i5-2410M CPU @ 2.30GHz"
    ///     (AMD OpenCL SDK 2.9.1 driver 1445.5 (sse2,avx))
    ///     4 units 2.294 GHz, WorkGroupSize: 8x8x8 -               62.8486 seconds     2.19053 seconds
    ///
    /// \todo cout
    /// \todo masking
    public : void applyGaussianFilterCL(
            int discreteRadius,
            float ellipsoidScaleFactorX = 1.0f,
            float ellipsoidScaleFactorY = 1.0f,
            float ellipsoidScaleFactorZ = 1.0f) throw (std::logic_error);

    /// Apply two-cut to mask (within)
    /// (i.e. set all _data elements, that are within cut levels, equal to (-_data elements)).
    /// 0.0f <= cutlevelA < cutLevelB <= 1.0f;
    /// Use only for normalized _data.
    /// It cleans previous mask.
    public : void applyTwoCutMaskInside(
            const float cutLevelA,
            const float cutLevelB) throw (std::logic_error);

    /// Apply two-cut to mask (outside)
    /// (i.e. set all _mask elements, that are outside cut levels, equal to (-_data elements)).
    /// 0.0f <= cutlevelA < cutLevelB <= 1.0f;
    /// Use only for normalized _data.
    /// It cleans previous mask.
    public : void applyTwoCutMaskOutside(
            const float cutLevelA,
            const float cutLevelB) throw (std::logic_error);

    /// Generate random ellipsoid at unmasked _data elements
    /// (i.e. where _data elements >=0),
    /// \todo move to device
    /// \todo random orientation
    /// \todo refactoring
    public : void generateRandomEllipsoidSmoothed(
            const int x,
            const int y,
            const int z,
            const int minRadius,
            const int maxRadius,
            const float transitionLayerSize = 0.0f,
            const float ellipsoidScaleFactorX = 1.0f,
            const float ellipsoidScaleFactorY = 1.0f,
            const float ellipsoidScaleFactorZ = 1.0f,
            const float coreValue = 1.0f) throw (std::logic_error);

    /// Generate overlapping random ellipsoids at unmasked _data elements
    /// (i.e. where _data elements >=0),
    /// \todo move to device
    /// \todo random orientation
    /// \todo refactoring
    public : void generateOverlappingRandomEllipsoids(
            const int ellipsoidNum,
            const int minRadius,
            const int maxRadius,
            const float transitionLayerSize = 0.0f,
            const float ellipsoidScaleFactorX = 1.0f,
            const float ellipsoidScaleFactorY = 1.0f,
            const float ellipsoidScaleFactorZ = 1.0f,
            const float coreValue = 1.0f,
            const float transitionLayerValue = 0.5f) throw (std::logic_error);

    /// Generate overlapping random ellipsoids at unmasked _data elements
    /// (i.e. where _data elements >=0),
    /// \todo move to device
    /// \todo random orientation
    /// \todo refactoring
    public : void generateOverlappingRandomEllipsoidsSmoothed(
            const int ellipsoidNum,
            const int minRadius,
            const int maxRadius,
            const float transitionLayerSize = 1.0f,
            const float ellipsoidScaleFactorX = 1.0f,
            const float ellipsoidScaleFactorY = 1.0f,
            const float ellipsoidScaleFactorZ = 1.0f,
            const float coreValue = 1.0f) throw (std::logic_error);

    public : ~RepresentativeVolumeElement()
    {
        delete [] _data;
//        delete [] _mask;
        delete [] _cuttedData;
    }
};

#endif // REPRESENTATIVEVOLUMEELEMENT_H
