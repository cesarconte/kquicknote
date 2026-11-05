add_library(kquicknote_core STATIC
    quicknotecontroller.cpp
    quicknotecontroller.h
    resources.qrc
)

target_include_directories(kquicknote_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(kquicknote_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    KF6::CoreAddons
    KF6::I18n
)

add_executable(kquicknote
    main.cpp
)

target_link_libraries(kquicknote PRIVATE
    kquicknote_core
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    KF6::GlobalAccel
    KF6::I18n
    KF6::I18nQml
)

install(TARGETS kquicknote DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES org.cesar.kquicknote.desktop DESTINATION ${KDE_INSTALL_APPDIR})
