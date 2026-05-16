NAME=webserv
CXX=c++
CXXFLAGS=-Wall -Wextra -Werror -std=c++98 -g -I includes
SRCS=main.cpp $(PARSER_SRCS) $(UTILS_SRCS)
OBJ_DIR=obj
OBJS=$(addprefix $(OBJ_DIR)/,$(SRCS:.cpp=.o))
RM=rm -f
RMDIR=rm -rf

UTILS_SRCS=utils/Utils.cpp

PARSER_SRCS=parser/Config.cpp \
			parser/ServerConfig.cpp \
			parser/Route.cpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RMDIR) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re