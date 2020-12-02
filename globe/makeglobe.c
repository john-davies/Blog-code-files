// makeglobe.cpp - 3D model creator
// Copyright (C) 2020 John Davies
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "makeglobe.h"

// Size of 1 arc minute files
#define SIZE_X 21600
#define SIZE_Y 10800
#define OUTPUT_FILE_NAME_SIZE 1024

// Used for writing image
#define RGB(r, g, b ) ((r) | ((g) << 8) | ((b) << 16))

// Array to store original file and mask file
// Note that this may not be portable
int16_t original[SIZE_X][SIZE_Y];
int16_t mask[SIZE_X][SIZE_Y];

int max_height;
int min_height;

int main( int argc, char *argv[] )
{
  FILE* input_file;
  FILE *mask_file;
  int xsize;
  int ysize;
  FILE* output_file;
  char output_file_name[OUTPUT_FILE_NAME_SIZE];

  printf( "makeimage, v0.1\n" );

  // Check command line
  if( argc != 4 )
  {
    printf( "ERROR: usage is: makeglobe <input file> <mask file> <xsize>\n" );
    printf( "  ( output will be written to <input file>.ply )\n" );
    return EXIT_FAILURE;
  }
  // Check input files
  input_file = fopen( argv[1], "r" );
  if( input_file == NULL )
  {
    printf( "ERROR: could not open input file: %s\n", argv[1] );
    return EXIT_FAILURE;
  }
  mask_file = fopen( argv[2], "r" );
  if( mask_file == NULL )
  {
    printf( "ERROR: could not open mask file: %s\n", argv[1] );
    return EXIT_FAILURE;
  }
  // Check output file
  // Create output file name
  snprintf( output_file_name, OUTPUT_FILE_NAME_SIZE, "%s.ply", argv[1] );
  output_file = fopen( output_file_name, "w" );
  if( output_file == NULL )
  {
    printf( "ERROR: could not open output file: %s\n", output_file_name );
    return EXIT_FAILURE;
  }
  // Check xsize factor
  xsize = atoi( argv[3] );
  if( ( xsize < 1 ) || ( xsize > SIZE_X ) || ( xsize % 2 != 0 ) )
  {
    printf( "ERROR: invalid X size: %s\n", argv[2] );
    return EXIT_FAILURE;
  }
  // Set ysize
  ysize = xsize / 2;

  // Create output file name
  snprintf( output_file_name, OUTPUT_FILE_NAME_SIZE, "%s.ply", argv[1] );

  // Read input file into Array
  int count = 0;
  int c1, c2;
  int16_t spot_height;
  max_height = 0;
  min_height = 0;
  printf( "Reading input file...\n" );
  for( int y = 0; y < ysize; y++ )
  {
    for( int x = 0; x < xsize; x++ )
    {
      // Read two characters and store
      c1 = fgetc( input_file );
      if( c1 == EOF )
      {
        printf( "ERROR: unexpected EOF reached while reading 1\n" );
        return EXIT_FAILURE;
      }
      c2 = fgetc( input_file );
      if( c2 == EOF )
      {
        printf( "ERROR: unexpected EOF reached while reading 2\n" );
        return EXIT_FAILURE;
      }
      spot_height = ( c1 << 8 ) + c2;
      if( spot_height > max_height )
      {
        max_height = spot_height;
      }
      if( spot_height < min_height )
      {
        min_height = spot_height;
      }
      original[x][y] = spot_height;
      count++;
    }
  }
  printf( "%d values read\n", count );
  printf( "  min: %d, max: %d\n", min_height, max_height );
  // Read mask file into Array
  printf( "Reading mask file...\n" );
  count = 0;
  for( int y = 0; y < ysize; y++ )
  {
    for( int x = 0; x < xsize; x++ )
    {
      // Read two characters and store
      c1 = fgetc( mask_file );
      if( c1 == EOF )
      {
        printf( "ERROR: unexpected EOF reached while reading 1\n" );
        return EXIT_FAILURE;
      }
      c2 = fgetc( mask_file );
      if( c2 == EOF )
      {
        printf( "ERROR: unexpected EOF reached while reading 2\n" );
        return EXIT_FAILURE;
      }
      mask[x][y] = ( c1 << 8 ) + c2;
      count++;
    }
  }
  printf( "%d values read\n", count );
  // Write image
  // Steps for shading
  int land_step = max_height / LAND_ROWS;
  int sea_step = -min_height / SEA_ROWS;

  printf( "Processing image file\n" );
  float planet_circumferance = 2 * M_PI * PLANET_RADIUS;
  int total_verticies = 0;
  // First a dummy run through to get the number of verticies
  for( int y = 0; y < ysize; y++ )
  {
    // Calculate latitude
    // Note 2nd term correction factor, e.g.
    // The 5 arc-min grids contain 2,160 x 4,320 data points, are 18 MB in
    // size and extend from -90+5/120 deg to  +90-5/120 deg in latitude
    // direction, and from -180+5/120 deg to +180-5/120 deg in longitude direction.
    float latitude = -90.0 + ( 180.0 / (float)ysize / 2 ) + ( (float)y * 180.0 ) / (float)ysize;
    // Calculate number of steps at this latitude
    // Radius at lat alpha degrees = radius of earth * cos( alpha )
    float radius = PLANET_RADIUS * cos( latitude * M_PI / 180.0 );
    // Calculate step for loop
    float step = planet_circumferance / ( 2 * M_PI * radius );
    // Loop through longitude values
    float x = 0;
    while( (int)x < xsize )
    {
      total_verticies ++;;
      x += step;
    }
  }
  printf( "Total verticies: %d\n", total_verticies );
  // Write 3D model to file
  printf( "Writing 3D file\n" );
  // Write PLY header information
  fprintf( output_file, "ply\n" );
  fprintf( output_file, "format ascii 1.0\n" );
  fprintf( output_file, "comment created by makeglobe\n" );
  fprintf( output_file, "element vertex %d\n", total_verticies );
  fprintf( output_file, "property float x\n" );
  fprintf( output_file, "property float y\n" );
  fprintf( output_file, "property float z\n" );
  fprintf( output_file, "property uchar red\n" );
  fprintf( output_file, "property uchar green\n" );
  fprintf( output_file, "property uchar blue\n" );
  fprintf( output_file, "property float nx\n" );
  fprintf( output_file, "property float ny\n" );
  fprintf( output_file, "property float nz\n" );
  fprintf( output_file, "end_header\n" );

  // Now a second run through to get the vertex values
  for( int y = 0; y < ysize; y++ )
  {
    float latitude = -90.0 + ( 180.0 / (float)ysize / 2 ) + ( (float)y * 180.0 ) / (float)ysize;
    float radius = PLANET_RADIUS * cos( latitude * M_PI / 180.0 );
    // Calculate step for loop
    float step = planet_circumferance / ( 2 * M_PI * radius );

    //printf( "Lat: %.6f, Step: %.6f, Count: %d\n", latitude, step, (int)(xsize/step) );


    // Loop through longitude values
    float x = 0;
    while( (int)x < xsize )
    {
      // Calculate longitude value
      // See note above about correction factor
      float longitude = -180.0 + ( 360.0 / (float)xsize / 2 ) + ( (float)x * 360.0 ) / (float)xsize;
      //printf( "%.6f|", longitude );
      // Convert to cartesian coordinates
      float xc = ( PLANET_RADIUS + ( original[(int)x][y] * MAG ) ) * cos( latitude * M_PI / 180.0 ) * cos( longitude * M_PI / 180.0 );
      float yc = ( PLANET_RADIUS + ( original[(int)x][y] * MAG ) ) * cos( latitude * M_PI / 180.0 ) * sin( longitude * M_PI / 180.0 );
      float zc = ( PLANET_RADIUS + ( original[(int)x][y] * MAG ) ) * sin( latitude * M_PI / 180.0 );
      // Calculate normals, set to point outwards
      float nxc = ( PLANET_RADIUS * 2 * cos( latitude * M_PI / 180.0 ) * cos( longitude * M_PI / 180.0) );
      float nyc = ( PLANET_RADIUS * 2 * cos( latitude * M_PI / 180.0 ) * sin( longitude * M_PI / 180.0) );
      float nzc = ( PLANET_RADIUS * 2 * sin( latitude * M_PI / 180.0 ) );
      // Get colours
      int r, g, b;
      int idx;
      // Process mask
      // http://ddfe.curtin.edu.au/models/Earth2014/readme_earth2014.dat
      // 0 - land topography above mean sea level (MSL)
      // 1 - land topography below MSL
      // 2 - ocean bathymetry
      // 3 - inland lake, bedrock above MSL
      // 4 - inland lake, bedrock below MSL
      // 5 - ice cover, bedrock above MSL
      // 6 - ice cover, bedrock below MSL
      // 7 - ice shelf
      // 8 - ice covered lake (Vostok)
      switch( mask[(int)x][y] )
      {
        case 0:
        case 1:
          if( original[(int)x][y] < 0 )
          {
            idx = 0;
          }
          else
          {
            idx = original[(int)x][y] / land_step;
          }
          r = land_gradient[idx][0];
          g = land_gradient[idx][1];
          b = land_gradient[idx][2];
          break;
        case 2:
        case 3:
        case 4:
          if( original[(int)x][y] > 0 )
          {
            idx = 0;
          }
          else
          {
            idx = -original[(int)x][y] / sea_step;
          }
          r = sea_gradient[idx][0];
          g = sea_gradient[idx][1];
          b = sea_gradient[idx][2];
          break;
        case 5:
        case 6:
        case 7:
        case 8:
          r = 255;
          g = 255;
          b = 255;
          break;
        default:
          printf( "ERROR: Invalid mask value found, setting to 0,0,0\n" );
          r = 0;
          g = 0;
          b = 0;
          break;
      }
      // Write values to ply file
      fprintf( output_file, "%.6f %.6f %.6f %d %d %d %.6f %.6f %.6f\n", xc, yc, zc, r, g, b, nxc, nyc, nzc );

      x += step;
    }
    //printf( "\n" );
  }

  fclose( output_file );
  return EXIT_SUCCESS;
}
