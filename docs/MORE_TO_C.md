## A collection of MoRe test samples and their C language equivalents

### Empty

MoRe:
```
program begin
    var x = 0
    var y = 0
    x := cons(1, 3)
    y := cons(2)
    dispose x
    dispose y
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    int* y = (int*)malloc(sizeof(int));
    *y = 2;

    free(x);
    free(y);

    return 0;
}
```

### Double free

MoRe:
```
program begin
    var x = 0
    x := cons(1, 3)
    y := cons(2)
    dispose x
    dispose x
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    free(x);
    free(x);

    return 0;
}
```

### Free unallocated

MoRe:
```
program begin
    var x = 0
    dispose x
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;
    free(x);

    return 0;
}
```

### Indirect free

MoRe:
```
program begin
    var x = 0
    var y = 0
    x := cons(1, 2)
    y := x
    dispose y
    dispose x
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;
    int* y = NULL;

    x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 2;

    y = x;

    free(y);
    free(x);

    return 0;
}
```

### Complete overwrite

MoRe:
```
program begin
    var x = 0
    x := cons(1, 3)
    x := 3
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;

    x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    x = 3;

    return 0;
}
```

### Safe overwrite

MoRe:
```
program begin
    var x = 0
    var y = 0
    x := cons(1, 3)
    y := x
    x := 3
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;
    int* y = NULL;

    x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    y = x;
    x = 3;

    return 0;
}
```

### Partial overwrite

MoRe:
```
program begin
    var x = 0
    var y = 0
    x := cons(1, 3)
    y := x + 1
    x := 3
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;
    int* y = NULL;

    x = (int*)malloc(2 * sizeof(int));
    x[0] = 1;
    x[1] = 3;

    y = x + 1;
    x = 3;

    return 0;
}
```

### Conditional overwrite

MoRe:
```
program begin
    var x = 0
    var y = 0
    x := cons(1, 2, 3, 4)
    if condition then begin
        y := x + 1
    end else begin
        y := x + 2
    end
    x := 3
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;
    int* y = NULL;

    x = (int*)malloc(4 * sizeof(int));
    x[0] = 1;
    x[1] = 2;
    x[2] = 3;
    x[3] = 4;

    if (1)
    {
        y = x + 1;
    }
    else
    {
        y = x + 2;
    }

    x = 3;

    return 0;
}
```

### While unallocated

This is actually interesting, depending on the while loop's condition, we can either free a null pointer (0 iterations), be fine (1 iteration), or leak memory (2 and more iterations). According to the calculus, we should get both warnings emitted.

MoRe:
```
program begin
    var x = 0
    while condition do begin
        x := cons(1, 2)
    end
    dispose x
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = NULL;

    while (1)
    {
        x = (int*)malloc(2 * sizeof(int));
        x[0] = 1;
        x[1] = 2;
    }

    free(x);

    return 0;
}
```

### Big arrays

MoRe:
```
program begin
    var x = 0
    var y = 0
    x := cons(1, 2, 3, 4, 5, 6)
    y := cons(1, 2, 3, 4, 5, 6)
    dispose x
    dispose y
```

C:
```c
#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(6 * sizeof(int));
    int* y = (int*)malloc(6 * sizeof(int));

    x[0] = 1;
    x[1] = 2;
    x[2] = 3;
    x[3] = 4;
    x[4] = 5;
    x[5] = 6;

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
```
