class CellItem inherits ItemArray {
    value: Bool;

    init(v: Bool): SELF_TYPE {
        { value <- v; self; }
    };

    size(): Int { 1 };

    serialize(): String {
        if value then "1" else "0" fi
    };

    deserialize(s: String): SELF_TYPE {
        if s = "1" then { value <- true; self; } else { value <- false; self; } fi
    };

    value(): Bool { value };
};

class Rule110 {
    current: Array;

    init(s: String): SELF_TYPE {
        let i: Int <- 0,
            l: Int <- s.length()
        in
            {
                current <- new Array.init(new CellItem.size());
                while i < l loop
                    {
                        current <- current.append(new CellItem.init(s.substr(i, 1) = "1"));
                        i <- i + 1;
                    }
                pool;
                self;
            }
    };

    alive(index: Int): Bool {
        case current.at(index, new CellItem) of i: CellItem => i.value(); esac
    };

    transition(index: Int): Bool {
        let l: Int <- current.length(),
            i0: Int <- (index - 1).mod(l),
            i1: Int <- index,
            i2: Int <- (index + 1).mod(l)
        in
            (alive(i0).and(alive(i1)).and(not alive(i2))) -- 110
            .or((alive(i0).and(not alive(i1)).and(alive(i2)))) -- 101
            .or(((not alive(i0)).and(alive(i1)).and(alive(i2)))) -- 011
            .or(((not alive(i0)).and(alive(i1)).and(not alive(i2)))) -- 010
            .or(((not alive(i0)).and(not alive(i1)).and(alive(i2)))) -- 001
    };

    iteration(): SELF_TYPE {
        let l: Int <- current.length(),
            i: Int <- 0,
            new_state: Array <- new Array.init(new CellItem.size())
        in
            {
                while i < l loop
                {
                    new_state <- new_state.append(new CellItem.init(transition(i)));
                    i <- i + 1;
                }
                pool;
                current <- new_state;
                self;
            }
    };

    show(): String {
        let l: Int <- current.length(),
            i: Int <- 0,
            s: String <- ""
        in
            {
                while i < l loop
                    {
                        s <- s.concat(case current.at(i, new CellItem) of i: CellItem => i.serialize(); esac);
                        i <- i + 1;
                    }
                pool;
                s;
            }
    };
};

class Main {
    main(): Object {
        let n: Int <- 10,
            rule110: Rule110
        in
            {
                rule110 <- new Rule110.init(new IO.in_string());
                new IO.out_string(rule110.show()).out_string("\n");
                while 0 < n loop
                    {
                        rule110 <- rule110.iteration();
                        new IO.out_string(rule110.show()).out_string("\n");
                        n <- n - 1;
                    }
                pool;
            }
    };
};
