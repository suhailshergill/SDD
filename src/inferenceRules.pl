%% define set operations, negation as failure etc.
:- include('utils.pl').


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
replaceWith(X, '') :- isExpression(X).


%% valid to remove
isRemovable(X) :- replaceWith(X, _), sourceRange(X, _, _, _),
	not(hasBeenDeleted(X)), not(essentialSet(X)), not(isInvalid(X)).

%% finer level dependencies
explicitDependsOn(X, Y) :- dependsOn(X, Y), not(containedWithin(X, Y)).
%% not sure if i can trust the statement level dependencies so will have this
%% for a sanity check
implicitDependsOn(X, Y) :- explicitDependsOn(X, Y).
implicitDependsOn(X, Y) :- containedWithin(X, Y).

%% X has to be bound for these to work
isTopLevel(X) :- not(implicitDependsOn(X, Y)), not(isInvalid(X)).
isBottomLevel(X) :- not(implicitDependsOn(Y, X)), not(isInvalid(X)).

transitiveDependsOn(X, Y) :- implicitDependsOn(X, Y).
transitiveDependsOn(X, Y) :- implicitDependsOn(X, Z), transitiveDependsOn(Z, Y).

%% transitiveRemoval(X, Y) :- Removing X would mean having to remove Y
%% separately too.
transitiveRemoval(X, Y) :- dependsOn(Y, X), isRemovable(Y),
	not(containedWithin(Y, X)).
transitiveRemoval(X, Y) :- containedWithin(Z, X), not(containedWithin(Y, X)),
	isRemovable(Y), transitiveRemoval(Z, Y).
transitiveRemoval(X, Y) :- dependsOn(Y, Z), not(containedWithin(Y, Z)),
	not(containedWithin(Y, X)), isRemovable(Y), transitiveRemoval(X, Z).


%% source range metric
sourceRangeSize(X, NumChar) :- sourceRange(X, B, E, _), NumChar is E - B.

sourceRangesSizeWorker([], Total, Total).
sourceRangesSizeWorker([H|T], A, B) :- sourceRangeSize(H, C), X is A + C,
	sourceRangesSizeWorker(T, X, B).
sourceRangesSize(X, NumChar) :- sourceRangesSizeWorker(X, 0, NumChar).

transitiveRemovalSize(X, NumChar) :- safeSetOf(Y, transitiveRemoval(X, Y), S),
	sourceRangeSize(X, A), sourceRangesSizeWorker(S, A, NumChar).


%% dependency metric
allDependingOn(X, L) :- safeSetOf(Y, transitiveDependsOn(Y, X), L).
allDependsOn(X, L) :- safeSetOf(Y, transitiveDependsOn(X, Y), L).
allTopLevelDependingOn(X, L) :- allDependingOn(X, L1), include(isTopLevel, L1,
	L).
allBottomLevelDependsOn(X, L) :- allDependsOn(X, L1), include(isBottomLevel, L1,
	L).


%% predicate to sort sourceranges in decreasing order by size
%% sourceRangeCompare(Order, Term1, Term2) :- transitiveRemovalSize(Term1, A),
%% 	transitiveRemovalSize(Term2, B), compare(Order, B, A).

%% sort terms in ascending order based on number of dependants
sortDependingOnAsc(Order, Term1, Term2) :- allDependingOn(Term1, L1),
	allDependingOn(Term2, L2), length(L1, Len1), length(L2, Len2), 
	( Len1 == Len2 -> compare(Order, Term1, Term2);
	    compare(Order, Len1, Len2)).

sortDependsOnAsc(Order, Term1, Term2) :- allBottomLevelDependsOn

sortDependingOnDescDependsOnAsc(Order, Term1, Term2) :-
	allTopLevelDependingOn(Term1, TL1), length(TL1, TL1len),
	allTopLevelDependingOn(Term2, TL2), length(TL2, TL2len),
	( TL1



allRemovable(L) :- safeSetOf(X, isRemovable(X), S), predsort(sourceRangeCompare,
	S, L).

transitiveRemovalList(X, L) :- safeSetOf(Y, transitiveRemoval(X, Y), S),
	append([X], S, L).

containedWithinList(X, Y) :- safeSetOf(Z, containedWithin(Z, Y), X).
containedWithinListSorted(X, Y) :- safeSetOf(Z, containedWithin(Z, Y), L),
	predsort(sourceRangeCompare, L, X).


allRemovableTopLevels(L) :- allRemovable(L1), include(isTopLevel, L1, L).
allRemovableBottomLevels(L) :- allRemovable(L1), include(isBottomLevel, L1, L).


%% compute transitive dependencies
%% - use setof to get unique results
%%   http://www.csupomona.edu/~jrfisher/www/prolog_tutorial/2_12.html
%% - use predsort to sort them based on total source range they encapsulate.
%%   (using number of characters to be altered as a surrogate for 'size' of change.

