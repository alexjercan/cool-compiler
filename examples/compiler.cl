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

class TokenArrow inherits Token {
    to_string(): String { "ARROW" };
};
class TokenAssign inherits Token {
    to_string(): String { "ASSIGN" };
};
class TokenAt inherits Token {
    to_string(): String { "AT" };
};
class TokenBool inherits Token {
    to_string(): String { "BOOL_LITERAL" };
};
class TokenCase inherits Token {
    to_string(): String { "CASE" };
};
class TokenClass inherits Token {
    to_string(): String { "CLASS" };
};
class TokenClassName inherits Token {
    to_string(): String { "CLASS_NAME" };
};
class TokenColon inherits Token {
    to_string(): String { "COLON" };
};
class TokenComma inherits Token {
    to_string(): String { "COMMA" };
};
class TokenDivide inherits Token {
    to_string(): String { "DIVIDE" };
};
class TokenDot inherits Token {
    to_string(): String { "DOT" };
};
class TokenElse inherits Token {
    to_string(): String { "ELSE" };
};
class TokenEnd inherits Token {
    to_string(): String { "END" };
};
class TokenEqual inherits Token {
    to_string(): String { "EQUAL" };
};
class TokenEsac inherits Token {
    to_string(): String { "ESAC" };
};
class TokenExtern inherits Token {
    to_string(): String { "EXTERN" };
};
class TokenFi inherits Token {
    to_string(): String { "FI" };
};
class TokenIdent inherits Token {
    to_string(): String { "IDENT" };
};
class TokenIf inherits Token {
    to_string(): String { "IF" };
};
class TokenIllegal inherits Token {
    to_string(): String { "ILLEGAL" };
};
class TokenIn inherits Token {
    to_string(): String { "IN" };
};
class TokenInherits inherits Token {
    to_string(): String { "INHERITS" };
};
class TokenInt inherits Token {
    to_string(): String { "INT_LITERAL" };
};
class TokenIsvoid inherits Token {
    to_string(): String { "ISVOID" };
};
class TokenLBrace inherits Token {
    to_string(): String { "LBRACE" };
};
class TokenLessThan inherits Token {
    to_string(): String { "LESS_THAN" };
};
class TokenLessThanEq inherits Token {
    to_string(): String { "LESS_THAN_EQ" };
};
class TokenLet inherits Token {
    to_string(): String { "LET" };
};
class TokenLoop inherits Token {
    to_string(): String { "LOOP" };
};
class TokenLParen inherits Token {
    to_string(): String { "LPAREN" };
};
class TokenMinus inherits Token {
    to_string(): String { "MINUS" };
};
class TokenMultiply inherits Token {
    to_string(): String { "MULTIPLY" };
};
class TokenNew inherits Token {
    to_string(): String { "NEW" };
};
class TokenNot inherits Token {
    to_string(): String { "NOT" };
};
class TokenOf inherits Token {
    to_string(): String { "OF" };
};
class TokenPlus inherits Token {
    to_string(): String { "PLUS" };
};
class TokenPool inherits Token {
    to_string(): String { "POOL" };
};
class TokenRBrace inherits Token {
    to_string(): String { "RBRACE" };
};
class TokenRParen inherits Token {
    to_string(): String { "RPAREN" };
};
class TokenSemicolon inherits Token {
    to_string(): String { "SEMICOLON" };
};
class TokenThen inherits Token {
    to_string(): String { "THEN" };
};
class TokenTilde inherits Token {
    to_string(): String { "TILDE" };
};
class TokenWhile inherits Token {
    to_string(): String { "WHILE" };
};


class Tokenizer {
    buffer: String;
    pos: Int;
    read_pos: Int;
    ch: Byte;

