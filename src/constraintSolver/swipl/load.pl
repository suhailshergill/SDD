:- dynamic hasBeenDeleted/1, hasBeenPermanentlyDeleted/1,
	isEssentialForFailure/1, hasUntrackedDependency/1.

:- multifile isInitializer/1, isFunction/1, isCompoundStatement/1,
	isDeclaration/1, isStatement/1, isCondition/1, isExpr/1, isMain/1,
	isInvalid/1, sourceRange/4, dependsOn/2.

:- ensure_loaded('out.txt').
:- ensure_loaded('inferenceRules.pl').
%% :- ['../tests/out.txt', 'inferenceRules.pl'].
