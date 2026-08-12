cmake_minimum_required(VERSION 3.25...4.3)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Set project name and languge.
project(mvdsv C)


######################################################################################################

# Checkout shared qwprot repository
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()


######################################################################################################

# Set where sources located.
set(DIR_SRC "src")

# Add sources
set(SRC_COMMON
	"${DIR_SRC}/bothtools.c"
	"${DIR_SRC}/build.c"
	"${DIR_SRC}/cmd.c"
	"${DIR_SRC}/cmodel.c"
	"${DIR_SRC}/common.c"
	"${DIR_SRC}/crc.c"
	"${DIR_SRC}/cvar.c"
	"${DIR_SRC}/fs.c"
	"${DIR_SRC}/hash.c"
	"${DIR_SRC}/mathlib.c"
	"${DIR_SRC}/md4.c"
	"${DIR_SRC}/net.c"
	"${DIR_SRC}/net_chan.c"
	"${DIR_SRC}/pmove.c"
	"${DIR_SRC}/pmovetst.c"
	"${DIR_SRC}/pr2_cmds.c"
	"${DIR_SRC}/pr2_edict.c"
	"${DIR_SRC}/pr2_exec.c"
	"${DIR_SRC}/vm.c"
	"${DIR_SRC}/vm_interpreted.c"
	"${DIR_SRC}/vm_x86.c"
	"${DIR_SRC}/pr_cmds.c"
	"${DIR_SRC}/pr_edict.c"
	"${DIR_SRC}/pr_exec.c"
	"${DIR_SRC}/sha1.c"
	"${DIR_SRC}/sha3.c"
	"${DIR_SRC}/sv_broadcast.c"
	"${DIR_SRC}/sv_ccmds.c"
	"${DIR_SRC}/sv_demo.c"
	"${DIR_SRC}/sv_demo_misc.c"
	"${DIR_SRC}/sv_demo_qtv.c"
	"${DIR_SRC}/sv_ents.c"
	"${DIR_SRC}/sv_init.c"
	"${DIR_SRC}/sv_login.c"
	"${DIR_SRC}/sv_main.c"
	"${DIR_SRC}/sv_master.c"
	"${DIR_SRC}/sv_mod_frags.c"
	"${DIR_SRC}/sv_move.c"
	"${DIR_SRC}/sv_nchan.c"
	"${DIR_SRC}/sv_phys.c"
	"${DIR_SRC}/sv_save.c"
	"${DIR_SRC}/sv_send.c"
	"${DIR_SRC}/sv_user.c"
	"${DIR_SRC}/vfs_os.c"
	"${DIR_SRC}/vfs_pak.c"
	"${DIR_SRC}/sv_world.c"
	"${DIR_SRC}/zone.c"
	)

# Check build target, and included sources
if(UNIX)
	list(APPEND SRC_COMMON
		"${DIR_SRC}/sv_sys_unix.c"
	)
	# Check for curl, and include sources and libs, if found
	find_package(CURL)
	if(NOT CURL_FOUND)
		message(STATUS "Curl library not found")
	else()
		list(APPEND SRC_COMMON
			"${DIR_SRC}/central.c"
		)
	endif()
else()
	list(APPEND SRC_COMMON
		"${DIR_SRC}/sv_sys_win.c"
		"${DIR_SRC}/sv_windows.c"
		"${DIR_SRC}/winquake.rc"
	)
endif()

######################################################################################################

# Check for PCRE. if found, include it; if not found, use bundled PCRE.
find_library(PCRE_LIBRARIES pcre)
if(PCRE_LIBRARIES)
	set(PCRE_FOUND 1)
	find_path(PCRE_INCLUDE_DIR pcre.h)
endif(PCRE_LIBRARIES)

if(NOT PCRE_FOUND)
	message(STATUS "PCRE library not found. Using bundled PCRE instead.")
	list(APPEND SRC_COMMON
	    "${DIR_SRC}/pcre/get.c"
	    "${DIR_SRC}/pcre/pcre.c"
	)
else()
	message(STATUS "Found PCRE: ${PCRE_LIBRARIES}")
endif()

######################################################################################################

# Set base compiler flags
set(CFLAGS -Wall)
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	# struct-padding warning (C4820)
    # 'symbol' is not defined as a preprocessor macro (C4668)
    # selected for inline (C4711)
    # spectre mitigation (C5045)
    # unreferenced parameter (C4100)
    # not inlined (C4710)
	list(APPEND CFLAGS /wd4820 /wd4668 /wd4711 /wd5045 /wd4100 /wd4710)
endif()
set(LFLAGS)

find_package(Threads REQUIRED)


######################################################################################################
# Mark the executable as a non-console application
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	set(TARGET_TYPE WIN32)
endif()
add_executable(${PROJECT_NAME} ${TARGET_TYPE} ${SRC_COMMON})
set_target_properties(${PROJECT_NAME}
	PROPERTIES #PREFIX "" # Strip lib prefix.
	C_VISIBILITY_PRESET hidden # Hide all symbols unless excplicitly marked to export.
	)


######################################################################################################

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_SRC}/qwprot/src)
if(PCRE_INCLUDE_DIR)
	target_include_directories(${PROJECT_NAME} PRIVATE ${PCRE_INCLUDE_DIR})
endif()


######################################################################################################

# Check build target, and included sources and libs
if(UNIX)
	target_link_libraries(${PROJECT_NAME} m)
	target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
else()
	target_link_libraries(${PROJECT_NAME} ws2_32)
	target_link_libraries(${PROJECT_NAME} winmm)
	if(NOT CMAKE_C_COMPILER_ID STREQUAL "MSVC")
		target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc)
	endif()
endif()


######################################################################################################

# Set defines for the build
target_compile_definitions(${PROJECT_NAME} PRIVATE SERVERONLY)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_PR2)
target_compile_definitions(${PROJECT_NAME} PRIVATE MVD_PEXT1_SERVERSIDEWEAPON)
target_compile_definitions(${PROJECT_NAME} PRIVATE MVD_PEXT1_SERVERSIDEWEAPON2)
target_compile_definitions(${PROJECT_NAME} PRIVATE FTE_PEXT2_VOICECHAT)
target_compile_definitions(${PROJECT_NAME} PRIVATE FTE_PEXT_ACCURATETIMINGS=0x40)

if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
	target_compile_definitions(${PROJECT_NAME} PRIVATE __BIG_ENDIAN__Q__)
	message(STATUS "BIG_ENDIAN")
else()
	target_compile_definitions(${PROJECT_NAME} PRIVATE __LITTLE_ENDIAN__Q__)
	message(STATUS "LITTLE_ENDIAN")
endif()

if(CURL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PRIVATE WWW_INTEGRATION)
	target_link_libraries(${PROJECT_NAME} CURL::libcurl)
endif()

if(PCRE_FOUND)
	target_link_libraries(${PROJECT_NAME} ${PCRE_LIBRARIES})
endif()


######################################################################################################

# Assign compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})

target_link_libraries(${PROJECT_NAME} Threads::Threads)


######################################################################################################
