#! /usr/local/bin/lua

-- lidar2ply.lua - script to convert LiDAR files to PLY
-- LiDAR files from http://lle.gov.wales/Catalogue/Item/LidarCompositeDataset/?lang=en
-- Usage: lidar2ply.lua <input file> [ options]
-- Options: -i <image file> : specify image overlay
--          -f              : add false colour
--          -x <value>      : add X axis offset to PLY model
--          -y <value>      : add Y axis offset to PLY model
--          -z <value>      : add Z axis offset to PLY model
--
-- Copyright (C) 2017 John Davies
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
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.


-- --------------------------------------------------------------------
-- Simple command line option parser. Options can be a single letter
-- only, valid list of options passed in as a string in "options"
-- Two types of return values:
-- -a      => option_list.a = true
-- -a text => option_list.a = text

function getopt( arg_list, options )

  local option_list = {}
  for k, v in ipairs(arg_list) do
    -- Check is this is an option: first char is '-' and the second
    -- char is part of the 'options' string
    if string.sub( arg_list[k], 1, 1 ) == "-" and
       string.find( options, string.sub( arg_list[k], 2, 2 ) ) ~= nil then
      if k == #arg_list then
        -- Check if this is the last argument, if so then just set it to true
        option_list[string.sub( v,2,2 )] = true
      elseif not( string.sub( arg_list[k+1], 1, 1 ) == "-" and
         string.find( options, string.sub( arg_list[k+1], 2, 2 ) ) ~= nil ) then
        -- Check if the next argument is a new option or a value for this argument
        option_list[string.sub( v,2,2 )] = arg_list[k+1]
      else
        -- Next argument is a new option
        option_list[string.sub( v,2,2 )] = true
      end
    end
  end

  return option_list

end

