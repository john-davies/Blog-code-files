 // img2bml.c - a converter to create BML files from image files
 //
 // Copyright (C) 2018 John Davies
 //
 // Usage: img2bml <file name>
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
 // along with this program.  If not, see <http://www.gnu.org/licenses/>

#include <stdlib.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// ------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
  // Check for image file present
  if( argc == 1 )
  {
    printf( "ERROR - no image file specified\n" );
    printf( "  Usage is img2bml <file name>\n" );
    return EXIT_FAILURE;
  }

  // Try to read file
  int rows, columns, n;
  unsigned char *data = stbi_load( argv[1], &columns, &rows, &n, 0);
  if( data == NULL )
  {
    printf( "ERROR - failed to read file\n" );
    return EXIT_FAILURE;
  }

  // Process data
  printf( "Image data - " );
  printf( "Rows: %d, ", rows );
  printf( "Columns: %d, ", columns );
  printf( "Depth: %d\n", n );

  // Setup output file
  char *output_file_name = malloc( strlen( argv[1] ) + 5 );
  if( output_file_name == NULL )
  {
    printf( "ERROR - malloc fail for output_file_name\n" );
    return EXIT_FAILURE;
  }
  strcpy( output_file_name, argv[1] );
  strcat( output_file_name, ".bml" );

  // Write the output file
  FILE *output_file = fopen( output_file_name, "w" );
  if( output_file == NULL )
  {
    printf( "ERROR - failed to create output file: %s\n", output_file_name );
    return EXIT_FAILURE;
  }

  fprintf( output_file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
  fprintf( output_file, "<blm width=\"%d\" height=\"%d\" bits=\"8\" channels=\"%d\">\n",
                          columns, rows, n );
  fprintf( output_file, "  <header>\n" );
  fprintf( output_file, "    <title>Created by img2bml</title>\n" );
  fprintf( output_file, "    <url>https://github.com/john-davies/Blog-code-files/tree/master/LED_Panel</url>\n" );
  fprintf( output_file, "  </header>\n" );
  fprintf( output_file, "  <frame duration=\"100\">\n" );

  // Loop through rows & columns
  int r, c;
  int row_ptr;
  for( r=0; r<rows; r++ )
  {
    row_ptr = r * columns * n;
    fprintf( output_file, "    <row>" );
      for( c=0; c<columns; c++ )
      {
          fprintf( output_file, "%02x%02x%02x", data[row_ptr], data[row_ptr+1], data[row_ptr+2] );
          row_ptr += n;
      }
    fprintf( output_file, "</row>\n" );
  }

  fprintf( output_file, "  </frame>\n" );
  fprintf( output_file, "</blm>\n" );

  // Clean up
  printf( "Written file: %s\n", output_file_name );
  fclose( output_file );
  free( output_file_name );
  stbi_image_free(data);

  return EXIT_SUCCESS;
}
