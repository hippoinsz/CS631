
TARGET := sws

CC := cc
CFLAGS := -Wall -Werror -Wextra -g -lmagic -lpthread

OBJS = sws.o server.o util.o cgi.o

all: ${TARGET}

$(TARGET) : $(OBJS)
	$(CC) ${CFLAGS} ${OBJS} -o ${TARGET}

clean:
	rm -rf $(TARGET) $(OBJS)

