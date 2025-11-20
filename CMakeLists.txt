cmake_minimum_required(VERSION 3.10)
project(3d-entity-manager VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgGA osgViewer osgUtil)
find_package(osgEarth REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OSG_INCLUDE_DIR}
    ${OSGEARTH_INCLUDE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
)

# Set automoc for Qt
set(CMAKE_AUTOMOC ON)

# Source files
set(SOURCES
    src/object3d.cpp
    src/sensorvolume.cpp
    src/trackline.cpp
    src/ShipModel.cpp
    src/MissileModel.cpp
    src/EntityManager.cpp
    src/PerformanceTestManager.cpp
)

# Header files
set(HEADERS
    include/LodConfig.h
    include/AttitudeUtils.h
    include/object3d.h
    include/sensorvolume.h
    include/trackline.h
    include/ShipModel.h
    include/MissileModel.h
    include/EntityManager.h
    include/DdsDataSimulator.h
    include/PerformanceTestManager.h
)

# Create library
add_library(3d-entity-manager STATIC ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(3d-entity-manager
    Qt5::Core
    ${OPENSCENEGRAPH_LIBRARIES}
    ${OSGEARTH_LIBRARY}
)

# Install targets
install(TARGETS 3d-entity-manager
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include/3d-entity-manager
)

# Optional: Build example
option(BUILD_EXAMPLES "Build example programs" OFF)

if(BUILD_EXAMPLES)
    add_executable(IntegrationExample examples/IntegrationExample.cpp)
    target_link_libraries(IntegrationExample
        3d-entity-manager
        Qt5::Core
        ${OPENSCENEGRAPH_LIBRARIES}
        ${OSGEARTH_LIBRARY}
    )
    
    add_executable(PerformanceTestExample examples/PerformanceTestExample.cpp)
    target_link_libraries(PerformanceTestExample
        3d-entity-manager
        Qt5::Core
        ${OPENSCENEGRAPH_LIBRARIES}
        ${OSGEARTH_LIBRARY}
    )
endif()

# Print configuration
message(STATUS "")
message(STATUS "3D Entity Manager Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "")
