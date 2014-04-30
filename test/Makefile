all: clean run_tests

run_tests: tests
	./tests

tests: pow2_tests.o multiply.o module.o gtest/gtest.a
	g++ -g -o $@ $^ -lpthread

%_tests.o : %_tests.cc
	g++ -c -o $@ -g -Wall -Werror -I$(GTEST_DIR)/include $^

%.o : ../%.c
	gcc -c -o $@ -g -Wall -Werror $^

clean:
	-rm *.o tests

GTEST_DIR = /home/chris/code/gtest-1.7.0


# Builds gtest.a and gtest_main.a.

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest/gtest-all.o : $(GTEST_SRCS_)
	-mkdir gtest
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GTEST_DIR) -I$(GTEST_DIR)/include $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest-all.cc -o $@

gtest/gtest.a : gtest/gtest-all.o
	$(AR) $(ARFLAGS) $@ $^
