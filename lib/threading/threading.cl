class Thread {
    run(): Object { abort() };
};

class PThread {
    run(thread: Thread): Object { thread.run() };

    spawn(thread: Thread): Int extern;
    join(thread_id: Int): Ref (* Object *) extern;
};
