class A {
    f(x : Int, y : Int) : Object {
        while x < y loop
            x <- x + 1
        pool
    };

    g(x : Int, y : Int) : Object {
        while x + y * 100 <= x / y loop
            x <- y + 100 - x
        pool
    };
};

