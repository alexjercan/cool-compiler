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

    h() : Object {
        c <- b
    };

    i() : Object {
        self <- self
    };

    j() : Object {
        x <- y
    };
};

class B {};
class C inherits B {};
