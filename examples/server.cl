class Main {
    linux: Linux <- new Linux;

    main(): Object {
        let sockfd: Int <- linux.socket(new SocketDomain.af_inet(), new SocketType.sock_stream(), 0),
            port: Int <- new HostToNetwork.htons(8080),
            addr: SockAddr <- new SockAddrIn.init(new SocketDomain.af_inet(), port, new InAddr.inaddr_any()),
            addrlen: Int <- addr.len(),
            bindres: Int <- linux.bind(sockfd, addr, addrlen),
            listenres: Int <- linux.listen(sockfd, 10)
        in
            {
                new IO.out_string("Socket: ").out_int(sockfd).out_string("\n")
                      .out_string("Addr: ").out_int(bindres).out_string("\n")
                      .out_string("Listen: ").out_int(listenres).out_string("\n");

                while true loop
                    let clientfd: Int <- linux.accept(sockfd, addr, addrlen),
                        buffer: String <- linux.read(clientfd, 1024)
                    in
                        {
                            new IO.out_string("Client: ").out_int(clientfd).out_string(" said: ").out_string(buffer).out_string("\n");
                            linux.write(clientfd, "Hello, World!\n", 14);
                            linux.close(clientfd);
                        }
                pool;
            }
    };
};
