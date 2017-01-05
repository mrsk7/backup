pr(X,L) :- NL is 0,
    (X =:= 5, NNL is NL+1;
    X < 10,  L is (NNL+1)).
