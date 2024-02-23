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
            let n : Int <- out_string("Calculăm factorial pentru: ").in_int()
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