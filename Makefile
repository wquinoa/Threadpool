CXX := clang++
CXXFLAGS := -Wall -Werror -Wextra -std=c++98 -MMD

NAME := test.out
SRC := test.cpp
OBJ := $(SRC:.cpp=.o)
DEP := $(SRC:.cpp=.d)

.PHONY: all clean fclean re run

-include $(DEP)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

run: $(NAME)
	./$<

clean:
	rm -rf $(OBJ) $(DEP)

fclean: clean
	rm -rf $(NAME)

re: fclean all
