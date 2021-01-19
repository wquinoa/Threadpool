CXX := clang++
CXXFLAGS := -Wall -Werror -Wextra -std=c++98

NAME := test.out
SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

.PHONY: all clean fclean re run

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

run: $(NAME)
	./$<

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all
