class Main inherits IO {
    x: Int <- 45 - 10 / 2 * 3;
    y: Int <- x + 39;
    z: String <- "not hello world";

    main(): Object {
        {
            if x = y then
                self.out_string("x = y\n")
            else
                self.out_string("x != y\n")
            fi;

            if y = 69 then
                self.out_string("nice\n")
            else
                self.out_string("not nice\n")
            fi;

            if z = "hello world" then
                self.out_string("z = hello world\n")
            else
                self.out_string("z != hello world\n")
            fi;
        }
    };
};
