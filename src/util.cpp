#include "util.h"

void errif(bool iferr, const char* str)
{
	if (iferr)
	{
		perror(str);
		exit(EXIT_FAILURE);
	}
}
	/*
	������������Զ������
	*/


