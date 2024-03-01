class A {
    a : A;

    f(x : Int) : Object {
        while 0 < x loop
            x <- x - 1
        pool
    };
    
    g(x : Int) : Object {
        a <- while a loop
                 x <- x - 1
             pool
    };
};