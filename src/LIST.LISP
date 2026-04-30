(DEFINE COUNT (LAMBDA (L)
    (IF (NOT L)
        0
        (+ 1 (COUNT (CDR L))))))
(DEFINE APPEND (LAMBDA (A B)
    (IF (NOT A)
        B
        (CONS (CAR A) (APPEND (CDR A) B)))))
(DEFINE REVERSE-X (LAMBDA (L A)
    (IF (NOT L)
        A
        (REVERSE-X (CDR L) (CONS (CAR L) A)))))
(DEFINE REVERSE (LAMBDA (L)
    (REVERSE-X L '())))
(DEFINE MEMBER (LAMBDA (X L)
    (IF (NOT L)
        L
        (IF (EQ? X (CAR L))
            L
            (MEMBER X (CDR L))))))
(DEFINE REMOVE-ALL (LAMBDA (X L)
    (IF (NOT L)
        L
        (IF (EQ? X (CAR L))
            (REMOVE-ALL X (CDR L))
            (CONS (CAR L) (REMOVE-ALL X (CDR L)))))))
(DEFINE REMOVE-DUPS (LAMBDA (L)
    (IF (NOT L)
        L
        (CONS (CAR L) (REMOVE-DUPS (REMOVE-ALL (CAR L) (CDR L)))))))
(DEFINE NTH (LAMBDA (N L)
    (IF (EQ? N 0)
        (CAR L)
        (NTH (- N 1) (CDR L)))))
(DEFINE FLATTEN (LAMBDA (L)
    (IF (NOT L)
        L
        (IF (ATOM? (CAR L))
            (CONS (CAR L) (FLATTEN (CDR L)))
            (APPEND (FLATTEN (CAR L)) (FLATTEN (CDR L)))))))
