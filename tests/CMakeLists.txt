# ============================================================================
# SEMULIKI ERP - Tests Configuration
# ============================================================================

# Vérifier que GoogleTest est disponible (optionnel)
find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
    message(STATUS "GoogleTest not found. Tests will be skipped.")
    message(STATUS "To enable tests, install: vcpkg install gtest:x64-windows")
    return()
endif()

# ============================================================================
# ENABLE TESTING
# ============================================================================

enable_testing()

# ============================================================================
# TEST SOURCES
# ============================================================================

set(TEST_SOURCES
    test_database.cpp
    test_repositories.cpp
    test_managers.cpp
    test_validators.cpp
)

# ============================================================================
# CREATE TEST EXECUTABLE
# ============================================================================

add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

# ============================================================================
# INCLUDE DIRECTORIES
# ============================================================================

target_include_directories(${PROJECT_NAME}_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# ============================================================================
# LINK LIBRARIES
# ============================================================================

target_link_libraries(${PROJECT_NAME}_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Sql
)

# ============================================================================
# REGISTER TESTS
# ============================================================================

add_test(
    NAME DatabaseTests
    COMMAND ${PROJECT_NAME}_tests
)

message(STATUS "Tests configured successfully")