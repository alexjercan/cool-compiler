class Object {
    abort(): Object extern;
    type_name(): String extern;
    copy(): SELF_TYPE extern;
    equals(x: Object): Bool extern;

    to_string(): String {
        self.type_name().concat(" object")
    };
};

class IO inherits Object {
    linux: Linux <- new Linux;

    out_string(x: String): SELF_TYPE {
        {
            linux.write(1, x);
            self;
        }
    };

    out_int(x: Int): SELF_TYPE {
        {
            out_string(x.to_string());
            self;
        }
    };

    in_string(): String {
        let s: String <- "",
            tmp: String,
            c: String
        in
            {
                tmp <- linux.read(0, 1024);
                s <- s.concat(tmp);

                while tmp.length() = 1024 loop
                    {
                        tmp <- linux.read(0, 1024);
                        s <- s.concat(tmp);
                    }
                pool;

                if s.substr(s.length() - 1, 1) = "\n" then
                    s.substr(0, s.length() - 1)
                else
                    s
                fi;
            }
    };

    in_int(): Int {
        in_string().to_int()
    };
};

class String inherits Object {
    l: Int <- extern;     -- TOOD: weird bug cannot use l variable in let
    str: String <- extern;

    length(): Int extern;
    concat(s: String): String extern;
    substr(i: Int, l: Int): String extern;
    equals(x: Object): Bool extern;

    to_string(): String {
        case self of
            me: String => me;
            me: Object => { abort(); new String; };
        esac
    };

    to_int(): Int {
        let s: String <- case self of
                             me: String => me;
                             me: Object => { abort(); new String; };
                         esac,
            c: String,
            n: Int <- 0,
            i: Int <- 0,
            d: Int
        in
            {
                while i < s.length() loop
                    {
                        c <- s.substr(i, 1);
                        if c = "0" then d <- 0 else
                        if c = "1" then d <- 1 else
                        if c = "2" then d <- 2 else
                        if c = "3" then d <- 3 else
                        if c = "4" then d <- 4 else
                        if c = "5" then d <- 5 else
                        if c = "6" then d <- 6 else
                        if c = "7" then d <- 7 else
                        if c = "8" then d <- 8 else
                        if c = "9" then d <- 9 else
                        { abort(); 0; }
                        fi fi fi fi fi fi fi fi fi fi;
                        i <- i + 1;
                        n <- n * 10 + d;
                    }
                pool;
                n;
            }
    };
};

class Int inherits Object {
    val: Int <- extern;

    equals(x: Object): Bool extern;

    to_string(): String {
        let x: Int <- case self of
                          me: Int => me;
                          me: Object => { abort(); 0; };
                      esac,
            s: String <- "",
            n: Int <- x.abs(),
            d: Int
        in
            {
                while not n = 0 loop
                    {
                        d <- n.mod(10);
                        if d = 0 then s <- "0".concat(s) else
                        if d = 1 then s <- "1".concat(s) else
                        if d = 2 then s <- "2".concat(s) else
                        if d = 3 then s <- "3".concat(s) else
                        if d = 4 then s <- "4".concat(s) else
                        if d = 5 then s <- "5".concat(s) else
                        if d = 6 then s <- "6".concat(s) else
                        if d = 7 then s <- "7".concat(s) else
                        if d = 8 then s <- "8".concat(s) else
                        if d = 9 then s <- "9".concat(s) else
                        { abort(); new String; }
                        fi fi fi fi fi fi fi fi fi fi;
                        n <- n / 10;
                    }
                pool;

                if x = 0 then "0" else if x < 0 then "-".concat(s) else s fi fi;
            }
    };

    abs(): Int {
        case self of
            me: Int => if me < 0 then ~me else me fi;
            me: Object => { abort(); 0; };
        esac
    };

    mod(x: Int): Int {
        case self of
            me: Int => me - (me / x) * x;
            me: Object => { abort(); 0; };
        esac
    };
};

class Bool inherits Object {
    val: Bool <- extern;

    equals(x: Object): Bool extern;

    and(x: Bool): Bool {
        case self of
            me: Bool => if me then x else false fi;
            me: Object => { abort(); false; };
        esac
    };

    or(x: Bool): Bool {
        case self of
            me: Bool => if me then true else x fi;
            me: Object => { abort(); false; };
        esac
    };

    to_string(): String {
        case self of
            me: Bool => if me then "true" else "false" fi;
            me: Object => { abort(); new String; };
        esac
    };
};

-- TODO: new String was not ""
