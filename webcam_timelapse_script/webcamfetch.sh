#!/bin/bash
#
# webcamfetch - a shell script to fetch webcam images to create time lapse movies
# 				      usage: webcamfetch <config file>
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
frame_count=100
frame_interval=60
url=""
store_location=""
file_prefix="image"

# Check that config file is valid
if [ ! -f $1 ]
then
echo "Config file \"$1\" not found"
exit 1
fi

# Read values
source $1

# Check results
if [ "$url" == "" ]
then
echo "No source URL specified"
exit 1
fi

echo "Frame count: $frame_count"
echo "Frame interval: $frame_interval"
echo "Store location: $store_location"
echo "File prefix: $file_prefix"

#Start now and loop throught count

for ((a=0; a < frame_count ; a++))
do
  fn=`printf "%04d.jpg" $(($a))`
  wget --output-document=$store_location$file_prefix$fn $url
  sleep $frame_interval
done
