class A {
    f(x : Int, y : Int) : Bool {
        if x < y then
            x = y
        else
            y = x
        fi
    };

    g(x : Int, y : Int) : Int {
        if x + y * 100 <= x / y then
            x
        else
            y + 100 - x
        fi
    };

    h(x : Int, y : Int) : Int {
        if isvoid x then
            x
        else
            y + 100 - x
        fi
    };
};
