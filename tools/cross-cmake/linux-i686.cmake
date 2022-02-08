# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER i686-linux-gnu-gcc)
# Turn off excess precision with -mfpmath=sse -msse2, otherwise KTX compiled with bots will hang.
set(CMAKE_C_FLAGS "-m32 -mfpmath=sse -msse2")
#set(CMAKE_CXX_COMPILER x86_64-linux-gnu-g++)
#set(CMAKE_CXX_FLAGS -m32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
