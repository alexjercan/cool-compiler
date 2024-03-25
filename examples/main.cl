class BoolItem inherits ItemArray {
    value: Bool;

    init(v: Bool): SELF_TYPE {
        { value <- v; self; }
    };

    size(): Int { 1 };

    serialize(): String {
        if value then "t" else "f" fi
    };

    deserialize(s: String): SELF_TYPE {
        if s = "t" then { value <- true; self; } else { value <- false; self; } fi
    };

    value(): Bool { value };
};

class Main inherits IO {
    main(): Object {
        let arr: Array <- new Array.init(new BoolItem.size()),
            item: ItemArray <- new BoolItem
        in
        {
            arr.append(new BoolItem.init(true)).append(new BoolItem.init(false)).capace(5, new BoolItem).append(new BoolItem.init(true)).debug();
            out_string("\n");
            item <- arr.pop(item);
            arr.debug();
            out_string("\n");
            case arr.pop(new BoolItem) of
            item: BoolItem => if item.value() then out_string("true") else out_string("false") fi;
            esac;
        }
    };
};
