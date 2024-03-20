class Main inherits IO {
    the_method(x: Object): Int {
        case x of
            x: Int => x;
            x: String => 0;
            x: Bool => 1;
            x: Object => 2;
        esac
    };

    main(): Object {
        let x: Int <- 5,
            y: Int <- 10
        in {
            out_int(the_method(x + y)).out_string("\n");
            out_int(the_method(true)).out_string("\n");
            out_int(the_method("hello")).out_string("\n");
        }
    };
};

