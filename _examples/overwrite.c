#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    x = NULL;

    return 0;
}
