CC = gcc
CFLAGS = -Wall -Werror 

EXECUTABLE = mySystemStatus
MY_SYS_STATUS_OBJ = mySystemStatus.o
STATS_FUNC_OBJ = stats.function.o
MY_SYS_STATUS = mySystemStatus.c
STATS_FUNC = stats.function.c
STATS_HEADER = stats.function.h

${EXECUTABLE}: ${MY_SYS_STATUS_OBJ} ${STATS_FUNC_OBJ}
	${CC} ${CFLAGS} -o $@ $^

%.o : %.c ${STATS_HEADER}
	${CC} ${CFLAGS} -c -o $@ $<

.PHONY: clean
clean:
	rm -f ${MY_SYS_STATUS_OBJ} ${STATS_FUNC_OBJ}