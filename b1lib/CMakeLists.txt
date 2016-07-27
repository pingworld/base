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
	add_definitions(-g)
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

	set(WIN32_SRC src/b1_mq.cc src/b1_mq.h
				  src/b1_log.cc src/b1_log.h
				  src/win32/win32.cc src/win32/win32.h 
				  src/win32/defs.cc src/win32/dlfcn.cc src/win32/config.h src/win32/dlfcn.h 
				  src/b1_thrdque.cc src/b1_thrdque.h)
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

	set(SRC src/b1_mq.cc src/b1_mq.h 
			src/b1_thrdque.cc src/b1_thrdque.h 
			src/b1_utils.cc src/b1_utils.h 
	        src/b1_log.cc src/b1_log.h 
			src/b1_socket.cc src/b1_socket.h
			src/b1_string.cc src/b1_string.h
			src/b1_redis_client.cc src/b1_redis_client.h)

	set(UT_MQ_SRC ${SRC} test/ut_b1_mq.cc)
	set(UT_TQ_SRC ${SRC} test/ut_b1_thrdque.cc)
	set(UT_LOG_SRC ${SRC} test/ut_b1_log.cc)
	set(UT_SOCK_SRC ${SRC} test/ut_b1_socket.cc)
	set(UT_STRING_SRC ${SRC} test/ut_b1_string.cc)
	set(UT_B1LIB_SRC ${SRC} test/ut_b1_lib.cc)
	set(UT_B1REDCLI_SRC ${SRC} test/ut_b1_redis_client.cc)
	set(UT_SPDLOG_SRC test/ut_spdlog.cc)
	set(UT_B1UTILS_SRC test/ut_b1_utils.cc)

	# for output source files name without path name
	foreach(f IN LISTS SRC UT_MQ_SRC UT_TQ_SRC UT_LOG_SRC UT_B1LIB_SRC UT_B1REDCLI_SRC)
	    get_filename_component(b ${f} NAME)
	    set_source_files_properties(${f} PROPERTIES COMPILE_DEFINITIONS "__MYFILE__=\"${b}\"")
	endforeach()

	add_library(b1 STATIC ${SRC})
	target_link_libraries (b1 dl event_core pthread hiredis)

	add_executable(ut_mq ${UT_MQ_SRC})
	add_executable(ut_tq ${UT_TQ_SRC})
	add_executable(ut_log ${UT_LOG_SRC})
	add_executable(ut_sock ${UT_SOCK_SRC})
	add_executable(ut_string ${UT_STRING_SRC})
	add_executable(ut_b1lib ${UT_B1LIB_SRC})
	add_executable(ut_b1_redis_cli ${UT_B1REDCLI_SRC})
	add_executable(ut_spdlog ${UT_SPDLOG_SRC})
	add_executable(ut_b1_utils ${UT_B1UTILS_SRC})

	target_link_libraries(ut_mq b1 gtest)
	target_link_libraries(ut_tq b1 gtest)
	target_link_libraries(ut_log b1 gtest)
	target_link_libraries(ut_sock b1 gtest)
	target_link_libraries(ut_string b1 gtest)
	target_link_libraries(ut_b1lib b1 gtest)
	target_link_libraries(ut_b1_redis_cli b1 gtest)
	target_link_libraries(ut_spdlog pthread gtest)
	target_link_libraries(ut_b1_utils b1 gtest)
endif(${build_ost} MATCHES "windows")

