APP_STL := gnustl_static
APP_ABI := armeabi-v7a
APP_CPPFLAGS += -std=c++11
NDK_TOOLCHAIN_VERSION := 4.8
# compile with debug symbols - remove on release build (otherwise the binaries are not stripped)
#APP_OPTIM := debug
