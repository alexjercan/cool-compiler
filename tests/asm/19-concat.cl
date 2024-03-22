class Main {
    message: String <- "Hello, World";
    suffix: String <- "!";
    times: Int <- 5;

    main(): Object {
        let x: Int <- times,
            message: String <- message,
            y: Int <- 0
        in
        {
            while 0 < x loop
                {
                    x <- x - 1;
                    message <- message.concat(suffix);
                }
            pool;

            message <- message.concat("\n");

            new IO.out_string(message);
        }
    };
};
