program allocatable_reassignment
  implicit none

  real, allocatable :: a(:)

  allocate(a(4))
  a = 1.0

  ! Later analysis should classify automatic reallocation on assignment.
  a = [10.0, 20.0, 30.0, 40.0, 50.0, 60.0]

  print *, size(a), sum(a)
end program allocatable_reassignment

