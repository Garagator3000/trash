STYLE_FLAGS = -Werror=all -Werror=extra -pedantic\
-Wno-error=format-nonliteral -Wformat-nonliteral \
-Werror=write-strings \
-Wno-error=discarded-qualifiers -Wno-error=incompatible-pointer-types \
-Wno-clobbered \
-fPIE -fstack-check -fstack-protector-all -ftrapv -D_FORTIFY_SOURCE=2 \
-Wl,-z,relro,-z,now -Wl,-z,noexecstack \
-Wbad-function-cast -Wcast-align -Werror=cast-qual -Wconversion \
-Werror=float-equal -Wformat-security -Werror=format-overflow=2 -Wformat-truncation \
-Winline -Winvalid-pch -Werror=jump-misses-init -Wlogical-op -Wmissing-declarations \
-Wmissing-prototypes -Wnested-externs -Wold-style-definition \
-Woverlength-strings -Werror=redundant-decls -Werror=shadow -Werror=strict-overflow=5 \
-Wsuggest-attribute=const -Werror=switch-default -Wtrampolines \
-Werror=undef -Wunsuffixed-float-constants \
-Werror=unused -Werror=stringop-overflow=4 -Wdeclaration-after-statement

LD_FLAGS = -L./bin/ -lalarm -Wl,-rpath,../bin/ -lzmq

LIBNAME = bin/libalarm.so
LIBNAME_obj = libalarm_connections.o libalarm_files.o \
libalarm_send_recv.o libalarm_additional_func.o
LIBNAME_src = src/libalarm_connections.c src/libalarm_files.c \
src/libalarm_send_recv.c src/libalarm_additional_func.c

MODULES_obj = alarm_manager.o module.o other_module.o filter.o
MODULES_src = src/alarm_manager.c src/module.c src/other_module.c src/filter.c

MANAGER_obj = alarm_manager.o
MANAGER_bin = bin/alarm_manager

MODULE_1_obj = module.o
MODULE_2_obj = other_module.o
MODULE_3_obj = filter.o

MODULE_1_bin = bin/module
MODULE_2_bin = bin/other_module
MODULE_3_bin = bin/filter

all: $(LIBNAME) $(MANAGER_bin) $(MODULE_1_bin) $(MODULE_2_bin) $(MODULE_3_bin) clean_obj

$(LIBNAME): $(LIBNAME_obj)
	mkdir -p bin/
	rm -f messages.txt
	gcc -shared -o $(LIBNAME) $(LIBNAME_obj)

$(LIBNAME_obj): $(LIBNAME_src)
	gcc $(STYLE_FLAGS) -fPIC -c $(LIBNAME_src)

$(MANAGER_bin): $(MANAGER_obj)
	gcc $(MANAGER_obj) -o $(MANAGER_bin) $(LD_FLAGS)

$(MODULE_1_bin): $(MODULE_1_obj)
	gcc $(MODULE_1_obj) -o $(MODULE_1_bin) $(LD_FLAGS)

$(MODULE_2_bin): $(MODULE_2_obj)
	gcc $(MODULE_2_obj) -o $(MODULE_2_bin) $(LD_FLAGS)

$(MODULE_3_bin): $(MODULE_3_obj)
	gcc $(MODULE_3_obj) -o $(MODULE_3_bin) $(LD_FLAGS)

$(MODULES_obj): $(MODULES_src)
	gcc $(STYLE_FLAGS) -c $(MODULES_src)

clean_obj:
	rm -f *.o

clean:
	rm -rf bin/
	rm -f messages.txt
