NAME=webserv
CXX=c++
CXXFLAGS=-Wall -Wextra -Werror -std=c++98 -g -I includes
SRCS=main.cpp $(PARSER_SRCS) $(UTILS_SRCS) $(CORE_SRCS) $(NETWORK_SRCS)
OBJ_DIR=obj
OBJS=$(addprefix $(OBJ_DIR)/,$(SRCS:.cpp=.o))
RM=rm -f
RMDIR=rm -rf

UTILS_SRCS=utils/Utils.cpp utils/Buffer.cpp

PARSER_SRCS=parser/Config.cpp parser/ServerConfig.cpp \
			parser/Route.cpp  parser/RequestParse.cpp \
			parser/ResponseParse.cpp 

CORE_SRCS= core/EventLoop.cpp

NETWORK_SRCS=network/ClientConnection.cpp network/AConnection.cpp \
			network/Session.cpp network/ServerSocket.cpp

TEMP_DIR=./temp

all: $(NAME) $(TEMP_DIR)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEMP_DIR):
	mkdir -p $(TEMP_DIR)

clean:
	$(RM) $(OBJS)
	$(RMDIR) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)
	$(RMDIR) $(TEMP_DIR)

re: fclean all

test: all
	clear; ./$(NAME) test.conf

.PHONY: all clean fclean re test