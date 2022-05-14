// An "empty" program without any warnings.
#include <stdlib.h>

int main()
{
    int* x = 0;
    int* y = 0;

    x = (int*)malloc(2 * sizeof(int));

    // Potential nullptr
    x[0] = 1;
    x[1] = 3;

    y = (int*)malloc(1 * sizeof(int));

    // Potential nullptr
    y[0] = 2;

    free(x);
    free(y);

    return 0;
}
