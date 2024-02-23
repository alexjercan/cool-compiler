class A {
    x : Int;
    
    f(y : Int) : Int {
        x <- y
    };
    
    f(y : Int, z : Int) : Int {
        x <- y <- z
    };
};