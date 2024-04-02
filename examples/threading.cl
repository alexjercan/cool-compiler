class MyThread inherits Thread {
    index: Int;

    init(i: Int): SELF_TYPE { { index <- i; self; } };

    run(): Object {
        new IO.out_string("Hello, World from thread ").out_int(index).out_string("\n")
    };
};

class Main {
    pthread: PThread <- new PThread;
    thread_1: Int;
    thread_2: Int;

    main(): Object {
        {
            thread_1 <- pthread.spawn(new MyThread.init(1));
            thread_2 <- pthread.spawn(new MyThread.init(2));
            pthread.join(thread_1);
            pthread.join(thread_2);
        }
    };
};
