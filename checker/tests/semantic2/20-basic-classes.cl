class A inherits IO {
    s : String <- type_name();

    s1 : String <- out_string("a").out_int(3).in_string();

    l1 : Int <- s1.length();
    s3 : String <- s1.concat("s2");
};
