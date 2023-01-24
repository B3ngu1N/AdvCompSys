/*
	This file was for testing open_memstream
	Results of testing: It doesn't work lol
*/
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

static const char cOut[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void test(FILE *fp, const char *name)
{
	long pos;

	printf("Running test: %s\n", name);

	// fwrite_orDie(cOut, 24, fp);
    fprintf(fp, cOut);

	pos = ftell(fp);
	printf("position after fwrite: %ld\n", pos);

	fseek(fp, 0, SEEK_SET);
	fwrite(cOut, 1, 4, fp);

	fseek(fp, 0, SEEK_END);
	pos = ftell(fp);
	/* this printf incorrect with recent open_memstream */
	printf("position after SEEK_END: %ld\n", pos);

	putchar('\n');
}

#define FMEMOPEN_BUF_SIZE 64

int main(void)
{
	FILE *fp;
	char *buf;
	size_t buf_size;

	buf = NULL;
	buf_size = 0;

	fp = open_memstream(&buf, &buf_size);
	test(fp, "open_memstream()");
	fclose(fp);
	/* this printf incorrect with recent open_memstream */
	printf("open_memstream() final buffer size: %zu\n", buf_size);
	free(buf);

	return 0;
}

/* EOF */