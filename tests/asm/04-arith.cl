class Main inherits IO {
    x: Int <- 45 - 10 / 2 * 3;
    y: Int <- x + 39;

    main(): Object {
        out_int(y).out_string("\n")
    };
};
