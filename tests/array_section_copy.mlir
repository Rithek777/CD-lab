module {
  func.func @array_section_copy(%matrix: !fir.ref<!fir.array<16x16xf32>>) {
    %c1 = arith.constant 1 : index
    %c2 = arith.constant 2 : index
    %slice = fir.slice %c1, %c2, %c1 : (index, index, index) -> !fir.slice<1>
    %copy = fir.allocmem !fir.array<8xf32> {fir.copy_in, fir.copy_out, fap.reason = "copy-in/copy-out section"} loc("examples/array_section_copy_in_out.f90":11:21)
    fir.call @scale_vector(%copy) : (!fir.heap<!fir.array<8xf32>>) -> ()
    fir.freemem %copy : !fir.heap<!fir.array<8xf32>>
    return
  }

  func.func private @scale_vector(!fir.heap<!fir.array<8xf32>>)
}

