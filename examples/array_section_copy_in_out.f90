program array_section_copy_in_out
  implicit none

  integer, parameter :: n = 16
  real :: matrix(n, n)

  matrix = 1.0

  ! Non-contiguous sections can require copy-in/copy-out temporaries.
  call scale_vector(matrix(1:n:2, 3), 2.0)

  print *, sum(matrix)

contains

  subroutine scale_vector(x, factor)
    real, intent(inout) :: x(:)
    real, intent(in) :: factor

    x = x * factor
  end subroutine scale_vector

end program array_section_copy_in_out

