class Main inherits IO {
    x: Int <- 45 - 10 / 2 * 3;
    y: Int <- x + 39;

    main(): Object {
        while (x < y) loop
            {
                x <- x + 1;
                out_string("x: ");
                out_int(x);
                out_string("\n");
            }
        pool
    };
};
