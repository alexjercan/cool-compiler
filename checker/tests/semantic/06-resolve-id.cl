class A {
    z : Int <- x;

    x : Int <- x;
    
    f : Bool;

    f(f : Int) : Int {
        let y : Int <- y
        in
            case y of
                z : Int => x <- x + z + f;
            esac
    };
    
    g() : A { self };
};