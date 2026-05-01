NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread

# Tu lista de archivos fuente ajustada
SRC         = main.c \
              coders/coder_routine.c \
              coders/init_variables.c \
              coders/parsing.c \
              coders/parsing_utils.c \
	      coders/monitor_routine.c

OBJ_DIR     = obj
# Esta línea transforma 'coders/archivo.c' en 'obj/coders/archivo.o'
OBJs        = $(SRC:%.c=$(OBJ_DIR)/%.o)

INCLUDES    = -I.

all: $(NAME)

$(NAME): $(OBJs)
	$(CC) $(CFLAGS) $(OBJs) -o $(NAME)

# Regla mejorada para manejar subcarpetas
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
