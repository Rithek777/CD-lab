program simple_array_expr
  implicit none

  integer, parameter :: n = 8
  integer :: i
  real :: a(n), b(n), c(n)
  real :: scalar

  b = 2.0
  c = 3.0
  scalar = 4.0

  A = B + C
  A = B * C
  A = B + scalar
  A(:) = B(:) + C(:)

  ! This should be skipped because it is not a simple binary array expression.
  A = sin(B) + C

  print *, sum(a)
end program simple_array_expr

