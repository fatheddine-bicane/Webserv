TARGET = webserv
SOURCE_FILES = $(shell find ./src -type f -name "*.cpp")
HEADER_FILES = $(shell find ./src -type f -name "*.hpp")
CPP_FLAGS = -Wall -Wextra -Werror -std=c++98 -g
COMPILER = c++
OBJ_FILES = $(SOURCE_FILES:./src/%.cpp=./src/%.o)


GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m


#--------------------------------------------------------#

.phony : all clean fclean re

all : $(TARGET)

$(TARGET) : $(OBJ_FILES)
	@echo "$(GREEN)Building webserv$(RESET)"
	@$(COMPILER) $(CPP_FLAGS) $(OBJ_FILES) -o $(TARGET)

./src/%.o : ./src/%.cpp $(HEADER_FILES)
	@$(COMPILER) $(CPP_FLAGS) -c $< -o $@

clean :
	@echo "$(RED)Removing object files$(RESET)"
	@rm -f $(OBJ_FILES)

fclean : clean
	@echo "$(RED)Removing webserv$(RESET)"
	@rm -f webserv

nginx_t:
	@echo "$(RED)Testing config file using nginx$(RESET)"
	@docker compose run --rm web nginx -t

re : fclean all
