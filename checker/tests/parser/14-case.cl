class A {
    f(x : Object) : String {
        case x of
            s : String => "String";
            i : Int    => "Int";
            o : Object => "Oops";
        esac
    };
};