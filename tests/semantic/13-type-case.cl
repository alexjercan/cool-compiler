class A {
    f(x : Int) : Object {
        x <- case x of
                 x : Int => 3;
             esac
    };
    
    g(x : Int) : Object {
        x <- case x of
                 x : Int => 3;
                 y : String => "3";
                 z : Bool => true;
             esac
    };
    
    h(b : B, c : C, d : D) : Object {
        b <- case b of
                 x : Int => b;
                 y : String => c;
                 z : Bool => d;
             esac
    };
    
    i(b : B, c : C, d : D) : Object {
        c <- case c of
                 x : Int => b;
                 y : String => c;
                 z : Bool => d;
             esac
    };
    
    j(b : B, c : C, d : D) : Object {
        d <- case d of
                 x : Int => b;
                 y : String => c;
                 z : Bool => d;
             esac
    };
};

class B {};
class C inherits B {};
class D inherits C {};