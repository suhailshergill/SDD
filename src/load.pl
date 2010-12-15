:- dynamic hasBeenDeleted/1, hasBeenPermanentlyDeleted/1,
	isEssentialForFailure/1, hasUntrackedDependency/1.

:- multifile isInitializer/1, isFunction/1, isCompoundStatement/1,
	isDeclaration/1, isStatement/1, isCondition/1, isExpr/1, isMain/1,
	isInvalid/1.

%% swipl complains if it runs into inference rules for which it has no examples
%% so going to give it some fake examples. Hmm it stopped complaining
%% isInitializer(thisisaninvalidsymbol).
%% isFunction(thisisaninvalidsymbol).
%% isCompoundStatement(thisisaninvalidsymbol).
%% isDeclaration(thisisaninvalidsymbol).
%% isStatement(thisisaninvalidsymbol).
%% isCondition(thisisaninvalidsymbol).
%% isExpression(thisisaninvalidsymbol).
%% isMain(thisisaninvalidsymbol).
%% isInvalid(thisisaninvalidsymbol).

:- ['out.txt', 'inferenceRules.pl'].
