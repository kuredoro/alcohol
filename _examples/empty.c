// An "empty" program without any warnings.
#include <stdlib.h>

int main()
{
    int* x = 0;
    int* y = 1;

    x = (int*)malloc(2 * sizeof(int));

    int z = 1;
    z = z;

    // Potential nullptr
    x[0] = 2 * 2;
    x[1] = 42 + 42 + 42 + 42;

    y = (int*)malloc(1 * sizeof(int));

    // Potential nullptr
    y[0] = 2;

    free(x);
    free(y);

    return 0;
}
