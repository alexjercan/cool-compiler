class A {
    n : Int;
    b : Bool <- 2 <= n;
    e : Bool <- not not 2 <= n;
    f : Bool <- not 2 = n;

    oA : A;
    oB : B;
    j : Bool <- oA = oA;
    k : Bool <- oA = oB;
};

class B {};
