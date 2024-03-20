class Main inherits IO {
    the_method(x: Int, y: Int): Int {
        {
            x <- x + 1;
            y <- y + 1;
            x * y;
        }
    };

    main(): Object {
        let x: Int <- 4,
            y: Int <- 9
        in
            out_int(the_method(x, y)).out_string("\n")
    };
};
