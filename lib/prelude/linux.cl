(* Linux interface *)

(* The Linux class responsible for handling syscalls. *)
class Linux {
    read(fd: Int, buffer: Ref (* String *), count: Int): Int extern;
    write(fd: Int, buf: String, count: Int): Int extern;
    close(fd: Int): Int extern;

    socket(domain: Int, type: Int, protocol: Int): Int extern;
    accept(sockfd: Int, addr: Ref (* SockAddr *), addrlen: Ref (* Int *)): Int extern;
    bind(sockfd: Int, addr: SockAddr, addrlen: Int): Int extern;
    listen(sockfd: Int, backlog: Int): Int extern;

    exit(status: Int): Object extern;

    read1(fd: Int, count: Int): String {
        let buffer: Ref <- new Ref.init(new String),
            ret: Int <- self.read(fd, buffer, count)
        in case buffer.value() of buffer: String => buffer; esac
    };
};

(* Container Object, used as a pointer to store the value. *)
class Ref {
    val: Object;

    init(value: Object): SELF_TYPE { { val <- value; self; } };

    value(): Object { val };
};

(* HostToNetwork class, responsible for converting host byte order to network byte order. *)
class HostToNetwork {
    (* Convert short integer from host byte order to network byte order. *)
    htons(short: Int): Int {
        short.mod(256) * 256 + (short / 256).mod(256)
    };

    (* Convert long integer from host byte order to network byte order. *)
    htonl(long: Int): Int {
        long.mod(256) * 256 * 256 * 256 + (long / 256).mod(256) * 256 * 256
            + (long / 256 / 256).mod(256) * 256 + (long / 256 / 256 / 256).mod(256)
    };
};

(* SocketDomain class, responsible for defining the socket domain. *)
class SocketDomain {
    af_inet(): Int { 2 };
};

(* SocketType class, responsible for defining the socket type. *)
class SocketType {
    sock_stream(): Int { 1 };
};

(* SockAddr class, responsible for defining the socket address. *)
class SockAddr {
    sa_data: String;

    len(): Int { sa_data.length() };
};

(* SockAddrIn class, responsible for defining the socket address for the Internet. *)
class SockAddrIn inherits SockAddr {
    init(sin_family: Int, sin_port: Int, sin_addr: Int): SELF_TYPE {
        let sin_family_1: Byte <- new Byte.from_int(sin_family),
            sin_family_2: Byte <- new Byte.from_int(sin_family / 256),
            sin_port_1: Byte <- new Byte.from_int(sin_port),
            sin_port_2: Byte <- new Byte.from_int(sin_port / 256),
            sin_addr_1: Byte <- new Byte.from_int(sin_addr),
            sin_addr_2: Byte <- new Byte.from_int(sin_addr / 256),
            sin_addr_3: Byte <- new Byte.from_int(sin_addr / 256 / 256),
            sin_addr_4: Byte <- new Byte.from_int(sin_addr / 256 / 256 / 256),
            zero: Byte <- new Byte.from_int(0)
        in
            {
                sa_data <- sin_family_1.to_string().concat(sin_family_2.to_string())
                    .concat(sin_port_1.to_string()).concat(sin_port_2.to_string())
                    .concat(sin_addr_1.to_string()).concat(sin_addr_2.to_string())
                    .concat(sin_addr_3.to_string()).concat(sin_addr_4.to_string())
                    .concat(zero.to_string().repeat(8));
                self;
            }

    };
};

(* InAddr class, responsible for defining the Internet address. *)
class InAddr {
    inaddr_any(): Int { 0 };
};
