// makeinage.c - PNG creator
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "makeimage.h"
#include "libattopng.h"

// Size of 1 arc minute files
#define SIZE_X 21600
#define SIZE_Y 10800
#define OUTPUT_FILE_NAME_SIZE 1024

// Used for writing image
#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))

// Array to store original file and mask file
// Note that this may not be portable
int16_t original[SIZE_X][SIZE_Y];
int16_t mask[SIZE_X][SIZE_Y];
int image[SIZE_X][SIZE_Y];

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
    printf( "ERROR: usage is: makeimage <input file> <mask file> <xsize>\n" );
    printf( "  ( output will be written to <input file>.png )\n" );
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
  // Check xsize factor
  xsize = atoi( argv[3] );
  if( ( xsize < 1 ) || ( xsize > SIZE_X ) || ( xsize % 2 != 0 ) )
  {
    printf( "ERROR: invalid X size: %s\n", argv[2] );
    return EXIT_FAILURE;
  }
  // Set ysize
  ysize = xsize / 2;

  // Check output file
  snprintf( output_file_name, OUTPUT_FILE_NAME_SIZE, "%s.png", argv[1] );
/*  output_file = fopen( output_file_name, "w" );
  if( output_file == NULL )
  {
    printf( "ERROR: could not open output file: %s.png\n", argv[1] );
    return EXIT_FAILURE;
  }*/
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
  printf( "Building image file\n" );
  for( int y = 0; y < ysize; y++ )
  {
    for( int x = 0; x < xsize; x++ )
    {
      int r, g, b, a;
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
      switch( mask[x][y] )
      {
        case 0:
        case 1:
          if( original[x][y] < 0 )
          {
            idx = 0;
          }
          else
          {
            idx = original[x][y] / land_step;
          }
          r = land_gradient[idx][0];
          g = land_gradient[idx][1];
          b = land_gradient[idx][2];
          a = land_gradient[idx][3];
          break;
        case 2:
        case 3:
        case 4:
          if( original[x][y] > 0 )
          {
            idx = 0;
          }
          else
          {
            idx = -original[x][y] / sea_step;
          }
          r = sea_gradient[idx][0];
          g = sea_gradient[idx][1];
          b = sea_gradient[idx][2];
          a = sea_gradient[idx][3];
          break;
        case 5:
        case 6:
        case 7:
        case 8:
          r = 255;
          g = 255;
          b = 255;
          a = 255;
          break;
        default:
          printf( "ERROR: Invalid mask value found, setting to 0,0,0\n" );
          r = 0;
          g = 0;
          b = 0;
          a = 0;
          break;
      }
      image[x][y] = RGBA( r, g, b, 255);
    }
  }
  // Write image to file
  printf( "Writing image file\n" );
  libattopng_t* png = libattopng_new( xsize, ysize, PNG_RGBA );
  for( int y = 0; y < ysize; y++ )
  {
    for( int x = 0; x < xsize; x++ )
    {
      libattopng_set_pixel(png, x, ysize - y, image[x][y]);
    }
    //printf( "%d\n", y );
  }
  printf( "Writing to disk\n" );
  libattopng_save( png, output_file_name );
  printf( "Cleaning up\n" );
  //libattopng_destroy( png );

  return EXIT_SUCCESS;
}
