#include "clmanager.h"

#include <sstream>

using namespace OpenCL;

template<typename T>
void CLManager::_appendBitfield(
        const T info,
        const T value,
        const cl::STRING_CLASS name,
        cl::STRING_CLASS &str)
{
    if (info & value)
    {
        if (str.length() > 0)
        {
            str.append(" | ");
        }
        str.append(name);
    }
}

std::string CLManager::printPlatformsInfo() const
{
    std::stringstream _str;
    if(!_platforms.empty())
    {
        _str << "Avaliable OpenCL platforms:" << std::endl;
        for(unsigned i=0; i<_platforms.size(); ++i)
        {
            _str << "platform[" << i << "]:" << std::endl;
            cl::STRING_CLASS _data;
            _platforms[i].getInfo(CL_PLATFORM_PROFILE,&_data);
            _str << "\tCL_PLATFORM_PROFILE:    " << _data.c_str() << std::endl;
            _platforms[i].getInfo(CL_PLATFORM_VERSION,&_data);
            _str << "\tCL_PLATFORM_VERSION:    " << _data.c_str() << std::endl;
            _platforms[i].getInfo(CL_PLATFORM_NAME,&_data);
            _str << "\tCL_PLATFORM_NAME:       " << _data.c_str() << std::endl;
            _platforms[i].getInfo(CL_PLATFORM_VENDOR,&_data);
            _str << "\tCL_PLATFORM_VENDOR:     " << _data.c_str() << std::endl;
            _platforms[i].getInfo(CL_PLATFORM_EXTENSIONS,&_data);
            _str << "\tCL_PLATFORM_EXTENSIONS: " << _data.c_str() << std::endl;
        }
    }
    else
        _str << "No avaliable OpenCL platforms" << std::endl;
    _str << std::endl;
    return _str.str();
}

std::string CLManager::printDevicesInfo() const
{
    std::stringstream _str;
    if(!_platforms.empty() && !_devices.empty())
    {
        for(unsigned i=0; i<_platforms.size();++i)
        {
            for(unsigned j=0; j<_devices[i].size();++j)
            {
                _str << "platform[" << i << "]:";
                _str << "device[" << j << "]:" << std::endl;
                cl::STRING_CLASS _data;

                cl_device_type _type;
                _devices[i][j].getInfo(CL_DEVICE_TYPE, &_type);
                _appendBitfield<cl_device_type>(_type, CL_DEVICE_TYPE_CPU, "CL_DEVICE_TYPE_CPU", _data);
                _appendBitfield<cl_device_type>(_type, CL_DEVICE_TYPE_GPU, "CL_DEVICE_TYPE_GPU", _data);
                _appendBitfield<cl_device_type>(_type, CL_DEVICE_TYPE_ACCELERATOR, "CL_DEVICE_TYPE_ACCELERATOR", _data);
                _appendBitfield<cl_device_type>(_type, CL_DEVICE_TYPE_DEFAULT, "CL_DEVICE_TYPE_DEFAULT", _data);
                _str << "\tCL_DEVICE_TYPE:                     " << _data.c_str() << "\n";

                cl_uint _number;
                _devices[i][j].getInfo(CL_DEVICE_VENDOR_ID, &_number);
                _str << "\tCL_DEVICE_VENDOR_ID:                " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &_number);
                _str << "\tCL_DEVICE_MAX_COMPUTE_UNITS:        " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &_number);
                _str << "\tCL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: " << _number << std::endl;

                /// \todo
                //devices[i][j].getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &data);
                //_str << "\tCL_DEVICE_MAX_WORK_ITEM_SIZES:      " << data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &_number);
                _str << "\tCL_DEVICE_MAX_WORK_GROUP_SIZE:      " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &_number);
                _str << "\tCL_DEVICE_MAX_CLOCK_FREQUENCY:      " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_ADDRESS_BITS, &_number);
                _str << "\tCL_DEVICE_ADDRESS_BITS:             " << _number << std::endl;

                cl_ulong _longNumber;
                _devices[i][j].getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &_longNumber);
                _str << "\tCL_DEVICE_MAX_MEM_ALLOC_SIZE:       " << _longNumber << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MAX_PARAMETER_SIZE, &_number);
                _str << "\tCL_DEVICE_MAX_PARAMETER_SIZE:       " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, &_number);
                _str << "\tCL_DEVICE_MEM_BASE_ADDR_ALIGN:      " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, &_number);
                _str << "\tCL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE: " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &_longNumber);
                _str << "\tCL_DEVICE_GLOBAL_MEM_SIZE:          " << _number << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &_longNumber);
                _str << "\tCL_DEVICE_LOCAL_MEM_SIZE:           " << _number << std::endl;

                cl_bool b;
                _devices[i][j].getInfo(CL_DEVICE_HOST_UNIFIED_MEMORY, &b);
                _data = b ? "TRUE" : "FALSE";
                _str << "\tCL_DEVICE_HOST_UNIFIED_MEMORY:      " << _data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_AVAILABLE, &b);
                _data = b ? "TRUE" : "FALSE";
                _str << "\tCL_DEVICE_AVAILABLE:                " << _data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_COMPILER_AVAILABLE, &b);
                _data = b ? "TRUE" : "FALSE";
                _str << "\tCL_DEVICE_COMPILER_AVAILABLE:       " << _data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_NAME, &_data);
                _str << "\tCL_DEVICE_NAME:              " << _data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_VENDOR, &_data);
                _str << "\tCL_DEVICE_VENDOR:                   " << _data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DRIVER_VERSION, &_data);
                _str << "\tCL_DRIVER_VERSION:                  " << _data.c_str() << std::endl;

                _devices[i][j].getInfo(CL_DEVICE_EXTENSIONS, &_data);
                _str << "\tCL_DEVICE_EXTENSIONS:               " << _data.c_str() << std::endl;
            }
        }
    }
    else
        _str << "No avaliable OpenCL platforms and devices" << std::endl;
    _str << std::endl;
    return _str.str();
}

