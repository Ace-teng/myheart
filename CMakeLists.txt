cmake_minimum_required(VERSION 3.16)

project(heart LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 并添加所需模块（Core 和 Network）
find_package(Qt6 REQUIRED COMPONENTS Core Network)

# 设备端可执行程序
add_executable(heart_device
    testdevice.cpp
    heartbeatdevice.h
    heartbeatdevice.cpp
)
target_link_libraries(heart_device PRIVATE Qt6::Core Qt6::Network)

# 服务器端可执行程序
add_executable(heart_server
    testserver.cpp
    heartbeatserver.h
    heartbeatserver.cpp
)
target_link_libraries(heart_server PRIVATE Qt6::Core Qt6::Network)

# 客户端可执行程序
add_executable(heart_client
    testclient.cpp
    heartbeatclient.h
    heartbeatclient.cpp
)
target_link_libraries(heart_client PRIVATE Qt6::Core Qt6::Network)

include(GNUInstallDirs)
# 安装配置（可选）
install(TARGETS heart_device heart_server heart_client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
