class Token {
    value: Optional (* String *) <- new None;
    pos: Int;

    with_value(v: String): SELF_TYPE {
        { value <- new Some.init(v); self; }
    };

    with_position(p: Int): SELF_TYPE {
        { pos <- p; self; }
    };

    value(): Optional (* String *) { value };

    to_string(): String { { abort(); ""; } };
};

class TokenEnd inherits Token {
    to_string(): String { "END" };
};
class TokenIllegal inherits Token {
    to_string(): String { "ILLEGAL" };
};
class TokenPlus inherits Token {
    to_string(): String { "PLUS" };
};
class TokenInt inherits Token {
    to_string(): String { "INT_LITERAL" };
};

class Tokenizer {
    buffer: String;
    pos: Int;
    read_pos: Int;
    ch: Byte;

    bEOF: Byte <- new Byte.from_int(0);
    bPLUS: Byte <- new Byte.from_string("+");

    init(text: String): SELF_TYPE {
        {
            buffer <- text;
            read_char();
            self;
        }
    };

    peek_char(): Byte {
        if buffer.length() <= read_pos then
            bEOF
        else
            buffer.at(read_pos)
        fi
    };

    read_char(): Byte {
        {
            ch <- peek_char();

            pos <- read_pos;
            read_pos <- read_pos + 1;

            ch;
        }
    };

    skip_whitespaces(): Object {
        while ch.isspace() loop read_char() pool
    };

    next_token(): Token {
        {
            skip_whitespaces();

            let position: Int <- pos
            in
                if ch = bEOF then
                    { read_char(); new TokenEnd; }
                else if ch = bPLUS then
                    { read_char(); new TokenPlus.with_position(position); }
                else if ch.isdigit() then
                    {
                        while ch.isdigit() loop read_char() pool;
                        let v: String <- buffer.substr(position, pos - position)
                        in new TokenInt.with_position(position).with_value(v);
                    }
                else
                    { read_char(); new TokenIllegal.with_value(buffer.substr(position, 1)).with_position(position); }
                fi fi fi;
        }
    };

    tokenize(): List (* Token *) {
        let token: Token <- next_token(),
            result: List <- new List.single(token)
        in
            {
                while case token of t: TokenEnd => false; t: Token => true; esac loop
                    {
                        token <- next_token();
                        result <- result.append(token);
                    }
                pool;

                result;
            }
    };
};

class Main {
    main(): Object {
        let text: String <- new IO.in_string(),
            tokens: List (* Token *) <- new Tokenizer.init(text).tokenize(),
            iter: List <- tokens
        in
            while not isvoid iter loop
                {
                    case iter.value() of
                        t: Token =>
                            {
                                new IO.out_string(t.to_string());
                                case t.value() of
                                    v: None => "";
                                    v: Some => case v.value() of
                                        v: String => new IO.out_string("(").out_string(v).out_string(")");
                                    esac;
                                esac;

                                new IO.out_string("\n");
                            };
                        o: Object => abort();
                    esac;

                    iter <- iter.next();
                }
            pool
    };
};
