CC      := gcc
CCFLAGS := -O0 -Wall -Wno-comment
LDFLAGS :=
LIBS    := -lm

TARGETS:= change-light change-light-lut grayscale noisetoppm
MAINS  := $(addsuffix .o, $(TARGETS) )
OBJ    := $(MAINS)
DEPS   := benchmark.h

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJ)

$(OBJ): %.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) %.o
	$(CC) -o $@ $^ $(LIBS) $(CCFLAGS) $(LDFLAGS)
