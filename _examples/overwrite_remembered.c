#include <stdlib.h>

int main()
{
    int *x = 0, *y = 0;
    x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    y = x;

    x = NULL;

    free(y);

    return 0;
}