    bEOF: Byte <- new Byte.from_int(0);
    bPLUS: Byte <- new Byte.from_string("+");
    bGREATER_THAN: Byte <- new Byte.from_string(">");
    bEQUAL: Byte <- new Byte.from_string("=");
    bLESS_THAN: Byte <- new Byte.from_string("<");
    bMINUS: Byte <- new Byte.from_string("-");
    bAT: Byte <- new Byte.from_string("@");
    bCOLON: Byte <- new Byte.from_string(":");
    bCOMMA: Byte <- new Byte.from_string(",");
    bDIVIDE: Byte <- new Byte.from_string("/");
    bDOT: Byte <- new Byte.from_string(".");
    bLBRACE: Byte <- new Byte.from_string("{");
    bRBRACE: Byte <- new Byte.from_string("}");
    bLPAREN: Byte <- new Byte.from_string("(");
    bRPAREN: Byte <- new Byte.from_string(")");
    bSTAR: Byte <- new Byte.from_string("*");
    bSEMICOLON: Byte <- new Byte.from_string(";");
    bTILDE: Byte <- new Byte.from_string("~");

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

    literal_to_token(v: String): Token {
        if (v = "true").or(v = "false") then
            new TokenBool.with_value(v)
        else if v = "class" then
            new TokenClass
        else if v = "inherits" then
            new TokenInherits
        else if v = "case" then
            new TokenCase
        else if v = "of" then
            new TokenOf
        else if v = "esac" then
            new TokenEsac
        else if v = "extern" then
            new TokenExtern
        else if v = "if" then
            new TokenIf
        else if v = "then" then
            new TokenThen
        else if v = "else" then
            new TokenElse
        else if v = "fi" then
            new TokenFi
        else if v = "let" then
            new TokenLet
        else if v = "in" then
            new TokenIn
        else if v = "loop" then
            new TokenLoop
        else if v = "while" then
            new TokenWhile
        else if v = "pool" then
            new TokenPool
        else if v = "isvoid" then
            new TokenIsvoid
        else if v = "new" then
            new TokenNew
        else if v = "not" then
            new TokenNot
        else
            new TokenIdent.with_value(v)
        fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
    };

    next_token(): Token {
        {
            skip_whitespaces();

            let position: Int <- pos
            in
                if ch = bEOF then
                    { read_char(); new TokenEnd; }
                else if ch = bSEMICOLON then
                    { read_char(); new TokenSemicolon; }
                else if ch = bPLUS then
                    { read_char(); new TokenPlus; }
                else if ch = bMINUS then
                    { read_char(); new TokenMinus; }
                else if ch = bSTAR then
                    { read_char(); new TokenMultiply; }
                else if ch = bDIVIDE then
                    { read_char(); new TokenDivide; }
                else if ch = bTILDE then
                    { read_char(); new TokenTilde; }
                else if ch = bEQUAL then
                    if read_char() = bGREATER_THAN then
                        { read_char(); new TokenArrow; }
                    else
                        { new TokenEqual; }
                    fi
                else if ch = bLESS_THAN then
                    let next: Byte <- read_char()
                    in
                        if next = bMINUS then
                            { read_char(); new TokenAssign; }
                        else if next = bEQUAL then
                            { read_char(); new TokenLessThanEq; }
                        else
                            { new TokenLessThan; }
                        fi fi
                else if ch = bAT then
                    { read_char(); new TokenAt; }
                else if ch = bCOLON then
                    { read_char(); new TokenColon; }
                else if ch = bCOMMA then
                    { read_char(); new TokenComma; }
                else if ch = bDOT then
                    { read_char(); new TokenDot; }
                else if ch = bLBRACE then
                    { read_char(); new TokenLBrace; }
                else if ch = bRBRACE then
                    { read_char(); new TokenRBrace; }
                else if ch = bLPAREN then
                    { read_char(); new TokenLParen; }
                else if ch = bRPAREN then
                    { read_char(); new TokenRParen; }
                else if ch.isupper() then
                    {
                        while ch.iscool() loop read_char() pool;
                        let v: String <- buffer.substr(position, pos - position)
                        in new TokenClassName.with_value(v);
                    }
                else if ch.islower() then
                    {
                        while ch.iscool() loop read_char() pool;
                        let v: String <- buffer.substr(position, pos - position)
                        in literal_to_token(v);
                    }
                else if ch.isdigit() then
                    {
                        while ch.isdigit() loop read_char() pool;
                        let v: String <- buffer.substr(position, pos - position)
                        in new TokenInt.with_value(v);
                    }
                else
                    { read_char(); new TokenIllegal.with_value(buffer.substr(position, 1)); }
                fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
                .with_position(position);
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
