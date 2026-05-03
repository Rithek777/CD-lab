module {
  func.func @array_expression_temporary(%a: !fir.ref<!fir.array<1024x1024xf64>>,
                                        %b: !fir.ref<!fir.array<1024x1024xf64>>,
                                        %c: !fir.ref<!fir.array<1024x1024xf64>>) {
    %expr = hlfir.elemental %a unordered : (!fir.ref<!fir.array<1024x1024xf64>>) -> !hlfir.expr<1024x1024xf64>
    %tmp = fir.allocmem !fir.array<1024x1024xf64> {fir.temp, fap.reason = "array temporary"} loc("examples/array_expression_temporary.f90":12:7)
    hlfir.assign %expr to %tmp : !hlfir.expr<1024x1024xf64>, !fir.heap<!fir.array<1024x1024xf64>>
    fir.freemem %tmp : !fir.heap<!fir.array<1024x1024xf64>>
    return
  }
}

