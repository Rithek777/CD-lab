program simple_array_expr
  implicit none

  integer, parameter :: n = 8
  integer :: i
  real :: a(n), b(n), c(n)
  real :: scalar

  b = 2.0
  c = 3.0
  scalar = 4.0

  ! transformed by transform_fortran.py: explicit loop avoids an array temporary
  do i = 1, size(A)
    A(i) = B(i) + C(i)
  end do
  ! transformed by transform_fortran.py: explicit loop avoids an array temporary
  do i = 1, size(A)
    A(i) = B(i) * C(i)
  end do
  ! transformed by transform_fortran.py: explicit loop avoids an array temporary
  do i = 1, size(A)
    A(i) = B(i) + scalar
  end do
  ! transformed by transform_fortran.py: explicit loop avoids an array temporary
  do i = 1, size(A)
    A(i) = B(i) + C(i)
  end do

  ! This should be skipped because it is not a simple binary array expression.
  A = sin(B) + C

  print *, sum(a)
end program simple_array_expr

