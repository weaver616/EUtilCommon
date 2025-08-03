cmake_minimum_required(VERSION 3.10)

# 项目名称
project(EUTIL)

# 使用与框架相同的C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加严格编译选项 (与框架一致)
if(UNIX AND NOT APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z defs -pthread -Wwrite-strings")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# 使用环境变量定义基础路径
if(DEFINED ENV{BM2_BASE_DIR})
    set(BM2_BASE_DIR $ENV{BM2_BASE_DIR})
    message(STATUS "Using BM2_BASE_DIR from environment: ${BM2_BASE_DIR}")
else()
    message(WARNING "BM2_BASE_DIR environment not set")
endif()

# 设置数据库和中间件类型 (与框架一致)
set(IPLAT4C_MIDDLEWARE_TYPE "ixt")
set(IPLAT4C_DATABASE_TYPE "db2")
set(IPLAT4C_BUILD_TYPE "${IPLAT4C_MIDDLEWARE_TYPE}${IPLAT4C_DATABASE_TYPE}")

# 所有源文件
set(SOURCES
    sendMultipartRequest.cpp
    multipartRequestBuilder.cpp
    FileUtil.cpp
    EcodeUtil.cpp
    EIClassBuilder.cpp     
    JsonParser.cpp
    jsonRequestBuilder.cpp
)

# 包含头文件目录 (添加框架所需路径)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${BM2_BASE_DIR}/Include
    $ENV{DB2_HOME}/include
    $ENV{IMTOP_HOME}/include
    /usr/include
    /usr/local/include
)

# 链接目录 (添加框架所需路径)
link_directories(
    ${BM2_BASE_DIR}/Build/ixtdb2/Lib
    $ENV{DB2_HOME}/lib
    $ENV{IMTOP_HOME}/lib
    $ENV{IMTOP_HOME}/px/lib
    /usr/lib64
    /home/db2inst1/sqllib/lib
    /usr/local/lib64
)


add_library(EUTIL SHARED ${SOURCES})

# 链接外部库 (添加框架所需库)
target_link_libraries(EUTIL
    bm2data
    bm2db
    bm2ei
    bm2core
    bm2log
    bm2cache
    bm2model
    bm2trsc
    crypto
    dl
    pthread
    db2
    jsoncpp
)


# jsoncpp --
# 查找 jsoncpp 包
# find_package(JsonCpp REQUIRED)

# # 检查找到的版本
# message(STATUS "JsonCpp version: ${JsonCpp_VERSION}")
#--

# 添加框架的宏定义
# target_compile_definitions(EUTIL PRIVATE
#     _NOT_EMBEDDED_
#     _IMTOP_
#     _DB2_
# )

# 设置运行时库路径
if(UNIX AND NOT APPLE)
    set_target_properties(EUTIL PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()


# 打印 JSONCPP 信息用于调试
message(STATUS "JSONCPP found: ${JSONCPP_VERSION}")
message(STATUS "  Include dirs: ${JSONCPP_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${JSONCPP_LIBRARIES}")