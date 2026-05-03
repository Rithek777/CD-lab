program benchmark_function_result
  implicit none

  integer, parameter :: n = 100000
  integer, parameter :: reps = 30
  integer :: iter
  real, allocatable :: a(:)
  real :: checksum

  checksum = 0.0

  do iter = 1, reps
    a = make_values(n)
    checksum = checksum + sum(a)
  end do

  print *, checksum

contains

  function make_values(count) result(values)
    integer, intent(in) :: count
    real :: values(count)
    integer :: i

    do i = 1, count
      values(i) = real(i) * 0.5
    end do
  end function make_values

end program benchmark_function_result

