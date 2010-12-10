%% define negation as failure
%% not(P) :- (call(P) -> fail ; true).

%% define set operations

%% always succeeds
safeSetOf(Template, Goal, Set) :- (
	setof(Template, Goal, X) -> Set = X;
	Set = []
    ).
