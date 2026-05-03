program array_expression_temporary
  implicit none

  integer, parameter :: n = 100
  real :: a(n), b(n), c(n), d(n)

  a = 1.0
  b = 2.0
  c = 3.0

  ! Later analysis should look for temporaries from array expression lowering.
  d = (a + b) * c

  print *, sum(d)
end program array_expression_temporary

