if(NOT THREADS_NUM)
	set(THREADS_NUM 1)
endif()

set(OPENCV_NAME opencv-4.6.0)
set(OPENCV_SOURCE ${CMAKE_SOURCE_DIR}/vendors/${OPENCV_NAME})
set(OPENCV_DEST ${CMAKE_SOURCE_DIR}/thirdparty/${OPENCV_NAME})
set(OPENCV_BUILD ${CMAKE_SOURCE_DIR}/thirdparty/${OPENCV_NAME}-build)
set(OPENCV_HEADERS ${OPENCV_DEST}/include/opencv4)
set(OPENCV_LIBRARIES ${OPENCV_DEST}/lib)
set(OPENCV_CONFIG " \
    -DBUILD_LIST=core,imgproc,imgcodecs \
    -DWITH_IPP=OFF \
    -DWITH_ITT=OFF \
    -DWITH_VA=OFF \
    -DWITH_PROTOBUF=OFF \
    -DWITH_OPENCL=OFF \
    -DWITH_V4L=OFF \
    -DWITH_GTK=OFF \
    -DBUILD_opencv_apps=OFF \
    -DBUILD_JAVA=OFF \
    -DBUILD_opencv_python2=OFF \
    -DBUILD_opencv_python3=OFF \
    -DWITH_LAPACK=OFF \
    -DWITH_VTK=OFF \
    -DWITH_EIGEN=OFF \
    -DWITH_1394=OFF \
    -DWITH_FFMPEG=OFF \
    -DWITH_GSTREAMER=OFF \
    -DBUILD_ZLIB=ON \
    -DBUILD_JPEG=ON \
    -DBUILD_WEBP=ON \
    -DBUILD_PNG=ON \
    -DBUILD_TIFF=ON \
    -DBUILD_OPENJPEG=ON \
    -DWITH_JASPER=OFF \
    -DBUILD_OPENEXR=ON \
    -DBUILD_TESTS=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DCV_TRACE=OFF \
    -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER} \
    -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
")
set(OPENCV_BUILDCMD " \
    ${CMAKE_MAKE_PROGRAM} -j${THREADS_NUM}
")

local_repo_cmake(
    ${OPENCV_NAME}
    SOURCE ${OPENCV_SOURCE}
    BUILD ${OPENCV_BUILD}
    RELEASE
    OPTIONS ${OPENCV_CONFIG}
    DESTINATION ${OPENCV_DEST}
    BUILD_COMMAND ${OPENCV_BUILDCMD}
)

include_directories(${OPENCV_HEADERS})
link_directories(${OPENCV_LIBRARIES})
link_directories(${OPENCV_LIBRARIES}/opencv4/3rdparty)

install(
    DIRECTORY ${OPENCV_SOURCE}/patches/
    DESTINATION patches
)
