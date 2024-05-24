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
    error: TokenError <- new TokenError;

    error(): TokenError { error };

    with_error(e: TokenError): SELF_TYPE {
        { error <- e; self; }
    };

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
class TokenStringLiteral inherits Token {
    to_string(): String { "STRING_LITERAL" };
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

class TokenError {
    to_string(): String { { abort(); ""; } };
};

class TokenErrorInvalidChar inherits TokenError {
    to_string(): String { "Invalid character" };
};
class TokenErrorStringConstantTooLong inherits TokenError {
    to_string(): String { "String constant too long" };
};
class TokenErrorStringContainsNull inherits TokenError {
    to_string(): String { "String contains null character" };
};
class TokenErrorStringUnterminated inherits TokenError {
    to_string(): String { "Unterminated string constant" };
};
class TokenErrorStringContainsEOF inherits TokenError {
    to_string(): String { "EOF in string constant" };
};
class TokenErrorUnmatchedComment inherits TokenError {
    to_string(): String { "Unmatched *)" };
};
class TokenErrorUnterminatedComment inherits TokenError {
    to_string(): String { "EOF in comment" };
};

class Tokenizer {
    buffer: String;
    pos: Int;
    read_pos: Int;
    ch: Byte;

    bEOF: Byte <- new Byte.from_int(~1);
    bNULL: Byte <- new Byte.from_int(0);
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
    bNEWLINE: Byte <- new Byte.from_string("\n");
    bQUOTE: Byte <- new Byte.from_string("\"");

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

    skip_until_semicolon(): Object {
        while (not ch = bSEMICOLON).and(not ch = bEOF) loop read_char() pool
    };

    skip_until_newline(): Object {
        while (not ch = bNEWLINE).and(not ch = bEOF) loop read_char() pool
    };

    skip_comment(): Optional (* TokenError *) {
        let stack: Int <- 1,
            has_error: Bool <- false,
            result: Optional (* TokenError *) <- new None
        in
            {
                while (0 < stack).and(not has_error) loop
                    {
                        if stack < 0 then
                            {
                                has_error <- true;
                                result <- new Some.init(new TokenErrorUnmatchedComment);
                            }
                        else if ch = bEOF then
                            {
                                has_error <- true;
                                result <- new Some.init(new TokenErrorUnterminatedComment);
                            }
                        else if ch = bLPAREN then
                            if read_char() = bSTAR then { read_char(); stack <- stack + 1; } else "" fi
                        else if ch = bSTAR then
                            if read_char() = bRPAREN then { read_char(); stack <- stack - 1; } else "" fi
                        else
                            read_char()
                        fi fi fi fi;
                    }
                pool;

                result;
            }
    };

    string_literal(): Token {
        let error: TokenError,
            position: Int <- pos,
            string: String <- "",
            char: String
        in
            {
                read_char();  -- skip first "
                while (not ch = bQUOTE).and(isvoid error) loop
                    if ch = bEOF then
                        {
                            skip_until_semicolon();
                            error <- new TokenErrorStringContainsEOF;
                        }
                    else if ch = bNULL then
                        {
                            skip_until_semicolon();
                            error <- new TokenErrorStringContainsNull;
                        }
                    else if ch = bNEWLINE then
                        {
                            skip_until_semicolon();
                            error <- new TokenErrorStringUnterminated;
                        }
                    else
                        {
                            char <- ch.to_string();
                            if char = "\\" then
                                {
                                    read_char();
                                    if ch.to_string() = "n" then
                                        char <- "\n"
                                    else if ch.to_string() = "t" then
                                        char <- "\t"
                                    else if ch.to_string() = "b" then
                                        char <- "\b"
                                    else if ch.to_string() = "f" then
                                        char <- "\f"
                                    else
                                        char <- ch.to_string()
                                    fi fi fi fi;
                                }
                            else 0 fi;
                            string <- string.concat(char);

                            read_char();
                        }
                    fi fi fi
                pool;

                if not isvoid error then
                    new TokenIllegal.with_error(error)
                else
                    {
                        read_char();  -- skip last "

                        if 1024 < string.length() then
                            new TokenIllegal.with_error(new TokenErrorStringConstantTooLong)
                        else
                            new TokenStringLiteral.with_value(string)
                        fi;
                    }
                fi;
            }
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
                    if read_char() = bMINUS then
                        { skip_until_newline(); next_token(); }
                    else
                        { new TokenMinus; }
                    fi
                else if ch = bSTAR then
                    if read_char() = bRPAREN then
                        { read_char(); new TokenIllegal.with_error(new TokenErrorUnmatchedComment); }
                    else
                        { new TokenMultiply; }
                    fi
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
                    if read_char() = bSTAR then
                        {
                            read_char();
                            case skip_comment() of
                                e: Some => case e.value() of e: TokenError => new TokenIllegal.with_error(e); esac;
                                e: None => next_token();
                            esac;
                        }
                    else
                        { new TokenLParen; }
                    fi
                else if ch = bRPAREN then
                    { read_char(); new TokenRParen; }
                else if ch = bQUOTE then
                    { string_literal(); }
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
                    { read_char(); new TokenIllegal.with_value(buffer.substr(position, 1)).with_error(new TokenErrorInvalidChar); }
                fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi fi
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
                                case t of
                                    t: TokenIllegal => {
                                        new IO.out_string("(").out_string(t.error().to_string());
                                        case t.value() of
                                            v: None => "";
                                            v: Some => case v.value() of
                                                v: String => {
                                                    new IO.out_string(" ").out_string(v);
                                                };
                                            esac;
                                        esac;
                                        new IO.out_string(")");
                                    };
                                    t: Token => {
                                        case t.value() of
                                            v: None => "";
                                            v: Some => case v.value() of
                                                v: String => {
                                                    new IO.out_string("(").out_string(v).out_string(")");
                                                };
                                            esac;
                                        esac;
                                    };
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
