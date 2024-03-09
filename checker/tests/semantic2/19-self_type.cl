class A {
    x : SELF_TYPE <- self;
    u : Int;

    f1() : SELF_TYPE { self };

    f2() : A { self };
};

class B inherits A {
    g() : Int { 0 };
};

class C inherits B {
    i() : Object {
        let a : A <- new B,
            b : B <- new B
        in
            {
                a.f1();

                b.f1();
                b.g();
                b.f1().g();

                b.f2();
                b.g();

                b <- b@A.f1();  -- f1 still returns B for type B, even if statically dispatched to A
                b@A.f1().g();

                x <- self.f1();
                x <- f1();
            }
    };
};

class D inherits C {
};
