cmake_minimum_required(VERSION 3.15)

#! Check every comment after the "#!"

#! CHANGE YOUR PROJECT NAME
#  It is used as your project's main executable name. 
set(PROJECT_NAME nyshporka)
set(MANAGER_NAME task_manager)
set(CRAWLER_NAME nysh_crawler)
set(SEARCHER_NAME nysh_search)
project(${PROJECT_NAME} C CXX) # project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_CXX_STANDARD 17)

##########################################################
# User configurable options of the template
##########################################################
# Note: symbols like WARNINGS_AS_ERRORS in configuration are intentionally variables
# and not CMake options --using options creates too much problem for students.

#! It is a good practice to set "WARNINGS_AS_ERRORS" ON,
#  but sometimes it creates too much trouble, so default is OFF.
set(WARNINGS_AS_ERRORS OFF)

#! Always use PVS Studio while developing. 
set(ENABLE_PVS_STUDIO OFF)

#! Select appropriate sanitizers.
#  Definitely enable sanitizers while developing.
#  Disable it for the production builds and before submitting for grading.
#  Only one of Memory (MSAN), Address (ASAN), or Thread (TSan)
#  sanitizers is applicable at the time -- the first defined.

#! UndefinedBehaviorSanitizer (UBSan)
#  Info: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
set(ENABLE_UBSan OFF)
#! AddressSanitizer -- detects use after free or after scope exit,
#  memory overflows and leaks.  
#  Info: https://github.com/google/sanitizers/wiki/AddressSanitizer
set(ENABLE_ASAN OFF)
#! ThreadSanitizer -- detects data races.
set(ENABLE_TSan OFF)
#! MemorySanitizer detects uninitialized memory reads 
#  Info: https://github.com/google/sanitizers/wiki/MemorySanitizer
set(ENABLE_MSAN OFF)


#! Be default -- build release version if not specified otherwise.
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

# Warnings as errors should be imported here -- do not move this line
include(cmake/CompilerWarnings.cmake)

##########################################################
# Project files, packages, libraries and so on
##########################################################

#! Project main executable source compilation
add_executable(
		${MANAGER_NAME}
		src/task_manager.cpp
		src/config_parser.cpp include/config_parser.h
		include/exceptions.h
		include/timed_pqueue.h)

add_executable(
		${SEARCHER_NAME} src/nysh_search.cpp
		include/db_connector.h src/db_connector.cpp
)

add_executable(
		${CRAWLER_NAME} src/crawler.cpp src/BSONPage.cpp
		include/BSONPage.h include/exceptions.h
		include/db_connector.h src/db_connector.cpp
)


#! Put path to your project headers
target_include_directories(${MANAGER_NAME} PRIVATE include)
target_include_directories(${SEARCHER_NAME} PRIVATE include)
target_include_directories(${CRAWLER_NAME} PRIVATE include)


#! Add external packages
# options_parser requires boost::program_options library
find_package(Boost 1.71.0 COMPONENTS program_options system locale json REQUIRED)
target_include_directories(${MANAGER_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${MANAGER_NAME} Boost::program_options Boost::system)
target_link_libraries(${CRAWLER_NAME} Boost::json)
set(Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
set(Boost_USE_MULTITHREAD ON)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
		GIT_TAG 1.10.2) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)
target_link_libraries(${MANAGER_NAME} cpr::cpr)
target_link_libraries(${CRAWLER_NAME} cpr::cpr)

find_package(mongocxx REQUIRED)
target_include_directories(${MANAGER_NAME} PRIVATE ${LIBMONGOCXX_INCLUDE_DIRS})
target_include_directories(${SEARCHER_NAME} PRIVATE ${LIBMONGOCXX_INCLUDE_DIRS})
target_link_libraries(${MANAGER_NAME} mongo::mongocxx_shared)
target_link_libraries(${SEARCHER_NAME} mongo::mongocxx_shared)
target_link_libraries(${CRAWLER_NAME} mongo::mongocxx_shared)

find_package(bsoncxx REQUIRED)
target_include_directories(${MANAGER_NAME} PRIVATE ${LIBBSONCXX_INCLUDE_DIRS})
target_include_directories(${SEARCHER_NAME} PRIVATE ${LIBBSONCXX_INCLUDE_DIRS})
target_include_directories(${CRAWLER_NAME} PRIVATE ${LIBBSONCXX_INCLUDE_DIRS})
target_link_libraries(${MANAGER_NAME} mongo::bsoncxx_shared)
target_link_libraries(${SEARCHER_NAME} mongo::bsoncxx_shared)
target_link_libraries(${CRAWLER_NAME} mongo::bsoncxx_shared)

find_package(TBB)
if(TBB_FOUND)
	message(STATUS "Found TBB: ${TBB_INCLUDE_DIR}")

	if (NOT TBB_FIND_QUIETLY)
		message(STATUS "Found TBB: ${TBB_INCLUDE_DIRS}")
	endif()
endif()
target_link_libraries(${CRAWLER_NAME} TBB::tbb)
target_link_libraries(${MANAGER_NAME} TBB::tbb)

##########################################################
# Fixed CMakeLists.txt part 
##########################################################

INSTALL(PROGRAMS
		$<TARGET_FILE:${MANAGER_NAME}>	# ${CMAKE_CURRENT_BINARY_DIR}/${MANAGER_NAME}
		DESTINATION bin)

# Define ALL_TARGETS variable to use in PVS and Sanitizers
set(ALL_TARGETS ${MANAGER_NAME} ${SEARCHER_NAME} ${CRAWLER_NAME})

# Include CMake setup
include(cmake/main-config.cmake)
