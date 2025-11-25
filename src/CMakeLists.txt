add_library(DragonData STATIC
        DragonData.cpp
        DragonData.h
)

target_include_directories(DragonData
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(DragonData
        PUBLIC
        Boost::locale
)

add_executable(DaragonData_Test
        DragonDataTest.cpp
)

target_link_libraries(
        DaragonData_Test
        PRIVATE
        DragonData
)

find_package(gtest 1.17.0 REQUIRED)

enable_testing()
include_directories(GoogleTest::gtest GoogleTest::gmock)

# 添加基于 GTest 的测试可执行文件
add_executable(DragonDataGTest
        DragonData_gtest.cpp
)

target_link_libraries(DragonDataGTest
        PRIVATE
        DragonData
        GTest::gtest_main
)

include(GoogleTest)
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
gtest_discover_tests(DragonDataGTest)
