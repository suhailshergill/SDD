%% define set operations, negation as failure etc.
:- include('utils.pl').

%% filter invalid source ranges
isInvalid(X) :- sourceRange(X, B, E, _), B >= E.

%% nested sourceRanges
containedWithin(X, Y) :- 
	sourceRange(X, B1, E1, F),
	sourceRange(Y, B2, E2, F),
	B1 >= B2,
	E2 >= E1.

%% valid replacements
replaceWith(X, ';') :- isInitializer(X).
replaceWith(X, ';') :- isFunction(X), not(isMain(X)).
replaceWith(X, ';') :- isCompoundStatement(X).
replaceWith(X, '') :- isDeclaration(X).
replaceWith(X, '') :- isStatement(X).
replaceWith(X, '0') :- isCondition(X).
%% replaceWith(X, '1') :- isCondition(X).
replaceWith(X, '') :- isExpr(X).


isElement(X) :- sourceRange(X, _, _, _), not(isInvalid(X)).
allElements(L) :- safeSetOf(X, isElement(X), L).
clearLabel(X) :- retractall(hasBeenDeleted(X)),
	retractall(hasBeenPermanentlyDeleted(X)),
	retractall(hasUntrackedDependency(X)),
	retractall(isEssentialForFailure(X)).
clearAllLabels(L) :- allElements(L), maplist(clearLabel, L).
%% valid to remove
isRemovable(X) :- replaceWith(X, _), sourceRange(X, _, _, _),
	not(hasBeenPermanentlyDeleted(X)), not(isEssentialForFailure(X)),
	not(isInvalid(X)), not(isMain(X)).

%% finer level dependencies
explicitDependsOn(X, Y) :- dependsOn(X, Y), not(containedWithin(X, Y)).
%% not sure if i can trust the statement level dependencies so will have this
%% for a sanity check
implicitDependsOn(X, Y) :- explicitDependsOn(X, Y).
implicitDependsOn(X, Y) :- containedWithin(X, Y).

implicitLiveDependsOn(X, Y) :- implicitDependsOn(X, Y), not(hasBeenPermanentlyDeleted(X)).

immediateDependsOn(X, Y) :- explicitDependsOn(X, Y).
immediateDependsOn(X, Y) :- explicitDependsOn(X, Z), containedWithin(Z, Y),
	not(containedWithin(X, Y)).

%% this needs tabling. swipl doesn't have that.
%% transitiveImplicitDependsOn(X, Y) :- implicitDependsOn(X, Y).
%% transitiveImplicitDependsOn(X, Y) :- implicitDependsOn(X, Z),
%% 	transitiveImplicitDependsOn(Z, Y).
%% allDependingOn(X, L) :- safeSetOf(Y, transitiveImplicitDependsOn(Y, X), S),
%% 	exclude(hasBeenPermanentlyDeleted, S, L).
%% allDependsOn(X, L) :- safeSetOf(Y, transitiveImplicitDependsOn(X, Y), S),
%% 	exclude(hasBeenPermanentlyDeleted, S, L).
%% transitiveRemovalList(X, L) :- allDependingOn(X, L1), append([X], L1, L).

%% these will work without tabling (essential performing graph reachability)
expandCurrentFrontierWorker([], NF, NF).
expandCurrentFrontierWorker([H|T], PF, NF) :- safeSetOf(Y, implicitDependsOn(Y,
	H), S), merge_set(S, PF, PF1), expandCurrentFrontierWorker(T, PF1, NF).
expandCurrentFrontier(CF, NF) :- expandCurrentFrontierWorker(CF, [], NF).
allDependingOnWorker([], L, L).
allDependingOnWorker([H|T], PL, L) :- expandCurrentFrontier([H|T], CF1),
	ord_subtract(CF1, PL, NF), merge_set(NF, PL, PL1),
	allDependingOnWorker(NF, PL1, L).
allDependingOn(X, L) :- allDependingOnWorker([X], [], L1), ord_subtract(L1, [X],
	L2), include(isRemovable, L2, L).
transitiveRemovalList(X, L) :- allDependingOnWorker([X], [X], L1),
	include(isRemovable, L1, L).

