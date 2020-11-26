// gradient.c - Extract colour gradient
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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main( int argc, char *argv[] )
{
  FILE* input_file;

  printf( "Gradient extractor, v0.1\n" );

  // Check command line
  if( argc != 2 )
  {
    printf( "ERROR: usage is: gradient <input file>\n" );
    return EXIT_FAILURE;
  }
  // Check input file
  input_file = fopen( argv[1], "r" );
  if( input_file == NULL )
  {
    printf( "ERROR: could not open input file: %s\n", argv[1] );
    return EXIT_FAILURE;
  }
  // Read input file
  int rows, columns, n;
  unsigned char *data = stbi_load( argv[1], &columns, &rows, &n, 0);
  if( data == NULL )
  {
    printf( "ERROR - failed to read file\n" );
    return EXIT_FAILURE;
  }
  printf( "Read %d rows, %d columns, %d channels\n", rows, columns, n );
  for( int i = 0; i < columns; i++ )
  {
    int index = n * i;
    printf( "{%d, %d, %d, %d },\n", data[index], data[index+1], data[index+2], data[index+3] );
  }

  return EXIT_SUCCESS;
}
