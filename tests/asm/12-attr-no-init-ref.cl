-- Now class A inherits IO instead of Object!
class A inherits IO {
    a : Int;
    
    f() : Int { 1 };
};

class B inherits A {
    b : String;
    
    g() : Int { 2 };
};

class C inherits A {
    c : Bool;
    
    f() : Int { 3 };
    h() : Int { 4 };
};

class D inherits B {};
class E inherits B {};

class F inherits C {};

class Main inherits E {
    x : SELF_TYPE;

    main() : Object {
        {
            out_int(a);
            out_string(b);
        }
    };
};