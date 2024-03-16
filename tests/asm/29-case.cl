-- Now class A inherits IO instead of Object!
class A inherits IO {
    a : Int <- 100;
    
    f() : Int { 1 };
};

class B inherits A {
    b : String <- "abc";
    
    g() : Int { 2 };
};

class C inherits A {
    c : Bool <- true;
    
    f() : Int { 3 };
    h() : Int { 4 };
};

class D inherits B {};
class E inherits B {};

class F inherits C {};

class Main inherits E {
    x : SELF_TYPE;
    
    out_string(str : String) : SELF_TYPE {
        self@IO.out_string(str.concat("\n"))
    };
    
    out_int(n : Int) : SELF_TYPE {
        self@IO.out_int(n)@IO.out_string("\n")
    };
    
    i(x : Object) : Object {
        case x of
            i : Int => out_int(1 + i);
            s : String => out_string("Found ".concat(s));
            a : A => out_int(a.f());
            m : Main => out_int(m.g());
        esac
    };

    main() : Object {
        {
            i(100);
            i("abc");
            i(self);
            i(new A);
        }
    };
};