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
};
