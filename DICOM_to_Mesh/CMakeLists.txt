cmake_minimum_required(VERSION 2.8)
 
PROJECT(dicomToMHD)

set( CMAKE_BUILD_TYPE Debug)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

FIND_PACKAGE(Boost COMPONENTS system filesystem program_options REQUIRED)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

SET( EIGEN3_INCLUDE_DIR "/Eigen/include" )
IF( NOT EIGEN3_INCLUDE_DIR )
    MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
ENDIF()
INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )


include_directories(
	${PROJECT_SOURCE_DIR}/include
	${Boost_INCLUDE_DIR} 
)

SET(NEEDED_LIBS
	${VTK_LIBRARIES}
    	${Boost_CHRONO_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
	${Boost_IOSTREAMS_LIBRARY}
	${Boost_SERIALIZATION_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY}
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
)


#----------------------------------------------------------------------------

file(GLOB SOURCE_FILES
	${PROJECT_SOURCE_DIR}/main.cpp
)

 
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_FILES})


TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${NEEDED_LIBS})
