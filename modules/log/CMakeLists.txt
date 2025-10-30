set(MODULE_NAME "log")

# ============ 创建库 ============
qt_add_library(${MODULE_NAME} STATIC
        log.cpp
)

# 设置包含目录
target_include_directories(${MODULE_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
)

# 链接所有依赖库
target_link_libraries(${MODULE_NAME} PUBLIC
        Qt6::Core
        spdlog::spdlog
)