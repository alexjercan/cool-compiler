class Main {
    linux: Linux <- new Linux;

    main(): Object {
        let sockfd: Int <- linux.socket(new SocketDomain.af_inet(), new SocketType.sock_stream(), 0),
            port: Int <- new HostToNetwork.htons(8080),
            addr_in: Int <- new HostToNetwork.htonl(new SocketDomain.af_inet()),
            addr: SockAddr <- new SockAddrIn.init(addr_in, port, new InAddr.inaddr_any()),
            addrlen: Int <- addr.len(),
            bindres: Int <- linux.bind(sockfd, addr, addrlen),
            listenres: Int <- linux.listen(sockfd, 10)
        in
            {
                new IO.out_string("Listening on port 8080\n");

                while true loop
                    let clientfd: Int <- linux.accept(sockfd, new Ref.init(addr), new Ref.init(addrlen)),
                        buffer: String <- linux.read1(clientfd, 1024)
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
