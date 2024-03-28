class Object {
    type_name(): String extern;
    copy(): SELF_TYPE extern;
    equals(x: Object): Bool extern;

    abort(): Object {
        {
            new IO.out_string("Abort called from class ").out_string(type_name()).out_string("\n");
            new Linux.exit(0);
        }
    };

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
    l: Int;
    str: String <- extern;

    length(): Int { l };
    concat(s: String): String extern;
    substr(i: Int, l: Int): String extern;
    ord(): Int extern;

    repeat(n: Int): String {
        let s: String <- case self of me: String => me; esac,
            i: Int <- 0,
            buf: String <- ""
        in
            {
                while i <= n loop
                    {
                        buf <- buf.concat(s);
                        i <- i + 1;
                    }
                pool;
                buf;
            }
    };

    equals(x: Object): Bool {
        let s: String <- case self of me: String => me; esac,
            other: String <- case x of me: String => me; esac,
            i: Int <- 0,
            l1: Int <- s.length(),
            l2: Int <- other.length(),
            c1: String,
            c2: String,
            eq: Bool <- true
        in
            {
                while (i < l1).and(i < l2) loop
                    {
                        c1 <- s.substr(i, 1);
                        c2 <- other.substr(i, 1);
                        eq <- eq.and(c1.ord() = c2.ord());
                        i <- i + 1;
                    }
                pool;

                if l1 = l2 then eq else false fi;
            }
    };

    to_string(): String {
        case self of me: String => me; esac
    };

    to_int(): Int {
        let s: String <- case self of me: String => me; esac,
            l: Int <- s.length(),
            c: String,
            n: Int <- 0,
            i: Int <- 0,
            d: Int
        in
            {
                while i < l loop
                    {
                        c <- s.substr(i, 1);
                        d <- c.ord() - "0".ord();
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

    chr(): String extern;

    equals(x: Object): Bool {
        let n: Int <- case self of me: Int => me; esac,
            m: Int <- case x of me: Int => me; esac
        in (n <= m).and(m <= n)
    };

    abs(): Int {
        case self of me: Int => if me < 0 then ~me else me fi; esac
    };

    mod(x: Int): Int {
        case self of me: Int => let v: Int <- me - (me / x) * x in if me < 0 then x + v else v fi; esac
    };

    to_string(): String {
        let x: Int <- case self of me: Int => me; esac,
            s: String <- "",
            n: Int <- x.abs(),
            d: Int
        in
            {
                while not n = 0 loop
                    {
                        d <- n.mod(10) + "0".ord();
                        s <- d.chr().concat(s);
                        n <- n / 10;
                    }
                pool;

                if x = 0 then "0" else if x < 0 then "~".concat(s) else s fi fi;
            }
    };
};

class Bool inherits Object {
    val: Bool <- extern;

    equals(x: Object): Bool {
        not (case self of me: Bool => me; esac).xor(case x of me: Bool => me; esac)
    };

    and(x: Bool): Bool {
        case self of me: Bool => if me then x else false fi; esac
    };

    or(x: Bool): Bool {
        case self of me: Bool => if me then true else x fi; esac
    };

    xor(x: Bool): Bool {
        case self of me: Bool => if me then not x else x fi; esac
    };

    to_int(): Int {
        case self of me: Bool => if me then 1 else 0 fi; esac
    };

    to_string(): String {
        case self of me: Bool => if me then "true" else "false" fi; esac
    };
};

class Float {
    val: Int;

    fromInt(x: Int): SELF_TYPE {
        { val <- x; self; }
    };

    toInt(): Int { val };
};
