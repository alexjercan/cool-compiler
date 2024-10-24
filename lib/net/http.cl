class HTTPSerde inherits Serde {
    deserialize(input: String): Tuple (* Message, String *) {
        new HTTPRequest.deserialize(input)
    };
};

class HTTPRequest inherits Message {
    method: String;
    path: String;

    init(m: String, p: String): SELF_TYPE {
        {
            method <- m;
            path <- p;
            self;
        }
    };

    method(): String { method };
    path(): String { path };

    -- TODO: need to implement serialize
    serialize(): String { { abort(); ""; } };

    deserialize(input: String): Tuple (* Message, String *) {
        let
            tup: Tuple <- input.split(" "),
            m: String <- case tup.fst() of m: String => m; esac,
            input: String <- case tup.snd() of i: String => i; esac,
            tup: Tuple <- input.split(" "),
            p: String <- case tup.fst() of p: String => p; esac,
            msg: Message <- new HTTPRequest.init(m, p)
        in
            new Tuple.init(msg, "")
    };
};

class HTTPResponse inherits Message {
    body: String;
    status: Int;
    body_length: Int;

    init(b: String, s: Int): SELF_TYPE {
        {
            body <- b;
            status <- s;
            body_length <- b.length();
            self;
        }
    };

    http_version_string(): String {
        "HTTP/1.1"
    };

    status_string(): String {
        if status = 200 then "200 OK"
        else "404 NOT FOUND"
        fi
    };

    headers_string(): String {
        "Content-Type: application/json"
            .concat("\n")
            .concat("Content-Length: ").concat(body_length.to_string())
            .concat("\n")
    };

    serialize(): String {
        http_version_string()
            .concat(" ")
            .concat(status_string())
            .concat("\n")
            .concat(headers_string())
            .concat("\n")
            .concat(body)
    };

    -- TODO: need to implement deserialize
    deserialize(input: String): Tuple { { abort(); new Tuple; } };
};

class HTTPServer {
    server: Server;

    init(port: Int): SELF_TYPE {
        {
            server <- new Server.init(port, new HTTPSerde).listen(100);
            self;
        }
    };

    handler(message: HTTPRequest): HTTPResponse { { abort(); new HTTPResponse; } };

    run(): Object {
        while true loop
            let clientfd: Int <- server.accept(),
                message: Message <- server.recv(clientfd),
                response: Message <- case message of m: HTTPRequest => handler(m); esac
            in
                server.send(clientfd, response)
        pool
    };
};
