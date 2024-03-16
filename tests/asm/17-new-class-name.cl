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

class F inherits C {
    getA() : Int { a };
};

class Main inherits E {
    x : SELF_TYPE;

    main() : Object {
        let f : F <- new F
        in {
            out_string(f.type_name());
            out_int(f.getA());
        }
    };
};