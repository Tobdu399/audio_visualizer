# Directories
DIR_MAIN   = ./
DIR_GUI    = ./lib/gui
DIR_FONTS  = ./lib/gui/fonts
DIR_AUDIOC = ./lib/audio
DIR_BIN    = ./bin

# Source files
OBJ_C = $(wildcard ${DIR_MAIN}/*.cpp ${DIR_GUI}/*.cpp ${DIR_FONTS}/*.cpp ${DIR_AUDIOC}/*.cpp)
OBJ_O = $(patsubst %.cpp,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

# Target executable
TARGET = audio_visualizer

# Librariess
LIBRARIES = -lSDL2 -lSDL2_ttf -lfftw3 -lm -lasound -pthread

# Compiler flags
CC = g++
# CFLAGS += -g -O0 -Wall

# Linking and compiling
${TARGET}: ${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LIBRARIES)

# Compilation rules
${DIR_BIN}/%.o: $(DIR_MAIN)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_MAIN) -I $(DIR_GUI) -I $(DIR_AUDIOC)

${DIR_BIN}/%.o: $(DIR_GUI)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_MAIN) -I $(DIR_GUI)

${DIR_BIN}/%.o: $(DIR_AUDIOC)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(DIR_MAIN)


# Clean up
clean:
	rm -f $(DIR_BIN)/*.o
	rm -f $(TARGET)
