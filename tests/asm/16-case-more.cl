class A {};
class B inherits A {};

class Main inherits IO {
    the_method(x: Object): Int {
        case x of
            x: Int => x;
            x: String => 0;
            x: Bool => 1;
            x: A => 2;
            x: Object => 3;
        esac
    };

    main(): Object {
        let x: Int <- 5,
            y: Int <- 10
        in {
            out_int(the_method(x + y)).out_string("\n");
            out_int(the_method(true)).out_string("\n");
            out_int(the_method("hello")).out_string("\n");
            out_int(the_method(new B)).out_string("\n");
            out_int(the_method(new IO)).out_string("\n");
        }
    };
};
