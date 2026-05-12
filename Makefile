NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread

# Tu lista de archivos fuente ajustada
SRC         = main.c \
              coders/coder/coder_routine.c \
	      coders/coder/dongle_logic.c \
	      coders/coder/coder_utils.c \
              coders/initialization/init_variables.c \
	      coders/initialization/init_simulation.c \
              coders/parsing/parsing.c \
              coders/parsing/parsing_utils.c \
	      coders/monitor/monitor_routine.c \
	      coders/heap/heap_utils.c \
	      coders/heap/heap_sift.c

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
