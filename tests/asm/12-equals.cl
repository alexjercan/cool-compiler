class Point {
    x: Int;
    y: Int;

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

class Main inherits IO {
    x: Int <- 45 - 10 / 2 * 3;
    y: Int <- x + 39;

    main(): Object {
        {
            if x = y then
                self.out_string("x = y\n")
            else
                self.out_string("x != y\n")
            fi;

            if y = 69 then
                self.out_string("nice\n")
            else
                self.out_string("not nice\n")
            fi;
        }
    };
};
