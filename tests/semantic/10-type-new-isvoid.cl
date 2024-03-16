class A {
    f(a : A) : Object {
        a <- new A
    };
    
    g(a : A) : Object {
        a <- new B
    };
    
    h(b : B) : Object {
        b <- new A
    };
    
    i(b : B) : Object {
        b <- new C
    };
    
    j : Bool <- not isvoid new B;
};

class B inherits A {};