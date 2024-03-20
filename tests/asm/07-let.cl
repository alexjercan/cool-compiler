class Main inherits IO {
    x: Int <- 45 - 10 / 2 * 3;
    y: Int <- x + 39;

    main(): Object {
        let x: Int <- 5,
            y: Int <- x + 10
        in
            out_int(y).out_string("\n")
    };
};
