class A {
    f(x : Int, y : Int) : Int { x + y };
    
    g(x : Int, y : Int) : Int { true };
    
    h(b : B, c : C) : B { b };
    
    i(b : B, c : C) : B { c };
    
    j(b : B, c : C) : C { b };
};

class B {};
class C inherits B {};