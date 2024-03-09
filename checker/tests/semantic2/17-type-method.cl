class A {
    f(x : Int, y : Int) : Int { x + y };

    h(b : B, c : C) : B { b };

    i(b : B, c : C) : B { c };
};

class B {};
class C inherits B {};
