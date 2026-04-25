(DEFINE MOD (LAMBDA (N M) (- N (* M (INT (/ N M))))))
(DEFINE ABS
    (LAMBDA (N)
        (IF (< N 0)
            (- 0 N)
            N)))
(DEFINE LPF (LAMBDA (N)
    (IF (< N 0)
        -1
        (IF (< N 4)
            N
            (IF (EQ? 0 (MOD N 2))
                2
                (LPF-X N 3))))))
(DEFINE LPF-X (LAMBDA (N I)
    (IF (EQ? 0 (MOD N I))
        I
        (IF (< (* I I) N)
            (LPF-X N (+ I 2))
            (IF (EQ? (* I I) N)
                I
                N)))))
(DEFINE FACTOR (LAMBDA (N)
    (IF (NOT (EQ? (INT N) N))
        ERR
        (IF (EQ? 1 (ABS N))
            (CONS N ())
            (FACTOR-X N (CONS 1 ()))))))
(DEFINE FACTOR-X (LAMBDA (N F)
    (IF (EQ? N (CAR F))
        F
        (FACTOR-X (/ N (CAR F)) (CONS (LPF (/ N (CAR F))) F)))))