-- --------------------------------------------------------------------
-- https://www.rosettacode.org/wiki/Tokenize_a_string#Lua
function string:split (sep)
    local sep, fields = sep or ":", {}
    local pattern = string.format("([^%s]+)", sep)
    self:gsub(pattern, function(c) fields[#fields+1] = c end)
    return fields
end

-- --------------------------------------------------------------------
-- http://lua-users.org/wiki/StringTrim
function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

-- --------------------------------------------------------------------
function getRGB3( value )

  local colourMap = { { 0, 0, 1 },
                      { 0, 1, 1 },
                      { 0, 1, 0 },
                      { 1, 1, 0 },
                      { 1, 0, 0 } }

  -- Find the two bounding values
  local lowBoundIdx, upperBoundIdx, lowBoundVal, upperBoundVal
  for i=2,#colourMap do
    lowBoundIdx = i-1
    upperBoundIdx = i
    -- Lua arrays start at 1
    lowBoundVal =  ( i - 2 ) / ( #colourMap - 1 )
    upperBoundVal = ( i - 1 ) / ( #colourMap - 1 )
    if value < upperBoundVal then
      break
    end
  end

  local red, green, blue
  -- Adjust the value to fit in the range
  local adjValue = value - lowBoundVal
  -- Using y = mx + c
  -- Red
  local m = ( colourMap[upperBoundIdx][1] - colourMap[lowBoundIdx][1] ) /
                    ( upperBoundVal - lowBoundVal)
  local c = colourMap[lowBoundIdx][1]
  red = ( m * adjValue ) + c
  -- Green
  m = ( colourMap[upperBoundIdx][2] - colourMap[lowBoundIdx][2] ) /
                    ( upperBoundVal - lowBoundVal)
  c = colourMap[lowBoundIdx][2]
  green = ( m * adjValue ) + c
  -- Blue
  m = ( colourMap[upperBoundIdx][3] - colourMap[lowBoundIdx][3] ) /
                    ( upperBoundVal - lowBoundVal)
  c = colourMap[lowBoundIdx][3]
  blue = ( m * adjValue ) + c


  return string.format( "%.0f %.0f %.0f", red * 255, green * 255, blue * 255 )

end

-- ========================================================================
-- Start
-- ========================================================================

-- ------------------------
-- Read the LiDAR data file
-- ------------------------

-- Check that an input file is specified and it exists
if arg[1] == nil then
  print( "Usage: lidar2ply.lua <input file> [ options]" )
  print( "Options: -i <image file> : specify image overlay" )
  print( "            ( note that image must be in Imagemagick text format )")
  print( "         -f              : add false colour" )
  print( "         -x <value>      : add X axis offset to PLY model" )
  print( "         -y <value>      : add Y axis offset to PLY model" )
  print( "         -z <value>      : add Z axis offset to PLY model" )
  os.exit(-1)
end

local inputFile  = io.open( arg[1], "r")

if inputFile == nil then
  print( "ERROR - could not open input file" )
  os.exit(-1)
end

-- Parse the command line arguments
local opts = getopt( arg, "ifxyz" )

-- If there's an image file specified then check that it exists
local imageFile = nil
if opts.i ~= nil and opts.i ~= true then
  imageFile = io.open( opts.i, "r")
  if imageFile == nil then
    print( "ERROR - could not open image file" )
    os.exit(-1)
  end
end

-- Check for false colour
local haveRGB = false
if opts.f == true then
  haveRGB = true
end

-- Output offsets
local xoffset = 0
local yoffset = 0
local zoffset = 0

if opts.x ~= nil and opts.x ~= true then
  xoffset = opts.x
  print( "Adding x offset to output: " .. xoffset )
end

if opts.y ~= nil and opts.y ~= true  then
  yoffset = opts.y
  print( "Adding y offset to output: " .. yoffset )
end

if opts.z ~= nil and opts.z ~= true then
  zoffset = opts.z
  print( "Adding y offset to output: " .. zoffset )
end

-- Parse LiDAR file headers
local ncols=nil
local nrows=nil
local xllcorner=nil
local yllcorner=nil
local cellsize=nil
local NODATA_value=nil

-- Read the header lines, not sure if the order is fixed so handle any order
for row = 1,6 do
  local line = inputFile:read("*line")
  local fields = line:split(" ")
  if fields[1] == "ncols" then
    ncols = fields[2]
  elseif fields[1] == "nrows" then
    nrows = fields[2]
  elseif fields[1] == "xllcorner" then
    xllcorner = fields[2]
  elseif fields[1] == "yllcorner" then
    yllcorner = fields[2]
  elseif fields[1] == "cellsize" then
    cellsize = fields[2]
  elseif fields[1] == "NODATA_value" then
    NODATA_value = fields[2]
  end
end

print( "Input file parameters:")
print( "ncols: " .. ncols )
print( "nrows: " .. nrows )
print( "xllcorner: " .. xllcorner )
print( "yllcorner: " .. yllcorner )
print( "cellsize: " .. cellsize )
print( "NODATA_value: " .. NODATA_value )

-- Check that they've all been read
assert( ncols~=nil and nrows~=nil and xllcorner~=nil and yllcorner ~=nil and cellsize~= nil and NODATA_value~=nil )

-- Create a multidimensional array to hold the height data and read in a row at a time
-- https://www.lua.org/pil/11.2.html

-- Data for output
local missingData = 0
local maxHeight = -1000
local minHeight = 1000

heightData = {}          -- create the matrix
local currentVertexNo = 0
--local currentRow = 1
local currentRow = tonumber(nrows)
line = inputFile:read("*line")
while line do
  -- Add a new row
  heightData[currentRow] = {}
  stripLine = string.gsub( line, "^%s*(.-)%s*$", "%1") --Trim string
  fields = stripLine:split(" ")
  -- Add row to array
  -- Parse row
  for i, v in ipairs(fields) do
    local vertex = {}
    vertex.heightValue = tonumber(v)
    if vertex.heightValue == tonumber(NODATA_value) then
      missingData = missingData + 1
      vertex.number = nil
    else
      vertex.number = currentVertexNo
      currentVertexNo = currentVertexNo + 1
    end
    heightData[currentRow][i] = vertex

    -- For RGB output
    if vertex.number ~= nil then
      if tonumber(v) > maxHeight then
        maxHeight = tonumber(v)
      elseif tonumber(v) < minHeight then
        minHeight = tonumber(v)
      end
    end
  end
  --currentRow = currentRow + 1
  currentRow = currentRow - 1
  line = inputFile:read("*line")
end

--[[ Debug
for r=nrows, 1, -1 do
  io.write( "r:" .. r .. " - ")
  for c=1, ncols do
    io.write( heightData[r][c].heightValue .. "/" )
    if( heightData[r][c].number ~= nil ) then
      io.write( heightData[r][c].number .. " ")
    else
      io.write( "* " )
    end
  end
  io.write( "\n" )
end]]

print( "Missing data: " .. missingData )
print( "Max height: " .. maxHeight )
print( "Min height: " .. minHeight )

-- -------------------------------------------------
-- Check to see if an image overlay file is supplied
-- -------------------------------------------------
local haveImage = false
local imageData = {}

-- Check for image overlay or false colour
if haveRGB == false and imageFile ~= nil then
  -- Image overlay
  local imageLine = imageFile:read("*line")
  -- Check that the image dimensions are correct
  -- Line is similar to: # ImageMagick pixel enumeration: 500,500,255,srgb
  -- Split on :
  local fields = imageLine:split( ":")
  local params = trim( fields[2] )
  local fields = params:split(",")
  local imageRows = tonumber(fields[2])
  local imageCols = tonumber(fields[1])
  if ( imageRows == tonumber(nrows) ) and ( imageCols == tonumber(ncols) ) then
    print( "Image size OK")
    -- Initially fill the image array with black pixels
    for y=1,imageRows do
      imageData[y] = {}     -- create a new row
      for x=1,imageCols do
        imageData[y][x] = "0 0 0"
      end
    end

    -- Import image
    -- Image lines are similar to: 0,0: (70,56,200)  #4638C8  srgb(70,56,200)
    --                             0,0: (89,169,193,1)  #59A9C1  srgba(89,169,193,1)
    local imageLine = imageFile:read("*line")
    while imageLine ~= nil do
      fields = imageLine:split(":")
      local pixel = fields[1]:split(",")
      local colour = trim(fields[2]):split(" ")
      local values = colour[1]:gsub("[()]",""):split(",")
      imageData[imageRows-(tonumber(pixel[2]))][tonumber(pixel[1])+1] = values[1] .. " " .. values[2] .. " " .. values[3]
      imageLine = imageFile:read("*line")
    end
    haveImage = true
  else
    print( "Image size mismatch - image will not be used")
  end
end

-- -------------------
-- Output the PLY file
-- -------------------

local outputFile  = io.open( arg[1] .. ".ply", "w")
if outputFile == nil then
  print( "ERROR - can't open output file" )
  os.exit(-1)
end
print("Writing PLY file")

-- Build faces list before writing headers
--element face 7
--property list uchar int vertex_index
faceList = {}
for row=1,nrows-1 do
  for col=0,ncols-2 do
    local a,b,c,d = nil
    if( tonumber(heightData[row][col+1].heightValue) ~= tonumber(NODATA_value) ) then
      a = heightData[row][col+1].number
    end
    if( tonumber(heightData[row][col+2].heightValue) ~= tonumber(NODATA_value) ) then
      b = heightData[row][col+2].number
    end
    if( tonumber(heightData[row+1][col+1].heightValue) ~= tonumber(NODATA_value) ) then
      c = heightData[row+1][col+1].number
    end
    if( tonumber(heightData[row+1][col+2].heightValue) ~= tonumber(NODATA_value) ) then
      d = heightData[row+1][col+2].number
    end
    if ( a ~= nil ) and ( d ~= nil ) and ( b ~= nil ) then
      faceList[#faceList+1] = string.format( "3 %d %d %d", a, d, b )
    end
    if ( a ~= nil ) and ( c ~= nil ) and ( d ~= nil ) then
      faceList[#faceList+1] = string.format( "3 %d %d %d", a, c, d )
    end
  end
end

print( "Writing " .. math.floor( ( nrows * ncols ) - missingData ) .. " verticies")
print( "Writing " .. #faceList .. " faces")

outputFile:write( "ply\n" )
outputFile:write( "format ascii 1.0\n" )
outputFile:write( "comment author: John Davies\n" )
outputFile:write( "comment object created from: " .. arg[1] .. "\n" )
outputFile:write( string.format( "element vertex %d\n", ( nrows * ncols ) - missingData ) )
outputFile:write( "property float x\n" )
outputFile:write( "property float y\n" )
outputFile:write( "property float z\n" )
outputFile:write( "property float nx\n" )
outputFile:write( "property float ny\n" )
outputFile:write( "property float nz\n" )
outputFile:write( "property uchar red\n" )
outputFile:write( "property uchar green\n" )
outputFile:write( "property uchar blue\n" )
outputFile:write( string.format( "element face %d\n", #faceList ) )
outputFile:write( "property list int int vertex_index\n" )
outputFile:write( "end_header\n" )

-- Verticies

for r=nrows,1,-1 do
  for c=1,ncols do
    local currentHeight = tonumber(heightData[r][c].heightValue)
    if currentHeight ~= tonumber(NODATA_value) then
      -- Get the RGB output string
      local RGB
      if haveImage == false then
        -- No image overlay so check for false colour
        if haveRGB == true then
          RGB = getRGB3( ( currentHeight - minHeight ) / ( maxHeight - minHeight ) )
        else
          -- Mid grey colour
          RGB = "128 128 128"
        end
      else
        -- Fetch image pixel colour
        RGB = imageData[r][c]
      end
      -- Make all normals point "up", make scale ( value ) = 1.0
      outputFile:write( ( (c-1) * cellsize ) + xoffset .. " " .. ( (r-1) * cellsize ) + yoffset .. " " .. currentHeight + zoffset .. " 0.0 0.0 1.0 " .. RGB .. "\n" )
    end
  end
end

-- Faces

for f = 1,#faceList do
  outputFile:write( faceList[f] .. "\n" )
end

outputFile:close()
