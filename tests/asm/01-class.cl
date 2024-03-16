class A {};

class B inherits A {};

class C inherits A {};

class D inherits B {};
class E inherits B {};

class F inherits C {};

class Main inherits E {
    main() : Object { 0 };
};