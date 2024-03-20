class A {};

class Main inherits IO {
    main(): Object {
        out_string(new A.type_name())
            .out_string("\n")
            .out_string(new A@Object.type_name())
            .out_string("\n")
    };
};
