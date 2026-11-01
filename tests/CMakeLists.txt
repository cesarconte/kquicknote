find_package(Qt6Test ${QT_MIN_VERSION} REQUIRED)

add_executable(test_kquicknote test_kquicknote.cpp)
target_compile_definitions(test_kquicknote PRIVATE KQUICKNOTE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(test_kquicknote PRIVATE
    kquicknote_core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Test
)

add_test(NAME test_kquicknote COMMAND test_kquicknote)
