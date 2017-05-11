#!/bin/bash
#
# expmod - a shell script to modify the exposure of RAW images
#          for use in timelapse movies
# 				 usage: expmod <config file>
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
start_exp=0.0
end_exp=0.0
crop_left=0
crop_right=0
crop_top=0
crop_bottom=0
size=0
file_ext="NEF"
output_ext="jpg"
out_path=""

# Check that config file is valid
if [ -f "$1" ]
then
	# Read values
	source "$1"
fi

# Read number of files to be processed
no_of_files=$( find . -maxdepth 1 -iname "*.$file_ext" -print | wc -l )

#If there are no files then exit at this point
if (( "$no_of_files" == "0" ))
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
		out_path_string="--out-path=$out_path"
	else
		#Output directory doesn't exist so exit with error
		echo "ERROR - Output directory \"$out_path\" doesn't exist"
		exit 1
	fi
fi

# Calculate exposure step, bc is used because floating point maths is needed
exp_step=$( echo "scale=4; ( ($end_exp)-($start_exp) ) / ( $no_of_files - 1)" | bc )

echo "#Files to be processed: $no_of_files"
echo "#Start exposure compensation: $start_exp"
echo "#End exposure compensation: $end_exp"
echo "#Exposure compensation step: $exp_step"

#Check for cropping needed
if (( "$crop_left" == "$crop_right" ))
then
	#No cropping
	crop_string=""
else
	crop_string="--crop-left $crop_left --crop-right $crop_right --crop-top $crop_top --crop-bottom $crop_bottom"
fi

#Check for resize needed
if [ "$size" == 0 ]
then
	#No resizing
	size_string=""
else
	size_string="--size $size"
fi

# Read through the files, modifying the exposure as we go
current_exp=$start_exp
for file in *.$file_ext
do
	echo "ufraw-batch $file --exposure $current_exp --restore clip --out-type $output_ext $crop_string $size_string $out_path_string --overwrite"
	current_exp=$( echo "scale=4; $current_exp + $exp_step" | bc )
done
