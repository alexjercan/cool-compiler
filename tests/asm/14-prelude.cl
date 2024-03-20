class A {};

class Main inherits IO {
    a: Int <- 1;
    b: Int <- 2;

    main(): Object {
        if (1 < 10).and(a < b) then
            out_string("1 < 10 and a < b\n")
        else
            out_string("1 >= 10 or a >= b\n")
        fi
    };
};

