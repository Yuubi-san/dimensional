
CPPFLAGS = -std=c++14 -fextended-identifiers
override CXXFLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wcast-align\
	-Wformat=2 -Wstrict-overflow=5 -Wsign-promo -Woverloaded-virtual $(CXXFLAGS)

srcs  := $(wildcard *.cpp)
tests := $(srcs:.cpp=)
exes  := $(srcs:.cpp=.exe)
oexes := $(srcs:.cpp=.opt.exe)
deps  := $(srcs:.cpp=.d)


all: $(tests)

what:
	@echo $(tests)

$(tests): %: %.exe %.opt.exe


$(exes): %.exe: %.d
	@$(MAKE) --no-print-directory -f $< $@
	./$@
$(oexes): %.opt.exe: %.d
	@$(MAKE) --no-print-directory -f $< $@
	./$@

$(deps): %.d: %.cpp inexistent-file
	@echo Building dependency list for $*...
	@$(CXX) $(CPPFLAGS) -M -MT $*.exe -MT $*.opt.exe -MF $@ $<
	@echo $*.exe: >> $@
	@echo '	'$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) \
		-o $$\@ $< $(LDLIBS) >> $@
	@echo $*.opt.exe: >> $@
	@echo '	'$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -O3 \
		-o $$\@ $< $(LDLIBS) >> $@
	@echo done

inexistent-file:


clean:
	-$(RM) $(exes) $(oexes) $(deps)

.PHONY: all what $(tests) inexistent-file clean
