rescale: rescale.c
	gcc rescale.c -Wall -o rescale

gradient: gradient.c stb_image.h
	gcc gradient.c -lm -Wall -o gradient

makeimage: makeimage.c makeimage.h libattopng.h
	gcc makeimage.c libattopng.c -Wall -o makeimage

#makeglobe: makeglobe.cpp makeglobe.hpp plylib.o
#	g++ makeglobe.cpp -Wall -o makeglobe

makeglobe: makeglobe.c makeglobe.h
	gcc makeglobe.c -Wall -lm -o makeglobe

clean:
	rm rescale gradient makeimage
	rm *.o
