class A {
    f(x : Int) : Object {
        x <- if x <= 5 then x else x + 1 fi
    };

    h(b : B, c : C) : Object {
        b <- if true then b else c fi
    };
};

class B {};
class C inherits B {};
