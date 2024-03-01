class A {
    n : Int;
    b : Bool <- 2 <= n;
    c : Bool <- 2 < b;
    d : Bool <- not n;
    e : Bool <- not not 2 <= n;
    f : Bool <- not 2 = n;
    g : Bool <- 2 = b;
    h : Bool <- 2 = "abc";
    i : Bool <- "abc" = b;
    
    oA : A;
    oB : B;
    j : Bool <- oA = oA;
    k : Bool <- oA = oB;
};

class B {};