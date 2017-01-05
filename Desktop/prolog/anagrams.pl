isEmpty([]).


possible_nodes([[H1|T1],Buf,[H2|T2]],List,Goal) :- List1=[],length([H1|T1],N1),length([H2|T2],N2),length(Goal,Ng),
   ((N2=:=Ng,
    append(List1,[[H2|[H1|T1]],Buf,T2,"21-"],List2),
    append(List2,[[H1|T1],[H2],T2,"20-"],List3), List = List3,!)
;
    (N1=:=Ng,
    append(List1,[T1,Buf,[H1|[H2|T2]],"12-"],List2),
    append(List2,[T1,[H1],[H2|T2],"10-"],List3),    List = List3,!)
;
    (N1 < Ng, N2 < Ng, N1 > 0, N2 > 0, isEmpty(Buf),
    append(List1,[T1,Buf,[H1|[H2|T2]],"12-"],List2),
    append(List2,[[H2|[H1|T1]],Buf,T2,"21-"],List3),
    append(List3,[T1,[H1],[H2|T2],"10-"],List4),
    append(List4,[[H1|T1],[H2],T2,"20-"],List5),     List = List5,!)
;
    (N1 < Ng, N2 < Ng, N1 > 0, N2 > 0, \+isEmpty(Buf),
    append(List1,[T1,Buf,[H1|[H2|T2]],"12-"],List2),
    append(List2,[[Buf|[H1|T1]],[],[H2|T2],"01-"],List3),
    append(List3,[[H1|T1],[],[Buf|[H2|T2]],"02-"],List4),
    append(List4,[[H2|[H1|T1]],Buf,T2,"21-"],List5),     List = List5,!)
;
    (N1 =:= 0, N2 < Ng, \+isEmpty(Buf),
    append(List1,[[H2|[H1|T1]],Buf,T2,"21-"],List2),
    append(List2,[[Buf|[H1|T1]],[],[H2|T2],"01-"],List3),
    append(List3,[[H1|T1],[],[Buf|[H2|T2]],"02-"],List4),    List = List4,!)
;
    (N2 =:= 0, N1 < Ng, \+isEmpty(Buf),
    append(List1,[T1,Buf,[H1|[H2|T2]],"12-"],List2),
    append(List2,[[Buf|[H1|T1]],[],[H2|T2],"01-"],List3),
    append(List3,[[H1|T1],[],[Buf|[H2|T2]],"02-"],List4),    List = List4,!)
).




bfs([],_,_,_) :- false.
bfs([[[],[],Two,_]|_],_,[],Two).
bfs([H|T],C,Res,Goal) :- (\+member(H,C)),H=[_,_,_,Move],NewC = [H|C],possible_nodes(H,L,Goal),append(T,L,NOpen),bfs(NOpen,NewC,Res2,Goal),append(Move,Res2,Res).


anagrams(Fst,Snd,Goal) :- bfs([[Fst,[],[],[]]],[],Goal,Snd).
	
