houses(a(norwegian,_,_,_,_),a(_,_,_,_,blue),a(_,_,milk,_,_),_,_):-
member(a(spanish,dog,_,_,_),houses).
member(a(_,_,coffee,_,green),houses).
member(a(ukrainian,_,tea,_,_),houses).
member(a(english,_,_,_,red),houses).
right_of(X,Y) :- houses(X,Y,_,_,_); houses(_,X,Y,_,_,_); houses(_,_,X,Y,_);
houses(_,_,_,X,Y).
next_of(X,Y) :- right_of(X,Y); right_of(Y,X).
right_of(a(_,_,_,_,white),a(_,_,_,_,green)).
member(a(_,snails,_,old_gold,_),houses).
member(a(_,_,_,kools,yellow),houses).
next_of(a(_,_,_,chesterfield,_),a(_,fox,_,_,_)).
next_of(a(_,_,_,kools,_),a(_,horse,_,_,_)).
member(a(_,_,orange_juice,lucky_strike,_),houses).
member(a(japanese,_,_,parliaments,_),houses).
