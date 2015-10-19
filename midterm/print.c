#include <stdio.h>
#include <ctype.h>

void print_name(const char* name, int qflag)
{
	const char *p = name;
	if (qflag)
	{
		while( (*p) != '\0') {
			if (isgraph(*p) != 0) {
				(void)putchar(*p);
			} else {
				(void)putchar('?');
			}
			p++;
		}
	} else {
		printf("%s", name);
	}
}
