#! /bin/bash
echo "Nat Geo Magazine PDF Creator and OCR scan"

# Imagemagick parameters
RESIZE="-resize 200%"
THRESHOLD1="-threshold 60%"
THRESHOLD2="-threshold 80%"

# Clean directory
rm ${1}/*.jpg
rm ${1}/*.txt
rm ${1}/*.pdf

# Convert the cng files to jpg
# Magazine page file names are usually of the format NGM_yyyy_mm_ppp_x.cng
# Other pages like additional maps have the format yyyy_mm_description.cng
# Process the magazine pages first then the maps
echo "Converting files"
#for filename in $(find ${1}/*.cng -maxdepth 1 -type f)
for filename in $( ls -1 ${1}/[!0-9]*.cng )
do
	./cng2jpg ${filename} ${filename}.jpg
done
# Force the maps to be at the end of the list by prepending "zz" to the filename
for filename in $( ls -1 ${1}/[0-9]*.cng )
do
	./cng2jpg ${filename} ${filename/\//"/zz_"}.jpg
done

# Renumber the jpgs
ls ${1}/*.jpg | cat -n | while read n f; do mv "${f}" `printf "${1}/%04d.jpg" ${n}`; done

# Make PDF
echo "Creating ${1}.pdf"
convert ${1}/*.jpg -density 72 ${1}/${1}.pdf

# OCR text
echo "Scanning files"
for filename in $(find ${1}/*.jpg -maxdepth 1 -type f)
do
	# Extract just the file name without the extension
	tmp=${filename#*/}   # remove prefix ending in "/"
	name=${tmp%.*}   	# remove suffix starting with "."
	# Run the two conversions
	convert ${filename} ${RESIZE} ${THRESHOLD1} ${1}/${name}_1.jpg
	tesseract ${1}/${name}_1.jpg ${1}/${name}
	# Combine any words hyphenated across lines
	sed -z 's/-\n//g' ${1}/${name}.txt > ${1}/${name}_1.txt
	convert ${filename} ${RESIZE} ${THRESHOLD2} ${1}/${name}_2.jpg
	tesseract ${1}/${name}_2.jpg ${1}/${name}
	# Combine any words hyphenated across lines
	sed -z 's/-\n//g' ${1}/${name}.txt > ${1}/${name}_2.txt
	# Check number of words failing spell check
	FAIL1=`cat ${1}/${name}_1.txt | aspell --extra-dicts en_US list | sort -u | wc -l`
	TOTAL1=`wc -w < ${1}/${name}_1.txt`
	PERCENT1=0
	if [ $TOTAL1 -ne 0 ] 
	then
		PERCENT1=$(( ( FAIL1*100 ) / TOTAL1  ))
	fi

	FAIL2=`cat ${1}/${name}_2.txt | aspell --extra-dicts en_US list | sort -u | wc -l`
	TOTAL2=`wc -w < ${1}/${name}_2.txt`
	PERCENT2=0
	if [ $TOTAL2 -ne 0 ] 
	then
		PERCENT2=$(( ( FAIL2*100 ) / TOTAL2  ))
	fi

	# Process the results
	if [ ${PERCENT1} -lt ${PERCENT2} ]
	then
		# echo ${name}.txt 1 ${TOTAL1} ${FAIL1} ${PERCENT1} >> ${1}/stats.txt
		mv ${1}/${name}_1.txt ${1}/${name}.txt
		rm ${1}/${name}_2.txt
	elif [ ${PERCENT1} -gt ${PERCENT2} ]
	then
		# echo ${name}.txt 2 ${TOTAL2} ${FAIL2} ${PERCENT2} >> ${1}/stats.txt
		mv ${1}/${name}_2.txt ${1}/${name}.txt
		rm ${1}/${name}_1.txt
	else
		# Same error percentage so pick the one with the biggest no of words
		if [ ${TOTAL1} -ge ${TOTAL2} ]
		then
			# echo ${name}.txt 1 ${TOTAL1} ${FAIL1} ${PERCENT1} >> ${1}/stats.txt
			mv ${1}/${name}_1.txt ${1}/${name}.txt
			rm ${1}/${name}_2.txt
		else
			# echo ${name}.txt 2 ${TOTAL2} ${FAIL2} ${PERCENT2} >> ${1}/stats.txt
			mv ${1}/${name}_2.txt ${1}/${name}.txt
			rm ${1}/${name}_1.txt
		fi
	fi
done

# Clean up the image files
rm ${1}/*.cng
rm ${1}/*.jpg






