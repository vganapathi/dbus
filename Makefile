#
# OSD dbus library
#

SRC := avl.c log_functions.c client_mgr.c server_stats.c dbus_server.c export_mgr.c
INC := common_utils.h abstract_atomic.h abstract_mem.h avltree.h client_mgr.h common_utils.h export_mgr.h ganesha_dbus.h log.h nfs_core.h server_stats.h server_stats_private.h wait_queue.h
OBJ := $(SRC:.c=.o)
LIB := libosddbus.a
#PAN_SANDBOX := /net/nfs.panwest.panasas.com/sb4/vganapathi/sam-dev
#LIBRARY_PATH := $(PAN_SANDBOX)/import/dbus/freebsd_72_amd64/lib/
#LIBS := -L$(PAN_SANDBOX)/import/dbus/freebsd_72_amd64/lib/:$(PAN_SANDBOX)/import/dbus-glib/freebsd_72_amd64/lib/:$(PAN_SANDBOX)/import/glib/freebsd_72_amd64/lib/ -ldbus-1
#-ldbus-glib-1 -ldbus-1 -lglib-2.0 -lgobject-2.0 -lgio-2.0
#LIBS := -L/usr/local/lib -ldbus-1

-include ../Makedefs

CC := gcc
CPP_M := -MM
#CPP_M := 
LD := $(CC)
COPTS := $(OPT) -fPIC
CWARN := -Wall -W -Wpointer-arith -Wwrite-strings
	#-Wall -W -Wpointer-arith -Wwrite-strings -Wcast-align -Wcast-qual \
	#-Wbad-function-cast -Wundef -Wmissing-prototypes \
	#-Wundef -Wmissing-prototypes \
	#-Wmissing-declarations -Wnested-externs
#CFLAGS := $(COPTS) $(CWARN) -I.
CFLAGS := $(COPTS) -DUSE_DBUS_STATS -I.

ifeq ($(FreeBSD_Make),1)
CFLAGS += -I../tgt/usr/bsd -I../tgt/usr 
CFLAGS += -I/usr/local/include/dbus-1.0 -I/usr/local/include/dbus-1.0/include -I/usr/include/ 
else
CFLAGS += -I/usr/include/dbus-1.0 -I/usr/include/dbus-1.0/include -I/usr/include/ 
endif

#-I/usr/include/sys

#all:: final $(LIB) $(OBJ)
all:: $(LIB) $(OBJ)

#final: $(OBJ)
	#$(CC) $^ -o $@ $(LIBS)

$(LIB): $(OBJ)
	ar cr $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
# $(CC) $(CFLAGS) $< -o $@ $(LIBS)

ifeq (,$(filter clean distclean dist,$(MAKECMDGOALS)))
-include .depend
endif
all:: .depend
.depend: $(SRC) $(INC) Makefile
	@$(CC) $(CPP_M) $(CFLAGS) $(SRC) > .depend

clean:
	rm -f $(LIB) $(OBJ)
	rm -f .depend

tags: FORCE
	ctags $(SRC) $(INC)

FORCE:;

# distribution tarball
.PHONY: dist
MV := dbus
MVD := $(MV)-$(shell date +%Y%m%d)
dist:
	rm -rf $(MV) $(MVD)
	mkdir $(MVD)
	svn list -R | grep -v '/$$' | rsync --files-from=/dev/stdin ./ $(MVD)/
	ln -s $(MVD) $(MV)
	tar cf - $(MV) $(MVD) | bzip2 -9c > $(MVD).tar.bz2
	rm -rf $(MV) $(MVD)

