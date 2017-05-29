#!/bin/bash
#
# panzoom - a shell script to simulate pand & zoom for use in timelapse movies
# 				  usage: panzoom <config file>
# Copyright (C) 2017 John Davies
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Set up some default values

originalXSize=0
originalYSize=0
initXSize=0
initYSize=0
finalXSize=0
finalYSize=0
outputXSize=0
outputYSize=0
startX=0
startY=0
endX=0
endY=0
startDelay=0
endDelay=0
noOfFrames=1
out_path=""

# Check that config file is valid
if [ -f "$1" ]
then
	# Read values
	source "$1"
fi

# Read number of files to be processed
no_of_files=$( find . -maxdepth 1 -iname "*.jpg" -print | wc -l )

#If there are no files then exit at this point
if [ "$no_of_files" == "0" ]
then
	echo "ERROR - No files to be processed"
	exit 1
fi

#Check if a different directory is to be used for the output files
if [ "$out_path" == "" ]
then
	#No output path
	out_path_string=""
else
	if [ -d "$out_path" ]
	then
		out_path_string="$out_path/"
	else
		#Output directory doesn't exist so exit with error
		echo "ERROR - Output directory \"$out_path\" doesn't exist"
		exit 1
	fi
fi

# Run some sanity checks

# Read through the files & apply edits
imageCount=0
# Number of frames that need to be edited
editFrames=$((noOfFrames - startDelay - endDelay))
editFrameCount=1
# Deltas for X & Y
xpDiff=$(( endX - startX ))
ypDiff=$(( endY - startY ))
xsDiff=$(( finalXSize - initXSize ))
ysDiff=$(( finalYSize - initYSize ))
for file in *.jpg
do
	if [ $imageCount -lt $startDelay ]
	then
		#echo "Start: $out_path_string$file: $startX, $startY"
		cropString="${initXSize}x${initYSize}+${startX}+${startY}"
		resizeString="\"${outputXSize}x${outputYSize}!\""
  elif [ $imageCount -lt $(( noOfFrames - endDelay )) ]
	then
		xp=$(( ( editFrameCount * xpDiff ) / editFrames ))
		yp=$(( ( editFrameCount * ypDiff ) / editFrames ))
		xs=$(( ( editFrameCount * xsDiff ) / editFrames ))
		ys=$(( ( editFrameCount * ysDiff ) / editFrames ))
		(( editFrameCount+=1))
		#echo "Move: $out_path_string$file: $(( startX + xp )), $(( startY + yp ))"
		cropString="$((initXSize+xs))x$((initYSize+ys))+$((startX+xp))+$((startY+yp))"
		resizeString="\"${outputXSize}x${outputYSize}!\""
	else
		#echo "End: $out_path_string$file: $endX, $endY"
		cropString="${finalXSize}x${finalYSize}+${endX}+${endY}"
		resizeString="\"${outputXSize}x${outputYSize}!\""
	fi

	# Convert command
	echo "convert $file -crop $cropString -resize $resizeString $out_path_string$file"

	(( imageCount+=1 ))
done

# convert DSC_6539.jpg -crop 2638x1484+1004+308 -resize "1280x720!" output.jpg
