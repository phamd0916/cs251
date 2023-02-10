;;This function uses recursion to find the sum of a list
(define sum
  (lambda (lst)
    (if (null? lst)
      0
      (+ (car lst) (sum (cdr lst))))))


;;This function uses recursion to find the number of elements in a list
(define list-len
  (lambda (lst)
    (if (null? lst)
    0
    (+ 1 (list-len (cdr lst))))))

;;This function takes in two parameters, a list and a selected integer x and uses recursion to keep the first x number of elements in a list and i
(define keep-first-n
  (lambda (x lst)
    (cond ((null? lst) '())
      ((equal? x 0) '())
      ((> 0 x) '())
      ((> x (list-len lst)) '())
      ((cons (car lst) (keep-first-n (- x 1) (cdr lst)))))))