class Player inherits Thread {
    pos_x: Int;
    pos_y: Int;
    size_x: Int;
    size_y: Int;
    player_id: Int;
    lobby: PlayerLobby;

    cell_size: Int;

    min_x: Int <- 0;
    max_x: Int <- 5;
    min_y: Int <- 0;
    max_y: Int <- 5;

    score: Int <- 0;

    thread_id: Int;
    thread_id(): Int { thread_id };
    set_thread_id(id: Int): SELF_TYPE { { thread_id <- id; self; } };

    keep_running: Bool <- true;

    player_id(): Int { player_id };
    score(): Int { score };

    init(x: Int, y: Int, sx: Int, sy: Int, pid: Int, lb: PlayerLobby, size: Int, xm: Int, xM: Int, ym: Int, yM: Int): SELF_TYPE {
        {
            pos_x <- x;
            pos_y <- y;
            size_x <- sx;
            size_y <- sy;
            player_id <- pid;
            lobby <- lb;
            cell_size <- size;
            min_x <- xm;
            max_x <- xM;
            min_y <- ym;
            max_y <- yM;
            self;
        }
    };

    random(): SELF_TYPE {
        {
            pos_x <- (new Random.random().mod(max_x - min_x) + min_x) * cell_size;
            pos_y <- (new Random.random().mod(max_y - min_y) + min_y) * cell_size;
            self;
        }
    };

    pos_x(): Int {
        pos_x
    };

    pos_y(): Int {
        pos_y
    };

    run(): Object {
        while keep_running loop
            case lobby.recv(player_id) of
                message: PlayerInput => update(message);
                message: DisconnectedMessage => { keep_running <- false; lobby.remove_player(self); };
                message: Message => abort();
            esac
        pool
    };

    update(input: PlayerInput): Object {
        {
            if (input.keyA()) then pos_x <- pos_x - size_x else
            if (input.keyD()) then pos_x <- pos_x + size_x else
            if (input.keyW()) then pos_y <- pos_y - size_y else
            if (input.keyS()) then pos_y <- pos_y + size_y else 0 fi fi fi fi;

            if pos_x < min_x * cell_size then pos_x <- max_x * cell_size
            else if max_x * cell_size < pos_x then pos_x <- min_x * cell_size
            else 0 fi fi;

            if pos_y < min_y * cell_size then pos_y <- max_y * cell_size
            else if max_y * cell_size < pos_y then pos_y <- min_y * cell_size
            else 0 fi fi;

            lobby.coin_collision(self);
            lobby.broadcast(new PlayerPosition.init(player_id, pos_x, pos_y));
            lobby.check_player_collision(self);
        }
    };

    score_increase(): Object {
        {
            score <- score + 1;
            lobby.broadcast(new PlayerScore.init(player_id, score));
        }
    };

    damage(): Object {
        {
            score <- score / 2;
            lobby.broadcast(new PlayerScore.init(player_id, score));
        }
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

    update(): Object {
        if not enabled then
        {
            pos_x <- (new Random.random().mod(max_x - min_x) + min_x) * cell_size;
            pos_y <- (new Random.random().mod(max_y - min_y) + min_y) * cell_size;
            enabled <- true;
        }
        else 0 fi
    };
};

class PlayerLobby inherits Thread {
    server: Server;
    pthread: PThread <- new PThread;

    time: Time <- new Time;
    random: Random <- new Random.srand(time.time());

    minX: Int <- 0;
    maxX: Int <- 16;
    minY: Int <- 0;
    maxY: Int <- 12;

    cell_size: Int <- 50;

    coin: Coin <- new Coin.init(0, 0, new Float.from_int(10), cell_size, minX + 1, maxX - 1, minY + 1, maxY - 1);
    players: List (* Player *) <- new List.single(new Object);

    init(s: Server): SELF_TYPE { { server <- s; self; } };

