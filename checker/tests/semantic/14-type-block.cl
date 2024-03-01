class A {
    f(x : Int) : Object {
        x <- {
                 "Done!";
                  x <- x + 1;
             }
    };
    
    g(x : Int) : Object {
        x <- {
                  x <- x + 1;
                  "Oops!";
             }
    };
};