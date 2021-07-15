# HandSanitizer

This project aims to do the following to extract all pure functions from an llvm bitcode file, and emit a separate executable for each.
Every executable is accompanied by a json file specifiying it's arguments which can be given as a direct argument to the binary.

mvp example 

```c
llvm containing bitcode for: 
int f(int x, int y){ return x + y}
~~~~
ouput/
    - f.cpp
    - f.json
    - f

~~~~ call function with
./f f.json
```


### Pointers
For pointers we do the following:
1. If a function does accept a pointer, either directly or indirectly as a struct member,
   we expose a setter to the _underlying_ value, regardless of how many levels of indirections there are.

2. To allow for C style strings, we also allow for the specification of what memory should look like directly behind the value


Example 1: Setting the underlying value
```
void f(int* x);
int x = parser.get<int>("--x"); // set the underlying value
f(&x);
```

Example 2: Multiple levels of indirection exist
```
f(char*** z);
char u = parser.get<char>("--z");
char x = &u;
char** y = &x;
char*** z = &y; 
f(z); // or f(&y);
```

Example 3: Strings
```
f(char* x);
----
int x_arraysize = json_input["--x"].size();
int x_buffer = malloc(sizeof(char) * x_arraysize);
for(int i =0; i < x_array_size; i++)
    x_buffer[i] = json_input["--x"][i]; 
```

Later on we might include support aliasing, but this feature is currently considered out of scope. An example might be following scenario:
```
f(int** x, int *y);
./handsanitzer --x=3 y=--x

int* x2 = &parser.get<char>("--x");
int** x1 = &x2;
int* y = x2;
f(x1, y);
```



