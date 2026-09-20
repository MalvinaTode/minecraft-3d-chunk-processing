.PHONY: clean libchunk all

# Flaguri de compilare
CFLAGS = -Wall -Wextra -std=c11 -g

# Sursele programului de testare
MAIN_SRCS = \
	src/main.c

# Obiectele rezultate din compilarea programului
MAIN_OBJS = $(patsubst %.c,%.o,$(MAIN_SRCS))

# Sursele bibliotecii
LIBCHUNK_HEADER = libchunk/chunk.h
LIBCHUNK_SRCS = \
	libchunk/chunk_gen.c \
	libchunk/chunk_transform.c \
	libchunk/chunk_process.c \
	libchunk/chunk_compress.c

# Obiectele rezultate din compilarea bibliotecii
LIBCHUNK_OBJS = $(patsubst %.c,%.o,$(LIBCHUNK_SRCS))

# Rezultatul final al compilării și arhivării bibliotecii
LIBCHUNK_TARGET = build/libchunk.a

# Regula principală: compilează biblioteca și programul de testare
all: build/main
	@:

# Regula programului de testare -- necesită obiectele proprii și biblioteca
build/main: $(MAIN_OBJS) $(LIBCHUNK_TARGET)
	@mkdir -p build
	gcc $(CFLAGS) $^ -o $@ -lm

# Regula bibliotecii
libchunk: $(LIBCHUNK_TARGET)
	@:

# Regula de arhivare a bibliotecii -- necesită obiectele bibliotecii
$(LIBCHUNK_TARGET): $(LIBCHUNK_OBJS)
	@mkdir -p build
	ar rcs $(LIBCHUNK_TARGET) $^

# Regula fișierelor obiect
%.o: %.c $(LIBCHUNK_HEADER)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf build/ tests_out/
	rm -f $(MAIN_OBJS) $(LIBCHUNK_OBJS)
	$(MAKE) -C checker clean
