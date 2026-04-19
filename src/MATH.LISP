(DEFINE ABS
    (LAMBDA (N)
        (IF (< N 0)
            (- 0 N)
            N)))
(DEFINE FRAC (LAMBDA (N) (- N (INT N))))
(DEFINE TRUNCATE INT)
(DEFINE FLOOR
    (LAMBDA (N)
        (INT
            (IF (< N 0)
                (- N 1)
                N))))
(DEFINE CEILING (LAMBDA (N) (- 0 (FLOOR (- 0 N)))))
(DEFINE ROUND (LAMBDA (N) (FLOOR (+ N 0.5))))
(DEFINE MOD (LAMBDA (N M) (- N (* M (INT (/ N M))))))
(DEFINE GCD
    (LAMBDA (N M)
        (IF (EQ? M 0)
            N
            (GCD M (MOD N M)))))
(DEFINE LCM (LAMBDA (N M) (/ (* N M) (GCD N M))))
(DEFINE EVEN? (LAMBDA (N) (EQ? (MOD N 2) 0)))
(DEFINE ODD? (LAMBDA (N) (EQ? (MOD N 2) 1)))
