// tif2bin.c - Converts a signed 16 bit TIF file to a binary format file for
//             creating an image or globe model
// Copyright (C) 2021 John Davies
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
#include <limits.h>
#include "tiffio.h"

int main( int argc, char *argv[] )
{
  FILE* input_file;
  FILE* output_file;

  printf( "tif2bin, v0.1\n" );

  // Check command line
  if( argc != 3 )
  {
    printf( "ERROR: usage is: tif2bin <input file> <output file>\n" );

    return EXIT_FAILURE;
  }

  // Check output file
  output_file = fopen( argv[2], "w" );
  if( output_file == NULL )
  {
    printf( "ERROR: could not open output file: %s\n", argv[2] );
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
  TIFF* tif = TIFFOpen( argv[1], "r" );
  // Check number of images in file
  if( tif )
  {
    int dircount = 0;
    do
    {
        dircount++;
    } while( TIFFReadDirectory( tif ) );
    if( dircount != 1 )
    {
      printf( "ERROR: too many images in TIFF file: %d\n", dircount );
      return EXIT_FAILURE;
    }
  }
  else
  {
    printf( "ERROR: could not read TIFF file: %s\n", argv[1] );
    return EXIT_FAILURE;
  }
  //
  uint32 imagelength;
  int nsamples;
	tdata_t buf;
	uint32 TIFFrow;

	TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &imagelength );
  TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &nsamples );
  printf( "%d rows read, sample size: %d, line size: %ld\n", imagelength, nsamples, TIFFScanlineSize( tif ) );
	buf = _TIFFmalloc( TIFFScanlineSize( tif ) );
  int max = INT_MIN;
  int min = INT_MAX;
  // First pass through to get min/max values
  for( TIFFrow = 0; TIFFrow < imagelength; TIFFrow++)
  {
	    TIFFReadScanline( tif, buf, TIFFrow, 0 );
      // Step through line data
      for( uint32 idx=0; idx<TIFFScanlineSize( tif ); idx+=2 )
      {
        int16 val = *((int16 *)( buf + idx ));
        if( val > max )
        {
          max = val;
        }
        if( val < min )
        {
          min = val;
        }
      }
  }
  // Second pass to write the data
  printf( "Minimum value: %d, maximum value: %d\n", min, max );
  printf( "Adding offset of: %d\n", -min );
	for( TIFFrow = 0; TIFFrow < imagelength; TIFFrow++)
  {
	    TIFFReadScanline( tif, buf, ( imagelength - TIFFrow - 1 ), 0 );
      // Step through line data
      for( uint32 idx=0; idx<TIFFScanlineSize( tif ); idx+=2 )
      {
        int16 val = *((int16 *)( buf + idx ));
        // Scale the value
        val = val - min;
        // Output data
        if( fputc( val >> 8, output_file ) == EOF )
        {
          printf( "ERROR: unexpected EOF reached while writing 1\n" );
          return EXIT_FAILURE;
        }
        if( fputc( val, output_file ) == EOF )
        {
          printf( "ERROR: unexpected EOF reached while writing 2\n" );
          return EXIT_FAILURE;
        }
      }
  }
	_TIFFfree(buf);
  TIFFClose(tif);

  return EXIT_SUCCESS;
}
