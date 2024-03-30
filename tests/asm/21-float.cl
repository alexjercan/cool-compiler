class Main {
    float(): Object {
        let my_float: Float <- new Float.from_fraction(1, 2)
                                .mul(new Float.from_int(4))
                                .add(new Float.from_int(3))
                                .sub(new Float.from_fraction(8, 2))
                                .div(new Float.from_int(2)),
            my_int: Int <- my_float.to_int()
        in
            if my_float = new Float.from_fraction(1, 2) then
                new IO.out_string("Float is 1/2 => ")
            else
                new IO.out_string("Float is not 1/2 => ")
            fi.out_int(my_int).out_string("\n")
    };

    byte(): Object {
        let my_byte: Byte <- new Byte.from_int(69),
            my_int: Int <- my_byte.to_int()
        in
            if my_byte = new Byte.from_int(69) then
                new IO.out_string("Byte is 69 => ")
            else
                new IO.out_string("Byte is not 69 => ")
            fi.out_int(my_int).out_string("\n")
    };

    word(): Object {
        let my_word: Word <- new Word.from_int(1000),
            my_int: Int <- my_word.to_int()
        in
            if my_word = new Word.from_int(1000) then
                new IO.out_string("Word is 1000 => ")
            else
                new IO.out_string("Word is not 1000 => ")
            fi.out_int(my_int).out_string("\n")
    };

    double_word(): Object {
        let my_double_word: DoubleWord <- new DoubleWord.from_int(1000000),
            my_int: Int <- my_double_word.to_int()
        in
            if my_double_word = new DoubleWord.from_int(1000000) then
                new IO.out_string("DoubleWord is 1000000 => ")
            else
                new IO.out_string("DoubleWord is not 1000000 => ")
            fi.out_int(my_int).out_string("\n")
    };

    main(): Object {
        {
            float();
            byte();
            word();
            double_word();
        }
    };
};
