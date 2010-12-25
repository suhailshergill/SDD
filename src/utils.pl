%% define negation as failure
%% not(P) :- (call(P) -> fail ; true).

%% define set operations

%% always succeeds
safeSetOf(Template, Goal, Set) :- (
	setof(Template, Goal, X) -> Set = X;
	Set = []
    ).

%% fails on an empty list
findMaxW(_, [], CurrentFav, CurrentFav).
findMaxW(Comparator, [H|T], CurrentFav, Result) :- (
	call(Comparator, >, H, CurrentFav) -> findMaxW(Comparator, T, H, Result);
	findMaxW(Comparator, T, CurrentFav, Result)).
findMax(Comparator, [H|T], X) :- findMaxW(Comparator, T, H, X).

findMinW(_, [], CurrentFav, CurrentFav).
findMinW(Comparator, [H|T], CurrentFav, Result) :- (
	call(Comparator, <, H, CurrentFav) -> findMinW(Comparator, T, H, Result);
	findMinW(Comparator, T, CurrentFav, Result)).
findMin(Comparator, [H|T], X) :- findMinW(Comparator, T, H, X).

%% pick random element from list
choose([], []).
choose(List, Elt) :-
        length(List, Length),
        R is random(Length),
        nth0(R, List, Elt).
