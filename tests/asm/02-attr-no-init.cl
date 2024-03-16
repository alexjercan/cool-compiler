class A {
    a : Int;
};

class B inherits A {
    b : String;
};

class C inherits A {
    c : Bool;
};

class D inherits B {};
class E inherits B {};

class F inherits C {};

class Main inherits E {
    x : SELF_TYPE;

    main() : Object { 0 };
};