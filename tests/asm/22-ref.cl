class Main {
    main(): Object {
        let my_ref: Ref <- new Ref.init(3)
        in case my_ref.deref() of x: Int => new IO.out_int(x).out_string("\n"); esac
    };
};
