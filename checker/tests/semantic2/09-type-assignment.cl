class A {
    x : Bool;
    y : Int;
    b : B;
    c : C;

    f() : Object {
        b <- b
    };

    g() : Object {
        b <- c
    };
};

class B {};
class C inherits B {};
