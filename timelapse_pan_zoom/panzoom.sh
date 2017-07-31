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
startXSize=0
startYSize=0
endXSize=0
endYSize=0
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
# Check that the movie images don't go outside the original image boundaries
if [ $(( startX + startXSize )) -gt $originalXSize ]
then
	echo "# WARNING - Start X dimension outside original image"
fi

if [ $(( endX + endXSize )) -gt $originalXSize ]
then
	echo "# WARNING - End X dimension outside original image"
fi

if [ $(( startY + startYSize )) -gt $originalYSize ]
then
	echo "# WARNING - Start Y dimension outside original image"
fi

if [ $(( endY + endYSize )) -gt $originalYSize ]
then
	echo "# WARNING - End Y dimension outside original image"
fi

# Check that the aspect ratios stay the same. Multiply by 1000 to
# avoid floating point arithmetic
startAspectRatio=$(( ( $startXSize * 1000 ) / $startYSize ))
endAspectRatio=$(( ( $endXSize * 1000 ) / $endYSize ))
outputAspectRatio=$(( ( $outputXSize * 1000 ) / $outputYSize ))

if [ $startAspectRatio -ne $endAspectRatio ]
then
	echo "# WARNING - start/end aspect ratio mismatch"
fi

if [ $endAspectRatio -ne $outputAspectRatio ]
then
	echo "# WARNING - end/output aspect ratio mismatch"
fi

if [ $startAspectRatio -ne $outputAspectRatio ]
then
	echo "# WARNING - start/output aspect ratio mismatch"
fi

# Check that the number of files match
if [ $no_of_files -ne $noOfFrames ]
then
	echo "# WARNING - \"noOfFrames\" does not match number of frame images"
fi

# Check the start and end delay frame lengths
if [ $(( startDelay + endDelay )) -ge $noOfFrames ]
then
	echo "# WARNING - start & end delays too long"
fi

# Read through the files & apply edits
imageCount=0
# Number of frames that need to be edited
editFrames=$((noOfFrames - startDelay - endDelay))
editFrameCount=1
# Deltas for X & Y position and image size
xpDiff=$(( endX - startX ))
ypDiff=$(( endY - startY ))
xsDiff=$(( endXSize - startXSize ))
ysDiff=$(( endYSize - startYSize ))
for file in *.jpg
do
	if [ $imageCount -lt $startDelay ]
	then
		cropString="${startXSize}x${startYSize}+${startX}+${startY}"
		resizeString="\"${outputXSize}x${outputYSize}!\""
  elif [ $imageCount -lt $(( noOfFrames - endDelay )) ]
	then
		xp=$(( ( editFrameCount * xpDiff ) / editFrames ))
		yp=$(( ( editFrameCount * ypDiff ) / editFrames ))
		xs=$(( ( editFrameCount * xsDiff ) / editFrames ))
		ys=$(( ( editFrameCount * ysDiff ) / editFrames ))
		(( editFrameCount+=1))
		cropString="$((startXSize+xs))x$((startYSize+ys))+$((startX+xp))+$((startY+yp))"
		resizeString="\"${outputXSize}x${outputYSize}!\""
	else
		cropString="${endXSize}x${endYSize}+${endX}+${endY}"
		resizeString="\"${outputXSize}x${outputYSize}!\""
	fi

	# Convert command
	echo "convert $file -crop $cropString -resize $resizeString $out_path_string$file"

	(( imageCount+=1 ))
done
