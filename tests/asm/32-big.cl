(*
    Listă nevidă eterogenă, cu elemente având tipul static Object și tipuri
    dinamice amestecate. Sfârșitul liste este semnalat de next = void.

    Adaptare după Alex Aiken.
*)
class List inherits IO {
    elem : Object;
    next : List;

    init(e : Object, n : List) : List {
        {
            elem <- e;
            next <- n;
            self;
        }
    };

    print() : IO {
        let str : String <-
                -- case permite ramificarea execuției în funcție de tipul
                -- dinamic. abort() oprește execuția. Șirul vid care îi urmează
                -- este necesar pentru verificarea statică a tipurilor.
                case elem of
                    s : String => s;
                    n : Int => new A2I.i2a(n);  -- A2I este definită mai jos
                    o : Object => { abort(); ""; };
                esac
        in
            {
                out_string(str.concat(" "));
                if (isvoid next) then out_string("\n") else next.print() fi;
            }
    };
};

class Main inherits IO {
    main() : Object {
        {
            let x : Int <- 0,
                y : String <- "!",
                z : Int <- x + 2,
                empty : List,  -- inițializată implicit la void
                list : List <-
                    new List.init(x,
                        new List.init(y,
                            new List.init(z, empty)))
            in
                list.print();

            -- out_string întoarce IO, și putem înlănțui mai multe operații.
            let n : Int <- out_string("Calculam factorial pentru: ").in_int()
            in
                {
                    out_string("Factorial recursiv: ").out_int(fact_rec(n))
                        .out_string("\n");
                    out_string("Factorial iterativ: ").out_int(fact_iter(n))
                        .out_string("\n");
                };
        }
    };

    -- factorial implementat recursiv
    fact_rec(n : Int) : Int {
        if n = 0 then 1 else n * fact_rec(n - 1) fi
    };

    -- factorial implementat iterativ
    fact_iter(n : Int) : Int {
        let res : Int <- 1
        -- Blocurile {} sunt văzute ca expresii. Valoarea ultimei expresii
        -- din bloc este valoarea blocului.
        in
            {
                while (not (n = 0)) loop
                    {
                        res <- res * n;
                        n <- n - 1;
                    }
                pool;
                res;
            }
    };
};

(*
   The class A2I provides integer-to-string and string-to-integer
conversion routines.  To use these routines, either inherit them
in the class where needed, have a dummy variable bound to
something of type A2I, or simpl write (new A2I).method(argument).
*)


(*
   c2i   Converts a 1-character string to an integer.  Aborts
         if the string is not "0" through "9"
*)
class A2I {

     c2i(char : String) : Int {
    if char = "0" then 0 else
    if char = "1" then 1 else
    if char = "2" then 2 else
        if char = "3" then 3 else
        if char = "4" then 4 else
        if char = "5" then 5 else
        if char = "6" then 6 else
        if char = "7" then 7 else
        if char = "8" then 8 else
        if char = "9" then 9 else
        { abort(); 0; }  -- the 0 is needed to satisfy the typchecker
        fi fi fi fi fi fi fi fi fi fi
     };

(*
   i2c is the inverse of c2i.
*)
     i2c(i : Int) : String {
    if i = 0 then "0" else
    if i = 1 then "1" else
    if i = 2 then "2" else
    if i = 3 then "3" else
    if i = 4 then "4" else
    if i = 5 then "5" else
    if i = 6 then "6" else
    if i = 7 then "7" else
    if i = 8 then "8" else
    if i = 9 then "9" else
    { abort(); ""; }  -- the "" is needed to satisfy the typchecker
        fi fi fi fi fi fi fi fi fi fi
     };

(*
   a2i converts an ASCII string into an integer.  The empty string
is converted to 0.  Signed and unsigned strings are handled.  The
method aborts if the string does not represent an integer.  Very
long strings of digits produce strange answers because of arithmetic
overflow.

*)
     a2i(s : String) : Int {
        if s.length() = 0 then 0 else
    if s.substr(0,1) = "-" then ~a2i_aux(s.substr(1,s.length()-1)) else
        if s.substr(0,1) = "+" then a2i_aux(s.substr(1,s.length()-1)) else
           a2i_aux(s)
        fi fi fi
     };

(*
  a2i_aux converts the usigned portion of the string.  As a programming
example, this method is written iteratively.
*)
     a2i_aux(s : String) : Int {
    (let int : Int <- 0 in
           {
               (let j : Int <- s.length() in
              (let i : Int <- 0 in
            while i < j loop
            {
                int <- int * 10 + c2i(s.substr(i,1));
                i <- i + 1;
            }
            pool
          )
           );
              int;
        }
        )
     };

(*
    i2a converts an integer to a string.  Positive and negative
numbers are handled correctly.
*)
    i2a(i : Int) : String {
    if i = 0 then "0" else
        if 0 < i then i2a_aux(i) else
          "-".concat(i2a_aux(i * ~1))
        fi fi
    };

(*
    i2a_aux is an example using recursion.
*)
    i2a_aux(i : Int) : String {
        if i = 0 then "" else
        (let next : Int <- i / 10 in
        i2a_aux(next).concat(i2c(i - next * 10))
        )
        fi
    };

};
