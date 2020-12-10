CXX_9=g++9.1
CXX=g++
CXX_VERSION_11 = -std=c++11
CXX_VERSION_14 = -std=c++14
CXX_VERSION_17 = -std=c++17
CXXFLAGS= -fno-inline -fno-inline-small-functions -fno-default-inline
CXXFLAGS_TEST= $(CXXFLAGS) -g -O0 -fprofile-arcs -ftest-coverage

# Compiler link flags
LINKFLAGS = -lrestbed -lpthread
LINK_DIR_TEST = -L test/gtest/lib
LINKFLAGS_TEST = -lgtest -lgmock -lgtest_main -lgcov $(LINKFLAGS)

# System binaries and executables
GCOV_9 = gcov9.1
GCOV = gcov
LCOV = lcov
STATIC_ANALYSIS = cppcheck
STYLE_CHECK = cpplint

# Output directory for coverage results
COVERAGE_DIR = coverage

# Output file for coverage results
COVERAGE_RESULTS = results.coverage

# Output directory for the documentation
DOCS_DIR = docs/html

# Executable programs
PROGRAM_SERVER = hotTicket-server
PROGRAM_CLIENT = hotTicket
PROGRAM_TEST = testProgram

# Include flag for the version of gtest provided with the project
TEST_DIR = test
GTEST_INCLUDE = -I $(TEST_DIR)/gtest/include

# Include directories
INCLUDE_DIR = include
CONTROLLERS_INCLUDE_DIR = $(INCLUDE_DIR)/controllers
SERVICES_INCLUDE_DIR = $(INCLUDE_DIR)/services
ENTITIES_INCLUDE_DIR = $(INCLUDE_DIR)/entities
UTILS_INCLUDE_DIR = $(INCLUDE_DIR)/utilities
VIEWS_INCLUDE_DIR = $(INCLUDE_DIR)/views
APP_INCLUDE_DIR = $(INCLUDE_DIR)/app
MOCKS_DIR = test/mocks

# External dependencies includes
DEPENDENCIES_DIR = dependencies
JSON_INCLUDE_DIR = $(DEPENDENCIES_DIR)/nlohmann
CXXOPTS_INCLUDE_DIR = $(DEPENDENCIES_DIR)/cxxopts

# Include flags
BASE_INCLUDE = -I $(INCLUDE_DIR)
CONTROLLERS_INCLUDE = -I $(CONTROLLERS_INCLUDE_DIR)
SERVICES_INCLUDE = -I  $(SERVICES_INCLUDE_DIR)
ENTITIES_INCLUDE = -I $(ENTITIES_INCLUDE_DIR)
JSON_INCLUDE = -I $(JSON_INCLUDE_DIR)
UTILS_INCLUDE = -I $(UTILS_INCLUDE_DIR)
VIEW_INCLUDE = -I $(VIEWS_INCLUDE_DIR)
MOCKS_INCLUDE = -I $(MOCKS_DIR)
CXXOPTS_INCLUDE = -I $(CXXOPTS_INCLUDE_DIR)
APP_INCLUDE = -I $(APP_INCLUDE_DIR)

# Include flags for server program
SERVER_INCLUDES = $(BASE_INCLUDE) $(CONTROLLERS_INCLUDE) $(SERVICES_INCLUDE) $(ENTITIES_INCLUDE) $(JSON_INCLUDE) $(UTILS_INCLUDE) $(CXXOPTS_INCLUDE) $(APP_INCLUDE)

# Client includes
CLIENT_INCLUDES = $(BASE_INCLUDE) $(VIEW_INCLUDE) $(ENTITIES_INCLUDE) $(JSON_INCLUDE) $(UTILS_INCLUDE) $(CXXOPTS_INCLUDE) $(APP_INCLUDE)

# Include flags for tests
TEST_INCLUDES = $(SERVER_INCLUDES) $(MOCKS_INCLUDE)

# Source code directories
SRC_DIR = src
SRC_DIR_SERVER = src/server
SRC_DIR_CLIENT = src/client
SRC_DIR_SERVICES = src/services
SRC_DIR_ENTITIES = src/entities
SRC_DIR_CONTROLLERS = src/controllers
SRC_DIR_UTILS = src/utilities

