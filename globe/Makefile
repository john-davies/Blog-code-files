rescale: rescale.c
	gcc rescale.c -o rescale

gradient: gradient.c stb_image.h
	gcc gradient.c -lm -o gradient

makeimage: makeimage.c makeimage.h libattopng.h
	gcc makeimage.c libattopng.c -o makeimage

clean:
	rm rescale gradient makeimage
