from openai import OpenAI
from argparse import ArgumentParser
from typing import Optional, List
import sys
from dataclasses import dataclass
from enum import Enum
from omegaconf import OmegaConf


class TokenKind(Enum):
    ARROW = "ARROW"
    ASSIGN = "ASSIGN"
    AT = "AT"
    BOOL_LITERAL = "BOOL_LITERAL"
    CASE = "CASE"
    CLASS = "CLASS"
    CLASS_NAME = "CLASS_NAME"
    COLON = "COLON"
    COMMA = "COMMA"
    DIVIDE = "DIVIDE"
    DOT = "DOT"
    ELSE = "ELSE"
    END = "END"
    EQUAL = "EQUAL"
    ESAC = "ESAC"
    FI = "FI"
    IDENT = "IDENT"
    IF = "IF"
    ILLEGAL = "ILLEGAL"
    IN = "IN"
    INHERITS = "INHERITS"
    INT_LITERAL = "INT_LITERAL"
    ISVOID = "ISVOID"
    LBRACE = "LBRACE"
    LESS_THAN = "LESS_THAN"
    LESS_THAN_EQ = "LESS_THAN_EQ"
    LET = "LET"
    LOOP = "LOOP"
    LPAREN = "LPAREN"
    MINUS = "MINUS"
    MULTIPLY = "MULTIPLY"
    NEW = "NEW"
    NOT = "NOT"
    OF = "OF"
    PLUS = "PLUS"
    POOL = "POOL"
    RBRACE = "RBRACE"
    RPAREN = "RPAREN"
    SEMICOLON = "SEMICOLON"
    STRING_LITERAL = "STRING_LITERAL"
    THEN = "THEN"
    TILDE = "TILDE"
    WHILE = "WHILE"


AVAILABLE_TOKENS = [x.value for x in TokenKind]

EXAMPLE_TEMPLATE = """For the following code: ```cool
{}
```
The lexer should produce the following tokens:
```
{}
```"""

EXAMPLE_1_CODE = """class Main inherits IO {
    (* This is a comment (* and we can have nested comments *) *)
    main(): SELF_TYPE {
        let
            void: List,
            guy: Person <- new Person.init("John", "pizza"),
            l: List <- new List.init("first", new List.init(guy, void))
        in
            l.print()
    };
};"""
EXAMPLE_1_TOKENS = """CLASS\nCLASS_NAME(Main)\nINHERITS\nCLASS_NAME(IO)\nLBRACE\nIDENT(main)\nLPAREN\nRPAREN\nCOLON\nCLASS_NAME(SELF_TYPE)\nLBRACE\nLET\nIDENT(void)\nCOLON\nCLASS_NAME(List)\nCOMMA\nIDENT(guy)\nCOLON\nCLASS_NAME(Person)\nASSIGN\nNEW\nCLASS_NAME(Person)\nDOT\nIDENT(init)\nLPAREN\nSTRING_LITERAL(John)\nCOMMA\nSTRING_LITERAL(pizza)\nRPAREN\nCOMMA\nIDENT(l)\nCOLON\nCLASS_NAME(List)\nASSIGN\nNEW\nCLASS_NAME(List)\nDOT\nIDENT(init)\nLPAREN\nSTRING_LITERAL(first)\nCOMMA\nNEW\nCLASS_NAME(List)\nDOT\nIDENT(init)\nLPAREN\nIDENT(guy)\nCOMMA\nIDENT(void)\nRPAREN\nRPAREN\nIN\nIDENT(l)\nDOT\nIDENT(print)\nLPAREN\nRPAREN\nRBRACE\nSEMICOLON\nRBRACE\nSEMICOLON\nEND"""
EXAMPLE_2_CODE = """class A {
    x : Int #;
};"""
EXAMPLE_2_TOKENS = """CLASS\nCLASS_NAME(A)\nLBRACE\nIDENT(x)\nCOLON\nCLASS_NAME(Int)\nILLEGAL(Invalid character #)\nSEMICOLON\nRBRACE\nSEMICOLON\nEND"""
EXAMPLE_1 = EXAMPLE_TEMPLATE.format(EXAMPLE_1_CODE, EXAMPLE_1_TOKENS)
EXAMPLE_2 = EXAMPLE_TEMPLATE.format(EXAMPLE_2_CODE, EXAMPLE_2_TOKENS)

