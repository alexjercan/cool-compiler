class Person {
    name: String;
    food: String;

    init(n: String, f: String): Person {
        {
            name <- n;
            food <- f;
            self;
        }
    };

    show(): String {
        name.concat(" likes to eat ").concat(food)
    };
};

class List inherits IO {
    value: Object;
    next: List;

    init(v: Object, n: List): List {
        {
            value <- v;
            next <- n;
            self;
        }
    };

    print(): IO {
        {
            let v: String <- case value of
                    x: String => x;
                    x: Person => x.show();
                    x: Object => { abort(); ""; };
                esac
            in {
                out_string(v.concat(" "));
                if (isvoid next) then out_string("\n") else next.print() fi;
            };
        }
    };
};

class A {
    a: String;
    b: String;

    foo(): String {
        a.concat(b)
    };

    bar(): String {
        b.concat(a)
    };
};

class B inherits A {
    c: String;

    foo(): String {
        a.concat(b).concat(c)
    };
};

class Main inherits IO {
    main(): IO {
        let
            void: List,
            guy: Person <- new Person.init("John", "pizza"),
            l: List <- new List.init("first", new List.init(guy, void))
        in
            l.print()
    };
};
