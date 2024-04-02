(* Stole the name from Rust *)
class Serde {
    serialize_byte(byte: Byte): String { byte.to_string() };
    deserialize_byte(input: String): Byte { new Byte.from_string(input.substr(0, 1)) };

    serialize_int(int: Int): String {
        let b1: Byte <- new Byte.from_int(int.mod(256)),
            b2: Byte <- new Byte.from_int((int / 256).mod(256)),
            b3: Byte <- new Byte.from_int((int / 256 / 256).mod(256)),
            b4: Byte <- new Byte.from_int((int / 256 / 256 / 256).mod(256)),
            b5: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256).mod(256)),
            b6: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256 / 256).mod(256)),
            b7: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256 / 256 / 256).mod(256)),
            b8: Byte <- new Byte.from_int((int / 256 / 256 / 256 / 256 / 256 / 256 / 256).mod(256))
        in
            serialize_byte(b8).concat(serialize_byte(b7)).concat(serialize_byte(b6)).concat(serialize_byte(b5))
            .concat(serialize_byte(b4)).concat(serialize_byte(b3)).concat(serialize_byte(b2)).concat(serialize_byte(b1))
    };
    deserialize_int(input: String): Int {
        let b1: Byte <- deserialize_byte(input.substr(0, 1)),
            b2: Byte <- deserialize_byte(input.substr(1, 1)),
            b3: Byte <- deserialize_byte(input.substr(2, 1)),
            b4: Byte <- deserialize_byte(input.substr(3, 1)),
            b5: Byte <- deserialize_byte(input.substr(4, 1)),
            b6: Byte <- deserialize_byte(input.substr(5, 1)),
            b7: Byte <- deserialize_byte(input.substr(6, 1)),
            b8: Byte <- deserialize_byte(input.substr(7, 1))
        in
            b8.to_int() + b7.to_int() * 256 + b6.to_int() * 256 * 256 + b5.to_int() * 256 * 256 * 256
            + b4.to_int() * 256 * 256 * 256 * 256 + b3.to_int() * 256 * 256 * 256 * 256 * 256
            + b2.to_int() * 256 * 256 * 256 * 256 * 256 * 256 + b1.to_int() * 256 * 256 * 256 * 256 * 256 * 256 * 256
    };
};

(* Base class for all messages *)
class Message {
    serialize(): String { { abort(); ""; } };
    deserialize(input: String): Tuple { { abort(); new Tuple; } };
};

(* The client class *)
class Client {
    linux: Linux <- new Linux;

    addr: String;
    port: Int;
    sockfd: Int;
    buffer: String;

    (* Initialize the client with the address and port *)
    init(a: String, p: Int): SELF_TYPE {
        {
            addr <- a;
            port <- p;
            self;
        }
    };

    (* Connect to the server *)
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

    (* Receive a message from the server *)
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

    (* Send a message to the server *)
    send(msg: Message): SELF_TYPE {
        let buffer: String <- msg.serialize()
        in { linux.write(sockfd, buffer, buffer.length()); self; }
    };
};

(* A buffer for the client *)
class ClientBuffer {
    buffer: String;
    sockfd: Int;

    buffer(): String { buffer };
    sockfd(): Int { sockfd };

    init(fd: Int): SELF_TYPE { { sockfd <- fd; self; } };
};

(* The server class *)
class Server {
    linux: Linux <- new Linux;

    port: Int;
    sockfd: Int;

    buffers: List (* ClientBuffer *) <- new List.single(new Object);

    (* Start the server on the given port *)
    init(p: Int): SELF_TYPE {
        {
            port <- p;
            self;
        }
    };

    (* Listen for incoming connections *)
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

    (* Accept a connection *)
    accept(): Int { linux.accept(sockfd, new Ref.null(), new Ref.null()) }; -- TODO: add new buffer to list

    (* Receive a message from the client *)
    recv(sockfd: Int): Tuple { new Message.deserialize(linux.read1(sockfd, 1024)) }; -- TODO: Handle buffer

    (* Send a message to the client *)
    send(sockfd: Int, msg: Message): SELF_TYPE {
        let buffer: String <- msg.serialize()
        in { linux.write(sockfd, buffer, buffer.length()); self; }
    };
};
