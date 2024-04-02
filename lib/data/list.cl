class List {
    value: Object;
    next: List;

    value(): Object {
        value
    };

    next(): List {
        next
    };

    init(v: Object, n: List): List {
        {
            value <- v;
            next <- n;
            self;
        }
    };

    single(v: Object): List {
        let void: List in new List.init(v, void)
    };

    append(v: Object): List {
        {
            if isvoid next then
                let void: List
                in next <- new List.init(v, void)
            else
                next.append(v)
            fi;
            self;
        }
    };

    concat(l: List): List {
        {
            if isvoid next then
                next <- l
            else
                next.concat(l)
            fi;
            self;
        }
    };

    index(i: Int): Object {
        if i = 0 then
            value
        else
            next.index(i - 1)
        fi
    };
};
