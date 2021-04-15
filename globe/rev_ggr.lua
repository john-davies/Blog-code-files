#! /usr/local/bin/lua
--
-- rev_ggr.sh - Script to reverse the colours in a Gimp ggr file
--              Usage is rev_ggr.lua <input file> <output file>
-- Copyright (C) 2021 John Davies
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http:--www.gnu.org/licenses/>.

print "rev_ggr.lua - V0.1"

-- Check command line parameters
if arg[1] == nil then
  print "Usage: rev_ggr.lua <input file> <output file>"
  os.exit(-1)
end

-- Check that the input file exists
local inputFile  = io.open( arg[1], "r")
if inputFile == nil then
  print( "ERROR - could not open input file: " + arg[1] )
  os.exit(-1)
end

-- Check that the output file is writable
local outputFile  = io.open( arg[2], "w")
if outputFile == nil then
  print( "ERROR - could not open output file: " + arg[2] )
  os.exit(-1)
end

-- Loop through input file
-- Read first three lines which stay the same
local line1 = inputFile:read("*line")
local line2 = inputFile:read("*line")
local line3 = inputFile:read("*line")
-- Then the rest
part1 = {}
part2 = {}
line = inputFile:read("*line")
while line do
  -- This is a slight bodge as it assumes that each of the first
  -- three values have 6 decimal places and are separated by a space
  local sub_string = string.sub( line, 1, 27 )
  part1[#part1+1] = sub_string
  sub_string = string.sub( line, 28 )
  part2[#part2+1] = sub_string
  line = inputFile:read("*line")
end

-- Write output file
-- First three lines
outputFile:write( string.format( "%s\n", line1 ) )
outputFile:write( string.format( "%s\n", line2 ) )
outputFile:write( string.format( "%s\n", line3 ) )
-- Then the rest
for i=1,#part1 do
  outputFile:write( string.format( "%s%s\n", part1[i], part2[#part2 + 1 - i] ) )
end
