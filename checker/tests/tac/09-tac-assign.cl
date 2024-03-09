class A {
    f(x : Int, y : Int) : Int { x <- 100 + y };
    g(x : Int, y : Int) : Int { x + (y <- 100) };
};
