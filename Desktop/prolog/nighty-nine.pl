myLast(H,[H]).
myLast(X,[_|T]) :- myLast(X,T).

last_but_one(H,[H|[_]]).
last_but_one(X,[_|T]) :- last_but_one(X,T).

elem_help(H,[H|_],N,N).
elem_help(X,[_|T],N,C) :- NewC is C+1, elem_help(X,T,N,NewC).

element_at(X,L,N) :- elem_help(X,L,N,1).

mylength(0,[]).
mylength(N,[_|T]) :- mylength(NewN,T), N is NewN+1.

myappend([],L,L).
myappend([H|T],L,L3) :- myappend(T,L,NL3), L3 = [H|NL3].

myrev(_,[]).
myrev(R,[H|T]) :- myrev(NR,T), append(NR,[H],R).

palindrome(L) :- myrev(L,L).

my_islist([]).
my_islist([_|T]) :- my_islist(T).

my_flatten([],[]).
my_flatten(X,[X]) :- \+is_list(X) .
my_flatten([H|T],L) :- my_flatten(H,NH),my_flatten(T,NT), append(NH,NT,L).

compress([X],[X]).
compress([X|[X|T]],NR) :- compress([X|T],NR).
compress([X|[Y|T]],[X|NR]) :- compress([Y|T],NR).

helpMePack([],[Acc],Acc):- !.
helpMePack([X|T],R,[]) :- helpMePack(T,R,[X]).
helpMePack([X|T],R,[X|TA]) :- helpMePack(T,R,[X|[X|TA]]).
helpMePack([X|T],R,[Y|TA]) :- helpMePack([X|T],NR,[]), append([[Y|TA]],NR,R).

pack(L,NR) :- helpMePack(L,NR,[]).

dupli([X],[X,X]).
dupli([X|T],[X|[X|R]]) :- dupli(T,R).

helpDrop([],_,[],_).
helpDrop([_|T],1,R,N):- helpDrop(T,N,R,N).
helpDrop([X|T],N1,[X|R],N) :- NR is N1-1,helpDrop(T,NR,R,N).

drop(L,N,R) :- helpDrop(L,N,R,N).


msplit(L,0,[],L).
msplit([X|T],N,[X|L1],L2) :- NewN is N-1, msplit(T,NewN,L1,L2).

slice([],_,_,[]) :- ! .
slice(_,1,0,[]).
slice([X|T],1,E,[X|L]) :- NE is E-1,slice(T,1,NE,L).
slice([_|T],S,E,L2) :- NS is S-1,NE is E-1,slice(T,NS,NE,L2).

range(S,S,[S]).
range(S,E,[S|L]) :- NS is S+1,range(NS,E,L).

combination(1,[H|_],[H]).
combination(N,[H|T],NL) :- (NN is N-1,combination(NN,T,L),NL = [H|L])
; (combination(N,T,NL)).

istree(nil).
istree(t(_,L,R)) :- istree(L),istree(R).

symmetric(nil).
symmetric(t(_,L,R)) :- mirror(L,R).

mirror(nil,nil).
mirror(t(_,L1,R1),t(_,L2,R2)) :- mirror(L1,R2),mirror(L2,R1).

count_leaves(nil,0).
count_leaves(t(_,nil,nil),1) :-!.
count_leaves(t(_,L,R),N) :- count_leaves(L,N1), count_leaves(R,N2), N is N1+N2.

leaves(nil,[]).
leaves(t(X,nil,nil),[X]) :- !.
leaves(t(_,L,R),LX) :- leaves(L,L1), leaves(R,L2), append(L1,L2,LX).

internals(nil,[]).
internals(t(_,nil,nil),[]) :- !.
internals(t(X,L,R),[X|List]) :- internals(L,L1),internals(R,L2), append(L1,L2,List).
