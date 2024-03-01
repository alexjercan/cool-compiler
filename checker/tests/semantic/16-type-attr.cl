class A {
    x : Int <- 0;
    y : Int;
    z : Int <- true;
    b1 : B <- new B;
    b2 : B <- new C;
    c  : C <- new B;
};

class B {};
class C inherits B {};