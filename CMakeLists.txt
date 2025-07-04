cmake_minimum_required(VERSION 3.10)

# 项目名称
project(SendMultipartRequest)

# C++17标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 所有源文件
set(SOURCES
    sendMultipartRequest.cpp      # ⬅️ 建议把 sendMultipartRequest 单独放在这里
    MultipartRequestBuilder.cpp
    FileUtil.cpp
    EIClassBuilder.cpp
    EcodeUtil.cpp
    JsonParser.cpp
)

# 包含头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    /usr/include
    /usr/local/include
    /ixtop/ixtop4/include
    /BSMesWare/BMBKM0/Include
)

# 链接目录
link_directories(
    /BSMesWare/BMBKM0/Build/ixtdb2/Lib
    /usr/lib64
    /home/db2inst1/sqllib/lib
    /ixtop/ixtop4/lib
)

# 创建共享库
add_library(SendMultipartRequest SHARED ${SOURCES})

# 链接外部库
target_link_libraries(SendMultipartRequest
    bm2data
    bm2db
    bm2ei
    bm2core
    bm2log
    bm2cache
    bm2model
    crypto
)

# 可选: 安装规则
# install(TARGETS SendM
