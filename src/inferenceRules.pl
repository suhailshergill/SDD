%% define negation as failure
not(P) :- (call(P) -> fail ; true).

%% nested sourceRanges
containedWithin(X, Y) :- 
	sourceRange(X, B1, E1, F),
	sourceRange(Y, B2, E2, F),
	X \= Y,
	B1 >= B2,
	E2 > E1.
containedWithin(X, Y) :- 
	sourceRange(X, B1, E1, F),
	sourceRange(Y, B2, E2, F),
	X \= Y,
	B1 > B2,
	E2 >= E1.


%% valid replacements
replaceWith(X, ';') :- isInitializer(X).
replaceWith(X, ';') :- isFunction(X), not(isMain(X)).
replaceWith(X, ';') :- isCompoundStatement(X).
replaceWith(X, '') :- isDeclaration(X).
replaceWith(X, '') :- isStatement(X).
replaceWith(X, '0') :- isCondition(X).
replaceWith(X, '1') :- isCondition(X).

%% valid to remove
isRemovable(X) :- replaceWith(X, _).
