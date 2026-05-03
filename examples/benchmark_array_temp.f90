program benchmark_array_temp
  implicit none

  integer, parameter :: n = 200000
  integer, parameter :: reps = 40
  integer :: iter, i
  real, allocatable :: a(:), b(:), c(:)
  real :: checksum

  allocate(a(n), b(n), c(n))
  b = 1.25
  c = 2.75

  do iter = 1, reps
    A = B + C
  end do

  checksum = sum(a)
  print *, checksum

  deallocate(a, b, c)
end program benchmark_array_temp
