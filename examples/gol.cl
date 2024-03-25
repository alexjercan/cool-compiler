class Main {
    current: String;
    width: Int;
    height: Int;

    alive(index: Int): Bool {
        current.substr(index, 1) = "1"
    };

    count_ns(index: Int): Int {
        let l: Int <- current.length(),
            row: Int <- index / width,
            col: Int <- index.mod(width),
            tl: Int <- (row - 1).mod(height) * width + (col - 1).mod(width),
            tm: Int <- (row - 1).mod(height) * width + col,
            tr: Int <- (row - 1).mod(height) * width + (col + 1).mod(width),
            ml: Int <- row * width + (col - 1).mod(width),
            mr: Int <- row * width + (col + 1).mod(width),
            bl: Int <- (row + 1).mod(height) * width + (col - 1).mod(width),
            bm: Int <- (row + 1).mod(height) * width + col,
            br: Int <- (row + 1).mod(height) * width + (col + 1).mod(width)
        in
            alive(tl).to_int() + alive(tm).to_int() + alive(tr).to_int() + alive(ml).to_int()
            + alive(mr).to_int() + alive(bl).to_int() + alive(bm).to_int() + alive(br).to_int()

    };

    transition(index: Int): Bool {
        let l: Int <- current.length(),
            ns: Int <- count_ns(index)
        in
            if alive(index).and(ns < 2) then false -- underpopulation
            else if alive(index).and((ns = 2).or(ns = 3)) then true -- ok
            else if alive(index).and(3 < ns) then false -- overpopulation
            else if ((not alive(index)).and(ns = 3)) then true -- reproduction
            else false
            fi fi fi fi
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

    parse(s: String): Object {
        let l: Int <- s.length(),
            i: Int <- 0,
            cnt: Int <- 0
        in
            {
                while i < l loop
                {
                    if s.substr(i, 1) = "1" then
                    {
                        current <- current.concat("1");
                        cnt <- cnt + 1;
                    }
                    else if s.substr(i, 1) = "0" then
                    {
                        current <- current.concat("0");
                        cnt <- cnt + 1;
                    }
                    else if s.substr(i, 1) = "\n" then
                        height <- height + 1
                    else
                        abort()
                    fi fi fi;
                    i <- i + 1;
                }
                pool;
                height <- height + 1;
                width <- cnt / height;
            }
    };

    print(): Object {
        let l: Int <- 0
        in
            {
                while l < height loop
                    {
                        new IO.out_string(current.substr(l * width, width)).out_string("\n");
                        l <- l + 1;
                    }
                pool;
                new IO.out_string("\n");
            }
    };

    main(): Object {
        let n: Int <- 1000
        in
            {
                parse(new IO.in_string());
                print();
                while 0 < n loop
                    {
                        iteration();
                        print();
                        n <- n - 1;
                    }
                pool;
            }
    };
};
