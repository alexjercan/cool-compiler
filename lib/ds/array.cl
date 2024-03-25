class Array {
    data: String;
    item_size: Int;

    init(item_sz: Int): SELF_TYPE {
        { data <- ""; item_size <- item_sz; self; }
    };

    length(): Int { data.length() / item_size };

    append(item: ItemArray): SELF_TYPE {
        { data <- data.concat(item.serialize()); self; }
    };

    set(i: Int, item: ItemArray): SELF_TYPE {
        {
            data <- data.substr(0, i * item_size)
                    .concat(item.serialize())
                    .concat(data.substr((i + 1) * item_size, data.length() - (i + 1) * item_size));
            self;
        }
    };

    capace(count: Int, item: ItemArray): SELF_TYPE {
        { data <- data.concat(item.serialize().repeat(count - length())); self; }
    };

    at(i: Int, item: ItemArray): ItemArray { item.deserialize(data.substr(i * item_size, item_size)) };

    pop(item: ItemArray): ItemArray {
        { item <- at(length() - 1, item); data <- data.substr(0, data.length() - item_size); item; }
    };

    debug(): Object { new IO.out_string(data) };
};

class ItemArray {
    size(): Int {
        { abort(); 0; }
    };

    serialize(): String {
        { abort(); ""; }
    };

    deserialize(s: String): SELF_TYPE {
        { abort(); self; }
    };
};
