(* COOL Standard Library *)

(* The Object class is the root of the class hierarchy. *)
class Object {
    (*
        The abort method can be called to halt the program.

        Returns: Object
    *)
    abort(): Object extern;

    (*
        The type_name method returns a string representation of the class of the
        object.

        Returns: String
    *)
    type_name(): String extern;

    (*
        The copy method returns a new object that is a copy of the object on which
        it is called.

        Returns: SELF_TYPE
    *)
    copy(): SELF_TYPE extern;
};

(* The IO class provides basic input and output operations. *)
class IO inherits Object {
    (*
        The out_string method writes a string to the output.

        x: String - The string to write to the output.

        Returns: SELF_TYPE
    *)
    out_string(x: String): SELF_TYPE extern;

    (*
        The out_int method writes an integer to the output.

        x: Int - The integer to write to the output.

        Returns: SELF_TYPE
    *)
    out_int(x: Int): SELF_TYPE extern;

    (*
        The in_string method reads a string from the input.

        Returns: String
    *)
    in_string(): String extern;

    (*
        The in_int method reads an integer from the input.

        Returns: Int
    *)
    in_int(): Int extern;
};

(* The String class provides methods for manipulating strings. *)
class String inherits Object {
    l: Int <- 0;        -- The default length of a String object.
    str: String <- "";  -- The default value of a String object.

    (*
        The length method returns the length of the string.

        Returns: Int
    *)
    length(): Int extern;

    (*
        The concat method returns a new string that is the concatenation of the
        string on which it is called and the string passed as an argument.

        s: String - The string to concatenate with the string on which the method
                    is called.

        Returns: String
    *)
    concat(s: String): String extern;

    (*
        The substr method returns a substring of the string on which it is called.

        i: Int - The starting index of the substring.
        l: Int - The length of the substring.

        Returns: String
    *)
    substr(i: Int, l: Int): String extern;
};

(* The Int class provides methods for manipulating integers. *)
class Int inherits Object {
    val: Int <- 0; -- The default value of an Int object.
};

(* The Bool class provides methods for manipulating booleans. *)
class Bool inherits Object {
    val: Bool <- false; -- The default value of a Bool object.
};
