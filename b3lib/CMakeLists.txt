cmake_minimum_required(VERSION 2.8)

project(b1)

# 说明
# 1. build_type: debug|release
# 2. build_ost: linux|windows|android|ios

if(NOT build_type)
	set(build_type "debug")
endif(NOT build_type)

if(NOT build_ost)
	set(build_ost "linux")
endif(NOT build_ost)

message(${build_ost})
message(${build_type})

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/../3rd/inc)

if(${build_type} MATCHES "debug")
	message("debug")
	add_definitions(-g -DDEBUG)
	if(${build_ost} MATCHES "windows")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	endif(${build_ost} MATCHES "windows")
elseif(${build_type} MATCHES "release")
	message("release")
	add_definitions(-O2)
	if(${build_ost} MATCHES "windows")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	endif(${build_ost} MATCHES "windows")
endif(${build_type} MATCHES "debug")

if(${build_ost} MATCHES "windows")
	message("building for windows")
	include_directories(${PROJECT_SOURCE_DIR}/../3rd/inc/libevent/windows)
	include_directories(${PROJECT_SOURCE_DIR}/../3rd/inc/pthread)
	link_directories(${PROJECT_SOURCE_DIR}/../3rd/lib/windows)
	add_definitions(-D_TIMESPEC_DEFINED -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)	

	set(WIN32_SRC b3_log.cc)
	set(UT_TQ_SRC_WIN32 ${WIN32_SRC} test/ut_b1_thrdque.cc)

	# for output source files name without path name
	foreach(f IN LISTS WIN32_SRC UT_TQ_SRC_WIN32)
	    get_filename_component(b ${f} NAME)
	    set_source_files_properties(${f} PROPERTIES COMPILE_DEFINITIONS "__MYFILE__=\"${b}\"")
	endforeach()

	add_library(b1_win32 STATIC ${WIN32_SRC})
	target_link_libraries (b1_win32 libevent_core.lib pthreadVC2.lib Ws2_32.lib)

	add_executable(ut_tq_win32 ${UT_TQ_SRC_WIN32})
	target_link_libraries(ut_tq_win32 b1_win32 gtest)

else(${build_ost} MATCHES "linux")
	message("building for linux")
	include_directories(${PROJECT_SOURCE_DIR}/../3rd/inc/libevent/nix)
	link_directories(${PROJECT_SOURCE_DIR}/../3rd/lib/linux)
	add_definitions(-DLINUX -std=c++11)

	set(SRC b3_log.cc b3_msg.cc b3_mq.cc b3_evloop.cc)

	set(UT_LOG_SRC ${SRC} ut_b3_log.cc)
	set(UT_MQ_SRC ${SRC} ut_b3_mq.cc)
	set(UT_EVL_SRC ${SRC} ut_b3_evloop.cc)

	# for output source files name without path name
	foreach(f IN LISTS SRC UT_MQ_SRC UT_EVL_SRC UT_TQ_SRC UT_LOG_SRC UT_B1LIB_SRC)
	    get_filename_component(b ${f} NAME)
	    set_source_files_properties(${f} PROPERTIES COMPILE_DEFINITIONS "__MYFILE__=\"${b}\"")
	endforeach()

	add_library(b3 STATIC ${SRC})
	target_link_libraries (b3 dl event_core pthread hiredis)

	add_executable(ut_log ${UT_LOG_SRC})
	add_executable(ut_mq ${UT_MQ_SRC})
	add_executable(ut_evloop ${UT_EVL_SRC})

	target_link_libraries(ut_log b3 gtest)
	target_link_libraries(ut_mq b3 gtest)
	target_link_libraries(ut_evloop b3 gtest)
endif(${build_ost} MATCHES "windows")

