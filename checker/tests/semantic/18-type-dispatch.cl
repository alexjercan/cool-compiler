class A {
    f(b : B, c : C, x : Int) : Int {
        {
            c <- b.f(x, true);
            b.f(x);
            b.f(true, true);
            c.f(x, true);
            c.g();
            c.f("abc", true);
                        
            c@SELF_TYPE.f(x, true);
            c@Y.f(x, true);
            c@A.f(x, true);
            c@B.f(x, true);
            c@Z.f(x, true);
            
            self.f(b, c, x);
            self.f(c, c, x);
            self.f(c, b, x);
            
            f(b, c, x);
            f(c, c, x);
            c <- f(c, b, x);
        }
    };
};

class Z {};

class B inherits Z {
    f(x : Int, y : Bool) : Int { 0 };
};

class C inherits B {
};