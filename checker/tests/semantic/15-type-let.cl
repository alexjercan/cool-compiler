class A {
    f(u : Int) : Object {
        u <- let x : Int <- u + 1,
                 y : Int <- true,
                 z : Int,
                 b1 : B <- new B,
                 b2 : B <- new C,
                 c  : C <- new B
             in
                 (x + y + z)
    };
    
    g(u : Bool) : Object {
        u <- let x : Int <- 1,
                 y : Int <- x,
                 z : Int
             in
                 (x + y + z)
    };
};

class B {};
class C inherits B {};