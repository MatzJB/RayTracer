CC = cc
STD = -std=c99
LINKS = -lm
SDL = `sdl-config --cflags --libs` 
BMP = bmpimage.c #Pers kod



ray:
	gcc $(STD) -Wall GSDL.c $(SDL)

ray2:
	gcc -O3 $(STD) raytracing.c GSDL.c $(BMP) $(SDL)


ray2_omp:
	gcc -std=c99 -O3 $(STD) -fopenmp raytracing_openmp.c GSDL.c $(BMP) $(SDL)


omp_test:
	gcc -std=c99 $(STD) -fopenmp openmp_test.c timing.a


csecond:
	gcc -std=c99 $(STD) -O3 -c csecond.c



cleanMS:	
	rm a.exe #if windows
clean:
	rm a.out

