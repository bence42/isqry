#choose wisely
GXX  := g++
CLXX := clang++
CXX  := $(CLXX)

ifeq ($(CXX),clang++)
  MINGW_MODE := --target=x86_64-pc-mingw  -fuse-ld=lld
endif

CXXFLAGS   := -std=c++17 -O2 -g -Wall -Wextra #-Werror
BOOSTLIBS  := -lboost_system-mt
WINSOCKLIBS:= -lwsock32 -lws2_32
FILESYSTEMLIBS  := -lstdc++fs

HOST_LLVM := /c/llvm8
BOOST     := $(HOST_LLVM)/include/boost



all: server.exe client.exe

clean:
	rm -rf $(CURDIR)/*.exe
	rm -rf $(CURDIR)/*.pch

%.exe: %.cpp asio.hpp.pch Makefile
	$(CXX) $< -o $@ $(CXXFLAGS) -I$(BOOST) $(BOOSTLIBS) $(WINSOCKLIBS) $(FILESYSTEMLIBS) $(MINGW_MODE) -include-pch asio.hpp.pch


asio.hpp.pch: $(BOOST)\asio.hpp Makefile
	$(CXX) $< -o $@ $(CXXFLAGS) -I$(BOOST) $(MINGW_MODE)