class A {
    x : Int <- 0;
    y : Int;
    b1 : B <- new B;
    b2 : B <- new C;
};

class B {};
class C inherits B {};
