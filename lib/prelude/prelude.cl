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
            linux.write(1, x, x.length());
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
        let tmp: String <- linux.read1(0, 1024),
            s: String <- tmp
        in
            {
                while tmp.length() = 1024 loop
                    {
                        tmp <- linux.read1(0, 1024);
                        s <- s.concat(tmp);
                    }
                pool;

                s.trim_right(new Byte.from_string("\n"));
            }
    };

    in_int(): Int {
        new Int.from_string(in_string())
    };
};

class Tuple {
    fst: Object;
    snd: Object;

    init(f: Object, s: Object): SELF_TYPE {
        {
            fst <- f;
            snd <- s;
            self;
        }
    };

    fst(): Object { fst };

    snd(): Object { snd };

    equals(x: Object): Bool {
        let t: Tuple <- case x of me: Tuple => me; esac
        in fst.equals(t.fst()).and(snd.equals(t.snd()))
    };
};

class String inherits Object {
    l: Int;
    str: String <- extern;

    concat(s: String): String extern;
    substr(i: Int, l: Int): String extern;

    length(): Int { l };

    at(i: Int): Byte { new Byte.from_string(substr(i, 1)) };

    split(delim: String): Tuple {
        let s: String <- case self of me: String => me; esac,
            i: Int <- 0,
            l: Int <- s.length(),
            d: Int <- delim.length(),
            found: Bool <- false,
            res: Tuple
        in
            {
                while (i < l).and(not found) loop
                    if s.substr(i, d) = delim then
                        found <- true
                    else
                        i <- i + 1
                    fi
                pool;

                res <- if found then
                    new Tuple.init(s.substr(0, i), s.substr(i + d, l - i - d))
                else
                    new Tuple.init(s, "")
                fi;
            }
    };

    trim_right(b: Byte): String {
        let s: String <- case self of me: String => me; esac,
            i: Int <- s.length() - 1,
            c: Byte <- new Byte.from_string(s.substr(i, 1))
        in
            {
                while (0 <= i).and(c = b) loop
                    {
                        i <- i - 1;
                        c <- new Byte.from_string(s.substr(i, 1));
                    }
                pool;

                s.substr(0, i + 1);
            }
    };

    repeat(n: Int): String {
        let s: String <- case self of me: String => me; esac,
            i: Int <- 0,
            buf: String <- ""
        in
            {
                while i < n loop
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
            eq: Bool <- true
        in
            {
                while (i < l1).and(i < l2) loop
                    {
                        let b1: Byte <- new Byte.from_string(s.substr(i, 1)),
                            b2: Byte <- new Byte.from_string(other.substr(i, 1))
                        in eq <- eq.and(b1 = b2);
                        i <- i + 1;
                    }
                pool;

                if l1 = l2 then eq else false fi;
            }
    };
};

class Bool inherits Object {
    val: Bool <- extern;

    equals(x: Object): Bool {
        case x of me: Bool => me.to_int() = to_int(); esac
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

    from_int(x: Int): Bool {
        not x = 0
    };

    to_int(): Int {
        case self of me: Bool => if me then 1 else 0 fi; esac
    };

    from_string(s: String): Bool {
        if s = "true" then true else false fi
    };

    to_string(): String {
        case self of me: Bool => if me then "true" else "false" fi; esac
    };
};

(* Byte is a signed 8-bit integer *)
class Byte {
    val: Int <- extern;

    from_string(s: String): Byte extern;
    to_string(): String extern;
    from_int(x: Int): Byte extern;
    to_int(): Int extern;

    isspace(): Bool {
        let c: String <- to_string()
        in (c = " ").or(c = "\n").or(equals(new Byte.from_int(13)))
    };

    islower(): Bool {
        let a: Int <- new Byte.from_string("a").to_int(),
            z: Int <- new Byte.from_string("z").to_int(),
            c: Int <- to_int()
        in (a <= c).and(c <= z)
    };

    isupper(): Bool {
        let a: Int <- new Byte.from_string("A").to_int(),
            z: Int <- new Byte.from_string("Z").to_int(),
            c: Int <- to_int()
        in (a <= c).and(c <= z)
    };

    isdigit(): Bool {
        let zero: Int <- new Byte.from_string("0").to_int(),
            nine: Int <- new Byte.from_string("9").to_int(),
            c: Int <- to_int()
        in (zero <= c).and(c <= nine)
    };

    isalnum(): Bool { islower().or(isupper()).or(isdigit()) };

    iscool(): Bool { isalnum().or(equals(new Byte.from_string("_"))) };

    equals(x: Object): Bool {
        case x of me: Byte => me.to_int() = to_int(); esac
    };
};

(* Word is a signed 16-bit integer *)
class Word {
    val: Int <- extern;

    from_int(x: Int): Word extern;
    to_int(): Int extern;

    equals(x: Object): Bool {
        case x of me: Word => me.to_int() = to_int(); esac
    };
};

(* DoubleWord is a signed 32-bit integer *)
class DoubleWord {
    val: Int <- extern;

    from_int(x: Int): DoubleWord extern;
    to_int(): Int extern;

    equals(x: Object): Bool {
        case x of me: DoubleWord => me.to_int() = to_int(); esac
    };
};

(* QuadWord is a signed 64-bit integer - also referred to as Int *)
class Int inherits Object {
    val: Int <- extern;

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

    from_string(s: String): Int {
        let l: Int <- s.length(),
            i: Int <- 0,
            n: Int <- 0,
            z: Int <- new Byte.from_string("0").to_int()
        in
            {
                while i < l loop
                    {
                        n <- n * 10 + new Byte.from_string(s.substr(i, 1)).to_int() - z;
                        i <- i + 1;
                    }
                pool;
                n;
            }
    };

    to_string(): String {
        let x: Int <- case self of me: Int => me; esac,
            s: String <- "",
            n: Int <- x.abs(),
            z: Int <- new Byte.from_string("0").to_int()
        in
            {
                while not n = 0 loop
                    {
                        s <- new Byte.from_int(n.mod(10) + z).to_string().concat(s);
                        n <- n / 10;
                    }
                pool;

                if x = 0 then "0" else if x < 0 then "~".concat(s) else s fi fi;
            }
    };
};

(* Single precision floating point number using 32-bit XMM registers *)
class Float {
    val: Int <- extern;

    from_fraction(n: Int, d: Int): Float extern;
    from_int(x: Int): Float extern;
    to_int(): Int extern;
    mul(x: Float): Float extern;
    div(x: Float): Float extern;
    add(x: Float): Float extern;
    sub(x: Float): Float extern;
    neg(): Float extern;
    equals(x: Object): Bool extern;
};
