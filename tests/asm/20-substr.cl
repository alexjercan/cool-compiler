class Main {
    message: String <- "Hello, World";

    main(): Object {
        {
            new IO.out_string(message.substr(0, 5)).out_string("\n");
            new IO.out_string(message.substr(7, 5)).out_string("\n");
        }
    };
};
