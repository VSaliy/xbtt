#pragma once
#pragma warning(disable: 4244 4554 4800)

#define NOMINMAX

#include <asio.hpp>
#include <cassert>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef WIN32
#include <io.h>

#define atoll _atoi64
#else
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cstdio>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define O_BINARY 0
#define _lseeki64 lseek
#endif
#include "bt_misc.h"
#include "bvalue.h"
#include "virtual_binary.h"

typedef unsigned char byte;
