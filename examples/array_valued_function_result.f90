program array_valued_function_result
  implicit none

  integer, parameter :: n = 64
  real :: values(n)

  values = make_values(n)
  print *, sum(values)

contains

  function make_values(count) result(result_values)
    integer, intent(in) :: count
    real :: result_values(count)
    integer :: i

    do i = 1, count
      result_values(i) = real(i)
    end do
  end function make_values

end program array_valued_function_result