expandCurrentFrontierBackwardWorker([], NF, NF).
expandCurrentFrontierBackwardWorker([H|T], PF, NF) :- safeSetOf(Y,
	implicitDependsOn(H, Y), S), merge_set(S, PF, PF1),
	expandCurrentFrontierBackwardWorker(T, PF1, NF).
expandCurrentFrontierBackward(CF, NF) :- expandCurrentFrontierBackwardWorker(CF,
	[], NF).
allDependsOnWorker([], L, L).
allDependsOnWorker([H|T], PL, L) :- expandCurrentFrontierBackward([H|T], CF1),
	ord_subtract(CF1, PL, NF), merge_set(NF, PL, PL1),
	allDependsOnWorker(NF, PL1, L).
allDependsOn(X, L) :- allDependsOnWorker([X], [], L1), ord_subtract(L1, [X],
	L2), include(isRemovable, L2, L).


%% transitiveImplicitLiveDependsOn(X, Y) :- implicitLiveDependsOn(X, Y).
%% transitiveImplicitLiveDependsOn(X, Y) :- implicitLiveDependsOn(X, Z),
%% 	transitiveImplicitLiveDependsOn(Z, Y).



%% X has to be bound for these to work
%% isTopLevel(X) :- not(implicitDependsOn(X, Y)), not(isInvalid(X)).
%% isBottomLevel(X) :- not(implicitDependsOn(Y, X)), not(isInvalid(X)).
isTopLevel(X) :- allDependsOn(X, L), length(L, 0).
filterOutContainedWithin(_, [], L, L).
filterOutContainedWithin(X, [H|T], L, R) :- (
	containedWithin(H, X) -> filterOutContainedWithin(X, T, L, R);
	filterOutContainedWithin(X, T, [H|L], R)).
isBottomLevel(X) :- allDependingOn(X, L1), filterOutContainedWithin(X, L1, [], L),
	length(L, 0).

assertDeletion(X) :- assert(hasBeenDeleted(X)).
assertUndoDeletion(X) :- retractall(hasBeenDeleted(X)).
assertPermanentDeletion(X) :- assert(hasBeenPermanentlyDeleted(X)), retractall(hasBeenDeleted(X)).
assertIsEssentialForFailure(X) :- assert(isEssentialForFailure(X)).
assertHasUntrackedDependency(X) :- assert(hasUntrackedDependency(X)).

recursivelyDelete(X) :- transitiveRemovalList(X, L), maplist(assertDeletion, L).
delete(X) :- ( isRemovable(X) -> recursivelyDelete(X); true ).
recursivelyUndoDelete(X) :- transitiveRemovalList(X, L), maplist(assertUndoDeletion,
	L).
undoDelete(X) :- ( isRemovable(X) -> recursivelyUndoDelete(X);
	false ).
undoAllDelete(L) :- safeSetOf(X, hasBeenDeleted(X), L), maplist(undoDelete, L).
recursivelyPermanentlyDelete(X) :- transitiveRemovalList(X, L),
	maplist(assertPermanentDeletion, L).
permanentlyDelete(X) :- ( isRemovable(X) -> recursivelyPermanentlyDelete(X);
	true ).
recursivelyMarkEssential(X) :- assertIsEssentialForFailure(X), allDependsOn(X,
	L1), exclude(isEssentialForFailure, L1, L),
	maplist(assertIsEssentialForFailure, L).


deleteList(L) :- maplist(assertDeletion, L).
undoDeleteList(L) :- maplist(assertUndoDeletion, L).
permanentlyDeleteList(L) :- maplist(assertPermanentDeletion, L).

%% source range metric. is flawed, not sure if that reall matters though.
sourceRangeSize(X, NumChar) :- sourceRange(X, B, E, _), NumChar is E - B.

sourceRangesSizeWorker([], Total, Total).
sourceRangesSizeWorker([H|T], A, B) :- sourceRangeSize(H, C), X is A + C,
	sourceRangesSizeWorker(T, X, B).
sourceRangesSize(X, NumChar) :- sourceRangesSizeWorker(X, 0, NumChar).

