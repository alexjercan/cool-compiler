class Player {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;

    init(x: Int, y: Int, sx: Int, sy: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            size_x <- sx;
            size_y <- sy;
            self;
        }
    };

    pos_x(): Int {
        pos_x
    };

    pos_y(): Int {
        pos_y
    };

    update(input: PlayerInput): Object {
        if (input.keyA()) then pos_x <- pos_x - size_x else
        if (input.keyD()) then pos_x <- pos_x + size_x else
        if (input.keyW()) then pos_y <- pos_y - size_y else
        if (input.keyS()) then pos_y <- pos_y + size_y else 0 fi fi fi fi
    };
};

class Coin {
    pos_x: Int;
    pos_y: Int;
    radius: Float;
    enabled: Bool <- false;

    cell_size: Int;

    min_x: Int <- 0;
    max_x: Int <- 5;
    min_y: Int <- 0;
    max_y: Int <- 5;

    pos_x(): Int { pos_x };

    pos_y(): Int { pos_y };

    consume(): SELF_TYPE { { enabled <- false; self; } };

    init(x: Int, y: Int, s: Float, size: Int, xm: Int, xM: Int, ym: Int, yM: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            radius <- s;
            cell_size <- size;
            min_x <- xm;
            max_x <- xM;
            min_y <- ym;
            max_y <- yM;
            self;
        }
    };

    update(raylib: Raylib): Object {
        {
            if not enabled then
            {
                pos_x <- raylib.getRandomValue(min_x, max_x) * cell_size;
                pos_y <- raylib.getRandomValue(min_y, max_y) * cell_size;
                enabled <- true;
            }
            else 0 fi;
        }
    };
};

class Server {
    linux: Linux <- new Linux;

    port: Int;
    sockfd: Int;

    init(p: Int): SELF_TYPE {
        {
            port <- p;
            self;
        }
    };

    listen(players: Int): SELF_TYPE {
        let sockfd1: Int <- linux.socket(new SocketDomain.af_inet(), new SocketType.sock_stream(), 0),
            port: Int <- new HostToNetwork.htons(port),
            addr_in: Int <- new SocketDomain.af_inet(),
            addr: SockAddr <- new SockAddrIn.init(addr_in, port, new InAddr.inaddr_any()),
            bindres: Int <- linux.bind(sockfd1, new Ref.init(addr), addr.len()),
            listenres: Int <- linux.listen(sockfd1, players)
        in
            if bindres < 0 then
            {
                new IO.out_string("Failed to bind\n");
                linux.close(sockfd1);
                abort();
                self;
            }
            else if listenres < 0 then
            {
                new IO.out_string("Failed to listen\n");
                linux.close(sockfd1);
                abort();
                self;
            }
            else
            {
                sockfd <- sockfd1;
                self;
            }
            fi fi
    };

    accept(): Int { linux.accept(sockfd, new Ref.null(), new Ref.null()) };

    recv(sockfd: Int): Message { new Message.deserialize(linux.read1(sockfd, 1024)) };

    send(sockfd: Int, msg: Message): SELF_TYPE {
        let buffer: String <- msg.serialize()
        in { linux.write(sockfd, buffer, buffer.length()); self; }
    };
};

class Main {
    linux: Linux <- new Linux;
    players: List;
    coin: Coin;
    server: Server <- new Server.init(8080).listen(10);

    main(): Object {
        {
            new IO.out_string("Listening on port 8080\n");

            while true loop
                let clientfd: Int <- server.accept()
                in
                    {
                        new IO.out_string("Client connected\n");
                        server.send(clientfd, new PlayerPosition.init(0, 50, 50));
                        linux.close(clientfd);
                    }
            pool;
        }
    };
};
