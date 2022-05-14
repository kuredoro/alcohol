#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(6 * sizeof(int));
    x[0] = 1;
    x[1] = 2;
    x[2] = 3;
    x[3] = 4;
    x[4] = 5;
    x[5] = 6;

    int* y = (int*)malloc(6 * sizeof(int));
    y[0] = 1;
    y[1] = 2;
    y[2] = 3;
    y[3] = 4;
    y[4] = 5;
    y[5] = 6;

    free(x);
    free(y);

    return 0;
}
