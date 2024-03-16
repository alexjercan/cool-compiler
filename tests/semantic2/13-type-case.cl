class A {
    f(x : Int) : Object {
        x <- case x of
                 x : Int => 3;
             esac
    };

    h(b : B, c : C, d : D) : Object {
        b <- case b of
                 x : Int => b;
                 y : String => c;
                 z : Bool => d;
             esac
    };
};

class B {};
class C inherits B {};
class D inherits C {};
