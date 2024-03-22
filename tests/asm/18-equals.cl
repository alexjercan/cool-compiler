class Point {
    x: Int;
    y: Int;

    init(xv: Int, yv: Int): SELF_TYPE {
        {
            x <- xv;
            y <- yv;
            self;
        }
    };

    x(): Int {
        x
    };

    y(): Int {
        y
    };

    equals(other: Object): Bool {
        case other of
            p: Point => (p.x() = x).and(p.y() = y);
            p: Object => false;
        esac
    };
};

class Something {};

class Main inherits IO {
    a: Point <- new Point.init(30, 40);
    b: Point <- new Point.init(30, 40);
    c: Point <- new Point.init(30, 50);
    d: Object <- new Object;
    e: Something <- new Something;
    f: Something <- new Something;

    main(): Object {
        {
            if a = b then
                self.out_string("a = b\n")
            else
                self.out_string("a != b\n")
            fi;

            if b = c then
                self.out_string("b = c\n")
            else
                self.out_string("b != c\n")
            fi;

            if a = d then
                self.out_string("a = d\n")
            else
                self.out_string("a != d\n")
            fi;

            if e = f then
                self.out_string("e = f\n")
            else
                self.out_string("e != f\n")
            fi;

            if e = e then
                self.out_string("e = e\n")
            else
                self.out_string("e != e\n")
            fi;
        }
    };
};
