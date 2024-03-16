class A inherits IO {
    f(value : Object) : String {
        case value of
            x: String => x;
            x: Object => { abort(); ""; };
        esac
    };
};
