class A {
    a : Int <- 100;
};

class B inherits A {
    b : String <- "abc";
};

class C inherits A {
    c : Bool <- true;
};

class D inherits B {};
class E inherits B {};

class F inherits C {};

class Main inherits E {
    x : SELF_TYPE;

    main() : Object { 0 };
};