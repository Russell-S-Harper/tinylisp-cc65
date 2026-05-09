(DEFINE MOD (LAMBDA (N M) (- N (* M (INT (/ N M))))))
(DEFINE LPF (LAMBDA (N)
    (IF (< N 0)
        -1
        (IF (EQ? 0 (MOD N 2))
            2
            (LET*
                (F 3)
                (_ (WHILE (AND (< 0 (MOD N F)) (< (* F F) N)) (INCR F F)))
                (IF (EQ? 0 (MOD N F))
                    F
                    N))))))
(DEFINE FACTOR (LAMBDA (N)
    (IF (NOT (EQ? (INT N) N))
        ERR
        (IF (OR (EQ? 0 N) (EQ? 1 N))
            (CONS N ())
            (FACTOR-X N ())))))
(DEFINE FACTOR-X (LAMBDA (N F)
    (LET*
        (D (IF (NOT F) 1 (CAR F)))
        (D (/ N D))
        (IF (EQ? D 1)
            F
            (FACTOR-X D (CONS (LPF D) F))))))
