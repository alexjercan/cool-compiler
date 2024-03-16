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
        let a1 : Int <- 100,
            a2 : Int,
            b1 : String <- "abc",
            b2 : String
        in {
            out_string(if 3 = 3 then "3 OK" else "3 failed" fi);
            out_string(if a = a then "a OK" else "a failed" fi);
            out_string(if a = a1 then "a1 OK" else "a1 failed" fi);
            out_string(if a = a2 then "a2 OK" else "a2 failed" fi);
            out_string(if b = b then "b OK" else "b failed" fi);
            out_string(if b = b1 then "b1 OK" else "b1 failed" fi);
            out_string(if b = b2 then "b2 OK" else "b2 failed" fi);
            out_string(if x = x then "x OK" else "x failed" fi);
            out_string(if self = self then "self OK" else "self failed" fi);
            out_string(if x = self then "comp OK" else "comp failed" fi);
            out_string(if self = self.copy() then "copy OK" else "copy failed" fi);
        }
    };
};