# All .cpp files, for style checks
ALL_CPP_FILES := \
	$(wildcard src/*/*.cpp) \
	$(wildcard test/*.cpp) \

# .cpp files for server program
SERVER_CPP_FILES := \
	$(wildcard $(SRC_DIR_SERVER)/*.cpp) \
	$(wildcard $(SRC_DIR_SERVICES)/*.cpp) \
	$(wildcard $(SRC_DIR_CONTROLLERS)/*.cpp) \
	$(wildcard $(SRC_DIR_ENTITIES)/*.cpp) \
	$(wildcard $(SRC_DIR_UTILS)/*.cpp) \

# .cpp files for client program
CLIENT_CPP_FILES := \
	$(wildcard $(SRC_DIR_CLIENT)/*.cpp) \
	$(wildcard $(SRC_DIR_ENTITIES)/*.cpp) \
	$(wildcard $(SRC_DIR_UTILS)/*.cpp) \

# .cpp files for the test program
TEST_CPP_FILES := \
	$(wildcard $(TEST_DIR)/*.cpp) \
	$(wildcard $(SRC_DIR_SERVICES)/*.cpp) \
	$(wildcard $(SRC_DIR_CONTROLLERS)/*.cpp) \
	$(wildcard $(SRC_DIR_ENTITIES)/*.cpp) \
	$(wildcard $(SRC_DIR_UTILS)/*.cpp) \

# All header files, for style checks and documentation
ALL_HEADER_FILES := \
	$(wildcard $(INCLUDE_DIR)/*/*.h) \
	$(wildcard $(INCLUDE_DIR)/*/*.hpp) \

# Build targets
.PHONY: all
all: $(PROGRAM_SERVER) $(PROGRAM_CLIENT) $(PROGRAM_TEST) coverage docs static style

# default rule for compiling .cc to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf *~ $(SRC)/*.o $(TEST_SRC)/*.o *.gcov *.gcda *.gcno \
	$(COVERAGE_RESULTS) \
	$(PROGRAM_SERVER) \
	$(PROGRAM_TEST) \
	$(PROGRAM_CLIENT) \
	$(COVERAGE_DIR) \
	$(COVERAGE_RESULTS) \
	$(DOCS_DIR) \
	obj bin \

# clears the screen. Probably won't work on Windows
clearScreen: 
	clear

# Build the server program
server: $(PROGRAM_SERVER)

# Build the client program
client: $(PROGRAM_CLIENT)

# Build (if not alredady built) and run the server in the background
runServer: server
	./$(PROGRAM_SERVER) &

# Kills the process associated with the server program
stopServer:
	kill -9 $(PROGRAM_SERVER)

# Command for building the server program
$(PROGRAM_SERVER): $(SRC_DIR_SERVER) $(SRC_DIR_SERVICES)
	$(CXX_9) $(CXX_VERSION_17) $(CXXFLAGS) -o $(PROGRAM_SERVER) $(SERVER_INCLUDES) \
	$(SERVER_CPP_FILES) $(LINKFLAGS)

# Command for building the client program 
$(PROGRAM_CLIENT): $(SRC_DIR_CLIENT)
	$(CXX_9) $(CXX_VERSION_14) $(CXXFLAGS) -o $(PROGRAM_CLIENT) $(CLIENT_INCLUDES) \
	$(CLIENT_CPP_FILES) $(LINKFLAGS)

# Command for building the test program
$(PROGRAM_TEST): $(TEST_DIR) $(SRC_DIR_SERVICES) $(SRC_DIR_SERVICES)
	$(CXX_9) $(CXX_VERSION_14) $(CXXFLAGS_TEST) -o $(PROGRAM_TEST) $(TEST_INCLUDES) $(GTEST_INCLUDE) \
	$(TEST_CPP_FILES) $(LINK_DIR_TEST) $(LINKFLAGS_TEST)

# Cleans the directory, clears the screen, and builds and runs the test program
cleanTests: clean clearScreen $(PROGRAM_TEST)
	./$(PROGRAM_TEST)

# Builds and runs the test program
tests: $(PROGRAM_TEST)
	./$(PROGRAM_TEST)

# Uses valgrind to check the test program for memory leaks
memcheck: $(PROGRAM_TEST)
	valgrind --tool=memcheck --leak-check=yes ./$(PROGRAM_TEST)

# Uses GCov to perform a code coverage check of the test program
.PHONY: coverage
coverage: $(PROGRAM_TEST)
	./$(PROGRAM_TEST)
	# Print the lcov version
	$(LCOV) --version
	# Determine code coverage
	$(LCOV) -c --gcov-tool $(GCOV_9) --ignore-errors gcov,source,graph --no-external --exclude '*/test/gtest/*' -d . -b . -o $(COVERAGE_RESULTS)
	# Only show code coverage for the source code files (not library files)
	$(LCOV) --extract $(COVERAGE_RESULTS) -d . -b . */$(SRC_DIR)/*.cpp */$(INCLUDE_DIR)/*.hpp -o $(COVERAGE_RESULTS)
	#Generate the HTML reports
	genhtml $(COVERAGE_RESULTS)  --output-directory $(COVERAGE_DIR)
	#Remove all of the generated files from gcov
	rm -f *.gcda *.gcno

# Performs a static analysis of the code
HEADER_DIRS = \
	$(CONTROLLERS_INCLUDE_DIR) \
	$(SERVICES_INCLUDE_DIR) \
	$(ENTITIES_INCLUDE_DIR) \
	$(UTILS_INCLUDE_DIR) \
	$(VIEWS_INCLUDE_DIR) \
	$(APP_INCLUDE_DIR) \

SRC_DIRS = \
	$(SRC_DIR) \
	$(SRC_DIR_SERVER) \
	$(SRC_DIR_CLIENT) \
	$(SRC_DIR_SERVICES) \
	$(SRC_DIR_UTILS) \

static: $(HEADER_DIRS) $(SRC_DIRS)
	$(STATIC_ANALYSIS) --verbose --enable=all $(SRC_DIR)  $(HEADER_DIRS) --check-config --suppress=missingInclude

# Uses cpplint.py to perform a style check
style: $(SRC_DIR) $(INCLUDE_DIR)
	$(STYLE_CHECK) --headers=hpp,h $(ALL_HEADER_FILES) $(ALL_CPP_FILES)

# Uses doxygen to create code documentation
docs: $(ALL_HEADER_FILES) $(DOCS_DIR)
	doxygen
	