#!/bin/bash
#
# panoexpmod - a shell script to modify the exposure of RAW images
#          for use in panoramic pictures
# 				 usage: panoexpmod <config file>
# Copyright (C) 2019 John Davies
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
exp_fusion_1=""
exp_fusion_2=""
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
exp_step=$( echo "scale=4; ( ($end_exp)-($start_exp) ) / ( ( $no_of_files - 1) / 2 )" | bc )

echo "#Files to be processed: $no_of_files"
echo "#Start exposure compensation: $start_exp"
echo "#End exposure compensation: $end_exp"
echo "#Exposure compensation step: $exp_step"
echo "#Exposure fusion mod 1: $exp_fusion_1"
echo "#Exposure fusion mod 2: $exp_fusion_2"

# Read through the files, modifying the exposure as we go
current_exp=$start_exp
file_count=0
for file in *.$file_ext
do
	if [ "$exp_fusion_1" != "" ]
	then
		temp_exp=$( echo "scale=4; ( $current_exp + $exp_fusion_1 )" | bc )
	echo "ufraw-batch $file --exposure $temp_exp --restore clip --output=$out_path/$file.a.$output_ext --out-type $output_ext  --overwrite"
	fi
	echo "ufraw-batch $file --exposure $current_exp --restore clip --output=$out_path/$file.b.$output_ext --out-type $output_ext  --overwrite"

	if [ "$exp_fusion_2" != "" ]
	then
		temp_exp=$( echo "scale=4; ( $current_exp + $exp_fusion_2 )" | bc )
	echo "ufraw-batch $file --exposure $temp_exp --restore clip --output=$out_path/$file.c.$output_ext --out-type $output_ext  --overwrite"
	fi

	echo "#"

	if [ "$file_count" -lt $(($no_of_files/2)) ]
	then
		current_exp=$( echo "scale=4; $current_exp + $exp_step" | bc )
	else
		current_exp=$( echo "scale=4; $current_exp - $exp_step" | bc )
	fi
	file_count=$(($file_count+1))
done
