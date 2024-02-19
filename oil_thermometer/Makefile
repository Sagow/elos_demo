SRCS =	main.cpp\
		../common/common.cpp

HEADERS = Common.hpp

OBJS =  ${SRCS:.cpp=.o}

CC =    c++

FLAGS = -Wall -Werror -Wextra

LIBS = -lelos -lsafu -ljson-c

INCLUDES = -I../common

NAME =  oil_thermometer

all: ${NAME}

${NAME}:	${OBJS}
		${CC} ${FLAGS} ${OBJS}  ${LIBS} ${INCLUDES} -o ${NAME}

%.o: %.cpp ${HEADERS}
		$(CC) $(FLAGS) -o $@ -c $< ${LIBS} ${INCLUDES}

clean:
		rm -rf $(OBJS)

fclean: clean
		rm -rf $(NAME)

re: fclean all

.PHONY: re all clean fclean