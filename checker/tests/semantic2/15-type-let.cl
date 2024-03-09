class A {
    f(u : Int) : Object {
        u <- let x : Int <- u + 1,
                 y : Int,
                 z : Int,
                 b1 : B <- new B,
                 b2 : B <- new C
             in
                 (x + y + z)
    };
};

class B {};
class C inherits B {};
