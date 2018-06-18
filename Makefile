CC=g++
CXX=g++
RANLIB=ranlib

LIBOBJ=$(LIBSRC:.cpp=.o)

CXXFLAGS = -Wall -std=c++11 -g $(INCS)


TAR=tar
TARFLAGS=-cvf
TARNAME=ex1.tar
TARSRCS=$(LIBSRC) Makefile README

all: makeserver makeclient

makeserver:
	g++ -std=c++11 -Wall whatsappServer.cpp whatsappio.h whatsappio.cpp ex4_utils.cpp ex4_utils.h -o whatsappServer

makeclient:
	g++ -std=c++11 -Wall whatsappClient.cpp whatsappio.h whatsappio.cpp ex4_utils.cpp ex4_utils.h -o whatsappClient

# DO NOT DELETE

whatsappServer.o: /usr/include/netdb.h /usr/include/features.h
whatsappServer.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
whatsappServer.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
whatsappServer.o: /usr/include/netinet/in.h /usr/include/stdint.h
whatsappServer.o: /usr/include/bits/wchar.h /usr/include/sys/socket.h
whatsappServer.o: /usr/include/sys/uio.h /usr/include/sys/types.h
whatsappServer.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
whatsappServer.o: /usr/include/time.h /usr/include/endian.h
whatsappServer.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
whatsappServer.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
whatsappServer.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
whatsappServer.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
whatsappServer.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/uio.h
whatsappServer.o: /usr/include/bits/socket.h /usr/include/bits/socket_type.h
whatsappServer.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
whatsappServer.o: /usr/include/asm-generic/socket.h
whatsappServer.o: /usr/include/asm/sockios.h
whatsappServer.o: /usr/include/asm-generic/sockios.h /usr/include/bits/in.h
whatsappServer.o: /usr/include/rpc/netdb.h /usr/include/bits/netdb.h
whatsappServer.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
whatsappServer.o: /usr/include/bits/environments.h
whatsappServer.o: /usr/include/bits/confname.h /usr/include/getopt.h
whatsappio.o: whatsappio.h
