FIND_PATH(OPENCL_INCLUDE_DIR
        NAMES
                CL/cl.h OpenCL/cl.h
        PATHS
                $ENV{AMDAPPSDKROOT}/include
                $ENV{INTELOCLSDKROOT}/include
                $ENV{NVSDKCOMPUTE_ROOT}/OpenCL/common/inc
				$ENV{CUDA_PATH}/include
                # Legacy Stream SDK
                $ENV{ATISTREAMSDKROOT}/include)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(OPENCL_LIB_SEARCH_PATH
                ${OPENCL_LIB_SEARCH_PATH}
                $ENV{AMDAPPSDKROOT}/lib/x86
                $ENV{INTELOCLSDKROOT}/lib/x86
                $ENV{NVSDKCOMPUTE_ROOT}/OpenCL/common/lib/Win32
				$ENV{CUDA_PATH}/lib/Win32
                # Legacy Stream SDK
                $ENV{ATISTREAMSDKROOT}/lib/x86)
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(OPENCL_LIB_SEARCH_PATH
                ${OPENCL_LIB_SEARCH_PATH}
                $ENV{AMDAPPSDKROOT}/lib/x86_64
                $ENV{INTELOCLSDKROOT}/lib/x64
                $ENV{NVSDKCOMPUTE_ROOT}/OpenCL/common/lib/x64
				$ENV{CUDA_PATH}/lib/x64
                # Legacy stream SDK
                $ENV{ATISTREAMSDKROOT}/lib/x86_64)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

FIND_LIBRARY(
    OPENCL_LIBRARY
    NAMES OpenCL
    PATHS ${OPENCL_LIB_SEARCH_PATH})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OpenCL
  DEFAULT_MSG
  OPENCL_LIBRARY OPENCL_INCLUDE_DIR)

if(OPENCL_FOUND)
  set(OPENCL_LIBRARIES ${OPENCL_LIBRARY})
else(OPENCL_FOUND)
  set(OPENCL_LIBRARIES)
endif(OPENCL_FOUND)

mark_as_advanced(
  OPENCL_INCLUDE_DIR
  OPENCL_LIBRARY
  )