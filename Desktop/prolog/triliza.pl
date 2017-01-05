numOfAppearences(_,[],0).
numOfAppearences(X,[Head|Rest],N) :- 
	(X = Head , numOfAppearences(X,Rest,NewN), N is NewN+1);
	(X \= Head, numOfAppearences(X,Rest,N)).

win(L) :-
        (numOfAppearences('x',L,N), N = 3);
	\+ member('o',L),
        member('b',L), numOfAppearences('b',L,N), N<2.

getDiagonals([[X1,_,X3],[_,Y2,_],[Z1,_,Z3]],C1,C2) :-
    C1 = [X1,Y2,Z3], C2 = [X3,Y2,Z1].


getColumns([[X1,X2,X3],[Y1,Y2,Y3],[Z1,Z2,Z3]],C1,C2,C3) :-
    C1 = [X1,Y1,Z1],C2 = [X2,Y2,Z2], C3 = [X3,Y3,Z3].

check([Row1,Row2,Row3]) :-
    win(Row1) ; win(Row2); win(Row3).
check(L) :-
    getColumns(L,Column1,Column2,Column3),
    win(Column1),win(Column2),win(Column3).
check(L) :-
    getDiagonals(L,D1,D2),
    win(D1); win(D2).
