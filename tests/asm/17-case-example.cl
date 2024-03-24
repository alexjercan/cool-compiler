class A {};
class B inherits A {};
class C inherits B {};

class D inherits A {};
class E inherits D {};

class F inherits D {};

class Main inherits IO {
    the_method(x: Object): Int {
        case x of
            c: C => 0;
            e: E => 1;
            f: F => 2;
            a: A => 3;
            o: Object => 4;
        esac
    };

    main(): Object {
        let x: Object <- new B,
            y: Object <- new F
        in {
            out_int(the_method(x)).out_string("\n");
            out_int(the_method(y)).out_string("\n");
        }
    };
};

