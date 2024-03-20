class B {};

class A {};

class Main inherits IO {
    main(): Object {
        let x: Int,
            a: A,
            b: B <- new B
        in
            {
                if isvoid x then
                    out_string("x is void\n")
                else
                    0
                fi;

                if isvoid a then
                    out_string("a is void\n")
                else
                    0
                fi;

                if isvoid b then
                    out_string("b is void\n")
                else
                    0
                fi;
            }
    };
};


