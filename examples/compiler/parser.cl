class Main {
    main(): Object {
        let text: String <- new IO.in_string(),
            tokens: List (* Token *) <- new Tokenizer.init(text).tokenize(),
            program: ProgramNode <- new Parser.init(tokens).parse_program()
        in
            program.show(0).out_string("\n")
    };
};

