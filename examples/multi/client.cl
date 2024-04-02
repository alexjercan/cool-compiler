class Player {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;
    player_id: Int;

    player_id(): Int { player_id };

    init(x: Int, y: Int, sx: Int, sy: Int, pid: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            size_x <- sx;
            size_y <- sy;
            player_id <- pid;
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
    buffer: String;

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
        {
            buffer <- if buffer = "" then linux.read1(sockfd, 1024) else buffer fi;
            let tup: Tuple <- new MessageHelper.deserialize(buffer)
            in
                {
                    case tup.snd() of buf: String => buffer <- buf; esac;
                    case tup.fst() of msg: Message => msg; esac;
                };
        }
    };

    send(msg: Message): SELF_TYPE {
        {
            let buffer: String <- msg.serialize()
            in linux.write(sockfd, buffer, buffer.length());
            self;
        }
    };
};

class PlayerLobby inherits Thread {
    player_id: Int;
    players: List <- new List.single(new Object);
    coin: Coin <- new Coin.init(0, 0, new Float.from_int(10));
    client: Client;

    player_id(): Int { player_id };

    draw(raylib: Raylib): Raylib {
        {
            let iter: List <- players
            in
                while not isvoid iter loop
                    {
                        case iter.value() of
                            player: Player => player.draw(raylib);
                            o: Object => 0;
                        esac;
                        iter <- iter.next();
                    }
                pool;
            coin.draw(raylib);
        }
    };

    init(c: Client): SELF_TYPE { { client <- c; self; } };

    run(): Object {
        while true loop
            case client.recv() of
                message: PlayerPosition => player_update(message);
                message: CoinPosition => coin_update(message);
                message: PlayerConnected => add_player(message);
                message: PlayerAuthorize => authorize_player(message);
                message: Message => 0;
            esac
        pool
    };

    player_update(msg: PlayerPosition): Object {
        {
            new IO.out_string("Player update\n");
            let iter: List <- players
            in
                while not isvoid iter loop
                    {
                        case iter.value() of
                            player: Player => if player.player_id() = msg.player_id() then player.update(msg) else 0 fi;
                            o: Object => 0;
                        esac;
                        iter <- iter.next();
                    }
                pool;
        }
    };

    coin_update(msg: CoinPosition): Object {
        {
            new IO.out_string("Coin update\n");
            coin.update(msg);
        }
    };

    spawn_player(id: Int): Object {
        players <- players.append(new Player.init(0, 0, 50, 50, id))
    };

    add_player(msg: PlayerConnected): Object {
        {
            new IO.out_string("Player connected ".concat(msg.player_id().to_string()).concat("\n"));
            spawn_player(msg.player_id());
        }
    };

    authorize_player(msg: PlayerAuthorize): Object {
        {
            new IO.out_string("Player authorize ".concat(msg.player_id().to_string()).concat("\n"));
            player_id <- msg.player_id();
            spawn_player(player_id);
        }
    };
};

class Main {
    screen_width: Int <- 800;
    screen_height: Int <- 600;

    raylib: Raylib <- new Raylib;
    client: Client <- new Client.init("127.0.0.1", 8080).connect();
    pthread: PThread <- new PThread;
    lobby: PlayerLobby <- new PlayerLobby.init(client);
    lobby_thread: Int <- pthread.spawn(lobby);

    main(): Object {
        {
            raylib.initWindow(screen_width, screen_height, "Coin Chase 2D").setTargetFPS(30);
            while raylib.windowShouldClose() = false loop
            {
                raylib.beginDrawing();
                raylib.clearBackground(raylib.raywhite());

                lobby.draw(raylib);

                raylib.endDrawing();
            }
            pool;
            raylib.closeWindow();
            pthread.join(lobby_thread);
        }
    };
};
