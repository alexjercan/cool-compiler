class Main {
    current: String;

    alive(index: Int): Bool {
        current.substr(index, 1) = "1"
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

    iteration(): String {
        let l: Int <- current.length(),
            i: Int <- 0,
            new_state: String <- ""
        in
            {
                while i < l loop
                {
                    new_state <- new_state.concat(if transition(i) then "1" else "0" fi);
                    i <- i + 1;
                }
                pool;
                current <- new_state;
            }
    };

    main(): Object {
        let n: Int <- 10
        in
            {
                current <- new IO.in_string();
                new IO.out_string(current).out_string("\n");
                while 0 < n loop
                    {
                        iteration();
                        new IO.out_string(current).out_string("\n");
                        n <- n - 1;
                    }
                pool;
            }
    };
};
