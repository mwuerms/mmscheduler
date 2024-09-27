# Martin Egli
# 2024-09-28
# cooperative operating system for MCU

TARGET = mmscheduler

DEP_DIR = .dep
VERSION_FILE = version.h

CC = gcc
CFLAGS  = -c -g -Wall -rdynamic
CFLAGS += -MD -MP -MT $(*F).o -MF $(DEP_DIR)/$(@F).d
#CFLAGS += $(shell pkg-config --cflags glib-2.0)
# preprocessor output
#CFLAGS += -E > preproc_output.c

#LDFLAGS = $(shell pkg-config --libs glib-2.0)

# Versionfile
VERSION_STRING_NAME = cVERSION
STORED_VERSION_STRING = $(subst ",,$(shell [ ! -r $(VERSION_FILE) ] || \
                                                read ignore ignore v <$(VERSION_FILE) && echo $$v))                                                              
VERSION_STRING := $(shell git describe --dirty --always || echo $(VERSION))
ifneq ($(STORED_VERSION_STRING),$(VERSION_STRING))
$(info updating $(VERSION_FILE) to $(VERSION_STRING))
$(shell echo // this file was automatically created by the Makefile, do not edit >$(VERSION_FILE))
$(shell echo \#define $(VERSION_STRING_NAME) \"$(VERSION_STRING)\" >>$(VERSION_FILE))
endif

RELEASEDIR = $(TARGET)-$(VERSION_STRING)

SRC=mmscheduler.c\
fifo.c\
main.c

OBJ = $(SRC:.c=.o)

all: $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET) 

%.o: %.c
	$(CC) -c $(CFLAGS) $<

#doc: 
#        doxygen setup.dox

clean:
	rm -v $(OBJ) $(TARGET)

release: all
	echo "make release: ${RELEASEDIR}.tar"
	mkdir ./release/${RELEASEDIR}
	cp arch.h ./release/${RELEASEDIR}
	cp debug.h ./release/${RELEASEDIR}
	cp project.h ./release/${RELEASEDIR}
	cp version.h ./release/${RELEASEDIR}
	cp process.c ./release/${RELEASEDIR}
	cp process.h ./release/${RELEASEDIR}
	cp fifo.c ./release/${RELEASEDIR}
	cp fifo.h ./release/${RELEASEDIR}
	tar -cf ./release/${RELEASEDIR}.tar ./release/${RELEASEDIR}
	rm -r ./release/${RELEASEDIR}

-include $(shell mkdir $(DEP_DIR) 2>/dev/null) $(wildcard $(DEP_DIR)/*)

