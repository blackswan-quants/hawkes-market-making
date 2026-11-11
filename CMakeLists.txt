cmake_minimum_required(VERSION 3.16)

project(hawkes_market_making LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Eigen3 REQUIRED NO_MODULE)

option(HMM_BUILD_TESTS "Build tests" ON)

add_library(hawkes_market_making
    src/main.cpp
    src/data/lobster_parser.cpp
    src/data/book_reconstructor.cpp
    src/market_making/avellaneda_stoikov.cpp
    src/market_making/glft.cpp
    src/market_making/hawkes_market_maker.cpp
    src/hawkes/hawkes_process.cpp
    src/hawkes/ogata_simulator.cpp
    src/hawkes/likelihood.cpp
    src/hawkes/mle.cpp
    src/hawkes/diagnostics.cpp
    src/control/grid.cpp
    src/control/interpolation.cpp
    src/control/hjb_solver.cpp
    src/backtest/replay_engine.cpp
    src/backtest/pnl_metrics.cpp
)

target_include_directories(hawkes_market_making
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hawkes_market_making
    PUBLIC
        Eigen3::Eigen
)

target_compile_features(hawkes_market_making PUBLIC cxx_std_17)

if(HMM_BUILD_TESTS)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        enable_testing()
        add_executable(hawkes_market_making_tests
            tests/unit/test_basic.cpp
        )
        target_include_directories(hawkes_market_making_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(hawkes_market_making_tests
            PRIVATE
                hawkes_market_making
                GTest::gtest_main
        )
        add_test(NAME hawkes_market_making_tests COMMAND hawkes_market_making_tests)
    else()
        message(STATUS "GTest not found; skipping tests.")
    endif()
endif()

add_executable(hawkes_market_making_app src/main.cpp)
target_include_directories(hawkes_market_making_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(hawkes_market_making_app PRIVATE hawkes_market_making)
