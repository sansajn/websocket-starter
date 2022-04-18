# dependencies: libsoup2.4-dev
cpp17 = Environment(
    CCFLAGS=['-Wall', '-Wextra', '-O0', '-ggdb3'],
    CXXFLAGS=['-std=c++17'])

cpp17.ParseConfig('pkg-config --cflags --libs libsoup-2.4')

wsch_objs = cpp17.Object(['websocket.cpp', 'glib_event_loop.cpp', 'echo_server.cpp'])

# unit tests
cpp17.Program(['test.cpp', wsch_objs])

# samples
cpp17.Program(['eserv.cpp', wsch_objs])
