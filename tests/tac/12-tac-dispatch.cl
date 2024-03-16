class A {
    f(x : Int, y : Int) : Bool {
        let b: B <- new B,
            s: Int <- x + y
        in
            b.f(s, y)
    };

    g(x : Int, y : Int) : Bool {
        new A@A.f(x, y)
    };

    h(x : Int, y : Int) : Bool {
        g(x, y)
    };
};

class B {
    f(x : Int, y : Int) : Bool { x = y };
};
