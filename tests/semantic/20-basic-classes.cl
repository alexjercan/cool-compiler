class A inherits IO {
    x : Int <- abort();
    s : String <- type_name();
    a : Int <- self.copy().copy().copy();
    
    s1 : String <- out_string("a").out_int(3).in_string();
    s2 : String <- out_string("a").out_int(3).in_int();
    
    l1 : Int <- s1.length();
    s3 : String <- s1.concat(s2);
    s4 : String <- s1.substr(x, s3);
};