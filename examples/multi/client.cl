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

    update(input: PlayerPosition): SELF_TYPE {
        {
            pos_x <- input.pos_x();
            pos_y <- input.pos_y();
            self;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawRectangle(pos_x - size_x / 2, pos_y - size_y / 2, size_x, size_y, raylib.black())
    };
};

class Coin {
    pos_x: Int;
    pos_y: Int;
    radius: Float;

    init(x: Int, y: Int, r: Float): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            radius <- r;
            self;
        }
    };

    update(input: CoinPosition): SELF_TYPE {
        {
            pos_x <- input.pos_x();
            pos_y <- input.pos_y();
            self;
        }
    };

    draw(raylib: Raylib): Raylib {
        raylib.drawCircle(pos_x, pos_y, radius, raylib.gold())
    };
};

class Client {
    linux: Linux <- new Linux;

    addr: String;
    port: Int;
    sockfd: Int;

    init(a: String, p: Int): SELF_TYPE {
        {
            addr <- a;
            port <- p;
            self;
        }
    };

    connect(): SELF_TYPE {
        let sockfd1: Int <- linux.socket(new SocketDomain.af_inet(), new SocketType.sock_stream(), 0),
            port: Int <- new HostToNetwork.htons(port),
            addr_in: Int <- new SocketDomain.af_inet(),
            server_addr: Int <- new InetHelper.af_inet_pton(addr),
            addr: SockAddr <- new SockAddrIn.init(addr_in, port, server_addr),
            connectres: Int <- linux.connect(sockfd1, new Ref.init(addr), addr.len())
        in
            if connectres < 0 then
                {
                    new IO.out_string("Error connecting to server\n");
                    abort();
                    self;
                }
            else
                {
                    sockfd <- sockfd1;
                    self;
                }
            fi
    };

    recv(): Message {
        let buffer: String <- linux.read1(sockfd, 1024)
        in new MessageHelper.deserialize(buffer)
    };

    send(msg: Message): SELF_TYPE {
        {
            let buffer: String <- msg.serialize()
            in linux.write(sockfd, buffer, buffer.length());
            self;
        }
    };
};

class Main {
    screen_width: Int <- 800;
    screen_height: Int <- 600;
    player_size: Int <- 50;

    raylib: Raylib <- new Raylib;
    player: Player <- new Player.init(screen_width / 2, screen_height / 2, player_size, player_size);
    coin: Coin <- new Coin.init(100, 100, new Float.from_int(10));
    client: Client <- new Client.init("127.0.0.1", 8080).connect();

    main(): Object {
        {
            raylib.initWindow(screen_width, screen_height, "Coin Chase 2D").setTargetFPS(30);
            while raylib.windowShouldClose() = false loop
            {
                case client.recv() of
                    message: PlayerPosition => player.update(message);
                    message: CoinPosition => coin.update(message);
                    message: Message => 0;
                esac;

                raylib.beginDrawing();
                raylib.clearBackground(raylib.raywhite());

                player.draw(raylib);
                coin.draw(raylib);

                raylib.endDrawing();
            }
            pool;
            raylib.closeWindow();
        }
    };
};
