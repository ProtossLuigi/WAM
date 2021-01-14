sub10 :-
    list10(L),
    subset(L,R),
    write(R),
    nl,
    fail.

sub11 :-
    list11(L),
    subset(L,R),
    write(R),
    nl,
    fail.

sub12 :-
    list12(L),
    subset(L,R),
    write(R),
    nl,
    fail.

sub13 :-
    list13(L),
    subset(L,R),
    write(R),
    nl,
    fail.

sub14 :-
    list14(L),
    subset(L,R),
    write(R),
    nl,
    fail.

sub15 :-
    list15(L),
    subset(L,R),
    write(R),
    nl,
    fail.

subset([X|L1],L2) :- subset(L1,L2).
subset([X|L1],[X|L2]) :- subset(L1,L2).
subset([],[]).

list10([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]).
list11([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]).
list12([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]).
list13([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]).
list14([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]).
list15([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]).

