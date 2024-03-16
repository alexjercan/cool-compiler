class A {
    f(b : B, c : C, x : Int) : Int {
        {
            c.f(x, true);

            c@B.f(x, true);

            self.f(b, c, x);
            self.f(c, c, x);

            f(b, c, x);
            f(c, c, x);
        }
    };
};

class Z {};

class B inherits Z {
    f(x : Int, y : Bool) : Int { 0 };
};

class C inherits B {
};
