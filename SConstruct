# dependencies: libsoup2.4-dev
AddOption('--test-coverage', action='store_true', dest='test_coverage', help='enable test coverage analyze (with gcov)', default=False)

cpp17 = Environment(
	CCFLAGS=['-Wall', '-Wextra', '-O0', '-ggdb3'],
	CXXFLAGS=['-std=c++17'])

cpp17.ParseConfig('pkg-config --cflags --libs libsoup-2.4')

if GetOption('test_coverage'):
	# see https://gcc.gnu.org/onlinedocs/gcc-10.1.0/gcc/Instrumentation-Options.html
	cpp17.Append(CXXFLAGS = ['-fprofile-arcs', '-ftest-coverage'],
		LIBS = ['gcov'])


common_objs = cpp17.Object(['websocket.cpp', 'glib_event_loop.cpp', 'echo_server.cpp'])

# unit tests
cpp17.Program(['test.cpp', common_objs])

# samples
cpp17.Program(['eserv.cpp', common_objs])
