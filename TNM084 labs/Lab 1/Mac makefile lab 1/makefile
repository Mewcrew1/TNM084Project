all :  lab1

lab1: lab1.cpp ../common/GL_utilities.c ../common/Mac/MicroGlut.m noise/noise1234.c noise/cellular.c noise/simplexnoise1234.c
	gcc -Wall -c -DGL_GLEXT_PROTOTYPES  -Wno-deprecated-declarations ../common/GL_utilities.c ../common/Mac/MicroGlut.m noise/noise1234.c noise/cellular.c noise/simplexnoise1234.c -I../common -I../common/Mac
	g++ -Wall -o lab1 -DGL_GLEXT_PROTOTYPES  -Wno-deprecated-declarations lab1.cpp GL_utilities.o MicroGlut.o noise1234.o cellular.o simplexnoise1234.o -I../common -I../common/Mac -Inoise -framework OpenGL -framework Cocoa -lm

clean :
	rm lab1
