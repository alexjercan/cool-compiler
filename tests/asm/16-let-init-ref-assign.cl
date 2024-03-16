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

    main() : Object {
        let x : Int    <- 100,
            y : String <- "abc"
        in {
            out_int(x);
            x <- 117;
            out_int(x);
            
            out_string(y);
            y <- "def";
            out_string(y);
        }
    };
};