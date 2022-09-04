cmake_minimum_required(VERSION 3.9.0)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Set project name and languge.
project(mvdsv C)


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
	"${DIR_SRC}/world.c"
	"${DIR_SRC}/zone.c"
	)

# Check build target, and included sources
if(UNIX)
	list(APPEND SRC_COMMON
	    "${DIR_SRC}/sv_sys_unix.c"
	)
else()
	list(APPEND SRC_COMMON
	    "${DIR_SRC}/sv_sys_win.c"
	    "${DIR_SRC}/sv_windows.c"
	    "${DIR_SRC}/winquake.rc"
	)
endif()


######################################################################################################

# Check for curl, and include sources and libs, if found
find_package(CURL)
if(NOT CURL_FOUND)
	message(STATUS "Curl library not found")
else()
	list(APPEND SRC_COMMON
	    "${DIR_SRC}/central.c"
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
set(LFLAGS)


######################################################################################################

# Set target
add_executable(${PROJECT_NAME} ${SRC_COMMON})
set_target_properties(${PROJECT_NAME}
	PROPERTIES #PREFIX "" # Strip lib prefix.
	C_VISIBILITY_PRESET hidden # Hide all symbols unless excplicitly marked to export.
	)


######################################################################################################

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PCRE_INCLUDE_DIR})


######################################################################################################

# Check build target, and included sources and libs
if(UNIX)
	target_link_libraries(${PROJECT_NAME} m)
if(NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
	target_link_libraries(${PROJECT_NAME} dl)
endif()
else()
	target_link_libraries(${PROJECT_NAME} ws2_32)
	target_link_libraries(${PROJECT_NAME} winmm)
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc")
endif()


######################################################################################################

# Set defines for the build
target_compile_definitions(${PROJECT_NAME} PRIVATE SERVERONLY)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_PR2)

include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	target_compile_definitions(${PROJECT_NAME} __BIG_ENDIAN__Q__)
	message(STATUS "BIG_ENDIAN")
else()
	target_compile_definitions(${PROJECT_NAME} PRIVATE __LITTLE_ENDIAN__Q__)
	message(STATUS "LITTLE_ENDIAN")
endif()

if(CURL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PRIVATE WWW_INTEGRATION)
	target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
endif()

if(PCRE_FOUND)
	target_link_libraries(${PROJECT_NAME} ${PCRE_LIBRARIES})
endif()


######################################################################################################

# Assign compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})

target_link_libraries(${PROJECT_NAME} -lpthread)


######################################################################################################
