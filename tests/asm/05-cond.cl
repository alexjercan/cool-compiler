class Main inherits IO {
    x: Int <- 45 - 10 / 2 * 3;
    y: Int <- x + 39;

    main(): Object {
        if (x < y) then
            self.out_string("x < y\n")
        else
            self.out_string("x >= y\n")
        fi
    };
};
