# ============ 源文件列表 ============
set(APP_SOURCES
    main.cpp
    MainWindow.cpp
    MpvGLWidget.cpp
)

set(APP_RESOURCES
    # resources.qrc
)

# ============ 创建可执行文件 ============
qt_add_executable(${CMAKE_PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_RESOURCES}
)

# 包含目录
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/import
        ${LIBMPV_INCLUDE_DIR}
)

# 链接所有依赖库
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    spdlog::spdlog
    ${LIBMPV_LIBRARY_DIR}/libmpv.dll.a
)

# [重要] Windows特定配置
if(WIN32)
    # 告诉编译器程序是窗口应用
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)

    # 将 mpv-2.dll 复制到可执行文件旁边
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBMPV_LIBRARY_DIR}/libmpv-2.dll"
        $<TARGET_FILE_DIR:NeoPlayer>
    )
endif()

## 安装规则
#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
