class A {
    f(a : A) : Object {
        a <- new A
    };

    g(a : A) : Object {
        a <- new B
    };

    j : Bool <- not isvoid new B;
};

class B inherits A {};