EXAMPLES = [EXAMPLE_1, EXAMPLE_2]

SYSTEM_PROMPT = """Write a lexer for the Cool programming language. \
The lexer should produce a list of tokens from the input code. \
The tokens should be newline-separated and each token should be in the format `TOKEN_KIND(VALUE)`. \
For example, `CLASS_NAME(Main)`. The lexer should be able to handle the following tokens:
{}

The lexer should also be able to handle errors and produce an `ILLEGAL` token when it encounters an invalid token. \
The ILLEGAL token should contain an appropriate error message. The errors that can occur are:
- Illegal character <char> (when an invalid character is encountered)
- String constant too long (when a string constant is longer than 1024 characters)
- String contains null character (when a string constant contains a null character)
- Unterminated string constant (when a string constant contains a newline character)
- EOF in string constant (when a string constant contains an EOF character)
- Unmatched *) (when a comment is not closed)
- EOF in comment (when a comment contains an EOF character)
After an error is encountered, the lexer should continue lexing the rest of the input code.

The comments in the input code should be ignored by the lexer. \
The lexer should be able to handle nested comments. Comments start with `(*` and end with `*)`.

Here are some examples of input code and the expected output tokens:
{}

Please provide the list of tokens for the following Cool code. \
Make sure to separate the tokens with newlines and use ONLY the presented format for the response. \
Do not add any additional information.
""".format(", ".join(AVAILABLE_TOKENS), "\n".join(EXAMPLES))


@dataclass
class Options:
    input: Optional[str]
    output: Optional[str]
    lex: bool


def parse_args() -> Options:
    parser = ArgumentParser(prog="coolc", description="A cool compiler")
    parser.add_argument("input", nargs='?', type=str, default=None, help="Input file")
    parser.add_argument("--lex", action="store_true", help="Lex the input file")
    parser.add_argument("-o", "--output", type=str, default=None, help="Output file")

    args = parser.parse_args()

    return Options(input=args.input, lex=args.lex, output=args.output)


@dataclass
class Config:
    openai_api_key: str


def read_config() -> Config:
    config = OmegaConf.load("config.yaml")
    return Config(openai_api_key=config.openai_api_key)


def read_file(file_path: Optional[str]) -> str:
    if file_path is None:
        return sys.stdin.read()
    with open(file_path, "r") as f:
        return f.read()


def write_file(file_path: Optional[str], content: str) -> None:
    if file_path is None:
        print(content)
    else:
        with open(file_path, "w") as f:
            f.write(content)


@dataclass
class Token:
    kind: TokenKind
    value: Optional[str]


def token_show(token: Token) -> str:
    if token.value is None:
        return token.kind.value

    return f"{token.kind.value}({token.value})"


def tokens_show(tokens: List[Token]) -> str:
    return "\n".join([token_show(token) for token in tokens])


def lexer_run(config: Config, buffer: str) -> List[Token]:
    client = OpenAI(api_key=config.openai_api_key)

    response = client.chat.completions.create(
        model="gpt-4",
        temperature=0,
        messages=[
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": buffer}
        ],
    )

    text = response.choices[0].message.content
    words = text.splitlines()

    tokens = []
    for word in words:
        if word in AVAILABLE_TOKENS:
            tokens.append(Token(TokenKind(word), None))
        else:
            kind, value = word.split("(")
            value = value[:-1]
            tokens.append(Token(TokenKind(kind), value))

    return tokens


if __name__ == "__main__":
    options = parse_args()
    config = read_config()

    buffer = read_file(options.input)

    tokens = lexer_run(config, buffer)
    if options.lex:
        write_file(options.output, tokens_show(tokens))
