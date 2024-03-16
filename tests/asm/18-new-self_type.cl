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
    
    getA() : Int { a };
    
    i() : Object {
        {
            x <- new SELF_TYPE;
            out_string(x.type_name());
            out_int(x.getA());
        }
    };

    main() : Object {
        new G.i()
    };
};

class G inherits Main {};