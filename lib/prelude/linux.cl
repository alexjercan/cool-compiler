(* Linux interface *)

(* The Linux class responsible for handling syscalls. *)
class Linux {
    read(fd: Int, count: Int): String extern;
    write(fd: Int, buf: String): Int extern;
    exit(status: Int): Object extern;
};
