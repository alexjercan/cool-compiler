class A {
    x : SELF_TYPE;
    
    f() : Object { 0 };
};

class B inherits A {
    y : Int <- 0;
    
    g(x : Int, y : Bool) : Int { 0 };
};