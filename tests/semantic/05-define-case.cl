class A {
    x : Bool;
    
    f(x : Int) : Int {
        case x of
            x    : Int => x;
            self : Object => 0;
            x    : SELF_TYPE => 0;
            x    : C => 0;
        esac
    };

};