cl::Program & CLManager::createProgram(
        const std::string &sourceCode,
        const cl::Context &targetContext,
        const std::vector<cl::Device> &targetDevices)
{
    _programs.push_back(cl::Program(targetContext, sourceCode, false, &_lastErrorCode));
    _programs.back().build(targetDevices);
    return _programs.back();
}

cl::Kernel &CLManager::createKernel(const cl::Program &program, const std::string &programName)
{
    _kernels.push_back(cl::Kernel(program, programName.c_str(), &_lastErrorCode));
    return _kernels.back();
}

CLManager::CLManager()
{
    // Get all avaliable platforms (Intel, AMD, NVidia, etc.)
    _lastErrorCode = cl::Platform::get(&_platforms);
    if (_lastErrorCode != CL_SUCCESS) return;

    // Get all avaliable devices per platform
    /// \todo put it after creating the context - get devices from context!
    _devices.resize(_platforms.size());
    for(unsigned i=0; i<_platforms.size();++i)
    {
        _lastErrorCode = _platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &_devices[i]);
        if (_lastErrorCode != CL_SUCCESS) return;
    }

    // Create context for devices per platform
    for(unsigned i=0; i<_platforms.size();++i)
    {
        cl_context_properties _properties[] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)(_platforms[i])(),
            0};
        _contexts.push_back(cl::Context(_devices[i],_properties));
    }

    // Create command queues for each device
    _commandQueues.resize(_platforms.size());
    for(unsigned i=0; i<_platforms.size();++i)
        for(unsigned j=0; j<_devices[i].size();++j)
            _commandQueues[i].push_back(cl::CommandQueue(_contexts[i],_devices[i][j]));
}

CLManager::~CLManager()
{
}

CLManager &CLManager::instance()
{
    static CLManager _instanceOfCLManager;
    return _instanceOfCLManager;
}