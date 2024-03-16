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

    main() : Object {
        let x : Int,
            s : Int,
            z : Object <-
                while x <= 10 loop
                    {
                        s <- s + x;
                        x <- x + 1;                
                    }
                pool
        in {
            out_int(s);
            out_int(x);
            out_string(if isvoid z then "OK" else "failed" fi);
        }
    };
};