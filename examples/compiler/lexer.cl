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