    run(): Object {
        while true loop
            let clientfd: Int <- server.accept(),
                player: Player <- new Player.init(0, 0, 50, 50, clientfd, self, cell_size, minX + 1, maxX - 1, minY + 1, maxY - 1).random(),
                player_thread: Int <- pthread.spawn(player),
                player: Player <- player.set_thread_id(player_thread)
            in
                {
                    new IO.out_string("Client ".concat(clientfd.to_string()).concat(" connected\n"));

                    broadcast(new PlayerConnected.init(clientfd));
                    send(clientfd, new PlayerAuthorize.init(clientfd));
                    let iter: List (* Player *) <- players
                    in
                        while not isvoid iter loop
                            {
                                case iter.value() of
                                    player: Player => {
                                        send(clientfd, new PlayerConnected.init(player.player_id()));
                                        send(clientfd, new PlayerScore.init(player.player_id(), player.score()));
                                    };
                                    player: Object => 0;
                                esac;
                                iter <- iter.next();
                            }
                        pool;

                    players <- players.append(player);

                    sync_coin();
                }
        pool
    };

    recv(player: Int): Message { server.recv(player) };

    send(player: Int, msg: Message): Object { server.send(player, msg) };

    broadcast(msg: Message): Object {
        let iter: List (* Player *) <- players
        in
            while not isvoid iter loop
                {
                    case iter.value() of
                        player: Player => send(player.player_id(), msg);
                        player: Object => 0;
                    esac;
                    iter <- iter.next();
                }
            pool
    };

    sync_coin(): Object {
        {
            coin.update();
            broadcast(new CoinPosition.init(coin.pos_x(), coin.pos_y()));
        }
    };

    coin_collision(player: Player): Object {
        {
            if ((player.pos_x() = coin.pos_x()).and(player.pos_y() = coin.pos_y())) then
                {
                    player.score_increase();
                    coin.consume();
                }
            else 0 fi;

            sync_coin();
        }
    };

    check_player_collision(player: Player): Object {
        let iter: List (* Player *) <- players
        in
            while not isvoid iter loop
                {
                    case iter.value() of
                        other: Player =>
                            if (not player.player_id() = other.player_id()).and(player.pos_x() = other.pos_x()).and(player.pos_y() = other.pos_y()) then
                                let total: Int <- player.score() + 1 + 5 + other.score() + 1,
                                    index: Int <- random.random().mod(total)
                                in
                                    if index < player.score() + 1 + 5 then
                                        {
                                            other.damage();
                                            player.score_increase();
                                            other.random();
                                            other.update(new PlayerInput.init(other.player_id(), false, false, false, false));
                                        }
                                    else
                                        {
                                            player.damage();
                                            other.score_increase();
                                            player.random();
                                            player.update(new PlayerInput.init(player.player_id(), false, false, false, false));
                                        }
                                    fi
                            else 0 fi;
                        other: Object => 0;
                    esac;
                    iter <- iter.next();
                }
            pool
    };

    remove_player(player: Player): Object {
        let iter: List (* Player *) <- players.next(),
            prev: List (* Player *) <- players
        in
            while not isvoid iter loop
                {
                    case iter.value() of
                        p: Player => {
                            if p.player_id() = player.player_id() then
                                {
                                    prev.set_next(iter.next());
                                    pthread.join(player.thread_id());
                                    broadcast(new PlayerDisconnected.init(p.player_id()));
                                    new IO.out_string("Client ".concat(p.player_id().to_string()).concat(" disconnected\n"));
                                }
                            else 0 fi;
                        };
                        p: Object => 0;
                    esac;
                    iter <- iter.next();
                    prev <- prev.next();
                }
            pool
    };
};

class Main {
    linux: Linux <- new Linux;
    server: Server <- new Server.init(8080, new MessageHelper).listen(10);
    pthread: PThread <- new PThread;
    lobby: PlayerLobby <- new PlayerLobby.init(server);
    lobby_thread: Int <- pthread.spawn(lobby);

    main(): Object {
        {
            new IO.out_string("Listening on port 8080\n");

            pthread.join(lobby_thread);
        }
    };
};
