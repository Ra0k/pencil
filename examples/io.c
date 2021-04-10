#include "stdio.h"

int64_t read_unsigned()
{
	int64_t ret;
	fscanf( stdin, "%u", &ret );
	return ret;
}

int read_signed()
{
	int ret;
	fscanf( stdin, "%d", &ret );
	return ret;
}

int read_boolean()
{
	char ret[256];
	fscanf( stdin, "%s", ret );
	return strcmp(ret,"true");
}

void write_unsigned(int64_t num)
{
	fprintf( stdout, "%u\n", num );
}

void write_signed( int64_t num )
{
	fprintf( stdout, "%d\n", num );
}

void write_boolean( int num )
{
	num &= 1;
	if( num )
		fprintf( stdout, "true\n" );
	else
		fprintf( stdout, "false\n" );
}
