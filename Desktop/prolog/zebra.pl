/* Solves the Zebra Puzzle:
**   http://en.wikipedia.org/wiki/Zebra_Puzzle
**
** The data structure representation for houses is:
**   house(Nationality, Color, Animal, Drink, Smoke)
*/

zebra(Houses) :-
  Houses = [house(norwegian, _, _, _, _),
	    house(_, blue, _, _, _),
	    house(_, _, _, milk, _), house(_,_,_,water,_), house(_,zebra,_,_,_)],
  member(house(english, red, _, _, _), Houses),
  member(house(spanish, _, dog, _, _), Houses),
  member(house(_, green, _, coffee, _), Houses),
  member(house(ukranian, _, _, tea, _), Houses),
  rightof(house(_, ivory, _, _, _), house(_, green, _, _, _), Houses),
  member(house(_, _, snails, _, old_gold), Houses),
  member(house(_, yellow, _, _, kools), Houses),
  nextto(house(_, _, _, _, chesterfields), house(_, _, fox, _, _), Houses),
  nextto(house(_, _, _, _, kools), house(_, _, horse, _, _), Houses),
  member(house(_, _, _, orange_juice, lucky_strikes), Houses),
  member(house(japanese, _, _, _, parliaments), Houses).

rightof(X, Y, [X,Y,_,_,_]).
rightof(X, Y, [_,X,Y,_,_]).
rightof(X, Y, [_,_,X,Y,_]).
rightof(X, Y, [_,_,_,X,Y]).

nextto(X, Y, Houses) :- rightof(X, Y, Houses).
nextto(X, Y, Houses) :- rightof(Y, X, Houses).