transitiveRemovalSize(X, NumChar) :- transitiveRemovalList(X, S), sourceRangeSize(X,
	A), sourceRangesSizeWorker(S, A, NumChar).


%% dependency metric
allTopLevelDependsOn(X, L) :- allDependsOn(X, L1), include(isTopLevel, L1,
	L).
allBottomLevelDependingOn(X, L) :- allDependingOn(X, L1), include(isBottomLevel, L1,
	L).

%% predicate to sort sourceranges in decreasing order by size
%% sourceRangeCompare(Order, Term1, Term2) :- transitiveRemovalSize(Term1, A),
%% 	transitiveRemovalSize(Term2, B), compare(Order, B, A).

%% sort terms in ascending order based on number of dependants
%% sortDependingOnAsc(Order, Term1, Term2) :- allDependingOn(Term1, L1),
%% 	allDependingOn(Term2, L2), length(L1, Len1), length(L2, Len2), 
%% 	( Len1 == Len2 -> compare(Order, Term1, Term2);
%% 	    compare(Order, Len1, Len2)).

sortDependingOnAsc(Order, Term1, Term2) :- allBottomLevelDependingOn(Term1,
	BL1), length(BL1, BL1len), allBottomLevelDependingOn(Term2, BL2),
	length(BL2, BL2len), 
	( BL1len == BL2len -> compare(Order, Term1, Term2);
	    compare(Order, BL1len, BL2len)).

sortDependsOnDescDependingOnAsc(Order, Term1, Term2) :-
	allTopLevelDependsOn(Term1, TL1), length(TL1, TL1len),
	allTopLevelDependsOn(Term2, TL2), length(TL2, TL2len),
	( TL1len == TL2len -> sortDependingOnAsc(Order, Term1, Term2);
	    compare(Order, TL2len, TL1len)).

sortDependsOnDescDependingOnDesc(Order, Term1, Term2) :-
	allTopLevelDependsOn(Term1, TL1), length(TL1, TL1len),
	allTopLevelDependsOn(Term2, TL2), length(TL2, TL2len),
	( TL1len == TL2len -> sortDependingOnAsc(Order, Term2, Term1);
	    compare(Order, TL2len, TL1len)).

sortAllDependingOnAsc(Order, Term1, Term2) :- allDependingOn(Term1,
	BL1), length(BL1, BL1len), allDependingOn(Term2, BL2),
	length(BL2, BL2len), 
	( BL1len == BL2len -> compare(Order, Term1, Term2);
	    compare(Order, BL1len, BL2len)).

sortAllDependsOnAsc(Order, Term1, Term2) :- allDependsOn(Term1,
	BL1), length(BL1, BL1len), allDependsOn(Term2, BL2),
	length(BL2, BL2len), 
	( BL1len == BL2len -> compare(Order, Term1, Term2);
	    compare(Order, BL1len, BL2len)).


sortAllDependsOnDescAllDependingOnDesc(Order, Term1, Term2) :-
	allDependsOn(Term1, TL1), length(TL1, TL1len),
	allDependsOn(Term2, TL2), length(TL2, TL2len),
	( TL1len == TL2len -> sortAllDependingOnAsc(Order, Term2, Term1);
	    compare(Order, TL2len, TL1len)).

sortAllDependingOnDescAllDependsOnDesc(Order, Term1, Term2) :-
	allDependingOn(Term1, TL1), length(TL1, TL1len),
	allDependingOn(Term2, TL2), length(TL2, TL2len),
	( TL1len == TL2len -> sortAllDependsOnAsc(Order, Term2, Term1);
	    compare(Order, TL2len, TL1len)).

sortAllHarmonic(Order, Term1, Term2) :- 
	allDependingOn(Term1, TLDO1), length(TLDO1, TLDO1len), 
	allDependingOn(Term2, TLDO2), length(TLDO2, TLDO2len),
	allDependsOn(Term1, TLD1), length(TLD1, TLD1len),
	allDependsOn(Term2, TLD2), length(TLD2, TLD2len),
	H1 is (TLDO1len * TLD1len)/(TLDO1len + TLD1len),
	H2 is (TLDO2len * TLD2len)/(TLDO2len + TLD2len),
	( H1 == H2 -> compare(Order, Term1, Term2);
	    compare(Order, H1, H2)).


