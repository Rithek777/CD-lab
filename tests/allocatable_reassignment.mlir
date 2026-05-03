module {
  func.func @allocatable_reassignment(%a: !fir.ref<!fir.box<!fir.heap<!fir.array<?xf32>>>>) {
    %new_shape = arith.constant 6 : index
    %assign = hlfir.assign %new_shape to %a realloc : index, !fir.ref<!fir.box<!fir.heap<!fir.array<?xf32>>>> loc("examples/allocatable_reassignment.f90":11:3)
    %storage = fir.allocmem !fir.array<6xf32> {fir.allocatable, fap.reason = "allocatable reallocation"} loc("examples/allocatable_reassignment.f90":11:3)
    fir.freemem %storage : !fir.heap<!fir.array<6xf32>>
    return
  }
}

