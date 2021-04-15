rescale: rescale.c
	gcc rescale.c -Wall -o rescale

gradient: gradient.c stb_image.h
	gcc gradient.c -lm -Wall -o gradient

tif2bin: tif2bin.c stb_image.h
	gcc tif2bin.c -lm -ltiff -Wall -o tif2bin

makeimage: makeimage.c makeimage.h libattopng.h
	gcc makeimage.c libattopng.c -Wall -o makeimage

makeglobe: makeglobe.c makeglobe.h
	gcc makeglobe.c -Wall -lm -o makeglobe

all: rescale gradient makeimage makeglobe tif2bin

clean:
	rm rescale gradient makeimage makeglobe tif2bin
	rm *.o
