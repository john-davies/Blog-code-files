// rescale.c - Rescaler for .bin files
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

// Size of 1 arc minute files
#define SIZE_X 21600
#define SIZE_Y 10800

// Array to store original file
// Note that this may not be portable
int16_t original[SIZE_X][SIZE_Y];

int main( int argc, char *argv[] )
{
  FILE* input_file;
  int scale;
  FILE* output_file;

  printf( "rescaler, v0.1\n" );

  // Check command line
  if( argc != 4 )
  {
    printf( "ERROR: usage is: rescale <input file> <scale> <output file >\n" );
    printf( "  ( Scale is an integer, e.g. 2 = reduce by half )\n" );
    return EXIT_FAILURE;
  }
  // Check input file
  input_file = fopen( argv[1], "r" );
  if( input_file == NULL )
  {
    printf( "ERROR: could not open input file: %s\n", argv[1] );
    return EXIT_FAILURE;
  }
  // Check scale factor
  scale = atoi( argv[2] );
  if( scale < 1 )
  {
    printf( "ERROR: invalid scale factor: %s\n", argv[2] );
    return EXIT_FAILURE;
  }
  // Check output file
  output_file = fopen( argv[3], "w" );
  if( output_file == NULL )
  {
    printf( "ERROR: could not open output file: %s\n", argv[3] );
    return EXIT_FAILURE;
  }
  // Read input file into Array
  int count = 0;
  int c1, c2;
  for( int y = 0; y < SIZE_Y; y++ )
  {
    for( int x = 0; x < SIZE_X; x++ )
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
      original[x][y] = ( c1 << 8 ) + c2;
      count++;
    }
  }
  printf( "%d values read\n", count );

  // Write out
  count = 0;
  for( int y = 0; y < SIZE_Y; y+=scale )
  {
    for( int x = 0; x < SIZE_X; x+=scale )
    {
      if( fputc( original[x][y] >> 8, output_file ) == EOF )
      {
        printf( "ERROR: unexpected EOF reached while writing 1\n" );
        return EXIT_FAILURE;
      }
      if( fputc( original[x][y], output_file ) == EOF )
      {
        printf( "ERROR: unexpected EOF reached while writing 2\n" );
        return EXIT_FAILURE;
      }
      count++;
    }
  }
  printf( "%d values written\n", count );

  return EXIT_SUCCESS;
}
