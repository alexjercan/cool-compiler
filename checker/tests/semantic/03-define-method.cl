class A {
    x : Bool;
    
    f(x : Int, y : Int) : Int { 0 };
    
    f() : Int { 0 };
    
    g(c : C) : C { c };
};

class B inherits A {
    y : Int;
    
    self(self : Int) : Int { 0 };
    
    h(x : Int, x : Int) : Int { 0 };
    
    i(x : SELF_TYPE) : Int { 0 };
    
    j(x : Z) : Int { 0 };
};

class C inherits A {
    f(u : Int, v : Int) : Int { 0 };
};

class D inherits A {
    f(u : Int) : Int { 0 };
};

class E inherits A {
    f(u : Int, v : Bool) : Int { 0 };
};

class F inherits A {
    f(u : Int, v : Int) : Bool { false };
};