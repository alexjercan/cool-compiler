class Main {
    main(): Object {
        let my_float: Float <- new Float.from_fraction(1, 2)
                                .mul(new Float.from_int(4))
                                .add(new Float.from_int(3))
                                .sub(new Float.from_fraction(8, 2))
                                .div(new Float.from_int(2)),
            my_int: Int <- my_float.to_int()
        in
            if my_float = new Float.from_fraction(1, 2) then
                new IO.out_string("Float is 1/2\n")
            else
                new IO.out_string("Float is not 1/2\n")
            fi
    };
};