sortSourceRangeSize(Order, Term1, Term2) :- sourceRangeSize(Term1, T1Size),
	sourceRangeSize(Term2, T2Size), 
	( T1Size == T2Size -> compare(Order, Term1, Term2);
	    compare(Order, T2Size, T1Size)).

%% containedWithinList(X, Y) :- safeSetOf(Z, containedWithin(Z, Y), X).
%% containedWithinListSorted(X, Y) :- safeSetOf(Z, containedWithin(Z, Y), L),
%% 	predsort(sourceRangeCompare, L, X).

allRemovable(L) :- safeSetOf(X, isRemovable(X), L).
allRemovableWUD(L) :- setof(X, isRemovable(X), L1),
	exclude(hasUntrackedDependency, L1, L).
allRemovableDeleted(L) :- safeSetOf(X, isRemovable(X), L1),
	include(hasBeenDeleted, L1, L).
allRemovableDeletedWUD(L) :- setof(X, isRemovable(X), L1),
	include(hasBeenDeleted, L1, L2), exclude(hasUntrackedDependency, L2,
	L).
allRemovableTopLevels(L) :- allRemovable(L1), include(isTopLevel, L1, L).
allRemovableBottomLevels(L) :- allRemovable(L1), include(isBottomLevel, L1, L).

allUntrackedDependencies(L) :- safeSetOf(X, hasUntrackedDependency(X), S),
	include(isRemovable, S, L1), predsort(sortSourceRangeSize, L1, L).
allNotPermanentlyDeleted(L) :- allRemovable(L1), safeSetOf(X,
	isEssentialForFailure(X), S), merge_set(L1, S, L2),
	predsort(sortSourceRangeSize, L2, L).

topScoringRemovable(X) :- allRemovable(L), !,
	findMin(sortAllDependsOnDescAllDependingOnDesc, L, X), !.
topScoringRemovableWUD(X) :- allRemovableWUD(L), !,
	findMin(sortAllDependingOnDescAllDependsOnDesc, L, X), !.
topScoringRemovableWUDH(X) :- allRemovableWUD(L), !,
	findMin(sortAllHarmonic, L, X), !.
topScoringRemovableWUDR(X) :- allRemovableWUD(L), !, choose(L, X), !.
topScoringRemovableWUD2(X) :- allRemovableWUD(L), !,
	findMin(sortAllDependsOnDescAllDependingOnDesc, L, X), !.
topScoringRemovableDeleted(X) :- allRemovableDeleted(L), !,
	findMin(sortAllDependsOnDescAllDependingOnDesc, L, X), !.
topScoringRemovableDeletedWUD(X) :- allRemovableDeletedWUD(L), !,
	findMin(sortAllDependingOnDescAllDependsOnDesc, L, X), !.
topScoringRemovableDeletedWUD2(X) :- allRemovableDeletedWUD(L), !,
	findMin(sortAllDependsOnDescAllDependingOnDesc, L, X), !.


markAllUntrackedDependencies(L) :- allRemovable(L),
	maplist(assertHasUntrackedDependency, L).

%% compute stuff which prolog driver has to do. Only works for 1 file
computeDeletionAction(X, (B, E, R)) :- sourceRange(X, B, E, _), replaceWith(X,
	R).
computeDeletionActionForList(L, L1) :- maplist(computeDeletionAction, L, L1), !.
recursivelyComputeDeletionAction(X, L1, L2) :- transitiveRemovalList(X, L1),!,
	maplist(computeDeletionAction, L1, L2), !.
transitiveRemovalListSorted(X, L) :- transitiveRemovalList(X, L1), !,
	predsort(sortAllDependsOnDescAllDependingOnDesc, L1, L).
transitiveRemovalListWUD(X, L) :- transitiveRemovalList(X, L1), !,
	exclude(hasUntrackedDependency, L1, L), !.
transitiveRemovalListWUDSorted(X, L) :- transitiveRemovalListWUD(X, L1),
	predsort(sortAllDependsOnDescAllDependingOnDesc, L1, L).
