#include <stdio.h>
#include <stdlib.h>

int main( int argv, char *argc[] )
{
	FILE *inputFile;
	FILE *outputFile;
	int val;

	if( argv != 3 )
	{
		printf( "ERROR: wrong no of arguments\n" );
		printf( "Usage: cng2jpg <source file> <destination file>\n");
		return EXIT_FAILURE;
	}
	else
	{
		inputFile = fopen( argc[1], "r" );
		if( inputFile == NULL )
		{
			printf( "ERROR: can't open input file\n" );
			return EXIT_FAILURE;
		}
		else
		{
			outputFile = fopen( argc[2], "w" );
			if( outputFile == NULL )
			{
				printf( "ERROR: can't open output file\n" );
				return EXIT_FAILURE;
			}
			else
			{
				while( ( val = fgetc( inputFile ) ) != EOF )
				{
					putc( ( val ^ 0xEF ), outputFile );
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
