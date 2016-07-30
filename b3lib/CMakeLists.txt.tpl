cmake_minimum_required (VERSION 2.8)

# 
# 使用说明
# 1. build_ost: 构建的系统平台，取值：linux|android|ios|windows
# 2. build_type: 构建的bin特性，取值：debug|release
# 3. project_name: 待构建工程的名字
# 4. 使用时需要设置SRCS、各平台的依赖库
# 
# FIXME
# 1. 在每个源码目录下面都放一个CMakeFiles.txt，这样好不好？单一的CMakeFiles文件好不好？
# 2. 在判断各平台不同的编译选项时会出现多次if-elseif-endif的语句，所以是否可以放在一起？
# 
# NOTE
# 1. 使用CMakeFiles可以选择性的编译，尽管这个文件在源码目录下了，而且可能是一些待验证的
# 	 想法，没关系，照样编译执行，IDE的话就得手动移除工程，移除之后就查看起来就不方便了，
#	 主要没有了语法高亮和引用等支持了（至少VS是这样的）。
# 


# 
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

set (default_pn "pw_test")
if (NOT DEFINED project_name)
	message ("WARN: undefined project_name, using default:${default_pn}")
	set (project_name ${default_pn})
endif()
message ("---------------------------")
message ("project name is: ${project_name}")
message ("---------------------------")

project (${project_name})

# 
# 一些通用的头文件和链接路径设置
# 
include_directories ("${PROJECT_SOURCE_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/..")
include_directories ("${PROJECT_SOURCE_DIR}/../3rd/include")

link_directories ("${PROJECT_SOURCE_DIR}/../3rd/lib")

# 设置一些公共的源码文件
set (CMN_SRC )
# 设置一些模块的源码文件
set (MOD_SRC )
# 所有需要编译的文件
set (SRCS ${LIB_CMN_SRC} ${MOD_SRC})

# 设置一些系统特有的源码文件
if (${build_ost} MATCHES "linux")
	list (APPEND SRCS ${LINUX_SRC})
elseif (${build_ost} MATCHES "windows")
	list (APPEND SRCS ${WIN32_SRC})	
elseif (${build_ost} MATCHES "android")
	list (APPEND SRCS ${ANDROID_SRC})
elseif (${build_ost} MATCHES "ios")
	list (APPEND SRCS ${IOS_SRC})
endif ()

# 在日志模块，GCC的__FILE__会使用绝对路径，所以打印出来的日志很长，可以用下面的方法解决
# 使用这里定义的__MYFILE__替代编译器宏__FILE__
foreach (f IN LISTS SRCS)
    get_filename_component (b ${f} NAME)
    set_source_files_properties (${f} PROPERTIES COMPILE_DEFINITIONS 
    							 "__MYFILE__=\"${b}\"")
endforeach()

if (${build_type} MATCHES "debug")
	# 增加debug版本的宏定义
    add_definitions ()
elseif (${build_type} MATCHES "release")
	# 增加release版本的宏定义
	add_definitions ()
endif ()

# 
# 类unix系统一般都会使用gcc工具链，所以这里增加对gcc的特殊编译选项
set (ost_nix "linux" "android" "ios")
list(FIND ost_nix ${build_ost} cur_ost)
if (NOT ${cur_ost} EQUAL -1)
    message("current build ost is ${cur_ost}")
    if (${build_type} MATCHES "debug")
        add_definitions(-g)
    elseif (${build_type} MATCHES "release")
        add_definitions(-O2)
    endif()
else (${cur_ost} NOTEQUAL -1)
    message("for windows target")
endif()

# 
# 针对各平台的一些特殊配置，如编译宏、链接目录、不同编译器的链接选项等等
if (${build_ost} MATCHES "linux")

	message("build for linux")
	include_directories ("${PROJECT_SOURCE_DIR}/../3rd/include")
    link_directories ("${PROJECT_SOURCE_DIR}/../3rd/lib/linux")
    set (LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/../bin/linux/")
	add_definitions (-std=c++11 -DLINUX)

    add_library (${project_name} STATIC ${SRCS})
    set (linux_dep_libs )
    target_link_libraries (${project_name} ${linux_dep_libs})
    install (TARGETS ${project_name} ARCHIVE DESTINATION .)

elseif (${build_ost} MATCHES "android")

	message("build for android")
	include_directories ("${PROJECT_SOURCE_DIR}/../3rd/include")
    link_directories ("${PROJECT_SOURCE_DIR}/../3rd/lib/android")
    # android cmake toolchain 会用到
    set (CMAKE_SYSTEM_NAME Android)
	set (LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/../bin/android/")
	add_definitions (-std=c++11 -DANDROID)

    add_library (${project_name} SHARED ${SRCS})
    set (android_dep_libs )
	target_link_libraries (${project_name} ${android_dep_libs})
    install (TARGETS ${project_name} LIBRARY DESTINATION .)

elseif (${build_ost} MATCHES "ios")

	message("build for ios")
	include_directories ("${PROJECT_SOURCE_DIR}/../3rd/include")	
    link_directories ("${PROJECT_SOURCE_DIR}/../3rd/lib/ios")
    # ios cmake toolchain 会用到
    set (CMAKE_SYSTEM_NAME Ios)
	set (LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/../bin/ios/")
	add_definitions (-std=c++11 -D__APPLE__)

    add_library (${project_name} STATIC ${SRCS})
    set (ios_dep_libs )
    target_link_libraries (${project_name} ${ios_dep_libs})
    install (TARGETS ${project_name} ARCHIVE DESTINATION .)

elseif (${build_ost} MATCHES "windows")

	message("build for windows")
	include_directories ("${PROJECT_SOURCE_DIR}/../3rd/include/")
	link_directories ("${PROJECT_SOURCE_DIR}/../3rd/lib/windows/${build_type}")
	set (LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/../bin/windows/")	
	
    if (${build_type} MATCHES "debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")		
    elseif(${build_type} MATCHES "release")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /O2 /Ob2 /D NDEBUG")
    endif()

	add_library (${project_name} SHARED ${SRCS})
	set (windows_dep_libs )
	target_link_libraries (${project_name} ${windows_dep_libs})
    install (TARGETS ${project_name} ARCHIVE DESTINATION .)

endif (${build_ost} MATCHES "linux")
