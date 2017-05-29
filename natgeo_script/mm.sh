#! /bin/bash
echo "Nat Geo Magazine PDF maker"

for filename in $(find $1/*.cng -maxdepth 1 -type f)
do
	echo "Converting: $filename"
	./cng2jpg $filename $filename.jpg
done

echo "Creating $1.pdf"
convert $1/*.jpg -density 72 $1.pdf
