class Optional {};

class None inherits Optional {};
class Some inherits Optional {
    value: Object;

    init(v: Object): SELF_TYPE {
        {
            value <- v;
            self;
        }
    };

    value(): Object { value };
};
