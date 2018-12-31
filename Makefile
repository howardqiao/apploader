#OBJS specifies which files to compile as part of the project
OBJS = apploader.cpp

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = apploader

#This is the target that compiles our executable
all : $(OBJS)
	g++ $(OBJS) -w -lm -lpthread -lSDL2 -lSDL2_image -o $(OBJ_NAME) -I/usr/include/SDL2 
