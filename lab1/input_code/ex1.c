/*
 *  ex1.c
 * As given in the lab text.
 */

#include <stdlib.h>
#include <stdio.h>

int fromStack(int *ptr) {
  int dummy;
	if (ptr > &dummy){
		return 1;
	}
	return 0;
}


int min(int a, int b, int c){
	int tmp_min;
	tmp_min = a <= b ? a : b;
	tmp_min = tmp_min <= c ? tmp_min : c;
	printf("%d\n", fromStack(&tmp_min));
	printf("%d\n", fromStack(&a));
	printf("%d\n", fromStack(&b));
	printf("%d\n", fromStack(&c));
	return tmp_min;
}
int main() {
	int min_val = min(3, 7, 5);
	printf("The min is: %d\n", min_val);
	exit(0);
}
