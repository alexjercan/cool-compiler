class BaseClass {
    print(): Object {
        new IO.out_string("BaseClass\n")
    };

    other(): Object {
        new IO.out_string("other\n")
    };
};

class DerivedClass inherits BaseClass {
    print(): Object {
        new IO.out_string("DerivedClass\n")
    };

    custom(): Object {
        new IO.out_string("custom\n")
    };
};

class Main {
    x: BaseClass <- new DerivedClass;
    y: DerivedClass <- new DerivedClass;

    main(): Object {
        {
            x.print();
            x@BaseClass.print();
            y.custom();
            y@BaseClass.print();
        }
    };
};
