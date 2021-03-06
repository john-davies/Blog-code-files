// makeglobe.c - 3D model creator
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

unsigned char land_gradient[LAND_ROWS][LAND_COLUMNS];
unsigned char sea_gradient[SEA_ROWS][SEA_COLUMNS];

int max_height;
int min_height;

int main( int argc, char *argv[] )
{
  FILE* input_file;
  FILE *mask_file;
  int xsize;
  int ysize;
  int planet_radius;
  FILE* output_file;
  FILE *terrain_LUT_file;
  FILE *bath_LUT_file;
  int magnification;
  char output_file_name[OUTPUT_FILE_NAME_SIZE];

  printf( "makeglobe, v0.1\n" );

  // Check command line
  if( ( argc != 7 ) && ( argc != 8 ) )
  {
    printf( "ERROR: usage is: makeglobe <input file> <mask file> <terrain LUT> <bathymetry LUT> <xsize> <planet radius> [magnification]\n" );
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
  // Read terrain LUT
  terrain_LUT_file = fopen( argv[3], "r" );
  // Read bathymetry LUT
  if( terrain_LUT_file == NULL )
  {
    printf( "ERROR: could not open terrain LUT file: %s\n", argv[3] );
    return EXIT_FAILURE;
  }
  // Read bathymetry LUT
  bath_LUT_file = fopen( argv[4], "r" );
  // Read bathymetry LUT
  if( bath_LUT_file == NULL )
  {
    printf( "ERROR: could not open bathymetry LUT file: %s\n", argv[3] );
    return EXIT_FAILURE;
  }
  // Check xsize factor
  xsize = atoi( argv[5] );
  if( ( xsize < 1 ) || ( xsize > SIZE_X ) || ( xsize % 2 != 0 ) )
  {
    printf( "ERROR: invalid X size: %s\n", argv[5] );
    return EXIT_FAILURE;
  }
  // Check planet radius
  planet_radius = atoi( argv[6] );
  if( planet_radius < 1 )
  {
    printf( "ERROR: invalid planet radius: %s\n", argv[6] );
    return EXIT_FAILURE;
  }
  // Check long parameter
  if( argc == 8 )
  {
    magnification = atoi(argv[7]);
    if( magnification < 0 )
    {
      // Out of range so set to 0
      printf( "WARNING: invalid magnification value: %s\n", argv[7] );
      printf( "  Must be > 0, setting to 1\n" );
      magnification = 1;
    }
  }
  else
  {
    magnification = 1;
  }
  printf( "Magnification = %d\n", magnification );

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

  // Read terrain LUT into array
  unsigned char channel[LAND_ROWS];
  // Read red channel
  if( fread( channel, LAND_ROWS, 1, terrain_LUT_file ) != 1 )
  {
    printf( "ERROR: unexpected EOF reached while reading terrain LUT red channel\n" );
    return EXIT_FAILURE;
  }
  for( int x = 0; x < LAND_ROWS; x++ )
  {
    land_gradient[x][0] = channel[x];
  }
  // Read green channel
  if( fread( channel, LAND_ROWS, 1, terrain_LUT_file ) != 1 )
  {
    printf( "ERROR: unexpected EOF reached while reading terrain LUT green channel\n" );
    return EXIT_FAILURE;
  }
  for( int x = 0; x < LAND_ROWS; x++ )
  {
    land_gradient[x][1] = channel[x];
  }
  // Read blue channel
  if( fread( channel, LAND_ROWS, 1, terrain_LUT_file ) != 1 )
  {
    printf( "ERROR: unexpected EOF reached while reading terrain LUT blue channel\n" );
    return EXIT_FAILURE;
  }
  for( int x = 0; x < LAND_ROWS; x++ )
  {
    land_gradient[x][2] = channel[x];
  }

  // Read bathymetry LUT into array
  unsigned char sea_channel[SEA_ROWS];
  // Read red channel
  if( fread( sea_channel, SEA_ROWS, 1, bath_LUT_file ) != 1 )
  {
    printf( "ERROR: unexpected EOF reached while reading bath LUT red channel\n" );
    return EXIT_FAILURE;
  }
  for( int x = 0; x < SEA_ROWS; x++ )
  {
    sea_gradient[x][0] = sea_channel[x];
  }
  // Read green channel
  if( fread( sea_channel, SEA_ROWS, 1, bath_LUT_file ) != 1 )
  {
    printf( "ERROR: unexpected EOF reached while reading bath LUT green channel\n" );
    return EXIT_FAILURE;
  }
  for( int x = 0; x < SEA_ROWS; x++ )
  {
    sea_gradient[x][1] = sea_channel[x];
  }
  // Read blue channel
  if( fread( sea_channel, SEA_ROWS, 1, bath_LUT_file ) != 1 )
  {
    printf( "ERROR: unexpected EOF reached while reading bath LUT blue channel\n" );
    return EXIT_FAILURE;
  }
  for( int x = 0; x < SEA_ROWS; x++ )
  {
    sea_gradient[x][2] = sea_channel[x];
  }

  // Write model
  // Steps for shading
  float land_step = (float) max_height / (float) LAND_ROWS;
  float sea_step = (float) -min_height / (float) SEA_ROWS;

  // Write 3D model to file
  printf( "Writing 3D file\n" );
  // Write PLY header information
  fprintf( output_file, "ply\n" );
  fprintf( output_file, "format ascii 1.0\n" );
  fprintf( output_file, "comment created by makeglobe\n" );
  fprintf( output_file, "element vertex %d\n", xsize * ysize );
  fprintf( output_file, "property float x\n" );
  fprintf( output_file, "property float y\n" );
  fprintf( output_file, "property float z\n" );
  fprintf( output_file, "property uchar red\n" );
  fprintf( output_file, "property uchar green\n" );
  fprintf( output_file, "property uchar blue\n" );
  fprintf( output_file, "property float nx\n" );
  fprintf( output_file, "property float ny\n" );
  fprintf( output_file, "property float nz\n" );
  fprintf( output_file, "element face %d\n", xsize * ( ysize-1 ) );
  fprintf( output_file, "property list int int vertex_index\n" );
  fprintf( output_file, "end_header\n" );

  // First write the verticies
  printf( "  Writing verticies ...\n");
  for( int y = 0; y < ysize; y++ )
  {
    float latitude = -90.0 + ( 180.0 / (float)ysize / 2 ) + ( (float)y * 180.0 ) / (float)ysize;
    //printf( "Lat: %.6f, Step: %.6f, Count: %d\n", latitude, step, (int)(xsize/step) );

    // Loop through longitude values
    for( int x = 0; x < xsize; x++ )
    {
      // Calculate longitude value
      // See note above about correction factor
      float longitude = -180.0 + ( 360.0 / (float)xsize / 2 ) + ( (float)x * 360.0 ) / (float)xsize;
      //printf( "%.6f|", longitude );
      // Convert to cartesian coordinates
      float xc = ( planet_radius + ( original[(int)x][y] * magnification ) ) * cos( latitude * M_PI / 180.0 ) * cos( longitude * M_PI / 180.0 );
      float yc = ( planet_radius + ( original[(int)x][y] * magnification ) ) * cos( latitude * M_PI / 180.0 ) * sin( longitude * M_PI / 180.0 );
      float zc = ( planet_radius + ( original[(int)x][y] * magnification ) ) * sin( latitude * M_PI / 180.0 );
      // Calculate normals, set to point outwards
      float nxc = ( planet_radius * 2 * cos( latitude * M_PI / 180.0 ) * cos( longitude * M_PI / 180.0) );
      float nyc = ( planet_radius * 2 * cos( latitude * M_PI / 180.0 ) * sin( longitude * M_PI / 180.0) );
      float nzc = ( planet_radius * 2 * sin( latitude * M_PI / 180.0 ) );
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
            idx = (float) original[(int)x][y] / land_step;
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
            idx = (float) -original[(int)x][y] / sea_step;
          }
          r = sea_gradient[SEA_ROWS - 1 - idx][0];
          g = sea_gradient[SEA_ROWS - 1 - idx][1];
          b = sea_gradient[SEA_ROWS - 1 - idx][2];
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
    }
    //printf( "\n" );
  }

  // Then the faces
  printf( "  Writing faces ...\n");
  for( int y = 0; y < ysize - 1; y++ )
  {
    for( int x = 0; x < xsize - 1; x++ )
    {
      fprintf( output_file, "4 %d %d %d %d\n",
                            ( x + ( y * xsize ) ), // bottom left
                            ( ( x + 1 ) + ( y * xsize ) ), // bottom right
                            ( ( x + 1 ) + ( ( y + 1 ) * xsize ) ), // top right
                            ( x + ( ( y + 1 ) * xsize ) ) // top left
                          );
    }
    // Loop back to start
    fprintf( output_file, "4 %d %d %d %d\n",
                          ( ( xsize - 1 ) + ( y * xsize ) ), // bottom left
                          ( 0 + ( y * xsize ) ), // bottom right
                          ( 0 + ( ( y + 1 ) * xsize ) ), // top right
                          ( ( xsize - 1 ) + ( ( y + 1 ) * xsize ) ) // top left
                        );

  }

  fclose( output_file );
  return EXIT_SUCCESS;
}
