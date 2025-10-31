cmake_minimum_required(VERSION 3.21)
project(NeoPlayer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/mingw_64")

# ============ Qt 全局设置 ============
set(CMAKE_AUTOMOC ON)      # 自动运行 moc
set(CMAKE_AUTORCC ON)      # 自动处理资源文件
set(CMAKE_AUTOUIC ON)      # 自动处理 ui 文件

# ============ 输出信息 ============
message(STATUS "========================================")
message(STATUS "  MpvPlayer - Qt + libmpv player")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "========================================")

# qt6的组件
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core OpenGLWidgets)

# ============ CPM 初始化 ============
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
# CPM 缓存设置（加速重复构建）
set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_BINARY_DIR}/_cpm_packages" CACHE PATH "CPM 缓存目录")

# 集成本地预编译 libmpv（shinchiro）
set(LIBMPV_ROOT         "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmpv")
set(LIBMPV_INCLUDE_DIR  "${LIBMPV_ROOT}/include")
set(LIBMPV_LIBRARY_DIR  "${LIBMPV_ROOT}/lib")

#add_library(libmpv STATIC IMPORTED)
if(MINGW)
#  set_target_properties(libmpv PROPERTIES
#          IMPORTED_LOCATION "${LIBMPV_LIBRARY_DIR}/libmpv-2.dll"
#          IMPORTED_LOCATION "${LIBMPV_LIBRARY_DIR}/libmpv.dll.a"
#          IMPORTED_IMPLIB  "${LIBMPV_LIBRARY_DIR}/libmpv.dll.a"
#          INTERFACE_INCLUDE_DIRECTORIES "${LIBMPV_INCLUDE_DIR}"
#  )
else()  # MSVC 情况
  set_target_properties(libmpv PROPERTIES
          IMPORTED_LOCATION "${LIBMPV_LIBRARY_DIR}/libmpv.dll"
          IMPORTED_IMPLIB  "${LIBMPV_LIBRARY_DIR}/libmpv.lib"
          INTERFACE_INCLUDE_DIRECTORIES "${LIBMPV_INCLUDE_DIR}"
  )
endif()

# 用 CPM 拉取常用三方库
CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.15.3
  OPTIONS
    "SPDLOG_BUILD_SHARED OFF"
    "SPDLOG_FMT_EXTERNAL OFF"
)
CPMAddPackage(
  NAME GTest
  GITHUB_REPOSITORY google/googletest
  VERSION 1.14.0
  OPTIONS
    "INSTALL_GTEST OFF"
)
if(GTest_ADDED)
  # enable_testing()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdeprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated-declarations")

#add_subdirectory(modules/log)
if(GTest_ADDED)
  add_subdirectory(tests)
endif()
add_subdirectory(src)

## 安装规则
#install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#install(DIRECTORY assets/ DESTINATION share/${PROJECT_NAME}/assets)