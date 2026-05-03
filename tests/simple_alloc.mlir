module {
  func.func @simple_alloc() {
    %tmp = fir.allocmem !fir.array<1024x1024xf64> {fir.temp, fap.reason = "array temporary"} loc("examples/simple_array_expr.f90":13:3)
    fir.freemem %tmp : !fir.heap<!fir.array<1024x1024xf64>>
    return
  }
}

