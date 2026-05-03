program benchmark_allocatable_realloc
  implicit none

  integer, parameter :: reps = 120
  integer :: iter, n, i
  real, allocatable :: a(:)
  real :: checksum

  checksum = 0.0

  do iter = 1, reps
    n = 1000 + iter
    ! This intentionally changes shape and can force allocatable reallocation.
    a = [(real(i), i = 1, n)]
    checksum = checksum + sum(a)
  end do

  print *, checksum

end program benchmark_allocatable_realloc
