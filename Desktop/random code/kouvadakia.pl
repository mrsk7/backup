gcd(X, 0, X):- !.
gcd(0, X, X):- !.
gcd(X, Y, D):- X > Y, !, Z is X mod Y, gcd(Y, Z, D).
gcd(X, Y, D):- Z is Y mod X, gcd(X, Z, D).

routeOne(Vg,_,_,_,Vg,0,[]).
routeOne(_,Vg,_,_,Vg,0,[]).
routeOne(0,Y,V1,V2,Vg,Cnt,['01'|Acc]) :- routeOne(V1,Y,V1,V2,Vg,NCnt,Acc),Cnt is NCnt+1.
routeOne(X,V2,V1,V2,Vg,Cnt,['20'|Acc]) :- routeOne(X,0,V1,V2,Vg,NCnt,Acc),Cnt is NCnt+1.
routeOne(X,Y,V1,V2,Vg,Cnt,Acc) :- (X>V2-Y, New is X+Y-V2, routeOne(New,V2,V1,V2,Vg,NCnt,Res1), Cnt is NCnt +1, Acc = ['12'|Res1]);
(X=<V2-Y, New is X+Y, routeOne(0,New,V1,V2,Vg,NCnt,Res1), Cnt is NCnt +1, Acc = ['12'|Res1]).

routeTwo(Vg,_,_,_,Vg,0,[]).
routeTwo(_,Vg,_,_,Vg,0,[]).
routeTwo(X,0,V1,V2,Vg,Cnt,['02'|Acc]) :- routeTwo(X,V2,V1,V2,Vg,NCnt,Acc),Cnt is NCnt+1.
routeTwo(V1,Y,V1,V2,Vg,Cnt,['10'|Res1]) :- routeTwo(0,Y,V1,V2,Vg,NCnt,Res1),Cnt is NCnt+1.
routeTwo(X,Y,V1,V2,Vg,Cnt,Acc) :- (Y>V1-X, New is X+Y-V1, routeTwo(V1,New,V1,V2,Vg,NCnt,Res1), Cnt is NCnt +1, Acc = ['21'|Res1]);
(Y=<V1- X, New is X+Y, routeTwo(New,0,V1,V2,Vg,NCnt,Res1), Cnt is NCnt +1, Acc = ['21'|Res1]).

kouvadakia(V1,V2,Vg,X) :-
gcd(V1,V2,Gc), Test is (Vg mod Gc), Test =:= 0, routeOne(V1,0,V1,V2,Vg,Cnt1,X1), routeTwo(0,V2,V1,V2,Vg,Cnt2,X2), (Cnt1<Cnt2, X = ['01'|X1]; Cnt2=<Cnt1, X = ['02'|X2]),!.
