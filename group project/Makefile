TARGET := sws

CC := gcc
CFLAGS := -Wall -Werror -Wextra

SRCS = sws.c server.c
INC = -I./.

OBJS = $(SRCS:.c=.o)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean:
	rm -rf $(TARGET) $(OBJS)
