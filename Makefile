.SUFFIXES:

BUILD := build
DEBUG := debug
SOURCE := source
INCLUDE := include

CONFIG :=

ifeq ($(notdir $(CURDIR)), $(BUILD))
CONFIG := $(BUILD)
else ifeq ($(notdir $(CURDIR)), $(DEBUG))
CONFIG := $(DEBUG)
endif

ifeq ($(CONFIG),)
export TARGET := $(notdir $(CURDIR))
export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)

LIBS := 
LIBDIR := 

export LIBPATH := $(LIBDIR)

export CFLAGS := -I $(TOPDIR)/$(INCLUDE) \
	-I $(TOPDIR)/$(INCLUDE)/libctru #not needed for 3ds

ifneq ($(strip $(LIBS)),)
CFLAGS += $(foreach dir, $(LIBPATH), -I $(dir)/$(INCLUDE))

export LDFLAGS := $(LIBS)
endif

export CXXFLAGS = $(CFLAGS)
export SFLAGS = $(CFLAGS)

export OPTIMIZE := -O2

export CC := gcc
export CXX := g++
export LD := gcc
export NM := nm
export MAKE := make

export CFILES := $(notdir $(wildcard $(SOURCE)/*.c))
export CPPFILES := $(notdir $(wildcard $(SOURCE)/*.cpp))
export SFILES := $(notdir $(wildcard $(SOURCE)/*.s))
export OFILES := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(SFILES:.s=.o)

export VPATH := $(TOPDIR)/$(SOURCE)

ifneq ($(CPPFILES),)
export LD := g++
endif

.PHONY: all clean $(DEBUG)
#-----------------------------------------------------
all: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#-----------------------------------------------------
$(BUILD):
	@mkdir -p $@

$(DEBUG):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(DEBUG) -f $(CURDIR)/Makefile

#-----------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(DEBUG) $(OUTPUT).exe $(OUTPUT).lst

#-----------------------------------------------------
else	#($(CONFIG),)

ifeq ($(CONFIG), $(DEBUG))
$(info **debug mode ...**)
CFLAGS += -g
OPTIMIZE := -O0
OUTPUT := $(CURDIR)/$(TARGET)
endif

#-----------------------------------------------------
# main target
#-----------------------------------------------------
$(OUTPUT).exe: $(OFILES)

%.exe:
	@echo linking $(@F) ...
	@$(LD) $(OFILES) -o $@ $(LDFLAGS)
	@$(NM) -n $@ > $*.lst

#-----------------------------------------------------


%.o: %.c
	@echo $(<F)
	@$(CC) -std=c11 $(OPTIMIZE) -Wall -MMD -MF $*.d $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo $(<F)
	@$(CXX) -std=c++11 $(OPTIMIZE) -Wall -MMD -MF $*.d $(CXXFLAGS) -c $< -o $@

%.o: %.s
	@echo $(<F)
	@$(CC) -MD -MF $*.d $(SFLAGS) -c $< -o $@

-include $(CURDIR)/*.d

endif	#($(CONFIG),)