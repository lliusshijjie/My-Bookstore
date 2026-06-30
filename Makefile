CXX ?= g++

CXXFLAGS += -std=c++17 -Wall -Wextra

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g -O0
else
    CXXFLAGS += -O2
endif

SRCS = src/core/main.cpp src/timer/lst_timer.cpp src/net/http/http_conn.cpp \
       src/db/user_cache.cpp src/log/log.cpp src/db/sql_connection_pool.cpp \
       src/core/webserver.cpp src/core/config.cpp

.PHONY: server clean

server: $(SRCS)
	$(CXX) -o server $^ $(CXXFLAGS) -lpthread -lmysqlclient

clean:
	rm -f server
