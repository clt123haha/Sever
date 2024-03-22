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
	这里后续更换自定义错误
	*/


