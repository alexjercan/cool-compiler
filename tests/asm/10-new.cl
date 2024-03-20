class A {
    x: Int;
    y: Int;
    io: IO <- new IO;

    init(a: Int, b: Int): SELF_TYPE {
        {
            x <- a;
            y <- b;
            self;
        }
    };

    show(): Object {
        io.out_int(x).out_string(" ").out_int(y).out_string("\n")
    };
};

class Main inherits IO {
    the_method(x: Int, y: Int): A {
        new A.init(x, y)
    };

    main(): Object {
        let x: Int <- 4,
            y: Int <- 9
        in
            the_method(x, y).show()
    };
};

