class A {
    f(x : Int) : Bool {
        f(x + 1)
    };
    
    g(x : Int, y : Int) : Bool {
        self.g(x + 1, y + 1)
    };
    
    h(x : Int) : A {
        new A@A.h(x + 1).h(x + 2)
    };
};